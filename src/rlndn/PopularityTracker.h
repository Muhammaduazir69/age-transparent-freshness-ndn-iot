//
// NDN-IoT-RL - PopularityTracker
// Sliding-window per-name request statistics feeding the DQN placement
// agent's popularity features (thesis 3.3.2): short/long-window frequency,
// recency, and trend. Bounded memory: at most maxNames tracked names (LRU
// discard), at most windowLong of history per name (lazy pruning).
//

#ifndef __NDNIOT_POPULARITYTRACKER_H
#define __NDNIOT_POPULARITYTRACKER_H

#include <omnetpp.h>
#include <algorithm>
#include <deque>
#include <vector>
#include <map>
#include <string>

namespace ndniot {

class PopularityTracker {
public:
    PopularityTracker(double windowShort = 60.0, double windowLong = 600.0,
                      size_t maxNames = 4096)
        : wShort(windowShort), wLong(windowLong), maxNames(maxNames) {}

    void onInterest(const std::string &name, omnetpp::simtime_t now) {
        auto &rec = table[name];
        rec.times.push_back(now);
        rec.lastSeen = now;
        prune(rec, now);
        if (table.size() > maxNames)
            discardOldest();
    }

    // requests within the short window
    double freqShort(const std::string &name, omnetpp::simtime_t now) const {
        return countSince(name, now - wShort);
    }
    // requests within the long window
    double freqLong(const std::string &name, omnetpp::simtime_t now) const {
        return countSince(name, now - wLong);
    }
    // seconds since last request (clipped to windowLong; windowLong = never seen)
    double recency(const std::string &name, omnetpp::simtime_t now) const {
        auto it = table.find(name);
        if (it == table.end()) return wLong;
        double r = (now - it->second.lastSeen).dbl();
        return r > wLong ? wLong : r;
    }
    // >0: rising popularity, <0: falling (short-window rate vs long-window rate)
    double trend(const std::string &name, omnetpp::simtime_t now) const {
        double s = freqShort(name, now) / wShort;
        double l = freqLong(name, now) / wLong;
        return s - l;
    }

private:
    struct Rec {
        std::deque<omnetpp::simtime_t> times;
        omnetpp::simtime_t lastSeen;
    };
    double wShort, wLong;
    size_t maxNames;
    std::map<std::string, Rec> table;

    void prune(Rec &rec, omnetpp::simtime_t now) {
        omnetpp::simtime_t cutoff = now - wLong;
        while (!rec.times.empty() && rec.times.front() < cutoff)
            rec.times.pop_front();
    }
    double countSince(const std::string &name, omnetpp::simtime_t cutoff) const {
        auto it = table.find(name);
        if (it == table.end()) return 0;
        double n = 0;
        for (auto rit = it->second.times.rbegin(); rit != it->second.times.rend(); ++rit) {
            if (*rit < cutoff) break;
            n++;
        }
        return n;
    }
    void discardOldest() {
        // Amortised: drop the oldest quarter in one pass instead of scanning
        // the whole table on every Interest once it is full (a stream of
        // never-repeated names would otherwise cost O(maxNames) per Interest).
        std::vector<omnetpp::simtime_t> seen;
        seen.reserve(table.size());
        for (const auto &kv : table) seen.push_back(kv.second.lastSeen);
        size_t cut = seen.size() / 4;
        std::nth_element(seen.begin(), seen.begin() + cut, seen.end());
        omnetpp::simtime_t cutoff = seen[cut];
        for (auto it = table.begin(); it != table.end();)
            it = (it->second.lastSeen < cutoff) ? table.erase(it) : std::next(it);
    }
};

} // namespace ndniot

#endif
