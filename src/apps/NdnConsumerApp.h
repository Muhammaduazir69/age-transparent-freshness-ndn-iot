//
// NDN-IoT-RL - Consumer application logic (Zipf request generator + metrics)
//

#ifndef __NDNIOT_NDNCONSUMERAPP_H
#define __NDNIOT_NDNCONSUMERAPP_H

#include <omnetpp.h>
#include "ndn/packets/NdnPackets_m.h"

using namespace omnetpp;

namespace ndniot {

class RlBridge;
class RegimeController;

class NdnConsumerApp : public cSimpleModule {
protected:
    bool enabled = true;
    std::string contentPrefix;
    int producerCount = 0;
    double validityScale = 1.0;
    int catalogSize = 50;
    double zipfAlpha = 0.8;
    double requestRate = 1.0;
    simtime_t startTime, stopTime, interestLifetime;
    int interestByteLength = 40;
    bool mustBeFresh = false;

    cMessage *requestTimer = nullptr;
    RlBridge *bridge = nullptr;
    uint32_t rlNodeId = 0;
    std::vector<double> zipfCdf;

    // ---- SHADE regime timeline (optional; absent = original behavior) ----
    RegimeController *regimes = nullptr;
    std::map<double, std::vector<double>> cdfByAlpha;
    bool attacker = false;
    cMessage *attackTimer = nullptr;
    long junkIssued = 0, junkCounter = 0;
    struct RegimeStats {
        long issued = 0, satisfied = 0, fresh = 0, stale = 0, fromCache = 0, freshFromCache = 0;
        double latencySum = 0, hopSum = 0;
    };
    std::vector<RegimeStats> perRegime;
    const std::vector<double> &cdfFor(double alpha);
    long popularityOffset(simtime_t t) const;
    void issueJunk();

    // pending Interests: name -> (issue time, regime) of every outstanding
    // request; a returning Data satisfies all of them that are still alive,
    // as PIT aggregation at the consumer's own forwarder does
    std::map<std::string, std::vector<std::pair<simtime_t, int>>> pending;

    long interestsIssued = 0, dataReceived = 0, staleReceived = 0, freshReceived = 0;
    // Deliveries and stale deliveries by the number of Content Stores that
    // handed the copy on (the re-stamp count M of the amplification model).
    // Index RESTAMP_BINS-1 collects everything at or above that count.
    static const int RESTAMP_BINS = 8;
    long deliveredByRestamp[RESTAMP_BINS] = {};
    long staleByRestamp[RESTAMP_BINS] = {};
    double ageRatioSumByRestamp[RESTAMP_BINS] = {};
    long remoteCacheHits = 0;

    simsignal_t retrievalLatencySignal, dataHopCountSignal, contentAgeSignal,
                staleDeliverySignal, freshDeliverySignal,
                contentAgeRatioSignal, restampsSignal;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    void buildZipf();
    int sampleZipfRank();
    void issueInterest();
    void handleData(DataPacket *data);

public:
    virtual ~NdnConsumerApp() { cancelAndDelete(requestTimer); cancelAndDelete(attackTimer); }
};

} // namespace ndniot

#endif
