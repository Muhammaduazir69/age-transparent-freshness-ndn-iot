//
// NDN-IoT-SHADE - GhostCache
// Metadata-only replica of a Content Store run under one caching expert.
// It holds names and timestamps, never payloads, and reproduces the real
// CS rules exactly: a lookup hits while the per-hop FreshnessPeriod has not
// run out; the hit is *valid* when the copy is also still fresh on the
// producer's clock (age <= FreshnessPeriod) and *stale* otherwise; at
// eviction time an expired entry is discarded first (expired-first sweep),
// then the expert's eviction rule picks the victim.
//

#ifndef __NDNIOT_GHOSTCACHE_H
#define __NDNIOT_GHOSTCACHE_H

#include <omnetpp.h>
#include <cmath>
#include <functional>
#include <list>
#include <string>
#include <unordered_map>

namespace ndniot {

enum class Admission { LCE, FRESH, DOOR, QL, LCD, PROB, TLFU, TARA, PCAST, FDC };
enum class Eviction { LRU, LFUA, LFFO, TARA, TARA_ADD };

enum GhostOutcome { GHOST_MISS = 0, GHOST_VALID = 1, GHOST_STALE = 2 };

struct GhostEntry {
    omnetpp::simtime_t insertTime, hopExpiry, originExpiry;
    double decayed = 1.0;              // aged-LFU score, enters with one credit
    omnetpp::simtime_t lastDecay;
    int validHits = 0;
    int hops = 1;                      // hops the Data traveled before being cached
    int qState = -1;                   // Q-learning state at admission (-1: none)
    std::list<std::string>::iterator lruPos;
};

class GhostCache {
  public:
    // Called with (name, entry) whenever an entry leaves the ghost.
    using LeaveHook = std::function<void(const std::string &, const GhostEntry &)>;

    GhostCache(int capacity = 10, Eviction ev = Eviction::LRU, double lfuTau = 60.0)
        : cap(capacity), ev(ev), tau(lfuTau) {}

    void setLeaveHook(LeaveHook h) { onLeave = std::move(h); }
    Eviction eviction() const { return ev; }
    int size() const { return (int)map.size(); }
    int capacity() const { return cap; }

    // A lookup for `name` at time `now`; updates recency/frequency on a hit.
    GhostOutcome lookup(const std::string &name, omnetpp::simtime_t now) {
        ops++;
        auto it = map.find(name);
        if (it == map.end())
            return GHOST_MISS;
        GhostEntry &e = it->second;
        if (now >= e.hopExpiry) {          // CS: expired entry is a miss and is dropped
            leave(it);
            return GHOST_MISS;
        }
        lru.splice(lru.begin(), lru, e.lruPos);
        e.decayed = aged(e, now) + 1.0;
        e.lastDecay = now;
        if (now <= e.originExpiry) {
            e.validHits++;
            return GHOST_VALID;
        }
        return GHOST_STALE;
    }

    bool containsFresh(const std::string &name, omnetpp::simtime_t now) const {
        auto it = map.find(name);
        return it != map.end() && now < it->second.hopExpiry;
    }

    // Insert after the expert has decided to admit.
    void insert(const std::string &name, omnetpp::simtime_t now,
                omnetpp::simtime_t freshness, omnetpp::simtime_t originTime,
                int qState = -1, int hops = 1) {
        ops++;
        auto it = map.find(name);
        if (it != map.end()) {
            if (now < it->second.hopExpiry)
                return;                    // already cached and fresh
            leave(it);
        }
        if (cap <= 0)
            return;
        while ((int)map.size() >= cap)
            evictOne(now);
        lru.push_front(name);
        GhostEntry e;
        e.insertTime = now;
        e.hopExpiry = freshness > SIMTIME_ZERO ? now + freshness : SIMTIME_MAX;
        e.originExpiry = freshness > SIMTIME_ZERO ? originTime + freshness
                                                           : SIMTIME_MAX;
        e.lastDecay = now;
        e.qState = qState;
        e.hops = hops;
        e.lruPos = lru.begin();
        map.emplace(name, e);
    }

    long operations() const { return ops; }

    // Expected-utility score of a resident copy (Section: method). The three
    // factors are the locally decayed request rate, the TRUE residual life on
    // the producer's clock, and the number of hops a hit would save.
    double taraScore(const GhostEntry &e, omnetpp::simtime_t now) const {
        double r = (e.originExpiry - now).dbl();
        if (r < 0) r = 0;
        return aged(e, now) * r * (double)(e.hops > 1 ? e.hops : 1);
    }

    // Equally weighted sum of the same three factors, each normalised by the
    // largest value among the residents (the additive control form).
    double taraAddScore(const GhostEntry &e, omnetpp::simtime_t now,
                        double maxRate, double maxRes, double maxHops) const {
        double r = (e.originExpiry - now).dbl();
        if (r < 0) r = 0;
        int h = e.hops > 1 ? e.hops : 1;
        double a = maxRate > 0 ? aged(e, now) / maxRate : 0.0;
        double b = maxRes  > 0 ? r / maxRes : 0.0;
        double c = maxHops > 0 ? (double)h / maxHops : 0.0;
        return (a + b + c) / 3.0;
    }

    // Largest value of each factor among the residents, for the additive form.
    void taraAddMaxima(omnetpp::simtime_t now, double &maxRate,
                       double &maxRes, double &maxHops) const {
        maxRate = maxRes = maxHops = 0;
        for (auto &kv : map) {
            double r = (kv.second.originExpiry - now).dbl();
            if (r < 0) r = 0;
            int h = kv.second.hops > 1 ? kv.second.hops : 1;
            double a = aged(kv.second, now);
            if (a > maxRate) maxRate = a;
            if (r > maxRes) maxRes = r;
            if ((double)h > maxHops) maxHops = (double)h;
        }
    }

    // The smallest resident score, or a negative value when the store is not
    // full or an already-expired entry would be swept first (both mean the
    // candidate is admitted without displacing anything of value).
    double minTaraScore(omnetpp::simtime_t now) const {
        if ((int)map.size() < cap || cap <= 0)
            return -1.0;
        for (auto &kv : map)
            if (now >= kv.second.hopExpiry) return -1.0;
        double worst = -1.0;
        bool first = true;
        for (auto &kv : map) {
            double s = taraScore(kv.second, now);
            if (first || s < worst) { worst = s; first = false; }
        }
        return worst;
    }

    // The entry the next insertion would evict, or nullptr when there is room or
    // an expired entry would go first (used by TinyLFU-style admission).
    const std::string *peekVictim(omnetpp::simtime_t now) const {
        if ((int)map.size() < cap || cap <= 0)
            return nullptr;
        for (auto &kv : map)
            if (now >= kv.second.hopExpiry) return nullptr;
        switch (ev) {
            case Eviction::LRU: return &lru.back();
            case Eviction::LFUA: {
                const std::string *v = nullptr; double worst = 0;
                for (auto &kv : map) {
                    double s = aged(kv.second, now);
                    if (!v || s < worst) { worst = s; v = &kv.first; }
                }
                return v;
            }
            case Eviction::LFFO: {
                auto best = map.end();
                for (auto it = map.begin(); it != map.end(); ++it)
                    if (best == map.end() || it->second.originExpiry < best->second.originExpiry) best = it;
                return best == map.end() ? nullptr : &best->first;
            }
            case Eviction::TARA: {
                auto best = map.end(); double worst = 0;
                for (auto it = map.begin(); it != map.end(); ++it) {
                    double s = taraScore(it->second, now);
                    if (best == map.end() || s < worst) { worst = s; best = it; }
                }
                return best == map.end() ? nullptr : &best->first;
            }
            case Eviction::TARA_ADD: {
                double mr, mx, mh; taraAddMaxima(now, mr, mx, mh);
                auto best = map.end(); double worst = 0;
                for (auto it = map.begin(); it != map.end(); ++it) {
                    double s = taraAddScore(it->second, now, mr, mx, mh);
                    if (best == map.end() || s < worst) { worst = s; best = it; }
                }
                return best == map.end() ? nullptr : &best->first;
            }
        }
        return nullptr;
    }

  private:
    int cap;
    Eviction ev;
    double tau;
    std::unordered_map<std::string, GhostEntry> map;
    std::list<std::string> lru;       // front = most recently used
    LeaveHook onLeave;
    long ops = 0;

    double aged(const GhostEntry &e, omnetpp::simtime_t now) const {
        double dt = (now - e.lastDecay).dbl();
        return (dt <= 0 || tau <= 0) ? e.decayed : e.decayed * std::exp(-dt / tau);
    }

    void leave(std::unordered_map<std::string, GhostEntry>::iterator it) {
        if (onLeave)
            onLeave(it->first, it->second);
        lru.erase(it->second.lruPos);
        map.erase(it);
    }

    void evictOne(omnetpp::simtime_t now) {
        // tier 1: an entry whose per-hop freshness ran out (CS expiredFirst)
        for (auto it = map.begin(); it != map.end(); ++it)
            if (now >= it->second.hopExpiry) { leave(it); return; }
        auto victim = map.end();
        switch (ev) {
            case Eviction::LRU:
                victim = map.find(lru.back());
                break;
            case Eviction::LFUA: {
                double worst = 0;
                for (auto it = map.begin(); it != map.end(); ++it) {
                    double s = aged(it->second, now);
                    if (victim == map.end() || s < worst) { worst = s; victim = it; }
                }
                break;
            }
            case Eviction::LFFO: {
                for (auto it = map.begin(); it != map.end(); ++it)
                    if (victim == map.end() ||
                        it->second.originExpiry < victim->second.originExpiry)
                        victim = it;
                break;
            }
            case Eviction::TARA: {
                double worst = 0;
                for (auto it = map.begin(); it != map.end(); ++it) {
                    double s = taraScore(it->second, now);
                    if (victim == map.end() || s < worst) { worst = s; victim = it; }
                }
                break;
            }
            case Eviction::TARA_ADD: {
                double mr, mx, mh; taraAddMaxima(now, mr, mx, mh);
                double worst = 0;
                for (auto it = map.begin(); it != map.end(); ++it) {
                    double s = taraAddScore(it->second, now, mr, mx, mh);
                    if (victim == map.end() || s < worst) { worst = s; victim = it; }
                }
                break;
            }
        }
        if (victim != map.end())
            leave(victim);
    }
};

} // namespace ndniot

#endif
