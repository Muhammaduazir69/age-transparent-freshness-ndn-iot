//
// NDN-IoT-RL - Content Store Implementation
//

#include "CS.h"
#include "ndn/core/NdnControlMessages_m.h"

namespace ndniot {

Define_Module(CS);

CS::CS() : cleanupTimer(nullptr), currentSize(0), currentSizeBytes(0),
           totalInsertions(0), totalHits(0), totalMisses(0),
           totalEvictions(0), totalExpired(0) {}

CS::~CS() {
    cancelAndDelete(cleanupTimer);
    for (auto &entry : entries) {
        delete entry.second;
    }
    entries.clear();
}

void CS::initialize() {
    maxSize = par("maxSize");
    maxEntries = par("maxEntries");
    maxSizeBytes = maxSize * 1024 * 1024;  // Convert MB to bytes
    enabled = par("enabled");

    std::string policyStr = par("replacementPolicy").stdstringValue();
    if (policyStr == "LRU") policy = ReplacementPolicy::LRU;
    else if (policyStr == "LFU") policy = ReplacementPolicy::LFU;
    else if (policyStr == "LFUA" || policyStr == "LFU-Aged") policy = ReplacementPolicy::LFU_AGED;
    else if (policyStr == "FIFO") policy = ReplacementPolicy::FIFO;
    else if (policyStr == "LFF") policy = ReplacementPolicy::LFF;
    else if (policyStr == "LFFO" || policyStr == "LFF-Origin") policy = ReplacementPolicy::LFF_ORIGIN;
    else if (policyStr == "TARA") policy = ReplacementPolicy::TARA;
    else if (policyStr == "TARAADD") policy = ReplacementPolicy::TARA_ADD;
    else policy = ReplacementPolicy::LRU;

    expiredFirst = par("expiredFirst");
    ageTransparent = par("ageTransparent");
    residualHeaderBytes = (int)par("residualHeaderBytes").intValue();
    const std::string ageModeStr = par("ageMode").stdstringValue();
    if (ageModeStr == "decrement") ageDecrement = true;
    else if (ageModeStr == "origin") ageDecrement = false;
    else throw cRuntimeError("ageMode must be \"origin\" or \"decrement\", got \"%s\"",
                             ageModeStr.c_str());
    lfuAgingTau = par("lfuAgingTau").doubleValue();
    cleanupInterval = par("cleanupInterval");

    // Register signals
    csSizeSignal = registerSignal("csSize");
    csHitSignal = registerSignal("csHit");
    csMissSignal = registerSignal("csMiss");
    csHitRatioSignal = registerSignal("csHitRatio");
    csInsertionEventSignal = registerSignal("csInsertionEvent");
    csEvictionEventSignal = registerSignal("csEvictionEvent");

    // Schedule cleanup
    cleanupTimer = new cMessage("csCleanup");
    scheduleAt(simTime() + cleanupInterval, cleanupTimer);

    EV_DEBUG << "CS initialized: maxSize=" << maxSize << "MB, policy=" << policyStr << endl;
}

void CS::handleMessage(cMessage *msg) {
    if (msg == cleanupTimer) {
        cleanupExpiredEntries();
        scheduleAt(simTime() + cleanupInterval, cleanupTimer);
    }
    else if (CSLookupRequest *req = dynamic_cast<CSLookupRequest*>(msg)) {
        handleLookupRequest(req);
    }
    else if (CSInsertRequest *req = dynamic_cast<CSInsertRequest*>(msg)) {
        handleInsertRequest(req);
    }
    else {
        delete msg;
    }
}

void CS::finish() {
    recordScalar("finalCSSize", currentSize);
    recordScalar("totalInsertions", totalInsertions);
    recordScalar("totalHits", totalHits);
    recordScalar("totalMisses", totalMisses);
    recordScalar("totalEvictions", totalEvictions);
    recordScalar("expiredFirstEvictions", expiredFirstEvictions);
    recordScalar("csLookupsDisabled", csLookupsDisabled);
    recordScalar("hitRatio", getHitRatio());
    recordScalar("annotationsAttached", annotationsAttached);
    recordScalar("annotationsCharged", annotationsCharged);

    EV_DEBUG << "CS statistics: hits=" << totalHits
            << ", misses=" << totalMisses
            << ", hitRatio=" << getHitRatio() << endl;
}

bool CS::insert(DataPacket *data) {
    if (!enabled || !data) {
        return false;
    }

    std::string name = data->getName();
    int dataSize = data->getContentLength();

    // Check if already cached
    if (contains(name)) {
        CSEntry *existing = findEntry(name);
        if (existing && existing->isFresh()) {
            return false;  // Already cached and fresh
        }
        removeEntry(name);  // Remove stale entry
    }

    // Check space
    while (!hasSpace(dataSize) && currentSize > 0) {
        evictEntry();
    }

    if (!hasSpace(dataSize)) {
        return false;  // Still no space
    }

    // Create copy and insert
    DataPacket *dataCopy = data->dup();
    insertEntry(name, dataCopy);

    totalInsertions++;
    emit(csSizeSignal, currentSize);
    emit(csInsertionEventSignal, 1L);

    EV_DEBUG << "Cached: " << name << " (size=" << currentSize << ")" << endl;
    return true;
}

DataPacket* CS::lookup(const std::string &name) {
    if (!enabled) {
        csLookupsDisabled++;
        return nullptr;
    }

    CSEntry *entry = findEntry(name);

    if (entry == nullptr || entry->isExpired()) {
        totalMisses++;
        emit(csMissSignal, 1L);
        emit(csHitRatioSignal, getHitRatio());

        if (entry && entry->isExpired()) {
            removeEntry(name);
            totalExpired++;
        }

        return nullptr;
    }

    // Cache hit
    entry->accessCount++;
    entry->lastAccess = simTime();
    entry->noteAccess(simTime(), lfuAgingTau);
    updateLRU(name);

    totalHits++;
    emit(csHitSignal, 1L);
    emit(csHitRatioSignal, getHitRatio());

    EV_DEBUG << "Cache HIT: " << name << endl;
    DataPacket *served = entry->data->dup();
    served->setRestamps(entry->data->getRestamps() + 1);
    if (ageTransparent && ageDecrement)
        annotateResidual(served, entry);
    return served;
}

bool CS::contains(const std::string &name) {
    CSEntry *entry = findEntry(name);
    return entry != nullptr && entry->isFresh();
}

void CS::clear() {
    for (auto &entry : entries) {
        delete entry.second;
    }
    entries.clear();
    lruList.clear();
    currentSize = 0;
    currentSizeBytes = 0;
}

CSEntry* CS::findEntry(const std::string &name) {
    auto it = entries.find(name);
    return (it != entries.end()) ? it->second : nullptr;
}

void CS::annotateResidual(DataPacket *served, const CSEntry *entry) {
    // Clock-free age transparency. The next cache will restart its freshness
    // timer from its own arrival time whatever we do, so the correction is
    // applied here, where the residence time is known locally: tell the next
    // hop how much life the copy has left instead of letting it assume a full
    // period. HTTP solves the same problem the same way with the Age header
    // (RFC 9111 Sec. 4.2).
    //
    // The value does NOT go into FreshnessPeriod. That field lives in the
    // Data packet's MetaInfo, which the producer's signature covers, so
    // rewriting it would invalidate the signature and change the implicit
    // digest. It travels instead as a hop-by-hop annotation beside the Data,
    // in the manner of an ignorable NDNLPv2 header field, and the Data packet
    // itself is forwarded untouched.
    //
    // An expired entry is never served (the lookup drops it first), so the
    // residual is strictly positive here.
    const simtime_t residual = entry->expiryTime - simTime();
    annotationsAttached++;
    if (!served->getHasResidual()) {    // charge the TLV once, where it is attached
        served->setByteLength(served->getByteLength() + residualHeaderBytes);
        annotationsCharged++;
    }
    served->setHasResidual(true);
    served->setResidualFreshness(residual > SIMTIME_ZERO ? residual : SIMTIME_ZERO);
}

void CS::insertEntry(const std::string &name, DataPacket *data) {
    CSEntry *entry = new CSEntry(name, data);
    // Age-transparent mode: the entry expires when the reading does, not one
    // FreshnessPeriod after this node happened to receive it.
    //
    // The timestamp realization reads the publisher's deadline off the
    // packet's signed generation time, which requires a common time base but
    // modifies nothing in flight.
    //
    // The clock-free realization needs no such step and no shared clock. The
    // residual lifetime arrives as a hop-by-hop annotation and the entry
    // constructor has already used it, so the deadline is set. A copy that
    // arrives without an annotation, from the publisher or from a node that
    // does not implement the field, keeps the standard per-hop deadline.
    if (ageTransparent && !ageDecrement && entry->originExpiryTime > SIMTIME_ZERO)
        entry->expiryTime = entry->originExpiryTime;
    else if (ageTransparent && ageDecrement && data && data->getHasResidual()) {
        // Honor the annotation, clamped at the lifetime the publisher signed.
        // The clamp costs one comparison and never binds in honest operation,
        // because a residual is what is left of F and so cannot exceed it. It
        // is what makes the security ceiling of Sec. 4.3 true rather than
        // merely plausible: a neighbor that inflates the annotation can buy a
        // copy a full declared period at this hop, which is exactly standard
        // NDN, and cannot buy more.
        simtime_t r = data->getResidualFreshness();
        if (r < SIMTIME_ZERO) r = SIMTIME_ZERO;
        const simtime_t f = data->getFreshnessPeriod();
        entry->expiryTime = simTime() + (r < f ? r : f);
    }
    entries[name] = entry;
    lruList.push_front(name);

    currentSize++;
    currentSizeBytes += entry->size;
}

void CS::removeEntry(const std::string &nameRef) {
    // Copy first: callers may pass a reference into the entry (e->name) or the
    // map key itself, both of which die inside this function (use-after-free
    // in lruList.remove otherwise).
    std::string name = nameRef;
    auto it = entries.find(name);
    if (it != entries.end()) {
        currentSizeBytes -= it->second->size;
        delete it->second;
        entries.erase(it);
        lruList.remove(name);
        currentSize--;
    }
}

bool CS::sweepExpiredOnce() {
    // Tier 1, now symmetric across every policy (previously RlCS-only).
    if (!expiredFirst)
        return false;
    for (const auto &kv : entries) {
        if (kv.second->isExpired()) {
            EV_DEBUG << "Evicting (expired-first): " << kv.first << endl;
            removeEntry(kv.first);
            totalEvictions++;
            expiredFirstEvictions++;
            emit(csEvictionEventSignal, 1L);
            return true;
        }
    }
    return false;
}

void CS::evictEntry() {
    if (sweepExpiredOnce())
        return;
    switch (policy) {
        case ReplacementPolicy::LRU:
            evictLRU();
            break;
        case ReplacementPolicy::LFU:
            evictLFU();
            break;
        case ReplacementPolicy::LFU_AGED:
            evictLFUAged();
            break;
        case ReplacementPolicy::FIFO:
            evictFIFO();
            break;
        case ReplacementPolicy::LFF:
            evictLFF();
            break;
        case ReplacementPolicy::LFF_ORIGIN:
            evictLFFOrigin();
            break;
        case ReplacementPolicy::TARA:
            evictTARA();
            break;
        case ReplacementPolicy::TARA_ADD:
            evictTARAAdd();
            break;
    }
}

void CS::evictLRU() {
    if (lruList.empty()) return;

    std::string name = lruList.back();
    EV_DEBUG << "Evicting (LRU): " << name << endl;
    removeEntry(name);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictLFU() {
    if (entries.empty()) return;

    auto lfu = entries.begin();
    int minAccess = lfu->second->accessCount;

    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->second->accessCount < minAccess) {
            minAccess = it->second->accessCount;
            lfu = it;
        }
    }

    EV_DEBUG << "Evicting (LFU): " << lfu->first << endl;
    removeEntry(lfu->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictFIFO() {
    if (entries.empty()) return;

    auto oldest = entries.begin();
    simtime_t oldestTime = oldest->second->insertionTime;

    for (auto it = entries.begin(); it != entries.end(); ++it) {
        if (it->second->insertionTime < oldestTime) {
            oldestTime = it->second->insertionTime;
            oldest = it;
        }
    }

    EV_DEBUG << "Evicting (FIFO): " << oldest->first << endl;
    removeEntry(oldest->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictLFF() {
    // Least Fresh First: evict the entry closest to (or past) expiry.
    if (entries.empty()) return;
    auto victim = entries.begin();
    simtime_t soonest = SIMTIME_MAX;
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        simtime_t exp = it->second->expiryTime;
        if (exp <= SIMTIME_ZERO) { victim = it; soonest = SIMTIME_ZERO; break; }  // no-freshness: stale
        if (exp < soonest) { soonest = exp; victim = it; }
    }
    EV_DEBUG << "Evicting (LFF): " << victim->first << endl;
    removeEntry(victim->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictLFFOrigin() {
    // Least Fresh First on the ORIGIN clock. expiryTime is reset at every hop
    // by NDN, so under a uniform freshness period the hop-clock variant sorts
    // by arrival time and is algebraically identical to FIFO. The learned
    // replacement agent is handed the origin-based remaining life (item
    // feature 7); this baseline gets the same information.
    if (entries.empty()) return;
    auto victim = entries.begin();
    simtime_t soonest = SIMTIME_MAX;
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        simtime_t exp = it->second->originExpiry();
        if (exp <= SIMTIME_ZERO) { victim = it; soonest = SIMTIME_ZERO; break; }
        if (exp < soonest) { soonest = exp; victim = it; }
    }
    EV_DEBUG << "Evicting (LFF-origin): " << victim->first << endl;
    removeEntry(victim->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictTARA() {
    // Expected-utility retention. The victim is the copy with the smallest
    // product of decayed request rate, true residual life and hops saved,
    // which is the smallest expected remaining return on its slot. A copy
    // whose reading has already expired on the producer's clock scores zero
    // however popular it is, which is the behaviour an additive
    // freshness-plus-popularity weight cannot reproduce.
    if (entries.empty()) return;
    simtime_t now = simTime();
    auto victim = entries.begin();
    double worst = taraScore(victim->second, now);
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        double s = taraScore(it->second, now);
        if (s < worst) { worst = s; victim = it; }
    }
    EV_DEBUG << "Evicting (TARA): " << victim->first << " score=" << worst << endl;
    removeEntry(victim->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictTARAAdd() {
    // Additive control for evictTARA(): the same rate, true-residual-life and
    // hop factors, each divided by the largest value among the residents and
    // summed with equal weight. An entry whose reading has already expired
    // still scores on its rate and distance, which is the behaviour the
    // multiplicative form removes.
    if (entries.empty()) return;
    simtime_t now = simTime();
    double maxRate = 0, maxRes = 0, maxHops = 0;
    for (const auto &kv : entries) {
        const CSEntry *e = kv.second;
        maxRate = std::max(maxRate, e->agedScore(now, lfuAgingTau));
        maxRes  = std::max(maxRes, e->trueResidual(now));
        maxHops = std::max(maxHops, (double)(e->hopCount > 1 ? e->hopCount : 1));
    }
    auto victim = entries.begin();
    double worst = taraAddScore(victim->second, now, maxRate, maxRes, maxHops);
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        double sc = taraAddScore(it->second, now, maxRate, maxRes, maxHops);
        if (sc < worst) { worst = sc; victim = it; }
    }
    EV_DEBUG << "Evicting (TARA-add): " << victim->first << " score=" << worst << endl;
    removeEntry(victim->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::evictLFUAged() {
    // Windowed LFU: exponentially decayed access score, so a burst of early
    // popularity cannot pin a slot for the rest of the run (the pollution
    // failure mode of raw LFU), and a newly admitted entry does not enter
    // with a guaranteed-minimum score of zero.
    if (entries.empty()) return;
    simtime_t now = simTime();
    auto victim = entries.begin();
    double worst = victim->second->agedScore(now, lfuAgingTau);
    for (auto it = entries.begin(); it != entries.end(); ++it) {
        double s = it->second->agedScore(now, lfuAgingTau);
        if (s < worst) { worst = s; victim = it; }
    }
    EV_DEBUG << "Evicting (LFU-aged): " << victim->first << endl;
    removeEntry(victim->first);
    totalEvictions++;
    emit(csEvictionEventSignal, 1L);
}

void CS::updateLRU(const std::string &name) {
    lruList.remove(name);
    lruList.push_front(name);
}

void CS::cleanupExpiredEntries() {
    std::vector<std::string> expiredNames;

    for (const auto &entry : entries) {
        if (entry.second->isExpired()) {
            expiredNames.push_back(entry.first);
        }
    }

    for (const std::string &name : expiredNames) {
        removeEntry(name);
        totalExpired++;
    }

    if (!expiredNames.empty()) {
        EV_DEBUG << "Cleaned " << expiredNames.size() << " expired entries" << endl;
    }
}

bool CS::isFull(int dataSize) const {
    return !hasSpace(dataSize);
}

bool CS::hasSpace(int dataSize) const {
    if (maxEntries > 0 && currentSize >= maxEntries)
        return false;
    return currentSizeBytes + dataSize <= maxSizeBytes;
}

void CS::handleLookupRequest(CSLookupRequest *request) {
    CSLookupResponse *response = new CSLookupResponse();
    response->setName(request->getName());
    response->setTransactionId(request->getTransactionId());

    if (!enabled) {
        // A disabled store is not a cache that missed: counting these as
        // misses drives the reported hit ratio toward zero for reasons that
        // have nothing to do with the caching policy. Counted separately.
        response->setFound(false);
        csLookupsDisabled++;
        send(response, "processorOut");
        delete request;
        return;
    }

    std::string name = request->getName();
    CSEntry *entry = findEntry(name);

    if (entry == nullptr || entry->isExpired()) {
        // Cache miss
        totalMisses++;
        emit(csMissSignal, 1L);
        emit(csHitRatioSignal, getHitRatio());

        if (entry && entry->isExpired()) {
            removeEntry(name);
            totalExpired++;
            EV_DEBUG << "Cache entry expired: " << name << endl;
        }

        response->setFound(false);
        EV_DEBUG << "Cache MISS: " << name << endl;
    } else {
        // Cache hit - return cached data
        entry->accessCount++;
        entry->lastAccess = simTime();
        entry->noteAccess(simTime(), lfuAgingTau);
        updateLRU(name);

        totalHits++;
        emit(csHitSignal, 1L);
        emit(csHitRatioSignal, getHitRatio());

        response->setFound(true);
        DataPacket *served = entry->data->dup();
        served->setRestamps(entry->data->getRestamps() + 1);
        // Clock-free age transparency. The next cache is going to restart
        // FreshnessPeriod from its own arrival time whatever we do, so the
        // correction is applied here, where the residence time is known
        // locally: send the remaining lifetime instead of the full period.
        // HTTP solves the same problem the same way with the Age header
        // (RFC 9111 Sec. 4.2). An expired entry is never served (the lookup
        // above drops it), so the residual is strictly positive here.
        if (ageTransparent && ageDecrement)
            annotateResidual(served, entry);
        response->setData(dynamic_cast<cMessage*>(served));

        EV_DEBUG << "Cache HIT: " << name << " (hits=" << totalHits
                << ", misses=" << totalMisses
                << ", ratio=" << getHitRatio() << ")" << endl;
    }

    send(response, "processorOut");
    delete request;
}

void CS::handleInsertRequest(CSInsertRequest *request) {
    CSInsertResponse *response = new CSInsertResponse();
    response->setName(request->getName());
    response->setTransactionId(request->getTransactionId());

    // The request's data field is @owned: deleting the request also deletes
    // the contained copy; storing it in the CS requires removeData().
    std::string name = request->getName();
    const DataPacket *dataRef = dynamic_cast<const DataPacket*>(request->getData());

    if (!enabled || !dataRef) {
        response->setSuccess(false);
        send(response, "processorOut");
        delete request;
        return;
    }

    int dataSize = dataRef->getContentLength();

    // Check if already cached
    if (contains(name)) {
        CSEntry *existing = findEntry(name);
        if (existing && existing->isFresh()) {
            // Already cached and fresh, no need to insert
            response->setSuccess(false);
            EV_DEBUG << "Data already cached and fresh: " << name << endl;
            send(response, "processorOut");
            delete request;
            return;
        }
        // Remove stale entry
        removeEntry(name);
    }

    // Check space and evict if necessary
    while (!hasSpace(dataSize) && currentSize > 0) {
        evictEntry();
    }

    if (!hasSpace(dataSize)) {
        // Still no space after eviction
        response->setSuccess(false);
        EV_WARN << "CS full, cannot cache: " << name << endl;
        send(response, "processorOut");
        delete request;
        return;
    }

    // Take ownership of the processor's copy (no second dup)
    DataPacket *data = dynamic_cast<DataPacket*>(request->removeData());
    insertEntry(name, data);

    totalInsertions++;
    emit(csSizeSignal, currentSize);
    emit(csInsertionEventSignal, 1L);

    response->setSuccess(true);

    EV_DEBUG << "Cached data: " << name
            << " (size=" << currentSize
            << "/" << maxSize
            << ", bytes=" << currentSizeBytes
            << "/" << maxSizeBytes << ")" << endl;

    send(response, "processorOut");
    delete request;
}



} // namespace ndniot
