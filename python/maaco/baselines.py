"""Learned cache-admission baselines from the prior literature.

These are drop-in replacements for `DQNPlacementAgent` on the placement
(cache-admission) side of the bridge: they expose the same four-method
interface (`act`, `push_transition`, `save`, `load` plus the `greedy`,
`epsilon`, `last_loss`, `train_steps` attributes that `MaacoAgents` and
`train.py` read), so they run on the *identical* substrate, with the
identical reward channel, the identical credit horizon, the identical
frozen-policy discipline and the identical checkpoint-selection rule as
MAACO's own placement agent. Nothing about the simulation changes.

Three families are implemented:

1. `CacheMABAgent` -- multi-armed bandit cache admission (the "Cache-MAB"
   family: Blasco & Gunduz 2014 "Learning-based optimization of cache
   content"; Sengupta et al. 2014 "Learning distributed caching strategies";
   Muller et al. 2017 "Context-aware proactive content caching"). Two arms
   {skip, admit} per *context*, where a context is either a discretized
   state bucket (contextual bandit) or the content identity itself
   (per-content arms, the classic formulation). Exploration by UCB1 or by
   epsilon-greedy. This is the nearest prior art to MAACO's placement agent,
   which at gamma = 0 *is* a contextual bandit -- the only differences are
   function approximation over the full 16-feature state and the exploration
   rule.

2. `TabularQAgent` -- tabular Q-learning admission over a discretized version
   of the same state (popularity, recency, Content Store occupancy and
   remaining freshness bucketed into a small table). Isolates how much of
   MAACO's gain comes from function approximation over the 16-feature state
   as opposed to from the credit-assignment design, since it shares MAACO's
   reward, horizon and (by default) discount exactly.

3. `RecencyHeuristicAgent` -- the one-line non-learned control: admit iff the
   content was requested at this node within the last T seconds. The feature
   attribution reported for MAACO names recency as the dominant driver, so
   this is the trivial policy that discussion implies.

Feature discretization
----------------------
Bin edges are *calibrated*, not guessed: the agent collects the first
`calib_n` states it sees and freezes empirical quantile edges from them.
Calibration therefore happens on training data only and the frozen edges
travel inside the checkpoint, so a frozen-policy evaluation uses exactly the
edges the checkpoint was trained with.

State indices used (mirror `shm_layout.DQN_FEATURES`):
    0  freqShort   (short-window popularity)
    2  recency     (seconds since last request / 600)
    7  csOccupancy (Content Store occupancy fraction)
    12 candRemainingFrac (remaining freshness of the candidate)
"""

import numpy as np
import torch

from . import shm_layout as L

# state feature indices used by the discretizers
F_POP = 0
F_REC = 2
F_OCC = 7
F_FRESH = 12

FEATURE_SETS = {
    # name -> (state indices, per-feature bin counts are given separately)
    "pop_rec": (F_POP, F_REC),
    "pop_rec_occ": (F_POP, F_REC, F_OCC),
    "pop_rec_occ_fresh": (F_POP, F_REC, F_OCC, F_FRESH),
    "pop_occ_fresh": (F_POP, F_OCC, F_FRESH),
}


class _Discretizer:
    """Quantile-calibrated multi-dimensional state bucketiser."""

    def __init__(self, feature_set="pop_rec_occ_fresh", bins=4, calib_n=5000):
        self.idx = FEATURE_SETS[feature_set]
        self.feature_set = feature_set
        self.bins = int(bins)
        self.calib_n = int(calib_n)
        self._buf = []
        self.edges = None          # list of arrays, len(idx)

    @property
    def n_states(self):
        return self.bins ** len(self.idx)

    def calibrated(self):
        return self.edges is not None

    def observe(self, state):
        """Feed a state during calibration; freezes the edges when full."""
        if self.edges is not None:
            return
        self._buf.append(np.asarray(state, dtype=np.float32)[list(self.idx)])
        if len(self._buf) >= self.calib_n:
            self.freeze()

    def freeze(self):
        if self.edges is not None:
            return
        if not self._buf:
            # never saw anything: uniform edges on [0, 1]
            self.edges = [np.linspace(0, 1, self.bins + 1)[1:-1]
                          for _ in self.idx]
            return
        arr = np.stack(self._buf)                      # [n, d]
        qs = np.linspace(0, 1, self.bins + 1)[1:-1]
        edges = []
        for j in range(arr.shape[1]):
            e = np.quantile(arr[:, j], qs)
            # strictly increasing edges, else np.searchsorted collapses bins
            e = np.maximum.accumulate(e + 1e-9 * np.arange(len(e)))
            edges.append(e.astype(np.float64))
        self.edges = edges
        self._buf = []

    def index(self, state):
        """Flat bucket index; -1 while still calibrating."""
        if self.edges is None:
            return -1
        s = np.asarray(state, dtype=np.float64)
        out = 0
        for k, j in enumerate(self.idx):
            b = int(np.searchsorted(self.edges[k], s[j], side="right"))
            out = out * self.bins + b
        return out

    def state_dict(self):
        return {"feature_set": self.feature_set, "bins": self.bins,
                "calib_n": self.calib_n,
                "edges": None if self.edges is None
                         else [e.tolist() for e in self.edges]}

    def load_state_dict(self, d):
        self.feature_set = d["feature_set"]
        self.idx = FEATURE_SETS[self.feature_set]
        self.bins = int(d["bins"])
        self.calib_n = int(d["calib_n"])
        self.edges = (None if d["edges"] is None
                      else [np.asarray(e, dtype=np.float64) for e in d["edges"]])
        self._buf = []


class CacheMABAgent:
    """Multi-armed bandit cache admission (Cache-MAB family).

    arms = "context"  -> one 2-armed bandit per discretized state bucket
                         (contextual bandit; the exact reduction MAACO's
                         placement agent performs at gamma = 0)
    arms = "content"  -> one 2-armed bandit per content name hash (the
                         classic per-content Cache-MAB formulation)
    arms = "both"     -> per (content, context) pair

    explore = "ucb1"  -> index = mu_hat + c * sqrt(2 ln N / n)
    explore = "eps"   -> epsilon-greedy with a linear anneal, matching the
                         DQN's exploration schedule shape

    Rewards arrive through exactly the same name-attributed credit channel
    the DQN placement agent uses (hits its own copy served, minus the misses
    its content caused at that node, minus the write cost, over the 60 s
    content-lifetime horizon), so the "reward on subsequent hits within the
    content lifetime" requirement is met by construction. UCB1 assumes
    bounded rewards, so the raw reward is squashed to [0, 1] by
    r_norm = (clip(r, -R, R) + R) / 2R with R = `reward_scale`.
    """

    wants_key = True
    wants_mask = True

    def __init__(self, arms="context", explore="ucb1", c=1.0,
                 eps_start=1.0, eps_end=0.05, eps_decay_steps=250_000,
                 feature_set="pop_rec_occ_fresh", bins=4, calib_n=5000,
                 reward_scale=2.0, default_action=1, n_content=4096, seed=0):
        assert arms in ("context", "content", "both")
        assert explore in ("ucb1", "eps")
        self.arms = arms
        self.explore = explore
        self.c = float(c)
        self.eps_start, self.eps_end = float(eps_start), float(eps_end)
        self.eps_decay_steps = int(eps_decay_steps)
        self.reward_scale = float(reward_scale)
        self.default_action = int(default_action)
        self.n_content = int(n_content)
        self.disc = _Discretizer(feature_set, bins, calib_n)

        n_ctx = self.disc.n_states
        if arms == "context":
            self.n_cells = n_ctx
        elif arms == "content":
            self.n_cells = self.n_content
        else:
            self.n_cells = self.n_content * n_ctx
        self.counts = np.zeros((self.n_cells, 2), dtype=np.float64)
        self.sums = np.zeros((self.n_cells, 2), dtype=np.float64)

        self.rng = np.random.default_rng(seed)
        self.steps = 0
        self.train_steps = 0
        self.last_loss = float("nan")
        self.greedy = False
        # act() and push_transition() are decoupled in time, so remember the
        # cell each (key, action) was chosen in; the credit arrives later.
        self._cell_of = {}

    # ---- exploration bookkeeping ----
    @property
    def epsilon(self):
        if self.greedy or self.explore != "eps":
            return 0.0
        frac = min(1.0, self.steps / self.eps_decay_steps)
        return self.eps_start + (self.eps_end - self.eps_start) * frac

    def _cell(self, state, key):
        ctx = self.disc.index(state)
        if ctx < 0:
            ctx = 0
        if self.arms == "context":
            return ctx
        content = int(key) % self.n_content if key is not None else 0
        if self.arms == "content":
            return content
        return content * self.disc.n_states + ctx

    def _mu(self, cell):
        n = self.counts[cell]
        s = self.sums[cell]
        return np.divide(s, n, out=np.full(2, 0.5), where=n > 0)

    # ---- agent interface ----
    def act(self, state, key=None, mask=3):
        self.steps += 1
        if not self.greedy:
            self.disc.observe(state)
        cell = self._cell(state, key)
        n = self.counts[cell]
        if not (mask >> 1) & 1:      # admit vetoed: only "skip" is legal
            if key is not None:
                self._cell_of[(int(key), 0)] = cell
            return 0

        if self.greedy:
            if n.sum() == 0:
                a = self.default_action
            else:
                mu = self._mu(cell)
                # an arm never pulled in this cell has no evidence; prefer the
                # arm that does, rather than its 0.5 prior
                mu = np.where(n > 0, mu, -np.inf)
                a = int(np.argmax(mu))
        elif self.explore == "eps":
            if self.rng.random() < self.epsilon:
                a = int(self.rng.integers(0, 2))
            else:
                mu = self._mu(cell)
                a = int(np.argmax(mu + 1e-12 * self.rng.random(2)))
        else:  # UCB1
            if (n == 0).any():
                a = int(np.argmin(n + 1e-12 * self.rng.random(2)))
            else:
                total = n.sum()
                bonus = self.c * np.sqrt(2.0 * np.log(total) / n)
                a = int(np.argmax(self._mu(cell) + bonus))

        if key is not None:
            self._cell_of[(int(key), a)] = cell
        else:
            self._cell_of[(None, a)] = cell
        return a

    def push_transition(self, s, a, r, s2, done, key=None):
        if self.greedy:          # frozen-policy evaluation must be frozen
            return
        a = int(a)
        cell = self._cell_of.pop((int(key) if key is not None else None, a), None)
        if cell is None:
            cell = self._cell(s, key)
        R = self.reward_scale
        rn = (min(R, max(-R, float(r))) + R) / (2.0 * R)
        self.counts[cell, a] += 1.0
        self.sums[cell, a] += rn
        self.train_steps += 1
        self.last_loss = 0.0

    def hparams(self):
        return {"family": "cache-mab", "arms": self.arms,
                "explore": self.explore, "c": self.c,
                "eps_start": self.eps_start, "eps_end": self.eps_end,
                "eps_decay_steps": self.eps_decay_steps,
                "feature_set": self.disc.feature_set, "bins": self.disc.bins,
                "calib_n": self.disc.calib_n,
                "reward_scale": self.reward_scale,
                "default_action": self.default_action,
                "n_content": self.n_content, "n_cells": int(self.n_cells)}

    def save(self, path):
        self.disc.freeze()
        torch.save({"family": "cache-mab", "counts": self.counts,
                    "sums": self.sums, "disc": self.disc.state_dict(),
                    "steps": self.steps, "train_steps": self.train_steps,
                    "hparams": self.hparams()}, path)

    def load(self, path):
        ck = torch.load(path, map_location="cpu", weights_only=False)
        self.counts = np.asarray(ck["counts"], dtype=np.float64)
        self.sums = np.asarray(ck["sums"], dtype=np.float64)
        self.disc.load_state_dict(ck["disc"])
        self.n_cells = self.counts.shape[0]
        self.steps = ck.get("steps", 0)

    # ---- diagnostics ----
    def coverage(self):
        visited = int((self.counts.sum(1) > 0).sum())
        both = int(((self.counts > 0).all(1)).sum())
        return {"cells": int(self.n_cells), "cellsVisited": visited,
                "cellsBothArms": both,
                "pulls": float(self.counts.sum())}


class TabularQAgent:
    """Tabular Q-learning cache admission over a discretized state.

    Same reward channel, same credit horizon and (by default) the same
    discount as MAACO's placement agent, so the only thing that differs is
    the representation: a `bins ** d` lookup table instead of a 256x256 MLP
    over the full 16-feature state.
    """

    wants_key = False
    wants_mask = True

    def __init__(self, alpha=0.1, gamma=0.0, eps_start=1.0, eps_end=0.05,
                 eps_decay_steps=250_000, feature_set="pop_rec_occ_fresh",
                 bins=4, calib_n=5000, optimistic=0.0, default_action=1,
                 seed=0):
        self.alpha = float(alpha)
        self.gamma = float(gamma)
        self.eps_start, self.eps_end = float(eps_start), float(eps_end)
        self.eps_decay_steps = int(eps_decay_steps)
        self.default_action = int(default_action)
        self.disc = _Discretizer(feature_set, bins, calib_n)
        self.q = np.full((self.disc.n_states, 2), float(optimistic),
                         dtype=np.float64)
        self.visits = np.zeros((self.disc.n_states, 2), dtype=np.float64)
        self.rng = np.random.default_rng(seed)
        self.steps = 0
        self.train_steps = 0
        self.last_loss = float("nan")
        self.greedy = False

    @property
    def epsilon(self):
        if self.greedy:
            return 0.0
        frac = min(1.0, self.steps / self.eps_decay_steps)
        return self.eps_start + (self.eps_end - self.eps_start) * frac

    def _s(self, state):
        i = self.disc.index(state)
        return 0 if i < 0 else i

    def act(self, state, mask=3):
        self.steps += 1
        if not self.greedy:
            self.disc.observe(state)
        i = self._s(state)
        if not (mask >> 1) & 1:
            return 0
        if not self.greedy and self.rng.random() < self.epsilon:
            return int(self.rng.integers(0, 2))
        if self.visits[i].sum() == 0:
            return self.default_action
        return int(np.argmax(self.q[i] + 1e-12 * self.rng.random(2)))

    def push_transition(self, s, a, r, s2, done):
        if self.greedy:
            return
        i, j = self._s(s), self._s(s2)
        a = int(a)
        target = float(r)
        if self.gamma > 0.0 and not done:
            target += self.gamma * float(self.q[j].max())
        td = target - self.q[i, a]
        self.q[i, a] += self.alpha * td
        self.visits[i, a] += 1.0
        self.train_steps += 1
        self.last_loss = float(td * td)

    def hparams(self):
        return {"family": "tabular-q", "alpha": self.alpha,
                "gamma": self.gamma, "eps_start": self.eps_start,
                "eps_end": self.eps_end,
                "eps_decay_steps": self.eps_decay_steps,
                "feature_set": self.disc.feature_set, "bins": self.disc.bins,
                "calib_n": self.disc.calib_n,
                "default_action": self.default_action,
                "n_states": int(self.disc.n_states)}

    def save(self, path):
        self.disc.freeze()
        torch.save({"family": "tabular-q", "q": self.q, "visits": self.visits,
                    "disc": self.disc.state_dict(), "steps": self.steps,
                    "train_steps": self.train_steps,
                    "hparams": self.hparams()}, path)

    def load(self, path):
        ck = torch.load(path, map_location="cpu", weights_only=False)
        self.q = np.asarray(ck["q"], dtype=np.float64)
        self.visits = np.asarray(ck["visits"], dtype=np.float64)
        self.disc.load_state_dict(ck["disc"])
        self.steps = ck.get("steps", 0)

    def coverage(self):
        visited = int((self.visits.sum(1) > 0).sum())
        both = int(((self.visits > 0).all(1)).sum())
        return {"states": int(self.disc.n_states), "statesVisited": visited,
                "statesBothActions": both,
                "updates": float(self.visits.sum())}


class RecencyHeuristicAgent:
    """Admit iff the content was requested at this node within the last
    `threshold` seconds. Non-learned; no training, no checkpoint.

    state[2] is `PopularityTracker::recency / 600`, and the tracker clips
    "never requested" to the 600 s long window, so threshold >= 600 is
    exactly LCE and threshold = 0 is admit-only-on-a-simultaneous-request.
    """

    wants_key = False
    wants_mask = True

    def __init__(self, threshold=30.0, scale=600.0):
        self.threshold = float(threshold)
        self.scale = float(scale)
        self.greedy = True
        self.steps = 0
        self.train_steps = 0
        self.last_loss = float("nan")
        self.epsilon = 0.0

    def act(self, state, mask=3):
        self.steps += 1
        if not (mask >> 1) & 1:
            return 0
        return 1 if float(state[F_REC]) * self.scale <= self.threshold else 0

    def push_transition(self, *a, **k):
        pass

    def hparams(self):
        return {"family": "recency-heuristic", "threshold_s": self.threshold}

    def save(self, path):
        torch.save({"family": "recency-heuristic",
                    "threshold": self.threshold,
                    "hparams": self.hparams()}, path)

    def load(self, path):
        ck = torch.load(path, map_location="cpu", weights_only=False)
        self.threshold = float(ck["threshold"])


class PopularityHeuristicAgent:
    """Admit iff the content's short-window request count at this node is at
    least `threshold` requests per 60 s window.

    This is the non-degenerate form of the "recency" control. A state audit
    over 8 792 real placement decisions shows feature 2 (recency) spans
    0.00000 to 0.00027 of its 600 s scale, i.e. 0 to 0.16 s: Data only ever
    arrives at a node moments after that node forwarded the Interest that
    fetched it, so "was this requested recently" is TRUE at every decision
    point and the recency rule degenerates to Leave Copy Everywhere for every
    threshold above about one second. Short-window popularity (feature 0) is
    the feature that actually varies (10 distinct levels over 1..10 requests
    per 60 s), so it is what a one-line demand-driven admission rule has to
    key on.
    """

    wants_key = False
    wants_mask = True

    def __init__(self, threshold=0.3, scale=10.0):
        self.threshold = float(threshold)   # on the normalized feature
        self.scale = float(scale)           # feature = min(1, requests/scale)
        self.greedy = True
        self.steps = 0
        self.train_steps = 0
        self.last_loss = float("nan")
        self.epsilon = 0.0

    def act(self, state, mask=3):
        self.steps += 1
        if not (mask >> 1) & 1:
            return 0
        return 1 if float(state[F_POP]) >= self.threshold else 0

    def push_transition(self, *a, **k):
        pass

    def hparams(self):
        return {"family": "popularity-heuristic",
                "threshold_norm": self.threshold,
                "threshold_requests_per_window": self.threshold * self.scale}

    def save(self, path):
        torch.save({"family": "popularity-heuristic",
                    "threshold": self.threshold,
                    "hparams": self.hparams()}, path)

    def load(self, path):
        ck = torch.load(path, map_location="cpu", weights_only=False)
        self.threshold = float(ck["threshold"])



# ---------------------------------------------------------------------------
class RLCacheAgent:
    """RL-Cache style admission: a feedforward net trained by Monte Carlo
    policy gradient (Kirilin et al., IEEE JSAC 2020).

    Two things make this a faithful reimplementation rather than another DQN.

    1.  **The decision rule is stochastic and the update is REINFORCE.**
        RL-Cache samples an admission decision from the network's output
        probability, observes what the resulting cache did, and raises the
        log-probability of the sampled decision in proportion to how well it
        did relative to a running baseline.  There is no value function, no
        replay buffer and no target network.

    2.  **It reads only the features RL-Cache defines**: object size, request
        frequency over a short and a long window, and recency.  The published
        method is a CDN admission policy, so it has no notion of a battery,
        a freshness period or a radio, and we do not give it one.

    That second point is not a detail.  Under Definition 2 of the manuscript a
    policy whose decision rule never reads a substrate-dependent coordinate is
    substrate-invariant, so this agent is a *learned* policy that the model
    predicts will not move when the energy model is deleted.  It is therefore
    a test of the mechanism, and it can falsify it.

    The reward is the same name-attributed credit channel every other agent
    here uses, so the comparison changes the policy class and nothing else.
    """

    wants_key = False
    wants_mask = True

    # RL-Cache's feature families, mapped onto this simulator's state vector:
    # short-window frequency, long-window frequency, recency, object size.
    FEATURES = (0, 1, 2, 5)

    def __init__(self, hidden=64, lr=1e-3, baseline_decay=0.99,
                 entropy_coef=0.01, batch=32, default_action=1, seed=0,
                 device="cpu"):
        self.lr = float(lr)
        self.baseline_decay = float(baseline_decay)
        self.entropy_coef = float(entropy_coef)
        self.batch = int(batch)
        self.default_action = int(default_action)
        self.hidden = int(hidden)
        self.device = device
        self.greedy = False
        self.steps = 0
        self.train_steps = 0
        self.last_loss = 0.0
        self._baseline = 0.0
        self._rvar = 1.0
        self._n_batches = 0
        self._buf = []
        self.rng = np.random.default_rng(seed)
        torch.manual_seed(seed)
        d = len(self.FEATURES)
        self.net = torch.nn.Sequential(
            torch.nn.Linear(d, self.hidden), torch.nn.ReLU(),
            torch.nn.Linear(self.hidden, self.hidden), torch.nn.ReLU(),
            torch.nn.Linear(self.hidden, 1)).to(device)
        self.opt = torch.optim.Adam(self.net.parameters(), lr=self.lr,
                                    weight_decay=1e-4)

    @property
    def epsilon(self):
        """No epsilon-greedy schedule: RL-Cache explores by sampling its own
        output probability, so the trainer's exploration column is zero."""
        return 0.0

    def _x(self, state):
        v = np.asarray(state, dtype=np.float32)[list(self.FEATURES)]
        return torch.from_numpy(v).unsqueeze(0).to(self.device)

    # Bound on the admission logit.  Without it nothing constrains the logit's
    # magnitude: Adam takes a step of order `lr` per parameter whatever the
    # gradient size, so over a 200-episode run (millions of decisions) the
    # logits drift until |log pi| reaches the hundreds, the printed loss runs
    # to 5e3, and the sampled policy is deterministic in practice.  Clamping to
    # +-8 keeps p in [3.4e-4, 0.99966], which is saturated enough to express a
    # confident rule and still leaves the sampling that RL-Cache explores by.
    LOGIT_CLAMP = 8.0

    def _logits(self, x):
        return self.net(x).squeeze(-1).clamp(-self.LOGIT_CLAMP, self.LOGIT_CLAMP)

    def _p(self, state):
        with torch.no_grad():
            return float(torch.sigmoid(self._logits(self._x(state))).item())

    def act(self, state, mask=3):
        self.steps += 1
        if not (mask >> 1) & 1:
            return 0
        p = self._p(state)
        if self.greedy:
            return int(p >= 0.5)
        return int(self.rng.random() < p)

    def push_transition(self, s, a, r, s2, done):
        """One Monte Carlo sample; the update fires once per batch."""
        if self.greedy:
            return
        self._buf.append((np.asarray(s, dtype=np.float32), int(a), float(r)))
        if len(self._buf) < self.batch:
            return
        xs = torch.from_numpy(
            np.stack([b[0][list(self.FEATURES)] for b in self._buf])).to(self.device)
        acts = torch.tensor([b[1] for b in self._buf], dtype=torch.float32,
                            device=self.device)
        rews = np.array([b[2] for b in self._buf], dtype=np.float64)
        # Moving-average baseline, seeded from the first batch rather than from
        # zero: with a decay of 0.99 a baseline starting at zero needs several
        # hundred batches to reach a reward mean of -0.74, and until it gets
        # there every advantage carries the same sign, which pushes the logits
        # of *both* actions the same way.
        d = self.baseline_decay
        if self._n_batches == 0:
            self._baseline = float(rews.mean())
            self._rvar = float(((rews - self._baseline) ** 2).mean())
        else:
            self._baseline = d * self._baseline + (1.0 - d) * float(rews.mean())
            self._rvar = (d * self._rvar + (1.0 - d)
                          * float(((rews - self._baseline) ** 2).mean()))
        self._n_batches += 1
        # Scale the advantage by its own running standard deviation.  The name
        # credit channel pays a popular admitted item many times inside the
        # 60 s horizon and charges everything else a fixed insert cost, so raw
        # advantages span roughly an order of magnitude.  Multiplied by a log
        # probability and accumulated by Adam, that drives the logits off to
        # saturation: measured offline, 99 % of states reach p > 0.99 within
        # 8k decisions and the policy degenerates into "admit everything",
        # which is LCE rather than a learned admission rule.  Normalizing
        # keeps the step size independent of the reward scale, which is what
        # the published method's normalized-reward formulation does.
        scale = max(float(np.sqrt(self._rvar)), 1e-3)
        adv = torch.tensor(np.clip((rews - self._baseline) / scale, -10.0, 10.0),
                           dtype=torch.float32, device=self.device)
        logits = self._logits(xs)
        logp = -torch.nn.functional.binary_cross_entropy_with_logits(
            logits, acts, reduction="none")
        p = torch.sigmoid(logits)
        ent = -(p * torch.log(p + 1e-8) + (1 - p) * torch.log(1 - p + 1e-8))
        loss = -(logp * adv).mean() - self.entropy_coef * ent.mean()
        self.opt.zero_grad()
        loss.backward()
        torch.nn.utils.clip_grad_norm_(self.net.parameters(), 5.0)
        self.opt.step()
        self.train_steps += 1
        self.last_loss = float(loss.item())
        self._buf.clear()

    def hparams(self):
        return {"family": "rl-cache", "hidden": self.hidden, "lr": self.lr,
                "baseline_decay": self.baseline_decay,
                "entropy_coef": self.entropy_coef, "batch": self.batch,
                "features": list(self.FEATURES),
                "default_action": self.default_action}

    def save(self, path):
        torch.save({"family": "rl-cache", "net": self.net.state_dict(),
                    "baseline": self._baseline, "rvar": self._rvar,
                    "n_batches": self._n_batches, "steps": self.steps,
                    "train_steps": self.train_steps,
                    "hparams": self.hparams()}, path)

    def load(self, path):
        ck = torch.load(path, map_location="cpu", weights_only=False)
        self.net.load_state_dict(ck["net"])
        self._baseline = float(ck.get("baseline", 0.0))
        self._rvar = float(ck.get("rvar", 1.0))
        self._n_batches = int(ck.get("n_batches", 0))
        self.steps = ck.get("steps", 0)

    def coverage(self):
        return {"updates": float(self.train_steps * self.batch),
                "batches": int(self.train_steps),
                "baseline": float(self._baseline),
                "reward_std": float(np.sqrt(max(self._rvar, 0.0)))}


def make_placement_agent(kind, seed=0, **kw):
    """Factory used by train.py / eval.py --placement."""
    if kind == "mab":
        return CacheMABAgent(seed=seed, **kw)
    if kind == "tabq":
        return TabularQAgent(seed=seed, **kw)
    if kind == "rlcache":
        return RLCacheAgent(seed=seed, **kw)
    if kind == "recency":
        return RecencyHeuristicAgent(**kw)
    if kind == "popularity":
        return PopularityHeuristicAgent(**kw)
    raise ValueError(f"unknown placement baseline {kind!r}")
