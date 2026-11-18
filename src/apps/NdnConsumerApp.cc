#include "NdnConsumerApp.h"
#include "rl/RlBridge.h"
#include "shade/RegimeController.h"
#include <climits>
#include <sstream>
#include <algorithm>

namespace ndniot {

Define_Module(NdnConsumerApp);

void NdnConsumerApp::initialize() {
    enabled = par("enabled");
    contentPrefix = par("contentPrefix").stdstringValue();
    catalogSize = par("catalogSize");
    producerCount = par("producerCount");
    validityScale = par("validityScale").doubleValue();
    if (validityScale <= 0)
        throw cRuntimeError("validityScale must be positive, got %g", validityScale);
    zipfAlpha = par("zipfAlpha");
    requestRate = par("requestRate");
    startTime = par("startTime");
    stopTime = par("stopTime");
    interestLifetime = par("interestLifetime");
    interestByteLength = par("interestByteLength");
    mustBeFresh = par("mustBeFresh");

    retrievalLatencySignal = registerSignal("retrievalLatency");
    dataHopCountSignal = registerSignal("dataHopCount");
    contentAgeSignal = registerSignal("contentAge");
    staleDeliverySignal = registerSignal("staleDelivery");
    contentAgeRatioSignal = registerSignal("contentAgeRatio");
    restampsSignal = registerSignal("restamps");
    freshDeliverySignal = registerSignal("freshDelivery");

    bridge = RlBridge::find(this);
    rlNodeId = (uint32_t)getParentModule()->par("nodeId").intValue();

    regimes = RegimeController::find(this);
    attacker = par("attacker");
    if (regimes) {
        perRegime.resize(regimes->count());
        if (attacker) {
            attackTimer = new cMessage("attackTimer");
            scheduleAt(startTime, attackTimer);
        }
    }

    if (!enabled || requestRate <= 0)
        return;

    buildZipf();
    requestTimer = new cMessage("consumerRequestTimer");
    scheduleAt(startTime, requestTimer);
}

void NdnConsumerApp::buildZipf() {
    // CDF of Zipf(alpha) over ranks 1..catalogSize
    zipfCdf.resize(catalogSize);
    double norm = 0;
    for (int k = 1; k <= catalogSize; k++)
        norm += 1.0 / std::pow(k, zipfAlpha);
    double acc = 0;
    for (int k = 1; k <= catalogSize; k++) {
        acc += (1.0 / std::pow(k, zipfAlpha)) / norm;
        zipfCdf[k - 1] = acc;
    }
}

int NdnConsumerApp::sampleZipfRank() {
    const std::vector<double> &cdf = regimes ? cdfFor(regimes->alphaNow()) : zipfCdf;
    double u = uniform(0, 1);
    auto it = std::lower_bound(cdf.begin(), cdf.end(), u);
    int rank = (int)std::distance(cdf.begin(), it) + 1;  // rank 1..catalogSize
    return rank > catalogSize ? catalogSize : rank;
}

const std::vector<double> &NdnConsumerApp::cdfFor(double alpha) {
    auto it = cdfByAlpha.find(alpha);
    if (it != cdfByAlpha.end())
        return it->second;
    std::vector<double> cdf(catalogSize);
    double norm = 0, acc = 0;
    for (int k = 1; k <= catalogSize; k++)
        norm += 1.0 / std::pow(k, alpha);
    for (int k = 1; k <= catalogSize; k++) {
        acc += (1.0 / std::pow(k, alpha)) / norm;
        cdf[k - 1] = acc;
    }
    return cdfByAlpha[alpha] = cdf;
}

long NdnConsumerApp::popularityOffset(simtime_t t) const {
    // Rank-to-item rotation: during a drift regime the whole ranking moves by
    // driftStep items every driftPeriod seconds (the hot set is replaced, not
    // reshuffled). The accumulated offset persists after the regime ends.
    if (!regimes)
        return 0;
    long off = 0;
    for (int r = 0; r < regimes->count(); r++) {
        double start = regimes->regimeStart(r);
        double end = (r + 1 < regimes->count()) ? regimes->regimeStart(r + 1) : 1e18;
        double period = regimes->driftPeriodOf(r), step = regimes->driftStepOf(r);
        if (period <= 0 || t.dbl() < start)
            continue;
        double span = std::min(t.dbl(), end) - start;
        off += (long)std::floor(span / period) * (long)step;
    }
    return off;
}

void NdnConsumerApp::issueJunk() {
    // Locality-disruption pollution: a compromised node requests names that
    // exist at the producer but are never requested twice, so every copy it
    // pulls through the path is a one-time object occupying a cache slot.
    auto *interest = new InterestPacket("Interest");
    long jc = junkCounter++;
    std::string name = producerCount > 1
        ? contentPrefix + "/n" + std::to_string((int)(jc % producerCount))
              + "/junk" + std::to_string(rlNodeId) + "-" + std::to_string(jc)
        : contentPrefix + "/junk" + std::to_string(rlNodeId) + "-" + std::to_string(jc);
    interest->setName(name.c_str());
    interest->setNonce(intuniform(0, INT_MAX));
    interest->setInterestLifetime(interestLifetime);
    interest->setTimestamp(simTime());
    interest->setByteLength(interestByteLength);
    junkIssued++;
    send(interest, "toNdn");
}

void NdnConsumerApp::handleMessage(cMessage *msg) {
    if (msg == attackTimer) {
        double r = regimes ? regimes->attackRateNow() : 0;
        if (r > 0) {
            issueJunk();
            scheduleAt(simTime() + exponential(1.0 / r), attackTimer);
        } else {
            scheduleAt(simTime() + 1.0, attackTimer);   // idle poll, no traffic
        }
        return;
    }
    if (msg == requestTimer) {
        issueInterest();
        // iCache-style dynamic environment: square-wave peak/idle request rate
        double rate = requestRate * (regimes ? regimes->rateScaleNow() : 1.0);
        double period = par("ratePeriod").doubleValue();
        double peakMult = par("peakRateMultiplier").doubleValue();
        if (period > 0 && peakMult != 1.0) {
            bool peak = fmod(simTime().dbl(), period) < period / 2;
            rate = peak ? requestRate * peakMult : requestRate;
        }
        simtime_t next = simTime() + exponential(1.0 / rate);
        if (stopTime < SIMTIME_ZERO || next < stopTime)
            scheduleAt(next, requestTimer);
        return;
    }
    if (auto *data = dynamic_cast<DataPacket *>(msg)) {
        handleData(data);
        return;
    }
    delete msg;  // NACK or unexpected
}

void NdnConsumerApp::issueInterest() {
    int rank = sampleZipfRank();
    long off = popularityOffset(simTime());
    // nameSpace >= catalogSize: with a negative drift step, never-seen names
    // enter at the top ranks and older items slide down (new-content arrival)
    long ns = std::max<long>(catalogSize, par("nameSpace").intValue());
    long item = (((rank - 1 + off) % ns) + ns) % ns + 1;
    // In a distributed sensing field the reading's publisher is part of its
    // name, so items of equal popularity sit at different path lengths.
    std::string name = producerCount > 1
        ? contentPrefix + "/n" + std::to_string((int)(item % producerCount))
              + "/item" + std::to_string(item)
        : contentPrefix + "/item" + std::to_string(item);
    double vI = par("versionInterval").doubleValue();
    if (vI > 0)   // iCache content churn: names change as data generations roll over
        name += "/v" + std::to_string((long)(simTime().dbl() / vI));

    auto *interest = new InterestPacket("Interest");
    interest->setName(name.c_str());
    interest->setNonce(intuniform(0, INT_MAX));
    interest->setInterestLifetime(interestLifetime);
    interest->setTimestamp(simTime());
    InterestSelector sel;
    sel.mustBeFresh = mustBeFresh;
    interest->setSelectors(sel);
    interest->setByteLength(interestByteLength);


    int r = regimes ? regimes->current() : 0;
    auto &lst = pending[name];
    // drop requests whose Interest lifetime has run out: they can no longer be answered
    lst.erase(std::remove_if(lst.begin(), lst.end(),
                             [&](const std::pair<simtime_t, int> &p) {
                                 return simTime() - p.first > interestLifetime; }),
              lst.end());
    lst.emplace_back(simTime(), r);
    interestsIssued++;
    if (regimes)
        perRegime[r].issued++;
    send(interest, "toNdn");
}

void NdnConsumerApp::handleData(DataPacket *data) {
    dataReceived++;
    std::string name = data->getName();

    if (name.find("/junk") != std::string::npos) {   // attacker's own traffic
        delete data;
        return;
    }
    uint32_t h = rlNameHash(name.c_str());
    // every still-alive request for this name is satisfied by this Data
    std::vector<int> regs;
    auto it = pending.find(name);
    if (it != pending.end()) {
        bool first = true;
        for (const auto &p : it->second) {
            simtime_t latency = simTime() - p.first;
            if (latency > interestLifetime)
                continue;                       // expired before the Data came back
            emit(retrievalLatencySignal, latency);
            if (regimes) {
                RegimeStats &s = perRegime[p.second];
                s.satisfied++;
                s.latencySum += latency.dbl();
                s.hopSum += data->getHopCount();
                if (data->getServedFromCache()) s.fromCache++;
                regs.push_back(p.second);
            }
            if (bridge && first)
                bridge->pushReward(rlNodeId, EV_DATA_DELIVERED, (float)latency.dbl(), h);
            first = false;
        }
        pending.erase(it);
    }
    emit(dataHopCountSignal, (long)data->getHopCount());
    if (bridge)
        bridge->pushReward(rlNodeId, EV_HOP_COUNT, (float)data->getHopCount(), h);

    // freshness bookkeeping: age vs publisher-declared freshness period
    simtime_t age = simTime() - data->getOriginTime();
    emit(contentAgeSignal, age);
    // Age as a fraction of the lifetime the publisher declared: values above
    // 1 are readings delivered after they expired. Normalising makes the
    // three freshness classes comparable in one distribution.
    // Measure against the lifetime the publisher declared, not against
    // whatever is left of it. In the clock-free realization FreshnessPeriod
    // carries the residual, so reading it here would shrink the yardstick in
    // step with the mechanism and flatter it. nominalFreshness is written once
    // at the publisher and never rewritten; the fallback keeps runs recorded
    // before this field existed comparable.
    const double nominalLife = data->getNominalFreshness() > SIMTIME_ZERO
                                   ? data->getNominalFreshness().dbl()
                                   : data->getFreshnessPeriod().dbl();
    const double appLife = nominalLife * validityScale;
    if (appLife > 0)
        emit(contentAgeRatioSignal, age.dbl() / appLife);
    emit(restampsSignal, (long)data->getRestamps());
    int rsBin = data->getRestamps();
    if (rsBin < 0) rsBin = 0;
    if (rsBin >= RESTAMP_BINS) rsBin = RESTAMP_BINS - 1;
    deliveredByRestamp[rsBin]++;
    if (appLife > 0)
        ageRatioSumByRestamp[rsBin] += age.dbl() / appLife;
    bool fresh = appLife <= 0 || age.dbl() <= appLife;
    for (int rg : regs) {
        RegimeStats &s = perRegime[rg];
        if (fresh) s.fresh++; else s.stale++;
        if (fresh && data->getServedFromCache()) s.freshFromCache++;
    }
    if (fresh) {
        freshReceived++;
        emit(freshDeliverySignal, 1L);
        if (bridge) bridge->pushReward(rlNodeId, EV_FRESH_DELIVERY, 1.0f, h);
    } else {
        staleReceived++;
        staleByRestamp[rsBin]++;
        emit(staleDeliverySignal, 1L);
        if (bridge)
            bridge->pushReward(rlNodeId, EV_STALE_DELIVERY,
                               (float)(age - data->getFreshnessPeriod()).dbl(), h);
    }

    if (data->getServedFromCache())
        remoteCacheHits++;

    delete data;
}

void NdnConsumerApp::finish() {
    recordScalar("interestsIssued", interestsIssued);
    recordScalar("dataReceived", dataReceived);
    recordScalar("satisfactionRatio",
                 interestsIssued > 0 ? (double)dataReceived / interestsIssued : 0);
    recordScalar("freshDeliveries", freshReceived);
    for (int b = 0; b < RESTAMP_BINS; b++) {
        if (deliveredByRestamp[b] <= 0)
            continue;
        std::string p = "M" + std::to_string(b) + ":";
        recordScalar((p + "delivered").c_str(), deliveredByRestamp[b]);
        recordScalar((p + "stale").c_str(), staleByRestamp[b]);
        recordScalar((p + "ageRatioSum").c_str(), ageRatioSumByRestamp[b]);
    }
    recordScalar("staleDeliveries", staleReceived);
    recordScalar("freshnessSatisfactionRatio",
                 dataReceived > 0 ? (double)freshReceived / dataReceived : 0);
    recordScalar("staleDeliveryRate",
                 dataReceived > 0 ? (double)staleReceived / dataReceived : 0);
    recordScalar("servedFromCacheCount", remoteCacheHits);
    recordScalar("junkIssued", junkIssued);
    for (size_t r = 0; r < perRegime.size(); r++) {
        const RegimeStats &s = perRegime[r];
        std::string p = "R" + std::to_string(r) + ":";
        recordScalar((p + "issued").c_str(), s.issued);
        recordScalar((p + "satisfied").c_str(), s.satisfied);
        recordScalar((p + "fresh").c_str(), s.fresh);
        recordScalar((p + "stale").c_str(), s.stale);
        recordScalar((p + "fromCache").c_str(), s.fromCache);
        recordScalar((p + "freshFromCache").c_str(), s.freshFromCache);
        recordScalar((p + "latencySum").c_str(), s.latencySum);
        recordScalar((p + "hopSum").c_str(), s.hopSum);
    }
}

} // namespace ndniot
