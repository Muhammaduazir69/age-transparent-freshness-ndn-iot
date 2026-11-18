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
    // content index = trailing digits of the name ("/wsn/area0/item17" -> 17);
    // class = index % numClasses
    size_t len = strlen(name);
    size_t i = len;
    while (i > 0 && isdigit((unsigned char)name[i - 1])) i--;
    long idx = (i < len) ? atol(name + i) : 0;
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
