//
// NDN-IoT-RL - RlCS
// Content Store subclass owning the CACHE REPLACEMENT decision (thesis RO2).
// When replacementPolicy = "RL": tiered eviction (thesis 3.5.4) — expired
// entries are evicted first without querying the agent; otherwise the A2C
// agent scores all cached items through the RlBridge and the sampled victim
// is evicted. Fallback (bridge inactive / invalid action): LRU.
// Any base policy string (LRU/LFU/FIFO/LFF) keeps the base behavior.
//

#ifndef __NDNIOT_RLCS_H
#define __NDNIOT_RLCS_H

#include "ndn/tables/CS.h"
#include "rl/RlShmLayout.h"

namespace ndniot {

class RlBridge;
class RlNDNProcessor;

class RlCS : public CS {
protected:
    bool useRl = false;
    RlBridge *bridge = nullptr;
    int rlAgentId = -1;
    RlNDNProcessor *processor = nullptr;   // node context provider (lazy)
    uint32_t nodeId = 0;

    bool admitVsRetain = true;
    double replacementMacsPerItem = 19584;
    double replacementMacsFixed = 34944;
    long admitVetoes = 0;
    simtime_t admissionGrace = SIMTIME_ZERO;
    long graceProtections = 0;
    // eviction-thrash instrumentation (admit-then-immediately-evict)
    long zeroHitEvictions = 0;
    double evictedAgeSum = 0;
    long evictedCount = 0;

    virtual void initialize() override;
    virtual void evictEntry() override;
    virtual void handleInsertRequest(CSInsertRequest *request) override;

    bool evictExpiredFirst();
    bool graceProtected(const CSEntry *e, simtime_t now) const {
        return admissionGrace > SIMTIME_ZERO &&
               (now - e->insertionTime) < admissionGrace;
    }
    void noteEviction(const CSEntry *e, simtime_t now);
    bool hasExpiredEntry() const;
    double remainingFreshFrac(const DataPacket *d) const;
    double worstResidentValue() const;
    void buildItemFeatures(std::vector<const CSEntry *> &order, float *items);

public:
    long rlEvictions = 0, tieredEvictions = 0, fallbackEvictions = 0;
    virtual void finish() override;
};

} // namespace ndniot

#endif
