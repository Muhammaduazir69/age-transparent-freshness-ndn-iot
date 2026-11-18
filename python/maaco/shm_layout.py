"""ctypes mirror of src/rl/RlShmLayout.h — MUST match field-for-field.

The C++ side sends its compile-time constants in the HELLO message; `verify()`
asserts they match these values, so any layout drift fails fast instead of
silently mis-reading memory.
"""

import ctypes
import mmap

SHM_MAGIC = 0x4D414143  # "MAAC"
import os

# MAACO_LAYOUT=3 attaches to the v3 binary (8 item features) so that the
# checkpoints of the first and corrected campaigns stay reproducible.
_LAYOUT = int(os.environ.get("MAACO_LAYOUT", "5"))
SHM_VERSION = _LAYOUT

STATE_DIM = 17 if _LAYOUT >= 5 else 16
ITEM_FEAT = 9 if _LAYOUT >= 4 else 8
MAX_ITEMS = 1024
REWARD_CAP = 65536

AGENT_DQN = 0
AGENT_A2C = 1

# RlEventKind
EV_CS_HIT = 0
EV_CS_MISS = 1
EV_DATA_DELIVERED = 2
EV_STALE_DELIVERY = 3
EV_FRESH_DELIVERY = 4
EV_CS_INSERT = 5
EV_CS_EVICT = 6
EV_ENERGY_DELTA = 7
EV_HOP_COUNT = 8
EV_INTEREST_SENT = 9

# DQN placement-state feature indices (mirror RlNDNProcessor::buildPlacementState)
DQN_FEATURES = [
    "freqShort", "freqLong", "recency", "trend",
    "hopCount", "contentSize", "freshnessPeriod",
    "csOccupancy", "csHitRatio", "residualEnergy",
    "neighborCached", "avgCacheFreshness",
    # candidate-validity block (added 2026-08-09 after the feature audit)
    "candRemainingFrac", "candRemainingSec", "candBeatsResidentFrac",
    "neighborCachedDensity",
    "csCapacityLog",          # layout v5
][:STATE_DIM]

# A2C per-item feature indices (mirror RlCS::buildItemFeatures)
A2C_ITEM_FEATURES = [
    "remainingFreshness", "accessRate", "timeSinceAccess", "size",
    "refetchCostHops", "lifetimeHits", "ageInCache", "appRemainingFrac",
    "agedFrequency",          # layout v4
][:ITEM_FEAT]


class RlShmHeader(ctypes.Structure):
    _pack_ = 8
    _fields_ = [
        ("magic", ctypes.c_uint32),
        ("version", ctypes.c_uint32),
        ("stateDim", ctypes.c_uint32),
        ("itemFeat", ctypes.c_uint32),
        ("maxItems", ctypes.c_uint32),
        ("rewardCap", ctypes.c_uint32),
        ("numAgents", ctypes.c_uint32),
        ("runNumber", ctypes.c_uint32),
    ]


class RlDecisionSlot(ctypes.Structure):
    _pack_ = 8
    _fields_ = [
        ("seq", ctypes.c_uint64),
        ("agentId", ctypes.c_uint32),
        ("agentType", ctypes.c_uint32),
        ("simTime", ctypes.c_double),
        ("numItems", ctypes.c_uint32),
        ("nodeId", ctypes.c_uint32),
        ("nameHash", ctypes.c_uint32),
        ("actionMask", ctypes.c_uint32),   # bit0 = skip legal, bit1 = admit legal
        ("pad0", ctypes.c_uint32),
        ("state", ctypes.c_float * STATE_DIM),
        ("items", (ctypes.c_float * ITEM_FEAT) * MAX_ITEMS),
        ("mask", ctypes.c_uint8 * MAX_ITEMS),
    ]


class RlActionSlot(ctypes.Structure):
    _pack_ = 8
    _fields_ = [
        ("seq", ctypes.c_uint64),
        ("action", ctypes.c_int32),
        ("aux", ctypes.c_float),
    ]


class RlRewardEvent(ctypes.Structure):
    _pack_ = 8
    _fields_ = [
        ("nodeId", ctypes.c_uint32),
        ("kind", ctypes.c_uint32),
        ("simTime", ctypes.c_double),
        ("value", ctypes.c_float),
        ("nameHash", ctypes.c_uint32),
    ]


class RlRewardRing(ctypes.Structure):
    _pack_ = 8
    _fields_ = [
        ("head", ctypes.c_uint64),
        ("tail", ctypes.c_uint64),
        ("dropped", ctypes.c_uint64),
        ("pad", ctypes.c_uint64),
        ("events", RlRewardEvent * REWARD_CAP),
    ]


class RlShmSegment(ctypes.Structure):
    _pack_ = 8
    _fields_ = [
        ("header", RlShmHeader),
        ("decision", RlDecisionSlot),
        ("action", RlActionSlot),
        ("rewards", RlRewardRing),
    ]


def verify_hello(magic, version, state_dim, item_feat, max_items, reward_cap):
    assert magic == SHM_MAGIC, f"magic mismatch: {magic:#x}"
    assert version == SHM_VERSION, f"version mismatch: {version} != {SHM_VERSION}"
    assert state_dim == STATE_DIM, f"stateDim mismatch: {state_dim} != {STATE_DIM}"
    assert item_feat == ITEM_FEAT, f"itemFeat mismatch: {item_feat} != {ITEM_FEAT}"
    assert max_items == MAX_ITEMS, f"maxItems mismatch: {max_items} != {MAX_ITEMS}"
    assert reward_cap == REWARD_CAP, f"rewardCap mismatch: {reward_cap} != {REWARD_CAP}"


class ShmView:
    """Attach to the C++-owned segment at /dev/shm/<name> (never unlink it)."""

    def __init__(self, name: str):
        path = "/dev/shm/" + name.lstrip("/")
        self._f = open(path, "r+b")
        self._mm = mmap.mmap(self._f.fileno(), ctypes.sizeof(RlShmSegment))
        self.seg = RlShmSegment.from_buffer(self._mm)
        hdr = self.seg.header
        verify_hello(hdr.magic, hdr.version, hdr.stateDim, hdr.itemFeat,
                     hdr.maxItems, hdr.rewardCap)

    def drain_rewards(self):
        """Yield and consume all pending reward events (SPSC ring consumer)."""
        ring = self.seg.rewards
        out = []
        while ring.tail < ring.head:
            e = ring.events[ring.tail % REWARD_CAP]
            out.append((e.nodeId, e.kind, e.simTime, e.value, e.nameHash))
            ring.tail += 1
        return out

    def close(self):
        # Release the ctypes view; if stray ctypes sub-views (loop locals)
        # still export pointers, leave the mmap to process teardown rather
        # than crash — the segment itself is owned (and unlinked) by C++.
        self.seg = None
        import gc
        gc.collect()
        try:
            self._mm.close()
        except BufferError:
            pass
        self._f.close()
