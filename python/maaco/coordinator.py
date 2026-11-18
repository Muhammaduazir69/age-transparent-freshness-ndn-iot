"""Shared-reward coordinator (thesis RO3, 3.5.1).

Consumes the C++ reward-event stream and turns it into per-transition rewards.
Two modes:
  * "shared":      every agent's reward window sums NETWORK-WIDE events —
                   the thesis's shared network utility (MAACO-Coord).
  * "independent": an agent only sees events from its own node (MAACO-Ind
                   ablation; A2C/DQN agents at node n use nodeId == n events).

Reward shaping (weights are constructor args so experiments iterate without
touching C++):
  + w_hit    per local cache hit          - w_miss  per cache miss
  + w_fresh  per fresh delivery           - w_stale per stale delivery
  - w_lat    * retrieval latency (s)      - w_hop   per data hop
  - w_evict_fresh * remaining-freshness fraction of evicted victims
    (penalizes evicting still-fresh content -> A2C freshness pressure)
  - w_energy * Joules drained per node energy report (EV_ENERGY_DELTA;
    default 0: headline configs express energy pressure through the hard
    guard and the residual-energy state feature instead)
"""

import bisect
from collections import defaultdict

from . import shm_layout as L


class RewardCoordinator:
    def __init__(self, mode="shared", w_hit=1.0, w_miss=0.2, w_fresh=0.2,
                 w_stale=1.0, w_lat=2.0, w_hop=0.02, w_evict_fresh=0.5,
                 w_energy=0.0, w_insert=0.0, w_local=0.0, w_shared=1.0,
                 clip=10.0):
        assert mode in ("shared", "independent")
        self.mode = mode
        self.w = dict(hit=w_hit, miss=w_miss, fresh=w_fresh, stale=w_stale,
                      lat=w_lat, hop=w_hop, evict_fresh=w_evict_fresh,
                      energy=w_energy, insert=w_insert)
        self.w_local = w_local
        self.w_shared = w_shared
        self.clip = clip
        # event log: sorted by simTime; per-node in independent mode
        self.times = []
        self.values = []          # per-event shaped reward contribution
        self.node_times = defaultdict(list)
        self.node_values = defaultdict(list)
        # per-decision credit channel (thesis 3.5.1 credit assignment fix):
        # content-name-attributed events, keyed by (nodeId, nameHash), so an
        # admission can be rewarded by the hits ITS OWN copy subsequently
        # served instead of by a network-wide window sum in which one
        # decision contributes about 1/13000 of the signal.
        self.local_times = defaultdict(list)
        self.local_values = defaultdict(list)
        # episode aggregates (diagnostics)
        self.counts = defaultdict(int)

    def _shape(self, kind, value):
        w = self.w
        if kind == L.EV_CS_HIT:
            return w["hit"]
        if kind == L.EV_CS_MISS:
            return -w["miss"]
        if kind == L.EV_FRESH_DELIVERY:
            return w["fresh"]
        if kind == L.EV_STALE_DELIVERY:
            return -w["stale"]
        if kind == L.EV_DATA_DELIVERED:
            return -w["lat"] * min(value, 2.0)
        if kind == L.EV_HOP_COUNT:
            return -w["hop"] * value
        if kind == L.EV_CS_EVICT:
            return -w["evict_fresh"] * value   # value = victim's remaining freshness
        if kind == L.EV_ENERGY_DELTA:
            return -w["energy"] * value        # value = Joules drained since last report
        if kind == L.EV_CS_INSERT:
            return -w["insert"]                # admission cost (cache-write energy)
        return 0.0

    # Events that carry usable per-(node, name) credit for a placement
    # decision: the hits its own copy served, the misses it left unserved,
    # the freshness it destroyed on eviction, and the write cost it paid.
    #
    # EV_CS_MISS is what makes the channel well-posed. Without it a "skip"
    # earns exactly 0 while an "admit" earns (hits - w_insert), which can be
    # negative — so the greedy policy has a degenerate all-skip attractor and
    # collapses into it (measured: greedy admission rate 0.03 to 0.11 and hit
    # ratio down to 0.06 in the first pilot). Charging a skip for the misses
    # its content subsequently causes at that node restores the real economics
    # of placement: pay a write now to convert future misses into hits.
    # EV_CS_EVICT is deliberately NOT here. It is the A2C's freshness-pressure
    # term and belongs to the replacement agent; charged through the name key
    # it also lands on whichever placement decision admitted the victim, so an
    # admission paid both its write cost AND -w_evict_fresh * remaining
    # freshness. With about 65% of entries evicted before serving anything,
    # that made the typical admission look worse than a skip and drove the
    # greedy admission rate down to 0.18-0.23 (measured) against an optimum
    # near 0.40. The eviction cost stays in the shared channel.
    _LOCAL_KINDS = (L.EV_CS_HIT, L.EV_CS_MISS, L.EV_CS_INSERT)

    def ingest(self, events):
        """events: iterable of (nodeId, kind, simTime, value, nameHash)."""
        for node, kind, t, value, h in events:
            self.counts[kind] += 1
            r = self._shape(kind, value)
            if r == 0.0:
                continue
            self.times.append(t)
            self.values.append(r)
            self.node_times[node].append(t)
            self.node_values[node].append(r)
            if self.w_local and kind in self._LOCAL_KINDS:
                key = (node, h)
                self.local_times[key].append(t)
                self.local_values[key].append(r)

    def shared_window(self, node, t0, t1):
        """Unclipped network-utility term over (t0, t1]."""
        if self.mode == "shared":
            times, values = self.times, self.values
        else:
            times, values = self.node_times[node], self.node_values[node]
        lo = bisect.bisect_right(times, t0)
        hi = bisect.bisect_right(times, t1)
        return sum(values[lo:hi])

    def local_window(self, node, name_hash, t0, t1):
        """Unclipped name-attributed term for content `name_hash` at `node`."""
        if not self.w_local or name_hash is None:
            return 0.0
        key = (node, name_hash)
        lt = self.local_times.get(key)
        if not lt:
            return 0.0
        lv = self.local_values[key]
        # [t0, t1]: the admission cost is pushed at exactly the decision's
        # sim time, so the lower bound must be inclusive or every decision
        # would be credited with its hits but never charged for its write.
        lo = bisect.bisect_left(lt, t0)
        hi = bisect.bisect_right(lt, t1)
        return self.w_local * sum(lv[lo:hi])

    def window_reward(self, node, t0, t1, name_hash=None, local_t1=None):
        """Shaped reward for a transition.

        The network term spans (t0, t1] exactly as before. When w_local > 0
        the name-attributed term spans (t0, local_t1 or t1] — normally a full
        content-lifetime horizon, which is what makes a single admission's
        consequences visible at all.
        """
        r = self.w_shared * self.shared_window(node, t0, t1)
        r += self.local_window(node, name_hash, t0,
                               t1 if local_t1 is None else local_t1)
        return max(-self.clip, min(self.clip, r))

    def episode_summary(self):
        hits = self.counts[L.EV_CS_HIT]
        misses = self.counts[L.EV_CS_MISS]
        fresh = self.counts[L.EV_FRESH_DELIVERY]
        stale = self.counts[L.EV_STALE_DELIVERY]
        return {
            "hitRatio": hits / max(1, hits + misses),
            "freshnessSat": fresh / max(1, fresh + stale),
            "staleCount": stale,
            "hits": hits,
            "misses": misses,
            "totalReward": sum(self.values),
        }

    def reset(self):
        self.times.clear()
        self.values.clear()
        self.node_times.clear()
        self.node_values.clear()
        self.local_times.clear()
        self.local_values.clear()
        self.counts.clear()
