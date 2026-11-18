//
// NDN-IoT-RL - RlNDNProcessor
// NDNProcessor subclass owning the CACHE PLACEMENT decision (thesis RO1).
// placementPolicy selects:
//   "lce"        Leave Copy Everywhere (base behavior)
//   "lcd"        Leave Copy Down (cache only directly below the hit point)
//   "prob"       ProbCache-style probabilistic admission (probCacheP param)
//   "freshCheck" LCE gated on remaining freshness > freshThreshold
//   "rl"         DQN agent via RlBridge (fallback: LCE when bridge inactive)
// Also feeds the PopularityTracker from every Interest and pushes reward
// telemetry events (hits/misses) to the bridge.
//

#ifndef __NDNIOT_RLNDNPROCESSOR_H
#define __NDNIOT_RLNDNPROCESSOR_H

#include "ndn/core/NDNProcessor.h"
#include "rlndn/PopularityTracker.h"
#include "rl/RlShmLayout.h"
#include "coord/BloomFilter.h"
#include <map>

namespace ndniot {

class RlBridge;
class CS;

class RlNDNProcessor : public NDNProcessor {
protected:
    std::string placementPolicy = "lce";
    double probCacheP = 0.5;
    double freshThreshold = 5.0;   // seconds of remaining freshness required
    double energyGuardFraction = 0.2;
    int energyCoordMode = 0;          // 0 residual, 1 clock, 2 constant
    int digestTrustMode = 0;          // 0 honest, 1 lossy, 2 stale, 3 misreport
    double digestLossProb = 0.5;
    double energyCoordHorizon = 600.0;
    int lcdCacheDepth = 1;
    bool coordAsActionMask = true;

    // ---- inference cost accounting (the model pays for its own compute) ----
    // MAC counts are derived from the actual PyTorch modules, not assumed:
    //   placement QNet     16x256 + 256x256 + 256x2            =  70 144 MAC
    //   replacement actor  (8+16)x128 + 128x128 + 128x1        =  19 584 MAC/item
    //   replacement critic (2*128+16)x128 + 128x1              =  34 944 MAC/decision
    double placementMacs = 70144;
    double inferenceEnergyPerMacJ = 0.0;   // 0 = inference charged nothing (legacy)
    double inferenceTimePerMacS = 0.0;
    simtime_t computeBusyUntil;            // single-threaded MCU occupancy model
    long inferenceCount = 0;
    double inferenceEnergyJ = 0.0;
    double inferenceDelaySumS = 0.0;
    simsignal_t rlInferenceEnergySignal;

    RlBridge *bridge = nullptr;
    int rlAgentId = -1;
    CS *csModule = nullptr;
    cModule *energyStorage = nullptr;   // host's IEpEnergyStorage (optional)

    PopularityTracker popularity;

    // ---- coordination layer (thesis RO3) ----
    bool enableDigests = false;
    simtime_t digestInterval;
    int redundancyThreshold = 2;
    bool redundancyVeto = true;   // false = soft coordination: digests only inform the learner
    uint32_t digestBits = 0;          // 0 = size the filter to the Content Store
    uint32_t digestHashes = 4;
    double digestFalsePositive = 0.01;
    bool digestSized = false;
    bool piggybackDigests = true;
    std::map<int, simtime_t> lastDigestTo;     // outFace -> last digest carried
    long digestsPiggybacked = 0;
    void sizeDigest();
  public:
    double capacityFeature() const;
  protected:
    std::string digestPayload();
    int64_t digestWireBytes() const;
    void attachDigest(NdnPacket *pkt, int outFace);
    bool absorbDigest(NdnPacket *pkt, int inFace);
    cMessage *digestTimer = nullptr;
    cMessage *energyTimer = nullptr;
    simtime_t energyReportInterval;
    double lastResidualJ = -1;
    std::map<int, BloomFilter> neighborDigests;   // inFace -> latest filter
    long coordBytesSent = 0, coordBytesReceived = 0, digestsSent = 0;
    long redundancyVetoes = 0, evictNotifies = 0;
    long admissionsOffered = 0, admissionsAccepted = 0, energyGuardVetoes = 0;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void processInterest(InterestPacket *interest, int inFace) override;
    virtual void handleCSResponse(cMessage *msg) override;
    virtual bool shouldCacheData(DataPacket *data) override;

    void buildPlacementState(DataPacket *data, float *state);
    // Charge `macs` multiply-accumulates of on-node inference: energy to the
    // battery, time to the node's single compute pipeline.
    void chargeInference(double macs);
    virtual void forwardData(DataPacket *data, int outFace) override;
    virtual void forwardInterest(InterestPacket *interest, int outFace) override;
    virtual void processData(DataPacket *data, int inFace) override;
    double residualEnergyFraction() const;
    void sendDigest();
    void handleDigest(InterestPacket *interest, int inFace);
    int neighborCachedCount(const std::string &name) const;

public:
    const PopularityTracker &getPopularityTracker() const { return popularity; }
    double getEnergyFraction() const { return residualEnergyFraction(); }
    // RlCS charges its replacement-network inference through the processor so
    // that both networks share one compute pipeline and one battery.
    void chargeInferenceExternal(double macs) { chargeInference(macs); }

    // EvictNotify (replacement -> placement, thesis 3.5.2): feedback on how
    // well an admitted item performed before eviction.
    virtual void onEvictNotify(const std::string &name, int hits, double retention) {
        evictNotifies++;
    }

    virtual ~RlNDNProcessor() {
        cancelAndDelete(digestTimer);
        cancelAndDelete(energyTimer);
    }
};

} // namespace ndniot

#endif
