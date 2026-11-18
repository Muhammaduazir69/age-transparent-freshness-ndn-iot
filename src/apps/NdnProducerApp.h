//
// NDN-IoT-RL - Producer application logic (sensor content source)
//

#ifndef __NDNIOT_NDNPRODUCERAPP_H
#define __NDNIOT_NDNPRODUCERAPP_H

#include <omnetpp.h>
#include "ndn/packets/NdnPackets_m.h"
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cctype>

using namespace omnetpp;

namespace ndniot {

class NdnProducerApp : public cSimpleModule {
protected:
    bool enabled = true;
    std::string producedPrefix;
    simtime_t freshnessPeriod;
    std::vector<double> freshnessClasses;
    simtime_t ttlFor(const char *name) const;
    int payloadBytes = 32;
    int dataByteLength = 80;
    simtime_t processingDelay;

    long interestsServed = 0;
    simsignal_t interestsServedSignal;

    std::set<cMessage *> pendingTimers;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    void serveInterest(InterestPacket *interest);

public:
    virtual ~NdnProducerApp() {
        for (cMessage *timer : pendingTimers) {
            delete static_cast<DataPacket *>(timer->getContextPointer());
            cancelAndDelete(timer);
        }
    }
};

} // namespace ndniot

#endif
