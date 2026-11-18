//
// NDN-IoT-RL - RlBridge
// One instance per network. Couples the simulation to the Python MAACO
// trainer with the user-required dual mechanism:
//   * ZeroMQ REQ/REP (tcp://127.0.0.1:port)  — lock-step control: HELLO,
//     DECISION_READY, EPISODE_END. The REQ recv() intentionally blocks the
//     (single-threaded) simulation at every decision point; training runs
//     faster than real time.
//   * POSIX shared memory (RlShmSegment)     — bulk data: decision state
//     vectors, A2C per-item feature matrices, action slot, reward-event ring.
//
// Modes: "off" (no bridge, callers fall back to heuristics), "random"
// (built-in random actions, no Python needed — bridge plumbing test),
// "train"/"infer" (Python drives; on connect failure or per-decision timeout
// the bridge degrades to fallback so runs never hang: fail-open).
//
// Endpoint/shm-name resolution order: env MAACO_PORT / MAACO_SHM (set by
// train.py for parallel episodes), else NED params.
//

#ifndef __NDNIOT_RLBRIDGE_H
#define __NDNIOT_RLBRIDGE_H

#include <omnetpp.h>
#include "RlShmLayout.h"

using namespace omnetpp;

namespace ndniot {

class RlBridge : public cSimpleModule {
protected:
    std::string mode;        // off | random | train | infer
    std::string endpoint;
    std::string shmName;
    int decisionTimeoutMs = 30000;
    bool degraded = false;   // fail-open latch

    void *zmqCtx = nullptr;
    void *zmqSock = nullptr;
    int shmFd = -1;
    RlShmSegment *shm = nullptr;

    uint64_t seqCounter = 0;
    long decisionsServed = 0, fallbacks = 0, rewardEventsPushed = 0;
    // Learning control-plane volume (item j): the paper charges the Bloom
    // digest channel but not its own decision/reward telemetry. These count
    // the bytes that would have to cross a link if the learner were not
    // co-located, using the wire-equivalent size of each shm record.
    long ctrlDecisionBytes = 0, ctrlActionBytes = 0, ctrlRewardBytes = 0;
    long dqnDecisions = 0, a2cDecisions = 0, a2cItemRows = 0;

    struct AgentReg { std::string path; RlAgentType type; };
    std::vector<AgentReg> agents;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override { delete msg; }
    virtual void finish() override;

    bool setupShm();
    bool setupZmq();
    void teardown();
    bool roundTrip(const void *req, size_t reqLen, void *rep, size_t repLen);

public:
    virtual ~RlBridge() { teardown(); }

    bool isActive() const { return !degraded && mode != "off"; }
    const std::string &getMode() const { return mode; }

    // dense agent id for a decision-making module
    int registerAgent(const char *modulePath, RlAgentType type);

    // Blocking decision. DQN: items=nullptr -> returns 0/1.
    // A2C: items = numItems x RL_ITEM_FEAT row-major -> returns victim row.
    // Returns -1 when the caller must use its heuristic fallback.
    int decide(int agentId, RlAgentType type, uint32_t nodeId,
               const float *state, const float *items, uint32_t numItems,
               uint32_t nameHash = 0, uint32_t actionMask = 0x3u);

    // Non-blocking telemetry into the reward ring (no-op when inactive).
    void pushReward(uint32_t nodeId, RlEventKind kind, float value, uint32_t nameHash = 0);

    // Locate the bridge from any module (nullptr if not present in network).
    static RlBridge *find(cModule *from);
};

} // namespace ndniot

#endif
