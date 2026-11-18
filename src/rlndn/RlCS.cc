#include "RlCS.h"
#include <cmath>
#include "rl/RlBridge.h"
#include "rlndn/RlNDNProcessor.h"

namespace ndniot {

Define_Module(RlCS);

void RlCS::initialize() {
    CS::initialize();
    useRl = par("replacementPolicy").stdstringValue() == "RL";
    admitVsRetain = par("admitVsRetain");
    replacementMacsPerItem = par("replacementMacsPerItem").doubleValue();
    replacementMacsFixed = par("replacementMacsFixed").doubleValue();
    admissionGrace = par("admissionGrace");
    bridge = RlBridge::find(this);
    if (bridge && useRl) {
        rlAgentId = bridge->registerAgent(getFullPath().c_str(), AGENT_A2C);
        cModule *ndnNode = getParentModule();
        nodeId = (uint32_t)ndnNode->par("nodeId").intValue();
        processor = dynamic_cast<RlNDNProcessor *>(ndnNode->getSubmodule("processor"));
    }
}

bool RlCS::hasExpiredEntry() const {
    for (const auto &kv : entries)
        if (kv.second->isExpired())
            return true;
    return false;
}

double RlCS::remainingFreshFrac(const DataPacket *d) const {
    if (!d || d->getFreshnessPeriod() <= SIMTIME_ZERO)
        return 0.0;
    simtime_t origin = d->getOriginTime() > SIMTIME_ZERO ? d->getOriginTime() : simTime();
    double rem = (origin + d->getFreshnessPeriod() - simTime()) / d->getFreshnessPeriod();
    return rem < 0 ? 0 : (rem > 1 ? 1 : rem);
}

double RlCS::worstResidentValue() const {
    // retention value proxy (thesis 3.5.4 tier 3): forecasted usefulness =
    // blend of remaining freshness and observed access rate
    double worst = 1e9;
    simtime_t now = simTime();
    for (const auto &kv : entries) {
        const CSEntry *e = kv.second;
        double rem = remainingFreshFrac(e->data);
        double age = (now - e->insertionTime).dbl();
        double rate = age > 1 ? e->accessCount / age : e->accessCount;
        double v = 0.5 * rem + 0.5 * std::min(1.0, rate * 10.0);
        worst = std::min(worst, v);
    }
    return worst == 1e9 ? 0.0 : worst;
}

void RlCS::handleInsertRequest(CSInsertRequest *request) {
    // Admit-vs-retain conflict resolution (thesis 3.5.4): when the cache is
    // full of fresh entries, admit only if the newcomer's value beats the
    // least valuable resident; otherwise refuse admission (Data still
    // forwarded, just not cached here).
    if (useRl && admitVsRetain) {
        const DataPacket *d = dynamic_cast<const DataPacket *>(request->getData());
        if (d && !hasSpace(d->getContentLength()) && currentSize > 0 &&
            !hasExpiredEntry()) {
            double vNew = remainingFreshFrac(d);
            double vWorst = worstResidentValue();
            if (vNew <= vWorst) {
                admitVetoes++;
                auto *resp = new CSInsertResponse();
                resp->setName(request->getName());
                resp->setTransactionId(request->getTransactionId());
                resp->setSuccess(false);
                send(resp, "processorOut");
                delete request;  // @owned data deleted with the request
                return;
            }
        }
    }
    CS::handleInsertRequest(request);
}

bool RlCS::evictExpiredFirst() {
    // Tier 1 of the conflict-resolution ladder: expired entries are worthless,
    // evict them without consuming an agent decision. The baseline CS now
    // performs the same sweep (CS::sweepExpiredOnce), so this tier is no
    // longer an advantage the learned model holds over the comparators; it
    // honours the same `expiredFirst` switch.
    if (!expiredFirst)
        return false;
    for (const auto &kv : entries) {
        if (kv.second->isExpired()) {
            EV_INFO << "Evicting (tiered, expired): " << kv.first << endl;
            noteEviction(kv.second, simTime());
            if (bridge)
                bridge->pushReward(nodeId, EV_CS_EVICT, 0.0f, rlNameHash(kv.first.c_str()));
            removeEntry(kv.first);
            tieredEvictions++;
            // was missing: tier-1 evictions never reached totalEvictions, so
            // every eviction-derived rate (zeroHitEvictionRate above all) was
            // computed against a denominator that omitted ~a third of them
            totalEvictions++;
            expiredFirstEvictions++;
            emit(csEvictionEventSignal, 1L);
            return true;
        }
    }
    return false;
}

void RlCS::noteEviction(const CSEntry *e, simtime_t now) {
    evictedCount++;
    evictedAgeSum += (now - e->insertionTime).dbl();
    if (e->accessCount == 0)
        zeroHitEvictions++;
}

void RlCS::buildItemFeatures(std::vector<const CSEntry *> &order, float *items) {
    simtime_t now = simTime();
    // Admission grace (tier-2 refinement): entries still inside their grace
    // window are not offered to the agent as eviction candidates, so an
    // admission always gets a minimum chance to serve. If every resident is
    // protected the shield is dropped (the cache must still make room).
    bool anyEligible = false;
    if (admissionGrace > SIMTIME_ZERO)
        for (const auto &kv : entries)
            if (!graceProtected(kv.second, now)) { anyEligible = true; break; }
    const bool useGrace = admissionGrace > SIMTIME_ZERO && anyEligible;

    uint32_t row = 0;
    for (const auto &kv : entries) {
        if (row >= RL_MAX_ITEMS)
            break;
        const CSEntry *e = kv.second;
        if (useGrace && graceProtected(e, now)) { graceProtections++; continue; }
        order.push_back(e);
        float *f = items + row * RL_ITEM_FEAT;

        simtime_t fp = e->data ? e->data->getFreshnessPeriod() : SIMTIME_ZERO;
        double remFrac = 0.0;
        if (fp > SIMTIME_ZERO && e->expiryTime > SIMTIME_ZERO) {
            remFrac = (e->expiryTime - now) / fp;
            remFrac = remFrac < 0 ? 0 : (remFrac > 1 ? 1 : remFrac);
        }
        double ageInCache = (now - e->insertionTime).dbl();
        double accessRate = ageInCache > 1 ? e->accessCount / ageInCache : e->accessCount;

        f[0] = (float)remFrac;                                        // remaining freshness
        f[1] = (float)std::min(1.0, accessRate * 10.0);               // access rate
        f[2] = (float)std::min(1.0, (now - e->lastAccess).dbl() / 60.0); // time since access
        f[3] = (float)std::min(1.0, e->size / 128.0);                 // size
        f[4] = (float)std::min(1.0, e->hopCount / 10.0);              // re-fetch cost proxy
        f[5] = (float)std::min(1.0, e->accessCount / 10.0);           // lifetime hits
        f[6] = (float)std::min(1.0, ageInCache / 600.0);              // age in cache
        // aged frequency: the exponentially decayed access score the aged-LFU
        // evictor ranks on, log-compressed into [0, 1] (a score of 20 saturates)
        if constexpr (RL_ITEM_FEAT > 8)
            f[8] = (float)std::min(1.0, std::log1p(e->agedScore(now, lfuAgingTau)) / std::log(21.0));
        f[7] = (float)remainingFreshFrac(e->data);   // application-level remaining life
                                                     // (origin-based; f[0] is
                                                     // the per-hop-reset NDN TTL)
        row++;
    }
}

void RlCS::evictEntry() {
    if (!useRl) {
        CS::evictEntry();
        return;
    }
    if (entries.empty())
        return;

    // tier 1: expired first
    if (evictExpiredFirst())
        return;

    // tier 2: A2C victim selection
    if (bridge && bridge->isActive() && rlAgentId >= 0) {
        static std::vector<float> itemBuf(RL_MAX_ITEMS * RL_ITEM_FEAT);
        float nodeState[RL_STATE_DIM] = {0};
        nodeState[7] = getOccupancyFraction();
        nodeState[8] = getHitRatio();
        nodeState[9] = processor ? (float)processor->getEnergyFraction() : 1.0f;
        nodeState[11] = getAvgRemainingFreshnessFraction();
        nodeState[16] = processor ? (float)processor->capacityFeature() : 0.0f;

        std::vector<const CSEntry *> order;
        order.reserve(entries.size());
        buildItemFeatures(order, itemBuf.data());
        uint32_t n = (uint32_t)order.size();

        // Replacement-network inference cost, charged through the processor so
        // both policy networks share one compute pipeline and one battery:
        //   actor  (8+16)x128 + 128x128 + 128x1 = 19 584 MAC per candidate item
        //   critic (2*128+16)x128 + 128x1       = 34 944 MAC per decision
        if (processor)
            processor->chargeInferenceExternal(replacementMacsPerItem * n
                                               + replacementMacsFixed);
        int victim = bridge->decide(rlAgentId, AGENT_A2C, nodeId,
                                    nodeState, itemBuf.data(), n);
        if (victim >= 0 && (uint32_t)victim < n) {
            const CSEntry *e = order[victim];
            simtime_t fp = e->data ? e->data->getFreshnessPeriod() : SIMTIME_ZERO;
            float remFrac = (fp > SIMTIME_ZERO && e->expiryTime > SIMTIME_ZERO)
                ? (float)std::max(0.0, std::min(1.0, (e->expiryTime - simTime()) / fp))
                : 0.0f;
            EV_INFO << "Evicting (A2C): " << e->name << endl;
            noteEviction(e, simTime());
            bridge->pushReward(nodeId, EV_CS_EVICT, remFrac, rlNameHash(e->name.c_str()));
            if (processor)   // EvictNotify: replacement -> placement feedback
                processor->onEvictNotify(e->name, e->accessCount,
                                         (simTime() - e->insertionTime).dbl());
            removeEntry(e->name);
            rlEvictions++;
            totalEvictions++;
            emit(csEvictionEventSignal, 1L);
            return;
        }
    }

    // tier 3: fallback LRU (grace-aware)
    fallbackEvictions++;
    simtime_t now = simTime();
    if (admissionGrace > SIMTIME_ZERO) {
        const CSEntry *victim = nullptr;
        for (const auto &kv : entries) {
            const CSEntry *e = kv.second;
            if (graceProtected(e, now))
                continue;
            if (!victim || e->lastAccess < victim->lastAccess)
                victim = e;
        }
        if (victim) {
            noteEviction(victim, now);
            std::string name = victim->name;
            removeEntry(name);
            totalEvictions++;
            emit(csEvictionEventSignal, 1L);
            return;
        }
    }
    evictLRU();
}

void RlCS::finish() {
    CS::finish();
    recordScalar("admitVetoes", admitVetoes);
    recordScalar("rlEvictions", rlEvictions);
    recordScalar("tieredEvictions", tieredEvictions);
    recordScalar("fallbackEvictions", fallbackEvictions);
    recordScalar("graceProtections", graceProtections);
    recordScalar("zeroHitEvictions", zeroHitEvictions);
    recordScalar("evictedMeanAge", evictedCount ? evictedAgeSum / evictedCount : 0.0);
    recordScalar("evictedCount", evictedCount);
}

} // namespace ndniot
