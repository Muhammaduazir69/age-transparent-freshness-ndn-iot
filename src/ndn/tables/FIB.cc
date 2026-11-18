//
// NDN-IoT-RL - FIB Implementation
//

#include "FIB.h"
#include <sstream>
#include "ndn/core/NdnControlMessages_m.h"

namespace ndniot {

Define_Module(FIB);

FIB::FIB() : cleanupTimer(nullptr), currentSize(0), totalInsertions(0),
             totalRemovals(0), totalLookups(0), totalUpdates(0) {}

FIB::~FIB() {
    cancelAndDelete(cleanupTimer);
    for (auto &entry : entries) {
        delete entry.second;
    }
    entries.clear();
}

void FIB::initialize() {
    // Read parameters
    maxSize = par("maxSize");
    enableDynamicRouting = par("enableDynamicRouting");
    entryLifetime = par("entryLifetime");
    cleanupInterval = par("cleanupInterval");

    // Initialize statistics
    currentSize = 0;
    totalInsertions = 0;
    totalRemovals = 0;
    totalLookups = 0;
    totalUpdates = 0;

    // Register signals
    fibSizeSignal = registerSignal("fibSize");
    fibLookupSignal = registerSignal("fibLookup");
    fibUpdateSignal = registerSignal("fibUpdate");
    fibRouteAddEventSignal = registerSignal("fibRouteAddEvent");
    fibRouteRemoveEventSignal = registerSignal("fibRouteRemoveEvent");

    // Schedule cleanup timer
    cleanupTimer = new cMessage("fibCleanupTimer");
    scheduleAt(simTime() + cleanupInterval, cleanupTimer);

    EV_DEBUG << "FIB initialized: maxSize=" << maxSize << endl;

    // ---- Static routes from NED parameter ----
    // Format: "prefix:face:cost;prefix:face:cost" (cost optional, default 1).
    // Convention kept from the two-face design: face 0 = APP (local app logic),
    // face 1..N = WIRE faces (UDP neighbors, registered in connection order).
    // In INET scenarios the NdnStaticConfigurator installs routes directly via
    // addRoute() instead; this parameter serves standalone/smoke topologies.
    std::string routesSpec = par("staticRoutes").stdstringValue();
    if (!routesSpec.empty()) {
        std::stringstream ss(routesSpec);
        std::string item;
        while (std::getline(ss, item, ';')) {
            if (item.empty()) continue;
            std::string prefix; int face = -1, cost = 1;
            size_t c1 = item.find(':');
            if (c1 == std::string::npos) continue;
            prefix = item.substr(0, c1);
            size_t c2 = item.find(':', c1 + 1);
            if (c2 == std::string::npos)
                face = std::stoi(item.substr(c1 + 1));
            else {
                face = std::stoi(item.substr(c1 + 1, c2 - c1 - 1));
                cost = std::stoi(item.substr(c2 + 1));
            }
            addRoute(prefix, face, cost);
            EV_DEBUG << "FIB static route: " << prefix << " -> face " << face
                    << " cost " << cost << endl;
        }
    }
}

void FIB::handleMessage(cMessage *msg) {
    if (msg == cleanupTimer) {
        cleanupStaleEntries();
        scheduleAt(simTime() + cleanupInterval, cleanupTimer);
    }
    else if (FIBLookupRequest *req = dynamic_cast<FIBLookupRequest*>(msg)) {
        handleLookupRequest(req);
    }
    else if (FIBAddRouteRequest *req = dynamic_cast<FIBAddRouteRequest*>(msg)) {
        handleAddRouteRequest(req);
    }
    else {
        delete msg;
    }
}

void FIB::handleLookupRequest(FIBLookupRequest *request) {
    FIBLookupResponse *response = new FIBLookupResponse();
    response->setName(request->getName());
    response->setTransactionId(request->getTransactionId());

    std::string name = request->getName();
    int inFace = request->getInFace();

    // Perform longest prefix match
    FIBEntry *entry = findLongestPrefixMatch(name);

    if (entry != nullptr && !entry->nextHops.empty()) {
        // Filter out incoming face
        std::vector<int> validNextHops;
        std::vector<int> validCosts;

        for (int nextHop : entry->nextHops) {
            if (nextHop != inFace) {  // Don't send back to incoming face
                validNextHops.push_back(nextHop);
                auto costIt = entry->costs.find(nextHop);
                int cost = (costIt != entry->costs.end()) ? costIt->second : 1;
                validCosts.push_back(cost);
            }
        }

        if (!validNextHops.empty()) {
            response->setFound(true);
            response->setNextHopsArraySize(validNextHops.size());
            response->setCostsArraySize(validCosts.size());

            for (size_t i = 0; i < validNextHops.size(); i++) {
                response->setNextHops(i, validNextHops[i]);
                response->setCosts(i, validCosts[i]);
            }


            EV_DEBUG << "FIB lookup for " << name << " matched prefix " << entry->prefix
                    << " with " << validNextHops.size() << " next hops" << endl;
        } else {
            response->setFound(false);
            EV_WARN << "FIB lookup for " << name << " - all next hops filtered (incoming face)" << endl;
        }
    } else {
        response->setFound(false);
        EV_WARN << "No FIB entry found for " << name << endl;
    }

    totalLookups++;
    emit(fibLookupSignal, 1L);

    send(response, "processorOut");
    delete request;
}

void FIB::handleAddRouteRequest(FIBAddRouteRequest *request) {
    FIBAddRouteResponse *response = new FIBAddRouteResponse();
    response->setTransactionId(request->getTransactionId());

    bool success = addRoute(request->getPrefix(), request->getNextHop(), request->getCost());
    response->setSuccess(success);

    if (success) {
        EV_DEBUG << "Added FIB route: " << request->getPrefix()
                << " -> face " << request->getNextHop()
                << " (cost=" << request->getCost() << ")" << endl;
    } else {
        EV_WARN << "Failed to add FIB route for " << request->getPrefix() << endl;
    }

    send(response, "processorOut");
    delete request;
}

void FIB::finish() {
    // Record final statistics
    recordScalar("finalFIBSize", currentSize);
    recordScalar("totalInsertions", totalInsertions);
    recordScalar("totalRemovals", totalRemovals);
    recordScalar("totalLookups", totalLookups);
    recordScalar("totalUpdates", totalUpdates);

    EV_DEBUG << "FIB statistics: "
            << "size=" << currentSize
            << ", lookups=" << totalLookups
            << ", updates=" << totalUpdates << endl;
}

bool FIB::addRoute(const std::string &prefix, int face, int cost) {
    Enter_Method_Silent();  // may be invoked cross-module by NdnStaticConfigurator
    FIBEntry *entry = findEntry(prefix);

    if (entry == nullptr) {
        // Create new entry
        if (isFull()) {
            EV_WARN << "FIB is full, evicting entry" << endl;
            evictEntry();
        }

        entry = createEntry(prefix);
        entries[prefix] = entry;
        currentSize++;
        totalInsertions++;
    }

    // Add next hop
    entry->addNextHop(face, cost);
    entry->timestamp = simTime();
    totalUpdates++;

    emit(fibSizeSignal, currentSize);
    emit(fibUpdateSignal, 1L);
    emit(fibRouteAddEventSignal, 1L);

    EV_DEBUG << "Added route: prefix=" << prefix << ", face=" << face << ", cost=" << cost << endl;
    return true;
}

bool FIB::removeRoute(const std::string &prefix, int face) {
    FIBEntry *entry = findEntry(prefix);

    if (entry == nullptr) {
        return false;
    }

    entry->removeNextHop(face);

    // Remove entry if no next hops left
    if (entry->nextHops.empty()) {
        removeEntry(prefix);
    }

    EV_DEBUG << "Removed route: prefix=" << prefix << ", face=" << face << endl;
    return true;
}

bool FIB::updateRoute(const std::string &prefix, int face, int cost) {
    FIBEntry *entry = findEntry(prefix);

    if (entry == nullptr) {
        return addRoute(prefix, face, cost);
    }

    entry->costs[face] = cost;
    entry->timestamp = simTime();
    totalUpdates++;

    emit(fibUpdateSignal, 1L);

    EV_DEBUG << "Updated route: prefix=" << prefix << ", face=" << face << ", cost=" << cost << endl;
    return true;
}

FIBEntry* FIB::lookup(const std::string &name) {
    totalLookups++;
    emit(fibLookupSignal, 1L);

    FIBEntry *entry = findLongestPrefixMatch(name);

    if (entry != nullptr) {
        entry->forwardCount++;
        entry->lastUsed = simTime();
        EV_DETAIL << "FIB lookup for " << name << " -> " << entry->prefix << endl;
    } else {
        EV_DETAIL << "FIB lookup for " << name << " -> no match" << endl;
    }

    return entry;
}

int FIB::getNextHop(const std::string &name) {
    FIBEntry *entry = lookup(name);

    if (entry == nullptr || entry->nextHops.empty()) {
        return -1;
    }

    return entry->getBestFace();
}

std::vector<int> FIB::getAllNextHops(const std::string &name) {
    std::vector<int> hops;
    FIBEntry *entry = lookup(name);

    if (entry != nullptr) {
        hops.assign(entry->nextHops.begin(), entry->nextHops.end());
    }

    return hops;
}

bool FIB::hasEntry(const std::string &prefix) const {
    return entries.find(prefix) != entries.end();
}

void FIB::updateLinkQuality(const std::string &prefix, double quality) {
    FIBEntry *entry = findEntry(prefix);
    if (entry != nullptr) {
        entry->linkQuality = quality;
    }
}

std::vector<std::string> FIB::getAllPrefixes() const {
    std::vector<std::string> prefixes;
    for (const auto &entry : entries) {
        prefixes.push_back(entry.first);
    }
    return prefixes;
}

void FIB::clear() {
    for (auto &entry : entries) {
        delete entry.second;
    }
    entries.clear();
    currentSize = 0;
}

FIBEntry* FIB::findEntry(const std::string &prefix) {
    auto it = entries.find(prefix);
    return (it != entries.end()) ? it->second : nullptr;
}

FIBEntry* FIB::findLongestPrefixMatch(const std::string &name) {
    FIBEntry *bestMatch = nullptr;
    size_t longestMatch = 0;

    for (auto &pair : entries) {
        const std::string &prefix = pair.first;
        if (isPrefix(prefix, name)) {
            if (prefix.length() > longestMatch) {
                longestMatch = prefix.length();
                bestMatch = pair.second;
            }
        }
    }

    return bestMatch;
}

FIBEntry* FIB::createEntry(const std::string &prefix) {
    FIBEntry *entry = new FIBEntry();
    entry->prefix = prefix;
    entry->timestamp = simTime();
    return entry;
}

void FIB::removeEntry(const std::string &prefix) {
    auto it = entries.find(prefix);
    if (it != entries.end()) {
        delete it->second;
        entries.erase(it);
        currentSize--;
        totalRemovals++;

        emit(fibSizeSignal, currentSize);
        emit(fibRouteRemoveEventSignal, 1L);
    }
}

bool FIB::isPrefix(const std::string &prefix, const std::string &name) const {
    // Check if 'prefix' is a prefix of 'name'
    if (name.length() < prefix.length()) {
        return false;
    }

    return name.compare(0, prefix.length(), prefix) == 0;
}

std::string FIB::getLongestMatchingPrefix(const std::string &name) const {
    std::string bestPrefix;
    size_t longestMatch = 0;

    for (const auto &pair : entries) {
        const std::string &prefix = pair.first;
        if (isPrefix(prefix, name)) {
            if (prefix.length() > longestMatch) {
                longestMatch = prefix.length();
                bestPrefix = prefix;
            }
        }
    }

    return bestPrefix;
}

void FIB::cleanupStaleEntries() {
    if (entryLifetime <= 0) {
        return;  // No lifetime limit
    }

    std::vector<std::string> staleEntries;
    simtime_t now = simTime();

    for (const auto &pair : entries) {
        if (now - pair.second->timestamp > entryLifetime) {
            staleEntries.push_back(pair.first);
        }
    }

    for (const std::string &prefix : staleEntries) {
        EV_DEBUG << "Removing stale FIB entry: " << prefix << endl;
        removeEntry(prefix);
    }
}

void FIB::evictEntry() {
    if (entries.empty()) {
        return;
    }

    // Evict least recently used entry
    auto lru = entries.begin();
    simtime_t oldestTime = lru->second->lastUsed;

    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->second->lastUsed < oldestTime) {
            oldestTime = it->second->lastUsed;
            lru = it;
        }
    }

    EV_WARN << "Evicting FIB entry: " << lru->first << endl;
    removeEntry(lru->first);
}

void FIB::penalizeRoute(const std::string &prefix, int face, double costMultiplier) {
    FIBEntry *entry = findEntry(prefix);
    if (entry != nullptr) {
        auto costIt = entry->costs.find(face);
        if (costIt != entry->costs.end()) {
            // Clamp multiplier to prevent overflow (max cost 10000)
            double rawCost = costIt->second * std::min(costMultiplier, 1000.0);
            int newCost = (int)std::min(rawCost, 10000.0);
            if (newCost < 1) newCost = 1;
            costIt->second = newCost;
            EV_DEBUG << "DP-IDS: Penalized route " << prefix << " face " << face
                    << " cost multiplied by " << costMultiplier
                    << " -> " << newCost << endl;
        }
    }
}


} // namespace ndniot
