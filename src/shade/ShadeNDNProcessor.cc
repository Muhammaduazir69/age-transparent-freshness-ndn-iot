#include "ShadeNDNProcessor.h"
#include "shade/ShadeCS.h"
#include "shade/RegimeController.h"
#include "ndn/core/NdnControlMessages_m.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <sstream>

namespace ndniot {

Define_Module(ShadeNDNProcessor);

ShadeNDNProcessor::~ShadeNDNProcessor() {
    cancelAndDelete(epochTimer);
}

void ShadeNDNProcessor::parseExperts(const std::string &spec, int capacity, double tau) {
    std::istringstream in(spec);
    std::string tok;
    while (in >> tok) {
        size_t dash = tok.find('-');
        if (dash == std::string::npos)
            throw cRuntimeError("SHADE: expert '%s' must read ADMISSION-EVICTION", tok.c_str());
        std::string a = tok.substr(0, dash), e = tok.substr(dash + 1);
        Expert x;
        x.name = tok;
        if (a == "LCE") x.adm = Admission::LCE;
        else if (a == "FRESH") x.adm = Admission::FRESH;
        else if (a == "DOOR") x.adm = Admission::DOOR;
        else if (a == "QL") x.adm = Admission::QL;
        else if (a == "LCD") x.adm = Admission::LCD;
        else if (a == "PROB") x.adm = Admission::PROB;
        else if (a == "TLFU") x.adm = Admission::TLFU;
        else if (a == "TARA") x.adm = Admission::TARA;
        else if (a == "PCAST") x.adm = Admission::PCAST;
        else throw cRuntimeError("SHADE: unknown admission rule '%s'", a.c_str());
        if (e == "LRU") x.ev = Eviction::LRU;
        else if (e == "LFUA") x.ev = Eviction::LFUA;
        else if (e == "LFFO") x.ev = Eviction::LFFO;
        else if (e == "TARA") x.ev = Eviction::TARA;
        else if (e == "TARAADD") x.ev = Eviction::TARA_ADD;
        else throw cRuntimeError("SHADE: unknown eviction rule '%s'", e.c_str());
        x.ghost = GhostCache(capacity, x.ev, tau);
        experts.push_back(std::move(x));
    }
    if (experts.empty())
        throw cRuntimeError("SHADE: empty expert portfolio");
    // credit the Q-learning expert when one of its admissions leaves its ghost
    for (auto &x : experts)
        if (x.adm == Admission::QL)
            x.ghost.setLeaveHook([this](const std::string &, const GhostEntry &g) {
                if (g.qState >= 0)
                    qLearn(g.qState, 1, g.validHits - qSlotCost);
            });
}

void ShadeNDNProcessor::initialize() {
    RlNDNProcessor::initialize();

    selector = par("selector").stdstringValue();
    if (selector != "shade" && selector != "fixed" && selector != "exp3" && selector != "random" &&
        selector != "ducb" && selector != "static" && selector != "shuffle" && selector != "assigned" &&
        selector != "hedge")
        throw cRuntimeError("SHADE: unknown selector '%s'", selector.c_str());
    ducbGamma = par("ducbGamma").doubleValue();
    ducbXi = par("ducbXi").doubleValue();
    fixedExpert = par("fixedExpert");
    epoch = par("epoch");
    discount = par("discount").doubleValue();
    hysteresis = par("hysteresis").doubleValue();
    staleWeight = par("staleWeight").doubleValue();
    usePageHinkley = par("pageHinkley");
    phDelta = par("phDelta").doubleValue();
    phLambda = par("phLambda").doubleValue();
    minLookups = par("minLookups");
    exp3Gamma = par("exp3Gamma").doubleValue();
    freshFrac = par("freshFrac").doubleValue();
    doorWindow = par("doorWindow");
    doorMin = par("doorMin");
    qAlpha = par("qAlpha").doubleValue();
    qEpsilon = par("qEpsilon").doubleValue();
    qSlotCost = par("qSlotCost").doubleValue();
    qSkipHorizon = par("qSkipHorizon");
    banditHold = std::max(1, (int)par("banditHold").intValue());
    banditSkipFirst = par("banditSkipFirst");
    banditFixedScale = par("banditFixedScale");
    hedgeEta = par("hedgeEta").doubleValue();
    probAdmit = par("probAdmit").doubleValue();
    lcdHops = par("lcdDepth");
    tlfuSample = par("tlfuSample");

    cModule *node = getParentModule();
    int capacity = (int)node->par("csMaxEntries").intValue();
    double tau = 60.0;
    if (cModule *cs = node->getSubmodule("cs"))
        if (cs->hasPar("lfuAgingTau"))
            tau = cs->par("lfuAgingTau").doubleValue();
    parseExperts(par("experts").stdstringValue(), capacity, tau);
    for (auto &x : experts)
        if (x.adm == Admission::TLFU) needTlfu = true;
    for (auto &x : experts)
        if (x.adm == Admission::TARA || x.adm == Admission::PCAST) needTara = true;
    pcastWb = par("pcastWb").doubleValue();
    pcastWo = par("pcastWo").doubleValue();
    pcastWf = par("pcastWf").doubleValue();
    // TARA admission compares the candidate against the residents of the real
    // Content Store, so it is only defined when one expert drives that store.
    if (needTara && experts.size() > 1)
        throw cRuntimeError("TARA and pCASTING admission require a single-expert portfolio");
    if (fixedExpert < 0 || fixedExpert >= (int)experts.size())
        throw cRuntimeError("SHADE: fixedExpert out of range");

    shadeCs = dynamic_cast<ShadeCS *>(csModule);
    if (needTara && !shadeCs)
        throw cRuntimeError("TARA and pCASTING admission require the Content Store to be ndniot.shade.ShadeCS");
    regimes = RegimeController::find(this);
    int nr = regimes ? regimes->count() : 1;
    rLookups.assign(nr, 0); rValid.assign(nr, 0); rStale.assign(nr, 0);
    for (auto &x : experts) {
        x.rValid.assign(nr, 0); x.rStale.assign(nr, 0); x.rActiveEpochs.assign(nr, 0);
    }
    exp3W.assign(experts.size(), 1.0);
    pendingAdmit.assign(experts.size(), 1);

    activeExpertSignal = registerSignal("activeExpert");
    switchSignal = registerSignal("expertSwitch");
    phAlarmSignal = registerSignal("phAlarm");
    realUtilitySignal = registerSignal("realUtility");
    epochLookupsSignal = registerSignal("epochLookups");
    epochValidSignal = registerSignal("epochValid");
    epochStaleSignal = registerSignal("epochStale");

    ducbN.assign(experts.size(), 0.0);
    ducbS.assign(experts.size(), 0.0);
    rAlarms.assign(nr, 0);
    if (selector == "fixed")
        active = fixedExpert;
    else if (selector == "static")       // one random expert per node, for the whole run
        active = intuniform(0, (int)experts.size() - 1);
    else if (selector == "assigned") {   // one given expert per node (space-separated list by nodeId)
        std::istringstream in(par("assignment").stdstringValue());
        std::vector<int> a; int x;
        while (in >> x) a.push_back(x);
        if (nodeId < 0 || nodeId >= (int)a.size())
            throw cRuntimeError("SHADE: assignment has no entry for node %d", nodeId);
        active = a[nodeId];
    }
    else
        active = 0;
    if (enableCaching) {
        epochTimer = new cMessage("shadeEpoch");
        // the first tick installs the initial expert in the Content Store,
        // which has read its own parameters by then
        scheduleAt(simTime(), epochTimer);
    }
}

int ShadeNDNProcessor::regimeNow() const {
    return regimes ? regimes->current() : 0;
}

void ShadeNDNProcessor::handleMessage(cMessage *msg) {
    if (msg == epochTimer) {
        if (simTime() == SIMTIME_ZERO) {
            int k = active;
            active = -1;
            activate(k);
            switches = 0;          // installing the first expert is not a switch
        } else {
            endEpoch();
        }
        scheduleAt(simTime() + epoch, epochTimer);
        return;
    }
    RlNDNProcessor::handleMessage(msg);
}

int ShadeNDNProcessor::recentCount(const std::string &name, simtime_t now) {
    auto it = recent.find(name);
    if (it == recent.end())
        return 0;
    auto &dq = it->second;
    while (!dq.empty() && dq.front() < now - doorWindow)
        dq.pop_front();
    return (int)dq.size();
}

int ShadeNDNProcessor::qState(const std::string &name, const DataPacket *data, simtime_t now) {
    int f = recentCount(name, now);
    int fb = f <= 1 ? 0 : f == 2 ? 1 : f <= 4 ? 2 : 3;
    double fp = data->getFreshnessPeriod().dbl();
    double rem = fp > 0 ? (data->getOriginTime().dbl() + fp - now.dbl()) / fp : 1.0;
    rem = std::max(0.0, std::min(0.999, rem));
    int rb = (int)(rem * 4);
    return fb * 4 + rb;
}

void ShadeNDNProcessor::qLearn(int s, int a, double r) {
    if (s < 0 || s >= QL_STATES)
        return;
    qTable[s][a] += qAlpha * (r - qTable[s][a]);
    qUpdates++;
}

bool ShadeNDNProcessor::admitBy(const Expert &x, const std::string &name,
                                const DataPacket *data, simtime_t now, int &qs) {
    qs = -1;
    switch (x.adm) {
        case Admission::LCE:
            return true;
        case Admission::FRESH: {
            simtime_t fp = data->getFreshnessPeriod();
            if (fp <= SIMTIME_ZERO)
                return true;
            simtime_t remaining = data->getOriginTime() + fp - now;
            return remaining.dbl() > freshFrac * fp.dbl();
        }
        case Admission::DOOR:
            return recentCount(name, now) >= doorMin;
        case Admission::QL: {
            qs = qState(name, data, now);
            if (uniform(0, 1) < qEpsilon)
                return intuniform(0, 1) == 1;
            return qTable[qs][1] >= qTable[qs][0];
        }
        case Admission::LCD:     // Laoutaris et al.: one hop below the hit or the producer
            return data->getLcdDepth() == lcdHops;
        case Admission::PROB:    // Prob(p)
            return uniform(0, 1) < probAdmit;
        case Admission::TLFU: {  // TinyLFU: admit only if more frequent than the victim
            const std::string *v = x.ghost.peekVictim(now);
            return v == nullptr || tlfuFreq(name) > tlfuFreq(*v);
        }
        case Admission::PCAST: {
            // pCASTING (Hail et al., IEEE RIoT 2015) as described by Pfender
            // et al., ACM ICN 2018: a caching probability directly
            // proportional to the battery level, inversely proportional to the
            // cache occupancy and directly proportional to the residual
            // freshness computed from the object's time of creation. The three
            // terms are normalised to [0,1] and combined with the configured
            // relative weights; equal weights are the default, as the original
            // gives no values.
            simtime_t fp = data->getFreshnessPeriod();
            double fhat = 1.0;
            if (fp > SIMTIME_ZERO) {
                fhat = (data->getOriginTime() + fp - now).dbl() / fp.dbl();
                fhat = fhat < 0 ? 0 : (fhat > 1 ? 1 : fhat);
            }
            double bhat = residualEnergyFraction();
            double ohat = shadeCs->getOccupancyFraction();
            if (ohat < 0) ohat = 0; else if (ohat > 1) ohat = 1;
            double wsum = pcastWb + pcastWo + pcastWf;
            if (wsum <= 0) wsum = 1;
            double pc = (pcastWb * bhat + pcastWo * (1.0 - ohat) + pcastWf * fhat) / wsum;
            return uniform(0, 1) < pc;
        }
        case Admission::TARA: {
            // Admit when the candidate's expected return beats the weakest
            // resident's. The candidate enters with the same unit rate credit
            // a newly admitted entry carries, so admission and retention are
            // scored by one estimator and the rule has no free threshold.
            double worst = shadeCs->minTaraScore(now);
            if (worst < 0)
                return true;                 // room, or an expired entry goes first
            simtime_t fp = data->getFreshnessPeriod();
            double r = fp <= SIMTIME_ZERO
                       ? 1e9 : (data->getOriginTime() + fp - now).dbl();
            if (r < 0) r = 0;
            int h = data->getHopCount() > 1 ? data->getHopCount() : 1;
            return 1.0 * r * (double)h > worst;
        }
    }
    return true;
}

void ShadeNDNProcessor::observeData(const DataPacket *data, bool decideForReal) {
    simtime_t now = simTime();
    const std::string name = data->getName();
    for (size_t k = 0; k < experts.size(); k++) {
        Expert &x = experts[k];
        int qs = -1;
        bool admit = admitBy(x, name, data, now, qs);
        pendingAdmit[k] = admit ? 1 : 0;
        if (x.ghost.containsFresh(name, now))
            continue;
        if (admit) {
            x.ghost.insert(name, now, data->getFreshnessPeriod(), data->getOriginTime(),
                           qs, data->getHopCount());
        } else if (x.adm == Admission::QL) {
            auto it = skipped.find(name);
            if (it != skipped.end()) {
                qLearn(it->second.state, 0, qSlotCost - (double)it->second.missed);
                skipped.erase(it);
            }
            SkipRec r;
            r.state = qs;
            r.when = now;
            r.validUntil = data->getFreshnessPeriod() > SIMTIME_ZERO
                ? data->getOriginTime() + data->getFreshnessPeriod() : SIMTIME_MAX;
            skipped.emplace(name, r);
        }
    }
    havePending = decideForReal;
}

void ShadeNDNProcessor::processInterest(InterestPacket *interest, int inFace) {
    if (enableCaching && strncmp(interest->getName(), "/coord/", 7) != 0) {
        simtime_t now = simTime();
        const std::string name = interest->getName();
        recent[name].push_back(now);
        if (needTlfu) {
            tlfuCount[name]++;
            if (++tlfuSeen >= tlfuSample) {      // aging: halve every count
                tlfuSeen = 0;
                for (auto it = tlfuCount.begin(); it != tlfuCount.end();) {
                    it->second /= 2;
                    it = it->second == 0 ? tlfuCount.erase(it) : std::next(it);
                }
            }
        }
        int r = regimeNow();
        for (auto &x : experts) {
            GhostOutcome o = x.ghost.lookup(name, now);
            x.eLookups++;
            if (o == GHOST_VALID) { x.eValid++; x.rValid[r]++; }
            else if (o == GHOST_STALE) { x.eStale++; x.rStale[r]++; }
            else if (x.adm == Admission::QL) {
                auto it = skipped.find(name);
                if (it != skipped.end() && now < it->second.validUntil &&
                    now - it->second.when <= qSkipHorizon)
                    it->second.missed++;     // a hit the skipped copy would have served
            }
        }
    }
    RlNDNProcessor::processInterest(interest, inFace);
}

void ShadeNDNProcessor::handleCSResponse(cMessage *msg) {
    if (auto *resp = dynamic_cast<CSLookupResponse *>(msg)) {
        int r = regimeNow();
        if (enableCaching) {
            eLookups++;
            rLookups[r]++;
        }
        if (resp->getFound()) {
            const DataPacket *d = dynamic_cast<const DataPacket *>(resp->getData());
            if (d) {
                simtime_t fp = d->getFreshnessPeriod();
                bool valid = fp <= SIMTIME_ZERO || simTime() - d->getOriginTime() <= fp;
                if (valid) { eValid++; rValid[r]++; }
                else       { eStale++; rStale[r]++; }
                // the experts whose ghosts missed see the copy this cache served
                observeData(d, false);
                havePending = false;
            }
        }
    }
    RlNDNProcessor::handleCSResponse(msg);
}

void ShadeNDNProcessor::processData(DataPacket *data, int inFace) {
    if (enableCaching)
        observeData(data, true);
    RlNDNProcessor::processData(data, inFace);
    havePending = false;
}

bool ShadeNDNProcessor::shouldCacheData(DataPacket *data) {
    if (!NDNProcessor::shouldCacheData(data))
        return false;
    admissionsOffered++;
    if (residualEnergyFraction() < energyGuardFraction) {
        energyGuardVetoes++;
        return false;
    }
    bool admit = havePending ? pendingAdmit[active] != 0 : true;
    if (admit)
        admissionsAccepted++;
    return admit;
}

bool ShadeNDNProcessor::pageHinkley(double x) {
    phCount++;
    phMean += (x - phMean) / phCount;
    phCumUp += x - phMean - phDelta;
    phMinUp = std::min(phMinUp, phCumUp);
    phCumDown += x - phMean + phDelta;
    phMaxDown = std::max(phMaxDown, phCumDown);
    bool alarm = (phCumUp - phMinUp > phLambda) || (phMaxDown - phCumDown > phLambda);
    if (alarm) {
        phMean = phCumUp = phMinUp = phCumDown = phMaxDown = 0;
        phCount = 0;
    }
    return alarm;
}

void ShadeNDNProcessor::activate(int k) {
    if (k == active)
        return;
    if (active >= 0) {
        switches++;
        emit(switchSignal, 1L);
    }
    epochsSinceSwitch = 0;
    active = k;
    if (shadeCs) {
        switch (experts[k].ev) {
            case Eviction::LRU:  shadeCs->setEviction(ReplacementPolicy::LRU); break;
            case Eviction::LFUA: shadeCs->setEviction(ReplacementPolicy::LFU_AGED); break;
            case Eviction::LFFO: shadeCs->setEviction(ReplacementPolicy::LFF_ORIGIN); break;
            case Eviction::TARA: shadeCs->setEviction(ReplacementPolicy::TARA); break;
            case Eviction::TARA_ADD: shadeCs->setEviction(ReplacementPolicy::TARA_ADD); break;
        }
    } else if (experts.size() > 1 || experts[k].ev != Eviction::LRU) {
        throw cRuntimeError("SHADE: the Content Store must be ndniot.shade.ShadeCS");
    }
    // the change detector watches a different stream from now on
    phMean = phCumUp = phMinUp = phCumDown = phMaxDown = 0;
    phCount = 0;
}

int ShadeNDNProcessor::tlfuFreq(const std::string &name) const {
    auto it = tlfuCount.find(name);
    return it == tlfuCount.end() ? 0 : it->second;
}

double ShadeNDNProcessor::scaledReward(double u) {
    if (banditFixedScale)      // utility lies in [-beta, 1]
        return (u + staleWeight) / (1.0 + staleWeight);
    // bandit rewards rescaled to [0,1] by the node's own observed utility range,
    // so small but real differences between experts are not compressed away
    uMin = std::min(uMin, u);
    uMax = std::max(uMax, u);
    double span = uMax - uMin;
    return span > 1e-12 ? (u - uMin) / span : 0.5;
}

void ShadeNDNProcessor::endEpoch() {
    epochsRun++;
    simtime_t now = simTime();
    int r = regimeNow();
    experts[active].rActiveEpochs[r]++;
    const size_t K = experts.size();
    long L = experts[0].eLookups;

    if (L >= minLookups) {
        for (auto &x : experts) {
            x.lastUtility = (x.eValid - staleWeight * x.eStale) / (double)L;
            x.score = discount * x.score + x.lastUtility;
        }
        scoreWeight = discount * scoreWeight + 1.0;   // W: discounted number of epochs
        double uReal = (eValid - staleWeight * eStale) / (double)std::max(1L, eLookups);
        emit(realUtilitySignal, uReal);

        // how far the real store is from the active expert's ghost, by the
        // number of epochs since the last switch (transition cost of a switch)
        if (switches > 0) {   // exclude the warm-up before the node's first real switch
            int bucket = (int)std::min<long>(epochsSinceSwitch, DIV_BUCKETS - 1);
            divSum[bucket] += std::fabs(uReal - experts[active].lastUtility);
            divSigned[bucket] += uReal - experts[active].lastUtility;
            divCount[bucket]++;
        }

        if (selector == "shade" || selector == "shuffle") {
            // under "shuffle", 'lead' is a shadow copy of SHADE's own decision, so the
            // switch times are exactly SHADE's while the real store goes elsewhere
            int &lead = (selector == "shuffle") ? shadowActive : active;
            if (lead < 0) lead = active;
            if (usePageHinkley && pageHinkley(experts[lead].lastUtility)) {
                phAlarms++;
                rAlarms[r]++;
                emit(phAlarmSignal, 1L);
                for (auto &x : experts)
                    x.score = x.lastUtility;     // forget the previous regime
                scoreWeight = 1.0;
            }
            int best = lead;
            for (size_t k = 0; k < K; k++)
                if (experts[k].score > experts[best].score)
                    best = (int)k;
            // G_k / W is a discounted mean utility, so the gap is on the scale
            // of one epoch's utility right after a reset as well as later on
            double gap = (experts[best].score - experts[lead].score) / scoreWeight;
            if (best != lead && gap > hysteresis) {
                if (selector == "shuffle") {
                    // control: SHADE's shadow decision moves to the leader, the real
                    // store moves to a uniformly random other expert
                    shadowActive = best;
                    phMean = phCumUp = phMinUp = phCumDown = phMaxDown = 0;
                    phCount = 0;
                    int k = intuniform(0, (int)K - 2);
                    activate(k >= active ? k + 1 : k);
                } else {
                    activate(best);
                }
            }
        }
        else if (selector == "exp3" || selector == "ducb" || selector == "hedge") {
            // bandit feedback: only the active expert's real utility is seen; a pull
            // is held for banditHold epochs, optionally without rewarding its first
            if (!(banditSkipFirst && banditHold > 1 && pullEpochs == 0)) {
                pullSum += uReal;
                pullN++;
            }
        }
    }
    bool pullEnds = false;
    // like SHADE's scores, a pull advances only in epochs with lookups, so no
    // selector redraws on an epoch in which it observed nothing
    if ((selector == "exp3" || selector == "ducb" || selector == "hedge") && L >= minLookups) {
        pullEpochs++;
        pullEnds = pullEpochs >= banditHold;
    }
    if (pullEnds && pullN > 0 && selector == "exp3") {
        double p = (1 - exp3Gamma) * exp3W[active] /
                       std::accumulate(exp3W.begin(), exp3W.end(), 0.0) +
                   exp3Gamma / K;
        double reward = scaledReward(pullSum / pullN);
        exp3W[active] *= std::exp(exp3Gamma * reward / (p * K));
        double mx = *std::max_element(exp3W.begin(), exp3W.end());
        for (auto &w : exp3W) w /= mx;
    }
    else if (pullEnds && pullN > 0 && selector == "ducb") {
        // discounted UCB (Garivier and Moulines 2011), one discount step per pull
        double reward = scaledReward(pullSum / pullN);
        for (size_t k = 0; k < K; k++) { ducbN[k] *= ducbGamma; ducbS[k] *= ducbGamma; }
        ducbN[active] += 1.0;
        ducbS[active] += reward;
    }
    if (pullEnds) { pullEpochs = 0; pullN = 0; pullSum = 0; }
    if (pullEnds && selector == "exp3") {
        double sum = std::accumulate(exp3W.begin(), exp3W.end(), 0.0);
        double u = uniform(0, 1), acc = 0;
        int pick = (int)K - 1;
        for (size_t k = 0; k < K; k++) {
            acc += (1 - exp3Gamma) * exp3W[k] / sum + exp3Gamma / K;
            if (u < acc) { pick = (int)k; break; }
        }
        activate(pick);
    }
    else if (pullEnds && selector == "ducb") {
        double n = std::accumulate(ducbN.begin(), ducbN.end(), 0.0);
        int pick = -1;
        double bestIdx = -1e18;
        for (size_t k = 0; k < K; k++) {
            if (ducbN[k] < 1e-9) { pick = (int)k; break; }   // untried arm first
            double idx = ducbS[k] / ducbN[k] +
                         2.0 * std::sqrt(ducbXi * std::log(std::max(n, 1.0)) / ducbN[k]);
            if (idx > bestIdx) { bestIdx = idx; pick = (int)k; }
        }
        activate(pick);
    }
    else if (pullEnds && selector == "hedge") {
        // discounted Hedge on the ghost utilities: full information, sampled choice
        if (scoreWeight > 0) {
            double mx = -1e18;
            for (auto &x : experts) mx = std::max(mx, x.score / scoreWeight);
            std::vector<double> w(K);
            double sum = 0;
            for (size_t k = 0; k < K; k++) {
                w[k] = std::exp(hedgeEta * (experts[k].score / scoreWeight - mx));
                sum += w[k];
            }
            double u = uniform(0, 1) * sum, acc = 0;
            int pick = (int)K - 1;
            for (size_t k = 0; k < K; k++) { acc += w[k]; if (u < acc) { pick = (int)k; break; } }
            activate(pick);
        }
    }
    else if (selector == "random") {
        activate(intuniform(0, (int)K - 1));
    }
    epochsSinceSwitch++;
    emit(activeExpertSignal, (long)active);

    // record-only time series of the real store (no effect on behavior)
    emit(epochLookupsSignal, eLookups);
    emit(epochValidSignal, eValid);
    emit(epochStaleSignal, eStale);

    // reset epoch counters, prune the request log, settle expired skip records
    for (auto &x : experts) x.eLookups = x.eValid = x.eStale = 0;
    eLookups = eValid = eStale = 0;
    long logEntries = 0;
    for (auto it = recent.begin(); it != recent.end();) {
        auto &dq = it->second;
        while (!dq.empty() && dq.front() < now - doorWindow) dq.pop_front();
        logEntries += (long)dq.size();
        it = dq.empty() ? recent.erase(it) : std::next(it);
    }
    peakLogNames = std::max(peakLogNames, (long)recent.size());
    peakLogEntries = std::max(peakLogEntries, logEntries);
    for (auto it = skipped.begin(); it != skipped.end();) {
        if (now - it->second.when > qSkipHorizon) {
            // skipping saved the slot (kappa) and lost the hits it would have served
            qLearn(it->second.state, 0, qSlotCost - (double)it->second.missed);
            it = skipped.erase(it);
        } else {
            ++it;
        }
    }
    peakSkipped = std::max(peakSkipped, (long)skipped.size());
}

void ShadeNDNProcessor::finish() {
    RlNDNProcessor::finish();
    if (!enableCaching)
        return;
    recordScalar("shadeSwitches", switches);
    recordScalar("shadePhAlarms", phAlarms);
    recordScalar("shadeEpochs", epochsRun);
    recordScalar("shadeFinalActive", active);
    recordScalar("qUpdates", qUpdates);
    recordScalar("peakDoorLogNames", peakLogNames);
    recordScalar("peakDoorLogEntries", peakLogEntries);
    recordScalar("peakSkipRecords", peakSkipped);
    for (int b = 0; b < DIV_BUCKETS; b++) {
        recordScalar(("divSum" + std::to_string(b)).c_str(), divSum[b]);
        recordScalar(("divSigned" + std::to_string(b)).c_str(), divSigned[b]);
        recordScalar(("divCount" + std::to_string(b)).c_str(), (double)divCount[b]);
    }
    // learned admission preference Q(s,admit) - Q(s,skip) per state
    for (int s = 0; s < QL_STATES; s++)
        recordScalar(("qAdv" + std::to_string(s)).c_str(), qTable[s][1] - qTable[s][0]);
    long ops = 0;
    for (auto &x : experts) ops += x.ghost.operations();
    recordScalar("ghostOperations", ops);
    recordScalar("ghostEntriesCapacity", (long)experts.size() * experts[0].ghost.capacity());
    for (size_t r = 0; r < rLookups.size(); r++) {
        std::string p = "R" + std::to_string(r) + ":";
        recordScalar((p + "lookups").c_str(), rLookups[r]);
        recordScalar((p + "validHits").c_str(), rValid[r]);
        recordScalar((p + "staleHits").c_str(), rStale[r]);
        recordScalar((p + "phAlarms").c_str(), rAlarms[r]);
        for (size_t k = 0; k < experts.size(); k++) {
            std::string q = p + "E" + std::to_string(k) + ":";
            recordScalar((q + "ghostValid").c_str(), experts[k].rValid[r]);
            recordScalar((q + "ghostStale").c_str(), experts[k].rStale[r]);
            recordScalar((q + "activeEpochs").c_str(), experts[k].rActiveEpochs[r]);
        }
    }
}

} // namespace ndniot
