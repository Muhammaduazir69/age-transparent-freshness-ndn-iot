"""MaacoAgents: the BridgeServer-facing front-end combining the DQN placement
agent, the A2C replacement agent, and the reward coordinator.

Transition assembly (lock-step semantics): every decision arrives via the
bridge with full state; for each agentId the previous (state, action) is
closed into a transition when the agent's NEXT decision arrives — reward is
the coordinator's window reward between the two decision times. Remaining
open transitions are flushed with done=True at episode end.
"""

from collections import deque

import numpy as np

from . import shm_layout as L
from .coordinator import RewardCoordinator
from .dqn_placement import DQNPlacementAgent


class HeuristicA2C:
    """Placeholder replacement scorer until/unless the real A2C is enabled:
    evicts the item with the least remaining freshness (LFF-like)."""

    def act(self, items: np.ndarray, num_items: int, node_state=None):
        return int(np.argmin(items[:num_items, 0])), 0.0

    def push_transition(self, *a, **k):
        pass

    def on_episode_end(self):
        return {}


class PendingDecision:
    """One open DQN placement decision awaiting its credit horizon.

    `s1` is filled by the agent's next decision (the TD bootstrap state);
    `t1` bounds the shared network-utility window, exactly as before. The
    name-attributed local window instead runs to `t0 + horizon`, so an
    admission is scored by what its own copy actually served over a content
    lifetime rather than by 0.75 s of network-wide noise.
    """
    __slots__ = ("t0", "s0", "a0", "node", "h", "t1", "s1", "lt1", "hz", "done")

    def __init__(self, t0, s0, a0, node, h, hz=0.0):
        self.t0, self.s0, self.a0, self.node, self.h = t0, s0, a0, node, h
        self.hz = hz      # this decision's credit horizon (s)
        self.done = False
        self.t1 = None    # next decision of this AGENT (TD bootstrap / shared window)
        self.s1 = None
        self.lt1 = None   # next decision of this agent on the SAME CONTENT:
                          # the local window must stop there or two decisions
                          # would both harvest the same copy's hits, and a
                          # "skip" right after an "admit" would be credited
                          # with the admit's hits at zero write cost


class MaacoAgents:
    def __init__(self, dqn: DQNPlacementAgent | None = None,
                 a2c=None, coordinator: RewardCoordinator | None = None,
                 log_cb=None, credit_horizon=0.0, horizon_mode="fixed",
                 bootstrap="post-horizon"):
        self.dqn = dqn
        self.a2c = a2c or HeuristicA2C()
        self.coord = coordinator or RewardCoordinator()
        self.log_cb = log_cb
        self.credit_horizon = float(credit_horizon)
        # "fixed": one horizon for every decision (the SmartFarm campaigns).
        # "lifetime": each decision is credited over the freshness period of
        # the packet it concerns, read back from state feature 6, so a 5 s
        # item is not credited with 60 s of unrelated activity and a 600 s
        # item is not cut off after a tenth of its life.
        self.horizon_mode = horizon_mode
        # Successor state for the placement TD target.
        #   "post-horizon": the first decision the same agent takes at or
        #     after this decision's credit window closes. The reward covers
        #     (t0, window end] and the bootstrap covers everything after it,
        #     so the two do not overlap and a discount gamma > 0 is well
        #     posed: the transition is a semi-Markov step of one horizon.
        #   "next": the agent's immediately following decision, which is the
        #     pre-2026 behaviour. Its window overlaps the reward window, so
        #     the bootstrap double counts the same hits; it is kept only so
        #     that the gamma = 0 campaigns replay bit for bit.
        self.bootstrap = bootstrap
        self.heaps: dict[int, list] = {}
        self._qowner: dict[int, int] = {}
        self._seq = 0
        # agentId -> deque of PendingDecision (FIFO by decision time)
        self.open_dqn: dict[int, deque] = {}
        # (agentId, nameHash) -> newest still-open decision on that content
        self.open_by_name: dict[tuple, PendingDecision] = {}
        self.open_a2c: dict[int, tuple] = {}
        self.dqn_decisions = 0
        self.a2c_decisions = 0
        self.rewards_seen: list[float] = []
        # per-action reward diagnostics (is admitting actually rewarded?)
        self.r_by_action: list[list[float]] = [[], []]

    # ---- BridgeServer protocol ----
    def observe_rewards(self, events):
        self.coord.ingest(events)

    _nan_warned = False

    def act(self, dec):
        state = np.ctypeslib.as_array(dec.state).copy()
        # NaN/Inf guard: a single bad feature (e.g. 0/0 ratios from C++) would
        # poison the network weights after one backward pass -> collapsed policy
        if not np.isfinite(state).all():
            if not MaacoAgents._nan_warned:
                bad = np.where(~np.isfinite(state))[0].tolist()
                print(f"[agents] WARNING: non-finite state features at indices {bad} "
                      f"(agent {dec.agentId}); clamping. Fix the C++ feature source.")
                MaacoAgents._nan_warned = True
            state = np.nan_to_num(state, nan=0.0, posinf=1.0, neginf=0.0)
        t = float(dec.simTime)
        agent_key = int(dec.agentId)

        if dec.agentType == L.AGENT_DQN:
            self.dqn_decisions += 1
            if self.dqn is not None:
                q = self.open_dqn.get(agent_key)
                if q is None:
                    q = self.open_dqn[agent_key] = deque()
                    self._qowner[id(q)] = agent_key
                # the immediately preceding decision of this agent gets its
                # bootstrap state and its shared-window end
                if q and q[-1].t1 is None:
                    q[-1].t1 = t
                    q[-1].s1 = state
                h = int(dec.nameHash)
                prev_same = self.open_by_name.get((int(dec.nodeId), h))
                if prev_same is not None and prev_same.lt1 is None:
                    prev_same.lt1 = t
                    if self.horizon_mode == "lifetime":
                        self._schedule(agent_key, t, prev_same)
                self._release_matured(q, t, state)
                # Action mask (coordination redundancy rule / energy guard):
                # the simulation now QUERIES the policy in constrained states
                # instead of vetoing before it, so the state is observed and
                # learned from; the mask restricts which action may be
                # returned. bit 0 = skip legal, bit 1 = admit legal.
                kw = {}
                # Bandit-style baselines index their arms by content identity
                # as well as by state; the DQN does not and its signature is
                # untouched (`wants_key` defaults to False).
                if getattr(self.dqn, "wants_key", False):
                    kw["key"] = h
                if getattr(self.dqn, "wants_mask", False):
                    kw["mask"] = int(getattr(dec, "actionMask", 3)) or 3
                action = self.dqn.act(state, **kw)
                p = PendingDecision(t, state, action, int(dec.nodeId), h,
                                    self._horizon(state))
                q.append(p)
                if self.horizon_mode == "lifetime":
                    self._schedule(agent_key, p.t0 + p.hz, p)
                self.open_by_name[(int(dec.nodeId), h)] = p
            else:
                action = 1  # LCE when DQN disabled (A2C-only runs)
            return action, 0.0

        # A2C replacement decision
        self.a2c_decisions += 1
        n = int(dec.numItems)
        items = np.ctypeslib.as_array(dec.items).reshape(L.MAX_ITEMS, L.ITEM_FEAT)
        items = items[:max(n, 1)].copy()
        if not np.isfinite(items).all():
            items = np.nan_to_num(items, nan=0.0, posinf=1.0, neginf=0.0)
        prev = self.open_a2c.get(agent_key)
        if prev is not None and hasattr(self.a2c, "close_transition"):
            t0 = prev[0]
            r = self.coord.window_reward(dec.nodeId, t0, t)
            self.rewards_seen.append(r)
            self.a2c.close_transition(agent_key, r, terminal=False,
                                      next_items=items, next_ctx=state, next_n=n)
        try:
            action, aux = self.a2c.act(items, n, node_state=state, agent_key=agent_key)
        except TypeError:  # HeuristicA2C has no agent_key param
            action, aux = self.a2c.act(items, n, node_state=state)
        self.open_a2c[agent_key] = (t,)
        return action, aux

    def _push(self, s, a, r, s2, done, key):
        """Close one placement transition on whichever placement learner is
        installed. Bandit baselines need the content key to find the arm they
        pulled; the DQN's signature is unchanged."""
        if getattr(self.dqn, "wants_key", False):
            self.dqn.push_transition(s, a, r, s2, done, key=key)
        else:
            self.dqn.push_transition(s, a, r, s2, done)

    def _horizon(self, state):
        if self.horizon_mode != "lifetime":
            return self.credit_horizon
        fp = float(state[6]) * 3600.0      # RlNDNProcessor: fp / 3600 s
        return fp if fp > 0 else self.credit_horizon

    def _schedule(self, agent_key, when, p):
        import heapq
        self._seq += 1
        heapq.heappush(self.heaps.setdefault(agent_key, []), (when, self._seq, p))

    def _local_end(self, p):
        """End of this decision's exclusive attribution window."""
        end = p.t0 + p.hz
        return end if p.lt1 is None else min(end, p.lt1)

    def _credit(self, p, t1):
        return self.coord.window_reward(p.node, p.t0, t1, p.h, self._local_end(p))

    def _successor(self, p, s_now):
        """TD bootstrap state: the decision that starts the unrewarded future."""
        if self.bootstrap == "next" or s_now is None:
            return p.s1
        return s_now

    def _release_matured(self, q, now, s_now=None):
        """Push every pending decision whose attribution window has closed."""
        if self.horizon_mode == "lifetime":
            return self._release_heap(q, now, s_now)
        while q and q[0].t1 is not None and now >= self._local_end(q[0]):
            p = q.popleft()
            r = self._credit(p, p.t1)
            self.rewards_seen.append(r)
            self.r_by_action[p.a0].append(r)
            self._push(p.s0, p.a0, r, self._successor(p, s_now), False, p.h)
            if self.open_by_name.get((p.node, p.h)) is p:
                self.open_by_name.pop((p.node, p.h), None)

    def _release_heap(self, q, now, s_now=None):
        """Lifetime mode: horizons differ per decision, so release in order of
        window end rather than of decision time (a 600 s decision must not
        hold back the 5 s ones queued behind it)."""
        import heapq
        if not q:
            return
        heap = self.heaps.get(self._agent_of(q), [])
        while heap and heap[0][0] <= now:
            _, _, p = heapq.heappop(heap)
            if p.done or p.t1 is None or now < self._local_end(p):
                if not p.done and p.t1 is None:
                    # the agent has not decided again yet; retry shortly
                    self._schedule(self._agent_of(q), now + 1.0, p)
                    break
                continue
            p.done = True
            r = self._credit(p, p.t1)
            self.rewards_seen.append(r)
            self.r_by_action[p.a0].append(r)
            self._push(p.s0, p.a0, r, self._successor(p, s_now), False, p.h)
            if self.open_by_name.get((p.node, p.h)) is p:
                self.open_by_name.pop((p.node, p.h), None)
        while q and q[0].done:
            q.popleft()

    def _agent_of(self, q):
        return self._qowner[id(q)]

    def abort_episode(self):
        """Discard an episode that died before its handshake completed.

        The simulator was killed mid-run, so every open placement decision has
        no successor state and no completed credit window.  on_episode_end()
        must NOT be used here: it flushes those decisions into the replay
        buffer as terminal transitions, and because it is never called on the
        crash path the leftovers used to survive into the next episode, where
        a different simulation with a restarted clock filled in their
        bootstrap state and reward.  That is what produced the doubled
        decision counts and the PARITY-MISMATCH line.  Drop them instead.
        """
        self.open_dqn.clear()
        self.heaps.clear()
        self.open_a2c.clear()
        self.open_by_name.clear()
        # The actor's half-built update for a dead episode is dropped rather
        # than applied: flush_episode() would fold it into a gradient step.
        pend = getattr(self.a2c, "pending_losses", None)
        if hasattr(pend, "clear"):
            pend.clear()
        self.dqn_decisions = 0
        self.a2c_decisions = 0
        self.rewards_seen.clear()
        self.r_by_action[0].clear()
        self.r_by_action[1].clear()

    def on_episode_end(self, total_decisions):
        # flush open DQN transitions as terminal (reward over trailing window)
        if self.dqn is not None:
            t_end = 0.0
            for q in self.open_dqn.values():
                for p in q:
                    t_end = max(t_end, p.t1 if p.t1 is not None else p.t0)
            t_end += max([60.0, self.credit_horizon] +
                         [p.hz for q in self.open_dqn.values() for p in q])
            for q in self.open_dqn.values():
                for p in q:
                    if p.done:
                        continue
                    t1 = p.t1 if p.t1 is not None else t_end
                    r = self._credit(p, t1)
                    self.rewards_seen.append(r)
                    s1 = p.s1 if p.s1 is not None else p.s0
                    self._push(p.s0, p.a0, r, s1, True, p.h)
        if hasattr(self.a2c, "flush_episode"):
            self.a2c.flush_episode(self.coord)
        self.open_dqn.clear()
        self.heaps.clear()
        self.open_a2c.clear()
        self.open_by_name.clear()

        stats = self.coord.episode_summary()
        stats["dqnDecisions"] = self.dqn_decisions
        stats["a2cDecisions"] = self.a2c_decisions
        stats["meanTransitionReward"] = float(np.mean(self.rewards_seen)) if self.rewards_seen else 0.0
        for a, lab in ((0, "Skip"), (1, "Admit")):
            v = self.r_by_action[a]
            stats[f"meanR{lab}"] = float(np.mean(v)) if v else float("nan")
            stats[f"n{lab}"] = len(v)
        if self.dqn is not None:
            stats["epsilon"] = self.dqn.epsilon
            stats["dqnLoss"] = self.dqn.last_loss
            stats["dqnTrainSteps"] = self.dqn.train_steps
        if hasattr(self.a2c, "last_entropy"):
            stats["entropy"] = self.a2c.last_entropy
            stats["a2cLoss"] = self.a2c.last_loss
            stats["entropyCoef"] = self.a2c.entropy_coef
        if self.log_cb:
            self.log_cb(stats)
        # reset per-episode counters (network stats accumulate in coord until pruned)
        self.dqn_decisions = 0
        self.a2c_decisions = 0
        self.rewards_seen.clear()
        self.r_by_action[0].clear()
        self.r_by_action[1].clear()
        return stats
