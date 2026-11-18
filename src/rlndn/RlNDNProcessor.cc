#include "RlNDNProcessor.h"
#include <cmath>
#include "rl/RlBridge.h"
#include "ndn/tables/CS.h"
#include "ndn/core/NdnControlMessages_m.h"

#include "inet/power/contract/IEpEnergyStorage.h"

namespace ndniot {

Define_Module(RlNDNProcessor);

void RlNDNProcessor::initialize() {
    NDNProcessor::initialize();

    placementPolicy = par("placementPolicy").stdstringValue();
    probCacheP = par("probCacheP");
    freshThreshold = par("freshThreshold");
    energyGuardFraction = par("energyGuardFraction");
    {
        const std::string m = par("energyCoordMode").stdstringValue();
        energyCoordMode = (m == "clock") ? 1 : (m == "constant") ? 2 : 0;
        if (m != "residual" && m != "clock" && m != "constant")
            throw cRuntimeError("energyCoordMode must be residual, clock or constant, got '%s'", m.c_str());
        energyCoordHorizon = par("energyCoordHorizon").doubleValue();
    }
    {
        const std::string t = par("digestTrustMode").stdstringValue();
        digestTrustMode = (t == "lossy") ? 1 : (t == "stale") ? 2
                        : (t == "misreport") ? 3 : 0;
        if (t != "honest" && t != "lossy" && t != "stale" && t != "misreport")
            throw cRuntimeError("digestTrustMode must be honest, lossy, stale or misreport, got '%s'", t.c_str());
        digestLossProb = par("digestLossProb").doubleValue();
    }
    lcdCacheDepth = (int)par("lcdCacheDepth").intValue();
    coordAsActionMask = par("coordAsActionMask");
    placementMacs = par("placementMacs").doubleValue();
    inferenceEnergyPerMacJ = par("inferenceEnergyPerMacJ").doubleValue();
    inferenceTimePerMacS = par("inferenceTimePerMacS").doubleValue();
    computeBusyUntil = simTime();
    rlInferenceEnergySignal = registerSignal("rlInferenceEnergy");

    csModule = dynamic_cast<CS *>(getParentModule()->getSubmodule("cs"));

    // host node = NdnOverlayApp's parent (SensorNode / NrUe / StandardHost)
    cModule *overlay = getParentModule()->getParentModule();
    if (overlay && overlay->getParentModule())
        energyStorage = overlay->getParentModule()->getSubmodule("energyStorage");

    bridge = RlBridge::find(this);
    if (bridge && placementPolicy == "rl")
        rlAgentId = bridge->registerAgent(getFullPath().c_str(), AGENT_DQN);

    // coordination layer (thesis RO3)
    enableDigests = par("enableDigests");
    digestInterval = par("digestInterval");
    redundancyThreshold = par("redundancyThreshold");
    redundancyVeto = par("redundancyVeto");
    digestBits = (uint32_t)par("digestBits").intValue();
    digestFalsePositive = par("digestFalsePositive").doubleValue();
    piggybackDigests = par("piggybackDigests");
    // sized lazily at the first digest: the Content Store may not have read
    // its own parameters yet when this module initializes
    if (enableDigests) {
        digestTimer = new cMessage("digestTimer");
        scheduleAt(simTime() + uniform(0.5, 1.5) * digestInterval, digestTimer);
    }
    energyReportInterval = par("energyReportInterval");
    if (bridge && energyStorage && energyReportInterval > SIMTIME_ZERO) {
        energyTimer = new cMessage("energyTimer");
        scheduleAt(simTime() + energyReportInterval, energyTimer);
    }
}

void RlNDNProcessor::handleMessage(cMessage *msg) {
    if (msg == digestTimer) {
        sendDigest();
        scheduleAt(simTime() + (piggybackDigests ? 0.5 : 1.0) * digestInterval, digestTimer);
        return;
    }
    if (msg == energyTimer) {
        auto *storage = dynamic_cast<inet::power::IEpEnergyStorage *>(energyStorage);
        if (storage) {
            double nowJ = storage->getResidualEnergyCapacity().get();
            if (lastResidualJ >= 0 && bridge)
                bridge->pushReward(nodeId, EV_ENERGY_DELTA,
                                   (float)(lastResidualJ - nowJ), 0);
            lastResidualJ = nowJ;
        }
        scheduleAt(simTime() + energyReportInterval, energyTimer);
        return;
    }
    NDNProcessor::handleMessage(msg);
}

void RlNDNProcessor::sizeDigest() {
    // A Bloom filter only has to answer "does this neighbor hold the name" for
    // at most n = csMaxEntries names, so its optimal size is
    //   m = -n ln p / (ln 2)^2,  k = (m / n) ln 2
    // for a target false-positive rate p. A fixed 1 024-bit filter on a
    // ten-entry store is sixty times larger than needed and, at 144 bytes,
    // no longer fits one IEEE 802.15.4 frame, so every digest was fragmented.
    digestSized = true;
    int n = csModule ? csModule->getMaxEntries() : 0;
    if (digestBits > 0)
        return;
    if (n <= 0 || n > (int)RL_MAX_ITEMS) {
        digestBits = 1024;
        return;
    }
    double m = std::ceil(-n * std::log(digestFalsePositive) / (M_LN2 * M_LN2));
    digestBits = std::max<uint32_t>(64, ((uint32_t)m + 7) / 8 * 8);
    digestHashes = (uint32_t)std::max(1.0, std::min(8.0,
                        std::round((double)digestBits / n * M_LN2)));
}

std::string RlNDNProcessor::digestPayload() {
    if (!digestSized)
        sizeDigest();
    BloomFilter filter(digestBits, digestHashes);
    if (digestTrustMode == 3 && nodeId == 0) {
        // misreporting neighbor: advertise "I hold everything"
        filter.setAll();
    }
    else if (csModule) {
        for (const auto &name : csModule->getCachedNames())
            filter.add(name);
    }
    // self-describing: receivers may size their own filters differently
    return std::to_string(digestBits) + ":" + std::to_string(digestHashes) +
           ":" + filter.toHex();
}

int64_t RlNDNProcessor::digestWireBytes() const {
    return (int64_t)(digestBits + 7) / 8 + 2;   // filter + one byte each for m/8 and k
}

void RlNDNProcessor::attachDigest(NdnPacket *pkt, int outFace) {
    // Piggybacking: the first packet a node sends on a wire face in each
    // digest interval carries the node's current filter, so coordination
    // costs bytes on frames that are sent anyway instead of frames of its own.
    if (!enableDigests || !piggybackDigests || outFace < 2 || !csModule)
        return;
    if (pkt->getAppPayload()[0] != '\0')
        return;
    auto it = lastDigestTo.find(outFace);
    if (it != lastDigestTo.end() && simTime() - it->second < digestInterval)
        return;
    pkt->setAppPayload(digestPayload().c_str());
    pkt->addByteLength(digestWireBytes());
    coordBytesSent += digestWireBytes();
    lastDigestTo[outFace] = simTime();
    digestsPiggybacked++;
}

bool RlNDNProcessor::absorbDigest(NdnPacket *pkt, int inFace) {
    std::string p = pkt->getAppPayload();
    if (p.empty())
        return false;
    size_t a = p.find(':'), b = a == std::string::npos ? a : p.find(':', a + 1);
    if (b != std::string::npos) {
        uint32_t m = (uint32_t)std::stoul(p.substr(0, a));
        uint32_t k = (uint32_t)std::stoul(p.substr(a + 1, b - a - 1));
        BloomFilter filter(m, k);
        if (filter.fromHex(p.substr(b + 1))) {
            // Coordination-trust degradation.  The packet is still accounted
            // for below, so only the agent's view of its neighbors changes.
            bool accept = true;
            if (digestTrustMode == 1)                     // lossy
                accept = (uniform(0, 1) >= digestLossProb);
            else if (digestTrustMode == 2)                // stale: never refresh
                accept = (neighborDigests.find(inFace) == neighborDigests.end());
            if (accept)
                neighborDigests[inFace] = filter;
        }
        int64_t bytes = (int64_t)(m + 7) / 8 + 2;
        coordBytesReceived += bytes;
        if (pkt->getByteLength() > bytes)
            pkt->addByteLength(-bytes);
    }
    pkt->setAppPayload("");
    return true;
}

void RlNDNProcessor::sendDigest() {
    // Standalone /coord/digest Interest (never forwarded), sent one hop on
    // every wire face that has not carried a digest within the last interval.
    if (!csModule)
        return;
    std::string payload;
    for (const auto &kv : faceToGate) {
        int face = kv.first;
        if (face < 2)
            continue;  // skip app faces (0 = consumer, 1 = producer)
        auto it = lastDigestTo.find(face);
        // with piggybacking, a face only gets a frame of its own after it has
        // been silent for one and a half intervals
        if (piggybackDigests && it != lastDigestTo.end() &&
            simTime() - it->second < 1.5 * digestInterval)
            continue;
        if (payload.empty())
            payload = digestPayload();
        auto *interest = new InterestPacket("CoordDigest");
        std::string name = "/coord/digest/" + std::to_string(nodeId);
        interest->setName(name.c_str());
        interest->setNonce(intuniform(0, INT_MAX));
        interest->setAppPayload(payload.c_str());
        interest->setByteLength(16 + digestWireBytes());
        coordBytesSent += interest->getByteLength();
        lastDigestTo[face] = simTime();
        NDNProcessor::forwardInterest(interest, face);
        digestsSent++;
    }
}

void RlNDNProcessor::handleDigest(InterestPacket *interest, int inFace) {
    coordBytesReceived += 16;   // header; absorbDigest counts the filter
    absorbDigest(interest, inFace);
    delete interest;
}

void RlNDNProcessor::forwardInterest(InterestPacket *interest, int outFace) {
    attachDigest(interest, outFace);
    NDNProcessor::forwardInterest(interest, outFace);
}

void RlNDNProcessor::processData(DataPacket *data, int inFace) {
    absorbDigest(data, inFace);   // strip before anything caches the packet
    NDNProcessor::processData(data, inFace);
}

int RlNDNProcessor::neighborCachedCount(const std::string &name) const {
    int count = 0;
    for (const auto &kv : neighborDigests)
        if (kv.second.contains(name))
            count++;
    return count;
}

void RlNDNProcessor::chargeInference(double macs) {
    // The paper's own thesis is that costs are measured rather than assumed,
    // so the policy networks pay for their own forward passes: energy is
    // drawn from the same battery as the radio and the cache writes, and the
    // compute time occupies a single-threaded node so packets queue behind
    // it. Both rates are ini-tunable; 0 reproduces the legacy "inference is
    // free" behaviour exactly.
    if (macs <= 0)
        return;
    if (inferenceEnergyPerMacJ > 0) {
        double j = macs * inferenceEnergyPerMacJ;
        inferenceEnergyJ += j;
        emit(rlInferenceEnergySignal, j);
    }
    if (inferenceTimePerMacS > 0) {
        simtime_t busy = SimTime(macs * inferenceTimePerMacS);
        simtime_t start = std::max(computeBusyUntil, simTime());
        computeBusyUntil = start + busy;
        inferenceDelaySumS += (computeBusyUntil - simTime()).dbl();
    }
    inferenceCount++;
}

void RlNDNProcessor::forwardData(DataPacket *data, int outFace) {
    attachDigest(data, outFace);
    // Single compute pipeline: a Data packet cannot leave the node before the
    // inference that was run for it (and anything queued ahead of it) has
    // finished.
    if (inferenceTimePerMacS > 0 && computeBusyUntil > simTime()) {
        simtime_t delay = computeBusyUntil - simTime();
        emit(dataSentSignal, 1L);
        dataSent++;
        int gateIndex = getGateForFace(outFace);
        if (gateIndex == -1 || gateIndex >= gateSize("ndnOut")) {
            delete data;
            return;
        }
        data->setHopCount(data->getHopCount() + 1);
        if (data->getLcdDepth() >= 0)
            data->setLcdDepth(data->getLcdDepth() + 1);
        sendDelayed(data, delay, "ndnOut", gateIndex);
        return;
    }
    NDNProcessor::forwardData(data, outFace);
}

void RlNDNProcessor::finish() {
    NDNProcessor::finish();
    recordScalar("inferenceCount", inferenceCount);
    recordScalar("inferenceEnergyJ", inferenceEnergyJ);
    recordScalar("inferenceMeanDelayS",
                 inferenceCount ? inferenceDelaySumS / inferenceCount : 0.0);
    recordScalar("coordBytesSent", coordBytesSent);
    recordScalar("coordBytesReceived", coordBytesReceived);
    recordScalar("digestsSent", digestsSent);
    recordScalar("digestBits", (double)(enableDigests ? digestBits : 0));
    recordScalar("digestsPiggybacked", digestsPiggybacked);
    recordScalar("redundancyVetoes", redundancyVetoes);
    recordScalar("evictNotifies", evictNotifies);
    recordScalar("admissionsOffered", admissionsOffered);
    recordScalar("admissionsAccepted", admissionsAccepted);
    recordScalar("energyGuardVetoes", energyGuardVetoes);
}

void RlNDNProcessor::processInterest(InterestPacket *interest, int inFace) {
    // coordination-plane traffic: consume one-hop, never forwarded/cached
    if (strncmp(interest->getName(), "/coord/digest/", 14) == 0) {
        handleDigest(interest, inFace);
        return;
    }
    absorbDigest(interest, inFace);
    popularity.onInterest(interest->getName(), simTime());
    if (bridge)
        bridge->pushReward(nodeId, EV_INTEREST_SENT, 1.0f, rlNameHash(interest->getName()));
    NDNProcessor::processInterest(interest, inFace);
}

void RlNDNProcessor::handleCSResponse(cMessage *msg) {
    if (bridge) {
        if (auto *resp = dynamic_cast<CSLookupResponse *>(msg))
            bridge->pushReward(nodeId, resp->getFound() ? EV_CS_HIT : EV_CS_MISS,
                               1.0f, rlNameHash(resp->getName()));
    }
    NDNProcessor::handleCSResponse(msg);
}

double RlNDNProcessor::residualEnergyFraction() const {
    if (!energyStorage)
        return 1.0;  // mains-powered / no battery model
    auto *storage = dynamic_cast<inet::power::IEpEnergyStorage *>(energyStorage);
    if (!storage)
        return 1.0;
    double nominal = storage->getNominalEnergyCapacity().get();
    // IdealEpEnergyStorage reports infinite capacity: inf/inf = NaN would
    // poison the RL state vector (learned never-cache policies) — treat any
    // non-finite or non-positive nominal as mains-powered (fraction 1.0)
    if (nominal <= 0 || !std::isfinite(nominal))
        return 1.0;
    double frac = storage->getResidualEnergyCapacity().get() / nominal;
    return std::isfinite(frac) ? frac : 1.0;
}

void RlNDNProcessor::buildPlacementState(DataPacket *data, float *state) {
    // Feature vector, thesis 3.3.2 (all roughly [0,1]); indices documented in
    // python/maaco/shm_layout.py DQN_FEATURES.
    for (uint32_t i = 0; i < RL_STATE_DIM; i++) state[i] = 0;
    const std::string name = data->getName();
    simtime_t now = simTime();

    state[0] = std::min(1.0, popularity.freqShort(name, now) / 10.0);
    state[1] = std::min(1.0, popularity.freqLong(name, now) / 100.0);
    state[2] = popularity.recency(name, now) / 600.0;
    state[3] = 0.5 + std::max(-0.5, std::min(0.5, popularity.trend(name, now) * 10.0));
    state[4] = std::min(1.0, data->getHopCount() / 10.0);
    state[5] = std::min(1.0, data->getContentLength() / 128.0);
    state[6] = std::min(1.0, data->getFreshnessPeriod().dbl() / 3600.0);
    state[7] = csModule ? csModule->getOccupancyFraction() : 0.0;
    state[8] = csModule ? csModule->getHitRatio() : 0.0;
    // Observed energy coordinate.  shouldCache() below still vetoes on the
    // TRUE residual fraction, so "clock" and "constant" blind the agent without
    // changing the node's physical behaviour.
    state[9] = (energyCoordMode == 1)
        ? (float)std::min(1.0, std::max(0.0, 1.0 - now.dbl() / energyCoordHorizon))
        : (energyCoordMode == 2) ? 1.0f
        : (float)residualEnergyFraction();
    state[10] = enableDigests
        ? std::min(1.0, (double)neighborCachedCount(data->getName()) / redundancyThreshold)
        : 0.0;
    state[11] = csModule ? csModule->getAvgRemainingFreshnessFraction() : 1.0;

    // 12..15: candidate-validity block (added after the feature audit — the
    // agent previously had NO signal for how much useful life the Data it is
    // being asked to admit still has, only for the class-level freshnessPeriod
    // in state[6], which is constant within a scenario).
    double fp = data->getFreshnessPeriod().dbl();
    double remS = fp > 0
        ? (data->getOriginTime() + data->getFreshnessPeriod() - now).dbl()
        : 0.0;
    double remFrac = fp > 0 ? remS / fp : 0.0;
    remFrac = remFrac < 0 ? 0 : (remFrac > 1 ? 1 : remFrac);
    state[12] = remFrac;                                   // remaining life (fraction)
    state[13] = std::min(1.0, std::max(0.0, remS) / 60.0); // remaining life (absolute, 60 s scale)
    state[14] = csModule ? csModule->fractionLessFreshThan(remFrac) : 1.0;
    state[15] = (enableDigests && !neighborDigests.empty())
        ? (double)neighborCachedCount(data->getName()) / neighborDigests.size()
        : 0.0;
    // store capacity on a log scale (5 entries -> 0.23, 50 -> 0.56): with it
    // one policy can learn that declining an admission is free when the
    // store is scarce and costly when it can hold most of the catalog
    if constexpr (RL_STATE_DIM > 16)
        state[16] = capacityFeature();
}

double RlNDNProcessor::capacityFeature() const {
    int cap = csModule ? csModule->getMaxEntries() : 0;
    return cap > 0 ? std::min(1.0, std::log2((double)cap) / 10.0) : 1.0;
}

bool RlNDNProcessor::shouldCacheData(DataPacket *data) {
    if (!NDNProcessor::shouldCacheData(data))
        return false;  // caching disabled or content not cacheable

    admissionsOffered++;

    // Constraint evaluation. Both constraints used to `return false` here,
    // BEFORE the learned policy was queried, which meant the agent never
    // observed a vetoed state and never learned from one. For the learned
    // policy they are now expressed as an action mask applied after the
    // query (see below); for the fixed policies the effect is identical to
    // the old early return.
    const bool energyVeto = residualEnergyFraction() < energyGuardFraction;
    const bool coordVeto =
        enableDigests && redundancyVeto &&
        neighborCachedCount(data->getName()) >= redundancyThreshold;
    if (energyVeto) energyGuardVetoes++;
    if (coordVeto) redundancyVetoes++;
    const bool masked = energyVeto || coordVeto;

    const bool rlMask = (placementPolicy == "rl") && coordAsActionMask;
    if (masked && !rlMask)
        return false;

    bool admit = true;
    if (placementPolicy == "lcd") {
        // Laoutaris LCD: cache one hop BELOW the serving node. lcdDepth is
        // stamped 0 at the serving point and incremented in forwardData on
        // the way out, so the node one hop downstream observes 1.
        admit = data->getLcdDepth() == lcdCacheDepth;
    }
    else if (placementPolicy == "prob") {
        admit = uniform(0, 1) < probCacheP;
    }
    else if (placementPolicy == "freshCheck") {
        simtime_t remaining = data->getFreshnessPeriod() - (simTime() - data->getOriginTime());
        admit = remaining.dbl() > freshThreshold;
    }
    else if (placementPolicy == "rl") {
        admit = !masked;   // fallback: LCE, still constraint-respecting
        if (bridge && bridge->isActive() && rlAgentId >= 0) {
            float state[RL_STATE_DIM];
            buildPlacementState(data, state);
            uint32_t h = rlNameHash(data->getName());
            // action mask: bit 0 = "skip" legal, bit 1 = "admit" legal.
            // Under a constraint only "skip" is legal, but the policy is
            // still queried, so the state enters the replay buffer and the
            // agent learns the consequences of constrained states instead of
            // never seeing them.
            uint32_t amask = masked ? 0x1u : 0x3u;
            chargeInference(placementMacs);
            int action = bridge->decide(rlAgentId, AGENT_DQN, nodeId, state,
                                        nullptr, 0, h, amask);
            if (action >= 0)
                admit = (action == 1);
            if (masked)
                admit = false;   // belt and braces: the mask is authoritative
            // Per-decision admission cost (thesis 3.5.1 energy term): the
            // agent previously paid nothing for admitting, so "always cache"
            // was a cost-free local optimum. EV_CS_INSERT closes that loop;
            // its Python weight (w_insert) is 0 by default.
            if (admit)
                bridge->pushReward(nodeId, EV_CS_INSERT, 1.0f, h);
        }
    }

    if (admit)
        admissionsAccepted++;
    return admit;
}

} // namespace ndniot
