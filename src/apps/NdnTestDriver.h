//
// NDN-IoT-RL - Standalone test driver (M1 smoke test only)
// Attaches directly to an NdnNode face; consumer issues Interests,
// producer answers with Data. INET-free.
//

#ifndef __NDNIOT_NDNTESTDRIVER_H
#define __NDNIOT_NDNTESTDRIVER_H

#include <omnetpp.h>
#include "ndn/packets/NdnPackets_m.h"

using namespace omnetpp;

namespace ndniot {

class NdnTestDriver : public cSimpleModule {
protected:
    std::string role;          // "consumer" | "producer"
    std::string prefix;        // name prefix requested/served
    simtime_t requestInterval;
    int numRequests;
    simtime_t freshnessPeriod;

    cMessage *requestTimer = nullptr;
    int requestsSent = 0;
    int dataReceived = 0;
    std::map<std::string, simtime_t> pendingSince;
    cOutVector latencyVector;
    simsignal_t retrievalLatencySignal;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    void sendNextInterest();
    void handleData(DataPacket *data);
    void handleInterest(InterestPacket *interest);

public:
    virtual ~NdnTestDriver() { cancelAndDelete(requestTimer); }
};

} // namespace ndniot

#endif
