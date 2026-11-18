//
// NDN-IoT-RL - shared-memory layout between the OMNeT++ simulation (C++) and
// the Python MAACO trainer. Mirrored field-for-field in python/maaco/shm_layout.py
// (offsets asserted at HELLO). Bump SHM_VERSION on ANY change.
//
// Synchronization model: the ZeroMQ REQ/REP round-trip is the only barrier.
// The simulation writes DecisionSlot, sends DECISION_READY, blocks on the
// reply; Python writes ActionSlot before replying. No atomics needed.
// The reward-event ring is single-producer (C++) / single-consumer (Python);
// Python drains it opportunistically at each decision and at EPISODE_END.
//

#ifndef __NDNIOT_RLSHMLAYOUT_H
#define __NDNIOT_RLSHMLAYOUT_H

#include <cstdint>

namespace ndniot {

constexpr uint32_t SHM_MAGIC = 0x4D414143;  // "MAAC"
// Layout is selectable at compile time so the checkpoints of an earlier
// campaign stay runnable.  v3 is the layout the substrate campaign trained
// under (16 state coordinates, 8 item features); v4 added the aged-frequency
// item feature and v5 the store-capacity state feature.  Build the historical
// layout with  -DRL_LAYOUT=3 .  The default is unchanged.
#ifndef RL_LAYOUT
#define RL_LAYOUT 5
#endif

constexpr uint32_t SHM_VERSION = RL_LAYOUT;

constexpr uint32_t RL_STATE_DIM = (RL_LAYOUT >= 5) ? 17 : 16;  // DQN state / A2C node context
constexpr uint32_t RL_ITEM_FEAT = (RL_LAYOUT >= 4) ? 9 : 8;    // per-cached-item features (A2C)
constexpr uint32_t RL_MAX_ITEMS = 1024; // CS capacity sweep upper bound
constexpr uint32_t RL_REWARD_CAP = 65536;  // reward ring capacity (power of 2)

enum RlAgentType : uint32_t { AGENT_DQN = 0, AGENT_A2C = 1 };

// reward/telemetry event kinds (correlated Python-side into shared rewards)
enum RlEventKind : uint32_t {
    EV_CS_HIT = 0,          // value: 1 (local cache hit at nodeId)
    EV_CS_MISS = 1,         // value: 1
    EV_DATA_DELIVERED = 2,  // value: retrieval latency (s), at consumer node
    EV_STALE_DELIVERY = 3,  // value: content age - freshness (s), at consumer
    EV_FRESH_DELIVERY = 4,  // value: 1
    EV_CS_INSERT = 5,       // value: content bytes (nameHash set)
    EV_CS_EVICT = 6,        // value: remaining freshness fraction of victim
    EV_ENERGY_DELTA = 7,    // value: Joules consumed since last report
    EV_HOP_COUNT = 8,       // value: hops of a delivered Data
    EV_INTEREST_SENT = 9,   // value: 1 (per-node upstream demand indicator)
};

#pragma pack(push, 8)

struct RlShmHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t stateDim;
    uint32_t itemFeat;
    uint32_t maxItems;
    uint32_t rewardCap;
    uint32_t numAgents;      // updated as agents register
    uint32_t runNumber;
};

struct RlDecisionSlot {
    uint64_t seq;            // monotonically increasing decision counter
    uint32_t agentId;
    uint32_t agentType;      // RlAgentType
    double simTime;
    uint32_t numItems;       // A2C: valid rows in items[]; DQN: 0
    uint32_t nodeId;
    uint32_t nameHash;       // DQN: FNV-1a of the candidate content name
                             // (per-decision credit assignment); A2C: 0
    uint32_t actionMask;     // DQN: bit 0 = "skip" legal, bit 1 = "admit"
                             // legal. The coordination redundancy rule and
                             // the energy guard are expressed here so the
                             // policy is still QUERIED in constrained states
                             // and learns from them, instead of being
                             // bypassed by a pre-policy veto. A2C: unused.
    uint32_t pad0;
    float state[RL_STATE_DIM];
    float items[RL_MAX_ITEMS][RL_ITEM_FEAT];
    uint8_t mask[RL_MAX_ITEMS];   // 1 = valid item row
};

struct RlActionSlot {
    uint64_t seq;            // must equal DecisionSlot.seq of the request
    int32_t action;          // DQN: 0=forward,1=cache; A2C: victim row index
    float aux;               // optional (e.g. value estimate, for logging)
};

struct RlRewardEvent {
    uint32_t nodeId;
    uint32_t kind;           // RlEventKind
    double simTime;
    float value;
    uint32_t nameHash;       // FNV-1a of content name (0 if n/a)
};

struct RlRewardRing {
    uint64_t head;           // written by C++ (producer)
    uint64_t tail;           // written by Python (consumer)
    uint64_t dropped;        // events lost to overflow (C++ increments)
    uint64_t pad;
    RlRewardEvent events[RL_REWARD_CAP];
};

struct RlShmSegment {
    RlShmHeader header;
    RlDecisionSlot decision;
    RlActionSlot action;
    RlRewardRing rewards;
};

#pragma pack(pop)

// FNV-1a 32-bit for content-name correlation across C++/Python
inline uint32_t rlNameHash(const char *s) {
    uint32_t h = 2166136261u;
    for (; *s; s++) { h ^= (uint8_t)*s; h *= 16777619u; }
    return h;
}

} // namespace ndniot

#endif
