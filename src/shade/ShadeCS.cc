#include "ShadeCS.h"
#include <cmath>

namespace ndniot {

Define_Module(ShadeCS);

void ShadeCS::initialize() {
    CS::initialize();
    lecar = par("replacementPolicy").stdstringValue() == "LECAR";
    lecarLearningRate = par("lecarLearningRate").doubleValue();
    int n = maxEntries > 0 ? maxEntries : 10;
    lecarDiscount = std::pow(0.005, 1.0 / n);
    if (lecar)
        policy = ReplacementPolicy::LRU;   // the base switch is unused under LeCaR
}

void ShadeCS::remember(Hist &h, const std::string &name) {
    int n = maxEntries > 0 ? maxEntries : 10;
    h.order.emplace_back(name, lecarTick);
    h.when[name] = lecarTick;
    while ((int)h.order.size() > n) {
        auto it = h.when.find(h.order.front().first);
        // only forget the name if this is its most recent history record
        if (it != h.when.end() && it->second == h.order.front().second)
            h.when.erase(it);
        h.order.pop_front();
    }
}

std::string ShadeCS::lfuVictim() const {
    auto best = entries.end();
    for (auto it = entries.begin(); it != entries.end(); ++it)
        if (best == entries.end() || it->second->accessCount < best->second->accessCount)
            best = it;
    return best == entries.end() ? std::string() : best->first;
}

void ShadeCS::evictEntry() {
    if (!lecar) {
        CS::evictEntry();
        return;
    }
    if (sweepExpiredOnce())
        return;
    if (entries.empty())
        return;
    // LeCaR: sample the expert in proportion to its weight, evict its victim
    // and remember the name in that expert's history.
    bool useLru = uniform(0, 1) < wLru / (wLru + wLfu);
    std::string victim = useLru ? lruList.back() : lfuVictim();
    if (victim.empty())
        return;
    if (useLru) { remember(histLru, victim); lecarLruChoices++; }
    else        { remember(histLfu, victim); lecarLfuChoices++; }
    removeEntry(victim);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void ShadeCS::handleLookupRequest(CSLookupRequest *request) {
    if (lecar && enabled) {
        lecarTick++;
        std::string name = request->getName();
        CSEntry *e = findEntry(name);
        bool miss = (e == nullptr || e->isExpired());
        if (miss) {
            // A miss on a name an expert evicted is regret for that expert:
            // the other expert's weight grows by exp(lambda * d^age).
            auto penal = [&](Hist &h, double &other) {
                auto it = h.when.find(name);
                if (it == h.when.end()) return false;
                double r = std::pow(lecarDiscount, (double)(lecarTick - it->second));
                other *= std::exp(lecarLearningRate * r);
                h.when.erase(it);
                return true;
            };
            if (!penal(histLru, wLfu))
                penal(histLfu, wLru);
            double s = wLru + wLfu;
            wLru /= s;
            wLfu /= s;
        }
    }
    CS::handleLookupRequest(request);
}

void ShadeCS::finish() {
    CS::finish();
    if (lecar) {
        recordScalar("lecarWeightLru", wLru);
        recordScalar("lecarLruChoices", lecarLruChoices);
        recordScalar("lecarLfuChoices", lecarLfuChoices);
    }
}

} // namespace ndniot
