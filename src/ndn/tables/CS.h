//
// NDN-IoT-RL - Content Store (Cache)
//

#ifndef __NDNIOT_CS_H
#define __NDNIOT_CS_H

#include <omnetpp.h>
// #include "inet/common/INETDefs.h"  // Removed INET dependency
#include "ndn/packets/NdnPackets_m.h"
#include "ndn/core/NdnControlMessages_m.h"
#include <map>
#include <list>
#include <string>
#include <cmath>

using namespace omnetpp;

namespace ndniot {

enum class ReplacementPolicy {
    LRU,        // Least Recently Used
    LFU,        // Least Frequently Used (raw monotone access count)
    LFU_AGED,   // Windowed/exponentially-decayed LFU (removes the cache-pollution
                // strawman: a once-hot object cannot hold a slot forever)
    FIFO,       // First In First Out
    LFF,        // Least Fresh First on the per-hop-reset NDN TTL
    TARA_ADD,   // The same three factors combined as an equally weighted sum
                // of min-max normalised terms, in the manner of the published
                // freshness-plus-popularity caching utility functions. Used to
                // separate the choice of factors from the functional form.
    TARA,       // Expected-utility retention: evict the smallest product of
                // decayed request rate, TRUE residual life and hops saved.
                // Derived in the method section as the expected number of
                // valid hits a copy will still serve times the retrieval cost
                // each saves; the three factors multiply rather than add.
    LFF_ORIGIN  // Least Fresh First on the ORIGIN clock, i.e. the same
                // application-level remaining life the learned agent is given
                // as item feature 7 and the same clock the freshness metric
                // is scored against. Under a uniform freshness period the
                // hop-reset variant degenerates to FIFO; this one does not.
};

class CSEntry {
public:
    std::string name;
    DataPacket *data;
    simtime_t insertionTime;
    simtime_t expiryTime;
    int accessCount;
    simtime_t lastAccess;
    int size;  // bytes
    int hopCount = 0;   // hops the Data traveled before being cached (re-fetch cost proxy)
    // Aged-LFU state: an exponentially decayed access score. The entry is
    // created because somebody asked for it, so it enters with one credit
    // instead of the raw-LFU zero that guarantees immediate eviction.
    double decayedCount = 1.0;
    simtime_t lastDecay;
    // Origin clock: NDN legitimately resets FreshnessPeriod at every hop, so
    // expiryTime is a per-hop TTL. originExpiry() is the application-level
    // deadline the freshness metric is actually scored against.
    simtime_t originExpiryTime;

    CSEntry(const std::string &n, DataPacket *d) :
        name(n), data(d), insertionTime(simTime()),
        expiryTime(0), accessCount(0), lastAccess(simTime()), size(0) {
        lastDecay = simTime();
        originExpiryTime = 0;
        if (d) {
            size = d->getContentLength();
            hopCount = d->getHopCount();
            if (d->getFreshnessPeriod() > 0) {
                expiryTime = simTime() + d->getFreshnessPeriod();
                simtime_t origin = d->getOriginTime() > SIMTIME_ZERO
                                       ? d->getOriginTime() : simTime();
                originExpiryTime = origin + d->getFreshnessPeriod();
            }
        }
    }

    // decayed access score with half-life `tau` seconds
    double agedScore(simtime_t now, double tau) const {
        double dt = (now - lastDecay).dbl();
        if (dt <= 0 || tau <= 0) return decayedCount;
        return decayedCount * std::exp(-dt / tau);
    }
    void noteAccess(simtime_t now, double tau) {
        decayedCount = agedScore(now, tau) + 1.0;
        lastDecay = now;
    }
    simtime_t originExpiry() const {
        return originExpiryTime > SIMTIME_ZERO ? originExpiryTime : expiryTime;
    }

    // Seconds of life the reading has left on the producer's clock, clamped at
    // zero. Distinct from (expiryTime - now), which NDN restarts at every hop.
    double trueResidual(simtime_t now) const {
        double r = (originExpiry() - now).dbl();
        return r > 0 ? r : 0.0;
    }

    ~CSEntry() {
        if (data) delete data;
    }

    bool isExpired() const {
        return (expiryTime > 0 && simTime() >= expiryTime);
    }

    bool isFresh() const {
        return !isExpired();
    }
};

class CS : public cSimpleModule {
protected:
    // Cache storage (protected: RlCS overrides eviction)
    std::map<std::string, CSEntry*> entries;
    std::list<std::string> lruList;  // For LRU policy

    // Configuration
    int maxSize;  // MB
    int maxEntries; // item-count cap (0 = unlimited)
    int maxSizeBytes;
    ReplacementPolicy policy;
    bool enabled;
    // Symmetric tier-1 sweep: at the moment the store is full, discard an
    // already-expired (worthless) entry before asking the replacement policy
    // to give up a live one. RlCS did this unconditionally while the baseline
    // CS did not, which handed the learned model a non-policy advantage of up
    // to one cleanup interval. Now every policy gets it, and it can be
    // switched off to measure exactly how much it was worth.
    bool expiredFirst = true;
    // Age-transparent freshness. NDN restarts FreshnessPeriod at every cache,
    // relative to local arrival, so a copy stays servable for one full period
    // at each hop and the age a consumer sees grows with path length. With
    // this switch on, a node expires a copy at the end of the producer's
    // lifetime instead, which is what a consumer's validity test means. It
    // needs no new packet field and no synchronised clocks: a forwarder that
    // subtracts the dwell time from FreshnessPeriod before sending carries the
    // same information hop by hop (verified in the age-transparency check).
    bool ageTransparent = false;
    // true selects the clock-free realization: keep the standard local TTL on
    // insertion, but hand the next hop what is left rather than a full period.
    bool ageDecrement = false;
    double lfuAgingTau = 60.0;   // aged-LFU decay constant (s)

    // Statistics
    int currentSize;
    int currentSizeBytes;
    int totalInsertions;
    int totalHits;
    int totalMisses;
    int totalEvictions;
    int totalExpired;
    int expiredFirstEvictions = 0;   // tier-1 sweeps that found a victim
    int csLookupsDisabled = 0;       // lookups answered while the store is off

    // Cleanup
    cMessage *cleanupTimer;
    simtime_t cleanupInterval;

    // Signals
    simsignal_t csSizeSignal;
    simsignal_t csHitSignal;
    simsignal_t csMissSignal;
    simsignal_t csHitRatioSignal;
    simsignal_t csInsertionEventSignal;
    simsignal_t csEvictionEventSignal;

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // Entry management
    CSEntry* findEntry(const std::string &name);
    void insertEntry(const std::string &name, DataPacket *data);
    void removeEntry(const std::string &name);
    virtual void evictEntry();
    void cleanupExpiredEntries();
    // Tier-1 sweep shared by every policy; returns true if it evicted.
    bool sweepExpiredOnce();

    // Replacement policies
    virtual void evictLRU();
    virtual void evictLFU();
    virtual void evictLFUAged();
    virtual void evictFIFO();
    virtual void evictLFF();
    virtual void evictLFFOrigin();
    virtual void evictTARA();
    virtual void evictTARAAdd();
    void updateLRU(const std::string &name);

    // Size management
    bool isFull(int dataSize) const;
    bool hasSpace(int dataSize) const;

    // Control message handlers
    virtual void handleLookupRequest(CSLookupRequest *request);
    virtual void handleInsertRequest(CSInsertRequest *request);

public:
    CS();
    virtual ~CS();

    int getMaxEntries() const { return maxEntries; }
    double getLfuAgingTau() const { return lfuAgingTau; }
    std::vector<std::string> getCachedNames() const {
        std::vector<std::string> out;
        out.reserve(entries.size());
        for (const auto &kv : entries)
            out.push_back(kv.first);
        return out;
    }

    // RL state helpers
    double getOccupancyFraction() const {
        return maxEntries > 0 ? (double)currentSize / maxEntries
                              : (maxSizeBytes > 0 ? (double)currentSizeBytes / maxSizeBytes : 0.0);
    }
    double getAvgRemainingFreshnessFraction() const {
        if (entries.empty()) return 1.0;
        double acc = 0; int n = 0;
        for (const auto &kv : entries) {
            const CSEntry *e = kv.second;
            simtime_t fp = e->data ? e->data->getFreshnessPeriod() : SIMTIME_ZERO;
            if (fp > SIMTIME_ZERO && e->expiryTime > SIMTIME_ZERO) {
                double rem = (e->expiryTime - simTime()) / fp;
                acc += rem < 0 ? 0 : (rem > 1 ? 1 : rem);
                n++;
            }
        }
        return n ? acc / n : 1.0;
    }

    // Fraction of resident entries whose remaining-freshness fraction is
    // strictly below `frac` (admission-vs-retention margin for the placement
    // agent: how many residents the candidate would out-value on freshness).
    double fractionLessFreshThan(double frac) const {
        if (entries.empty()) return 1.0;   // empty cache: candidate beats all
        int below = 0, n = 0;
        for (const auto &kv : entries) {
            const CSEntry *e = kv.second;
            simtime_t fp = e->data ? e->data->getFreshnessPeriod() : SIMTIME_ZERO;
            double rem = 0;
            if (fp > SIMTIME_ZERO && e->expiryTime > SIMTIME_ZERO) {
                rem = (e->expiryTime - simTime()) / fp;
                rem = rem < 0 ? 0 : (rem > 1 ? 1 : rem);
            }
            if (rem < frac) below++;
            n++;
        }
        return n ? (double)below / n : 1.0;
    }

    // Cache operations
    bool insert(DataPacket *data);
    DataPacket* lookup(const std::string &name);
    bool contains(const std::string &name);
    void clear();

    // Statistics
    int getSize() const { return currentSize; }
    int getCurrentSizeEntries() const { return currentSize; }
    int getTotalHits() const { return totalHits; }
    int getTotalMisses() const { return totalMisses; }
    int getTotalInsertions() const { return totalInsertions; }
    int getMaxSizeValue() const { return maxSize; }
    double getHitRatio() const {
        int total = totalHits + totalMisses;
        return total > 0 ? (double)totalHits / total : 0.0;
    }

    // DP-IDS Penalisation API (Section 4.5)
    void reduceFreshness(const std::string &name, double factor);
    void bypassVehicleEntries(const std::string &vehicleId);

    // --- Expected-utility (TARA) scoring -------------------------------
    // U(i) = lambda_i * R_i * c_i: decayed local request rate, true residual
    // life on the producer's clock, and hops a hit would save. Returned in
    // units of (expected valid hits) x (hops saved).
    double taraScore(const CSEntry *e, simtime_t now) const {
        if (!e) return 0.0;
        int h = e->hopCount > 1 ? e->hopCount : 1;
        return e->agedScore(now, lfuAgingTau) * e->trueResidual(now) * (double)h;
    }

    // Smallest resident score; negative when nothing of value would be
    // displaced (store not full, or an expired entry is swept first).
    double minTaraScore(simtime_t now) const {
        if (maxEntries <= 0 || currentSize < maxEntries)
            return -1.0;
        for (const auto &kv : entries)
            if (kv.second->isExpired()) return -1.0;
        double worst = 0.0; bool first = true;
        for (const auto &kv : entries) {
            double s = taraScore(kv.second, now);
            if (first || s < worst) { worst = s; first = false; }
        }
        return first ? -1.0 : worst;
    }

    // Equally weighted sum of the same three factors, each normalised by the
    // largest value among the residents. Returns scores in [0,1].
    double taraAddScore(const CSEntry *e, simtime_t now,
                        double maxRate, double maxRes, double maxHops) const {
        if (!e) return 0.0;
        int h = e->hopCount > 1 ? e->hopCount : 1;
        double a = maxRate > 0 ? e->agedScore(now, lfuAgingTau) / maxRate : 0.0;
        double b = maxRes  > 0 ? e->trueResidual(now) / maxRes : 0.0;
        double c = maxHops > 0 ? (double)h / maxHops : 0.0;
        return (a + b + c) / 3.0;
    }

    // Access to entries for divergence analysis
    const std::map<std::string, CSEntry*>& getEntries() const { return entries; }
};

} // namespace ndniot

#endif
