#include "NdnTestDriver.h"
#include <climits>

namespace ndniot {

Define_Module(NdnTestDriver);

void NdnTestDriver::initialize() {
    role = par("role").stdstringValue();
    prefix = par("prefix").stdstringValue();
    requestInterval = par("requestInterval");
    numRequests = par("numRequests");
    freshnessPeriod = par("freshnessPeriod");

    latencyVector.setName("retrievalLatency");
    retrievalLatencySignal = registerSignal("retrievalLatency");

    if (role == "consumer") {
        requestTimer = new cMessage("requestTimer");
        scheduleAt(simTime() + par("startTime").doubleValue(), requestTimer);
    }
}

void NdnTestDriver::handleMessage(cMessage *msg) {
    if (msg == requestTimer) {
        sendNextInterest();
        if (requestsSent < numRequests)
            scheduleAt(simTime() + requestInterval, requestTimer);
        return;
    }
    if (auto *data = dynamic_cast<DataPacket *>(msg)) {
        handleData(data);
        return;
    }
    if (auto *interest = dynamic_cast<InterestPacket *>(msg)) {
        handleInterest(interest);
        return;
    }
    delete msg; // NACKs and anything else
}

void NdnTestDriver::sendNextInterest() {
    // Same content name every time so the 2nd+ request can hit caches.
    std::string name = prefix + "/reading";
    auto *interest = new InterestPacket("Interest");
    interest->setName(name.c_str());
    interest->setNonce(intuniform(0, INT_MAX));
    interest->setTimestamp(simTime());
    InterestSelector sel;
    sel.mustBeFresh = false;
    interest->setSelectors(sel);
    interest->setByteLength(40);

    pendingSince[name] = simTime();
    requestsSent++;
    send(interest, "toNdn");
    EV_INFO << "Driver consumer sent Interest #" << requestsSent << " for " << name << endl;
}

void NdnTestDriver::handleData(DataPacket *data) {
    dataReceived++;
    auto it = pendingSince.find(data->getName());
    if (it != pendingSince.end()) {
        simtime_t latency = simTime() - it->second;
        latencyVector.record(latency);
        emit(retrievalLatencySignal, latency);
        pendingSince.erase(it);
    }
    EV_INFO << "Driver consumer got Data " << data->getName()
            << " (cached copy: " << (data->getServedFromCache() ? "yes" : "no") << ")" << endl;
    delete data;
}

void NdnTestDriver::handleInterest(InterestPacket *interest) {
    if (role != "producer") { delete interest; return; }
    auto *data = new DataPacket("Data");
    data->setName(interest->getName());
    data->setContent("42.0");
    data->setContentLength(8);
    data->setByteLength(64);
    data->setTimestamp(simTime());
    data->setOriginTime(simTime());
    data->setFreshnessPeriod(freshnessPeriod);
    data->setIsCacheable(true);
    delete interest;
    send(data, "toNdn");
    EV_INFO << "Driver producer answered " << data->getName() << endl;
}

void NdnTestDriver::finish() {
    if (role == "consumer") {
        recordScalar("requestsSent", requestsSent);
        recordScalar("dataReceived", dataReceived);
        recordScalar("satisfactionRatio", requestsSent > 0 ? (double)dataReceived / requestsSent : 0.0);
    }
}

} // namespace ndniot
