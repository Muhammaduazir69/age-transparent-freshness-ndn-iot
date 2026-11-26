#include "NdnProducerApp.h"
#include "shade/RegimeController.h"

namespace ndniot {

Define_Module(NdnProducerApp);

void NdnProducerApp::initialize() {
    enabled = par("enabled");
    producedPrefix = par("producedPrefix").stdstringValue();
    freshnessPeriod = par("freshnessPeriod");
    {   // parse the optional heterogeneous TTL class list
        std::string spec = par("freshnessClasses").stdstringValue();
        size_t pos = 0;
        while (pos < spec.size()) {
            size_t comma = spec.find(',', pos);
            std::string tok = spec.substr(pos, comma == std::string::npos
                                                   ? std::string::npos
                                                   : comma - pos);
            if (!tok.empty())
                freshnessClasses.push_back(atof(tok.c_str()));
            if (comma == std::string::npos) break;
            pos = comma + 1;
        }
    }
    payloadBytes = par("payloadBytes");
    dataByteLength = par("dataByteLength");
    processingDelay = par("processingDelay");
    interestsServedSignal = registerSignal("interestsServed");
}

void NdnProducerApp::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        // delayed response carrying the prepared Data packet
        DataPacket *data = static_cast<DataPacket *>(msg->getContextPointer());
        pendingTimers.erase(msg);
        delete msg;
        send(data, "toNdn");
        return;
    }
    if (auto *interest = dynamic_cast<InterestPacket *>(msg)) {
        if (enabled && !producedPrefix.empty() &&
            strncmp(interest->getName(), producedPrefix.c_str(), producedPrefix.size()) == 0) {
            serveInterest(interest);
        }
        delete interest;
        return;
    }
    delete msg;  // Data/NACK arriving at producer app: ignore
}

simtime_t NdnProducerApp::ttlFor(const char *name) const {
    if (freshnessClasses.empty())
        return freshnessPeriod;
    // Content index, which fixes the content's freshness class. The index is
    // the number following the last "item" component of the name, so a name
    // that carries a trailing version component still maps to the item it
    // names: "/campus/env/item17" and "/campus/env/item17/v5" are both item
    // 17. Names with no "item" component, such as the compromised sensors'
    // junk, fall back to the trailing digits, which is what every name
    // reported before the versioned arm was added resolved to anyway.
    size_t len = strlen(name);
    long idx = 0;
    const char *last = nullptr;
    for (const char *q = name; (q = strstr(q, "item")) != nullptr; q += 4)
        last = q;
    if (last != nullptr && isdigit((unsigned char)last[4])) {
        idx = atol(last + 4);
    } else {
        size_t i = len;
        while (i > 0 && isdigit((unsigned char)name[i - 1])) i--;
        idx = (i < len) ? atol(name + i) : 0;
    }
    size_t k = (size_t)((idx % (long)freshnessClasses.size() +
                         (long)freshnessClasses.size()) %
                        (long)freshnessClasses.size());
    return SimTime(freshnessClasses[k]);
}

void NdnProducerApp::serveInterest(InterestPacket *interest) {
    auto *data = new DataPacket("Data");
    data->setName(interest->getName());

    // fresh sensor reading generated on demand
    std::string content(payloadBytes, 'x');
    data->setContent(content.c_str());
    data->setContentLength(payloadBytes);
    data->setByteLength(dataByteLength);
    data->setTimestamp(simTime());
    data->setOriginTime(simTime());
    simtime_t ttl = ttlFor(interest->getName());
    if (auto *rc = RegimeController::find(this))
        ttl = ttl * rc->freshScaleNow();
    data->setFreshnessPeriod(ttl);
    data->setNominalFreshness(ttl);
    data->setIsCacheable(true);
    data->setContentType("sensor");
    data->setLcdDepth(0);   // LCD: producer is the serving point

    interestsServed++;
    emit(interestsServedSignal, 1L);

    if (processingDelay > SIMTIME_ZERO) {
        auto *timer = new cMessage("produceDelay");
        timer->setContextPointer(data);
        pendingTimers.insert(timer);
        scheduleAt(simTime() + processingDelay, timer);
    } else {
        send(data, "toNdn");
    }
}

void NdnProducerApp::finish() {
    recordScalar("interestsServedTotal", interestsServed);
}

} // namespace ndniot
