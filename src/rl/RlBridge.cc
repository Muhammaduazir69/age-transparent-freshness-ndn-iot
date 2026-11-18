#include "RlBridge.h"

#include <zmq.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

namespace ndniot {

Define_Module(RlBridge);

// ---- tiny wire protocol (over ZMQ; bulk data goes through shm) ----
#pragma pack(push, 1)
struct MsgHello {
    uint8_t type;            // 'H'
    uint32_t magic, version;
    uint32_t stateDim, itemFeat, maxItems, rewardCap;
    uint32_t runNumber;
};
struct MsgDecision { uint8_t type; uint64_t seq; };          // 'D'
struct MsgEpisodeEnd { uint8_t type; uint64_t decisions; };  // 'E'
struct MsgReply { uint8_t ok; uint64_t seq; };
#pragma pack(pop)

void RlBridge::initialize() {
    mode = par("mode").stdstringValue();
    endpoint = par("endpoint").stdstringValue();
    shmName = par("shmName").stdstringValue();
    decisionTimeoutMs = par("decisionTimeoutMs");

    if (const char *p = getenv("MAACO_PORT"))
        endpoint = std::string("tcp://127.0.0.1:") + p;
    if (const char *s = getenv("MAACO_SHM"))
        shmName = s;

    if (mode == "off" || mode == "random")
        return;  // no external coupling needed

    if (!setupShm() || !setupZmq()) {
        EV_ERROR << "RlBridge: setup failed, degrading to heuristic fallback" << endl;
        degraded = true;
        teardown();
        return;
    }

    // HELLO handshake: layout guard + run metadata
    MsgHello hello;
    hello.type = 'H';
    hello.magic = SHM_MAGIC;
    hello.version = SHM_VERSION;
    hello.stateDim = RL_STATE_DIM;
    hello.itemFeat = RL_ITEM_FEAT;
    hello.maxItems = RL_MAX_ITEMS;
    hello.rewardCap = RL_REWARD_CAP;
    hello.runNumber = getEnvir()->getConfigEx()->getActiveRunNumber();
    MsgReply rep{};
    if (!roundTrip(&hello, sizeof(hello), &rep, sizeof(rep)) || rep.ok != 1) {
        EV_ERROR << "RlBridge: HELLO failed (Python trainer not reachable at "
                 << endpoint << "), degrading to heuristic fallback" << endl;
        degraded = true;
        teardown();
        return;
    }
    // handshake done: switch to the full per-decision timeout
    zmq_setsockopt(zmqSock, ZMQ_RCVTIMEO, &decisionTimeoutMs, sizeof(decisionTimeoutMs));
    EV_INFO << "RlBridge: connected to trainer at " << endpoint
            << ", shm " << shmName << ", mode " << mode << endl;
}

bool RlBridge::setupShm() {
    shm_unlink(shmName.c_str());  // clear stale segment from a crashed run
    shmFd = shm_open(shmName.c_str(), O_CREAT | O_RDWR | O_EXCL, 0600);
    if (shmFd < 0) {
        EV_ERROR << "shm_open(" << shmName << ") failed: " << strerror(errno) << endl;
        return false;
    }
    if (ftruncate(shmFd, sizeof(RlShmSegment)) != 0) {
        EV_ERROR << "ftruncate failed: " << strerror(errno) << endl;
        return false;
    }
    shm = (RlShmSegment *)mmap(nullptr, sizeof(RlShmSegment),
                               PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
    if (shm == MAP_FAILED) {
        shm = nullptr;
        EV_ERROR << "mmap failed: " << strerror(errno) << endl;
        return false;
    }
    memset(shm, 0, sizeof(RlShmSegment));
    shm->header.magic = SHM_MAGIC;
    shm->header.version = SHM_VERSION;
    shm->header.stateDim = RL_STATE_DIM;
    shm->header.itemFeat = RL_ITEM_FEAT;
    shm->header.maxItems = RL_MAX_ITEMS;
    shm->header.rewardCap = RL_REWARD_CAP;
    shm->header.runNumber = getEnvir()->getConfigEx()->getActiveRunNumber();
    return true;
}

bool RlBridge::setupZmq() {
    zmqCtx = zmq_ctx_new();
    zmqSock = zmq_socket(zmqCtx, ZMQ_REQ);
    int linger = 0;
    zmq_setsockopt(zmqSock, ZMQ_LINGER, &linger, sizeof(linger));
    int helloTimeout = 3000;   // fail fast on HELLO (GUI responsiveness);
    zmq_setsockopt(zmqSock, ZMQ_RCVTIMEO, &helloTimeout, sizeof(helloTimeout));
    int sndTimeout = 5000;
    zmq_setsockopt(zmqSock, ZMQ_SNDTIMEO, &sndTimeout, sizeof(sndTimeout));
    if (zmq_connect(zmqSock, endpoint.c_str()) != 0) {
        EV_ERROR << "zmq_connect(" << endpoint << ") failed: " << zmq_strerror(errno) << endl;
        return false;
    }
    return true;
}

bool RlBridge::roundTrip(const void *req, size_t reqLen, void *rep, size_t repLen) {
    if (zmq_send(zmqSock, req, reqLen, 0) < 0)
        return false;
    int n = zmq_recv(zmqSock, rep, repLen, 0);
    return n == (int)repLen;
}

int RlBridge::registerAgent(const char *modulePath, RlAgentType type) {
    Enter_Method_Silent();
    agents.push_back({modulePath, type});
    if (shm)
        shm->header.numAgents = (uint32_t)agents.size();
    return (int)agents.size() - 1;
}

int RlBridge::decide(int agentId, RlAgentType type, uint32_t nodeId,
                     const float *state, const float *items, uint32_t numItems,
                     uint32_t nameHash, uint32_t actionMask) {
    Enter_Method_Silent();
    if (mode == "off" || degraded)
        return -1;

    // Control-plane accounting (wire-equivalent bytes, counted for every mode
    // that actually consults a policy): a decision record carries the fixed
    // header (agentId, agentType, simTime, numItems, nodeId, nameHash,
    // actionMask = 32 B) plus the state vector, plus the A2C item matrix.
    if (type == AGENT_DQN) {
        dqnDecisions++;
        ctrlDecisionBytes += 32 + 4 * (long)RL_STATE_DIM;
    } else {
        a2cDecisions++;
        a2cItemRows += numItems;
        ctrlDecisionBytes += 32 + 4 * (long)RL_STATE_DIM
                             + 4 * (long)RL_ITEM_FEAT * (long)numItems;
    }
    ctrlActionBytes += 16;   // seq + action + aux

    if (mode == "random") {
        decisionsServed++;
        if (type == AGENT_DQN)
            return (actionMask & 0x2u) ? intuniform(0, 1) : 0;
        return numItems > 0 ? intuniform(0, numItems - 1) : -1;
    }

    // write DecisionSlot
    RlDecisionSlot &d = shm->decision;
    d.seq = ++seqCounter;
    d.agentId = (uint32_t)agentId;
    d.agentType = type;
    d.simTime = simTime().dbl();
    d.nodeId = nodeId;
    d.nameHash = nameHash;
    d.actionMask = actionMask;
    d.numItems = numItems;
    memcpy(d.state, state, sizeof(float) * RL_STATE_DIM);
    if (items && numItems > 0) {
        uint32_t n = std::min(numItems, RL_MAX_ITEMS);
        memcpy(d.items, items, sizeof(float) * RL_ITEM_FEAT * n);
        memset(d.mask, 0, RL_MAX_ITEMS);
        memset(d.mask, 1, n);
    } else {
        memset(d.mask, 0, RL_MAX_ITEMS);
    }

    MsgDecision msg{'D', d.seq};
    MsgReply rep{};
    if (!roundTrip(&msg, sizeof(msg), &rep, sizeof(rep)) || rep.ok != 1 ||
        shm->action.seq != d.seq) {
        EV_ERROR << "RlBridge: decision round-trip failed at seq " << d.seq
                 << " — degrading to heuristic fallback for the rest of the run" << endl;
        degraded = true;
        fallbacks++;
        return -1;
    }
    decisionsServed++;
    int action = shm->action.action;
    if (type == AGENT_A2C && (action < 0 || (uint32_t)action >= numItems))
        return -1;  // out-of-range guard
    if (type == AGENT_DQN && action != 0 && action != 1)
        return -1;
    return action;
}

void RlBridge::pushReward(uint32_t nodeId, RlEventKind kind, float value, uint32_t nameHash) {
    if (!shm || mode == "off" || mode == "random" || degraded)
        return;
    RlRewardRing &r = shm->rewards;
    if (r.head - r.tail >= RL_REWARD_CAP) {
        r.dropped++;
        return;  // ring full: drop newest, never block
    }
    RlRewardEvent &e = r.events[r.head % RL_REWARD_CAP];
    e.nodeId = nodeId;
    e.kind = kind;
    e.simTime = simTime().dbl();
    e.value = value;
    e.nameHash = nameHash;
    r.head++;
    rewardEventsPushed++;
    ctrlRewardBytes += 24;   // nodeId, kind, simTime, value, nameHash
}

void RlBridge::finish() {
    if (zmqSock && !degraded && mode != "off" && mode != "random") {
        MsgEpisodeEnd msg{'E', (uint64_t)decisionsServed};
        MsgReply rep{};
        roundTrip(&msg, sizeof(msg), &rep, sizeof(rep));
    }
    recordScalar("rlDecisionsServed", decisionsServed);
    recordScalar("rlFallbacks", fallbacks);
    recordScalar("rlRewardEvents", rewardEventsPushed);
    recordScalar("rlDegraded", degraded ? 1 : 0);
    recordScalar("ctrlDecisionBytes", ctrlDecisionBytes);
    recordScalar("ctrlActionBytes", ctrlActionBytes);
    recordScalar("ctrlRewardBytes", ctrlRewardBytes);
    recordScalar("ctrlTotalBytes",
                 ctrlDecisionBytes + ctrlActionBytes + ctrlRewardBytes);
    recordScalar("rlDqnDecisions", dqnDecisions);
    recordScalar("rlA2cDecisions", a2cDecisions);
    recordScalar("rlA2cItemRows", a2cItemRows);
    teardown();
}

void RlBridge::teardown() {
    if (zmqSock) { zmq_close(zmqSock); zmqSock = nullptr; }
    if (zmqCtx) { zmq_ctx_term(zmqCtx); zmqCtx = nullptr; }
    if (shm) { munmap(shm, sizeof(RlShmSegment)); shm = nullptr; }
    if (shmFd >= 0) { close(shmFd); shmFd = -1; shm_unlink(shmName.c_str()); }
}

RlBridge *RlBridge::find(cModule *from) {
    cModule *sys = from->getSystemModule();
    return dynamic_cast<RlBridge *>(sys->getSubmodule("rlBridge"));
}

} // namespace ndniot
