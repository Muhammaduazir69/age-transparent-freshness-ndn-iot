//
// NDN-IoT-RL - NDN Processor Implementation
// Complete NDN Interest/Data forwarding logic with PIT/FIB/CS integration
//

#include "NDNProcessor.h"
#include "NdnControlMessages_m.h"

namespace ndniot {

Define_Module(NDNProcessor);

NDNProcessor::NDNProcessor() : nextFaceId(0), nextTransactionId(1) {}

NDNProcessor::~NDNProcessor() {
    // Clear pending transactions
    for (auto &pair : pendingTransactions) {
        if (pair.second.packet) {
            delete pair.second.packet;
        }
    }
    pendingTransactions.clear();
}

void NDNProcessor::initialize() {
    nodeId = par("nodeId");
    nodeIdentifier = par("nodeType").stdstringValue() + std::to_string(nodeId);
    enableCaching = par("enableCaching");
    enableSignatureVerification = par("enableSignatureVerification");
    signatureVerificationDelay = par("signatureVerificationDelay");
    forwardingStrategy = par("forwardingStrategy").stdstringValue();

    // Register signals
    interestSentSignal = registerSignal("interestSent");
    interestReceivedSignal = registerSignal("interestReceived");
    dataSentSignal = registerSignal("dataSent");
    dataReceivedSignal = registerSignal("dataReceived");
    cacheHitSignal = registerSignal("cacheHit");
    cacheMissSignal = registerSignal("cacheMiss");
    nackSentSignal = registerSignal("nackSent");
    nackReceivedSignal = registerSignal("nackReceived");
    packetDroppedSignal = registerSignal("packetDropped");
    forwardingDelaySignal = registerSignal("forwardingDelay");

    nextFaceId = 0;
    nextTransactionId = 1;

    // WATCH for Qtenv inspector
    WATCH(interestsSent);
    WATCH(interestsReceived);
    WATCH(dataSent);
    WATCH(dataReceived);
    WATCH(cacheHits);
    WATCH(cacheMisses);

    EV_DEBUG << "NDNProcessor initialized: " << nodeIdentifier << endl;

    // Pre-register one face per connected ndnOut gate.  Without this the
    // FIB-installed default routes (which point at face 0) can never be
    // resolved by getGateForFace() because faces were only registered
    // lazily on the first arriving packet.  That left interestSent at 0
    // for the entire run.  We now build the gate->face mapping up front
    // so forwardInterest() / forwardData() can dispatch from t=0.
    EV_DEBUG << "=== NDNProcessor Gate Configuration ===" << endl;
    EV_DEBUG << "Node: " << nodeIdentifier << endl;
    EV_DEBUG << "Total ndnOut gates: " << gateSize("ndnOut") << endl;
    for (int i = 0; i < gateSize("ndnOut"); i++) {
        cGate *gate = this->gate("ndnOut", i);
        if (gate->isConnected()) {
            cGate *nextGate = gate->getNextGate();
            cModule *dest = nextGate->getOwnerModule();
            int fid = nextFaceId++;
            faceToGate[fid] = i;
            gateToFace[i]   = fid;
            EV_DEBUG << "  ndnOut[" << i << "] --> " << dest->getFullPath()
                    << " (gate: " << nextGate->getName() << ") face=" << fid << endl;
        } else {
            EV_DEBUG << "  ndnOut[" << i << "] --> NOT CONNECTED" << endl;
        }
    }
    EV_DEBUG << "=======================================" << endl;
}

void NDNProcessor::handleMessage(cMessage *msg) {
    // Handle messages from different sources
    if (msg->arrivedOn("ndnIn")) {
        // Network packet from application/network layer
        handleNetworkPacket(msg);
    }
    else if (msg->arrivedOn("pitIn")) {
        // Response from PIT module
        handlePITResponse(msg);
    }
    else if (msg->arrivedOn("fibIn")) {
        // Response from FIB module
        handleFIBResponse(msg);
    }
    else if (msg->arrivedOn("csIn")) {
        // Response from CS module
        handleCSResponse(msg);
    }
    else {
        EV_WARN << "Message from unknown gate" << endl;
        delete msg;
    }
}

void NDNProcessor::handleNetworkPacket(cMessage *msg) {
    int gateIndex = msg->getArrivalGate()->getIndex();
    int inFace = getFaceForGate(gateIndex);

    if (inFace == -1) {
        inFace = registerFace(gateIndex);
    }

    if (InterestPacket *interest = dynamic_cast<InterestPacket*>(msg)) {
        processInterest(interest, inFace);
    }
    else if (DataPacket *data = dynamic_cast<DataPacket*>(msg)) {
        processData(data, inFace);
    }
    else if (NackPacket *nack = dynamic_cast<NackPacket*>(msg)) {
        processNack(nack, inFace);
    }
    else {
        EV_WARN << "Unknown packet type" << endl;
        emit(packetDroppedSignal, 1L);
        delete msg;
    }
}

void NDNProcessor::refreshDisplay() const {
    // Show live NDN table stats on the NdnNode compound module
    cModule *ndnNode = getParentModule();
    if (!ndnNode) return;

    // Get table sizes
    int pitSize = 0, csSize = 0, fibSize = 0;
    cModule *pitMod = ndnNode->getSubmodule("pit");
    cModule *csMod = ndnNode->getSubmodule("cs");
    cModule *fibMod = ndnNode->getSubmodule("fib");

    if (pitMod && pitMod->hasPar("currentSize"))
        pitSize = pitMod->par("currentSize");
    if (csMod && csMod->hasPar("currentSize"))
        csSize = csMod->par("currentSize");

    char label[128];
    snprintf(label, sizeof(label), "PIT:%d CS:%d I:%d D:%d",
             pitSize, csSize, interestsReceived, dataReceived);
    ndnNode->getDisplayString().setTagArg("t", 0, label);
    ndnNode->getDisplayString().setTagArg("t", 2, "blue");

    // Tooltip with detailed NDN state
    char tt[512];
    snprintf(tt, sizeof(tt),
             "NDN Processor: %s\n"
             "Interests Sent: %d | Received: %d\n"
             "Data Sent: %d | Received: %d\n"
             "Cache Hits: %d | Misses: %d\n"
             "Pending Transactions: %d",
             nodeIdentifier.c_str(),
             interestsSent, interestsReceived,
             dataSent, dataReceived,
             cacheHits, cacheMisses,
             (int)pendingTransactions.size());
    ndnNode->getDisplayString().setTagArg("tt", 0, tt);
}

void NDNProcessor::finish() {
    EV_DEBUG << "NDNProcessor " << nodeIdentifier << " finishing" << endl;
    recordScalar("pendingTransactions", (long)pendingTransactions.size());
}

void NDNProcessor::processInterest(InterestPacket *interest, int inFace) {
    emit(interestReceivedSignal, 1L);
    interestsReceived++;

    std::string name = interest->getName();
    EV_DEBUG << "Processing Interest: " << name << " from face " << inFace << endl;

    // Step 1: Query Content Store for cached data
    int transactionId = nextTransactionId++;
    CSLookupRequest *csReq = new CSLookupRequest();
    csReq->setName(name.c_str());
    csReq->setTransactionId(transactionId);

    // Store context for this transaction
    PendingTransaction trans;
    trans.transactionId = transactionId;
    trans.type = TRANS_CS_LOOKUP;
    trans.packet = interest;  // Keep interest for later use
    trans.inFace = inFace;
    trans.timestamp = simTime();
    pendingTransactions[transactionId] = trans;

    send(csReq, "csOut");
}

void NDNProcessor::handleCSResponse(cMessage *msg) {
    CSLookupResponse *response = dynamic_cast<CSLookupResponse*>(msg);
    if (!response) {
        delete msg;
        return;
    }

    int transactionId = response->getTransactionId();
    auto it = pendingTransactions.find(transactionId);
    if (it == pendingTransactions.end()) {
        EV_WARN << "CS response for unknown transaction " << transactionId << endl;
        delete msg;
        return;
    }

    PendingTransaction &trans = it->second;
    InterestPacket *interest = dynamic_cast<InterestPacket*>(trans.packet);

    if (response->getFound()) {
        // Cache hit!
        emit(cacheHitSignal, 1L);
        cacheHits++;
        bubble("Cache HIT");
        EV_DEBUG << "Cache HIT for " << response->getName() << endl;

        DataPacket *cachedData = dynamic_cast<DataPacket*>(response->removeData());
        if (cachedData) {
            // We own the CS's copy now; mark and forward it (no extra dup).
            cachedData->setServedFromCache(true);
            cachedData->setLcdDepth(0);   // LCD: this node is the hit point
            forwardData(cachedData, trans.inFace);
        }

        // Clean up
        delete interest;
        delete response;
        pendingTransactions.erase(it);
    }
    else {
        // Cache miss - proceed to PIT
        emit(cacheMissSignal, 1L);
        cacheMisses++;
        EV_DEBUG << "Cache MISS for " << response->getName() << endl;

        delete response;

        // Query PIT
        queryPIT(interest, trans.inFace, transactionId);
    }
}

void NDNProcessor::queryPIT(InterestPacket *interest, int inFace, int transactionId) {
    PITInsertRequest *pitReq = new PITInsertRequest();
    pitReq->setName(interest->getName());
    pitReq->setTransactionId(transactionId);
    pitReq->setInFace(inFace);
    pitReq->setNonce(interest->getNonce());
    pitReq->setInterestLifetime(interest->getInterestLifetime().dbl());

    // Update transaction type
    pendingTransactions[transactionId].type = TRANS_PIT_INSERT;

    send(pitReq, "pitOut");
}

void NDNProcessor::handlePITResponse(cMessage *msg) {
    if (PITInsertResponse *response = dynamic_cast<PITInsertResponse*>(msg)) {
        handlePITInsertResponse(response);
    }
    else if (PITSatisfyResponse *response = dynamic_cast<PITSatisfyResponse*>(msg)) {
        handlePITSatisfyResponse(response);
    }
    else {
        delete msg;
    }
}

void NDNProcessor::handlePITInsertResponse(PITInsertResponse *response) {
    int transactionId = response->getTransactionId();
    auto it = pendingTransactions.find(transactionId);
    if (it == pendingTransactions.end()) {
        delete response;
        return;
    }

    PendingTransaction &trans = it->second;
    InterestPacket *interest = dynamic_cast<InterestPacket*>(trans.packet);

    if (!response->getSuccess()) {
        EV_WARN << "PIT insert failed for " << response->getName() << endl;
        emit(packetDroppedSignal, 1L);
        delete interest;
        delete response;
        pendingTransactions.erase(it);
        return;
    }

    if (response->getAggregated()) {
        // Interest aggregated - no need to forward
        EV_DEBUG << "Interest aggregated for " << response->getName() << endl;
        delete interest;
        delete response;
        pendingTransactions.erase(it);
        return;
    }

    // Need to forward - query FIB
    delete response;
    queryFIB(interest, trans.inFace, transactionId);
}

void NDNProcessor::queryFIB(InterestPacket *interest, int inFace, int transactionId) {
    FIBLookupRequest *fibReq = new FIBLookupRequest();
    fibReq->setName(interest->getName());
    fibReq->setTransactionId(transactionId);
    fibReq->setInFace(inFace);

    // Update transaction type
    pendingTransactions[transactionId].type = TRANS_FIB_LOOKUP;

    send(fibReq, "fibOut");
}

void NDNProcessor::handleFIBResponse(cMessage *msg) {
    FIBLookupResponse *response = dynamic_cast<FIBLookupResponse*>(msg);
    if (!response) {
        delete msg;
        return;
    }

    int transactionId = response->getTransactionId();
    auto it = pendingTransactions.find(transactionId);
    if (it == pendingTransactions.end()) {
        delete response;
        return;
    }

    PendingTransaction &trans = it->second;
    InterestPacket *interest = dynamic_cast<InterestPacket*>(trans.packet);

    if (!response->getFound() || response->getNextHopsArraySize() == 0) {
        // No route - send NACK back
        EV_WARN << "No route for " << response->getName() << endl;
        sendNack(interest->dup(), trans.inFace, NACK_NO_ROUTE);
        delete interest;
        delete response;
        pendingTransactions.erase(it);
        return;
    }

    // Forward interest to next hops
    interest->setHopCount(interest->getHopCount() + 1);

    simtime_t forwardDelay = simTime() - trans.timestamp;
    emit(forwardingDelaySignal, forwardDelay);

    for (unsigned int i = 0; i < response->getNextHopsArraySize(); i++) {
        int nextHop = response->getNextHops(i);
        if (nextHop != trans.inFace) {  // Don't send back to incoming face
            forwardInterest(interest->dup(), nextHop);
        }
    }

    delete interest;
    delete response;
    pendingTransactions.erase(it);
}

void NDNProcessor::processData(DataPacket *data, int inFace) {
    emit(dataReceivedSignal, 1L);
    dataReceived++;

    std::string name = data->getName();
    EV_DEBUG << "Processing Data: " << name << " from face " << inFace << endl;

    // Verify signature if enabled
    if (enableSignatureVerification && data->isSigned()) {
        if (!verifySignature(data)) {
            EV_WARN << "Signature verification failed for: " << name << endl;
            emit(packetDroppedSignal, 1L);
            delete data;
            return;
        }
    }

    // Cache data if enabled and cacheable
    if (enableCaching && shouldCacheData(data)) {
        CSInsertRequest *csReq = new CSInsertRequest();
        csReq->setName(name.c_str());
        csReq->setData(dynamic_cast<cMessage*>(data->dup()));
        csReq->setTransactionId(nextTransactionId++);
        send(csReq, "csOut");
    }

    // Query PIT to find requesting faces
    int transactionId = nextTransactionId++;
    PITSatisfyRequest *pitReq = new PITSatisfyRequest();
    pitReq->setName(name.c_str());
    pitReq->setTransactionId(transactionId);
    pitReq->setInFace(inFace);

    // Store data for forwarding
    PendingTransaction trans;
    trans.transactionId = transactionId;
    trans.type = TRANS_PIT_SATISFY;
    trans.packet = data;
    trans.inFace = inFace;
    trans.timestamp = simTime();
    pendingTransactions[transactionId] = trans;

    send(pitReq, "pitOut");
}

void NDNProcessor::handlePITSatisfyResponse(PITSatisfyResponse *response) {
    int transactionId = response->getTransactionId();
    auto it = pendingTransactions.find(transactionId);
    if (it == pendingTransactions.end()) {
        delete response;
        return;
    }

    PendingTransaction &trans = it->second;
    DataPacket *data = dynamic_cast<DataPacket*>(trans.packet);

    if (!response->getFound()) {
        // No matching PIT entry - unsolicited data
        EV_WARN << "Unsolicited data: " << response->getName() << endl;
        emit(packetDroppedSignal, 1L);
        delete data;
        delete response;
        pendingTransactions.erase(it);
        return;
    }

    // Forward data to all requesting faces
    for (unsigned int i = 0; i < response->getOutFacesArraySize(); i++) {
        int outFace = response->getOutFaces(i);
        forwardData(data->dup(), outFace);
    }

    delete data;
    delete response;
    pendingTransactions.erase(it);
}

void NDNProcessor::processNack(NackPacket *nack, int inFace) {
    emit(nackReceivedSignal, 1L);

    std::string name = nack->getName();
    EV_DEBUG << "Processing NACK: " << name << " reason=" << nack->getReason() << endl;

    // In a full implementation, would update PIT/FIB based on NACK
    // For now, just log and discard
    delete nack;
}

void NDNProcessor::forwardInterest(InterestPacket *interest, int outFace) {
    emit(interestSentSignal, 1L);
    interestsSent++;

    int gateIndex = getGateForFace(outFace);
    if (gateIndex == -1 || gateIndex >= gateSize("ndnOut")) {
        EV_WARN << "Invalid face: " << outFace << endl;
        delete interest;
        return;
    }

    // hopCount already incremented once per FIB-forwarding step in
    // handleFIBResponse (before the per-face dup loop)
    EV_DEBUG << "Forwarding Interest: " << interest->getName() << " to face " << outFace << endl;
    send(interest, "ndnOut", gateIndex);
}

void NDNProcessor::forwardData(DataPacket *data, int outFace) {
    emit(dataSentSignal, 1L);
    dataSent++;

    int gateIndex = getGateForFace(outFace);
    if (gateIndex == -1 || gateIndex >= gateSize("ndnOut")) {
        EV_WARN << "Invalid face: " << outFace << endl;
        delete data;
        return;
    }

    data->setHopCount(data->getHopCount() + 1);
    if (data->getLcdDepth() >= 0)
        data->setLcdDepth(data->getLcdDepth() + 1);   // LCD: distance below the hit point
    EV_DEBUG << "Forwarding Data: " << data->getName() << " to face " << outFace << endl;
    send(data, "ndnOut", gateIndex);
}

void NDNProcessor::sendNack(InterestPacket *interest, int outFace, NackReason reason) {
    emit(nackSentSignal, 1L);

    NackPacket *nack = new NackPacket();
    nack->setName(interest->getName());
    nack->setNonce(interest->getNonce());
    nack->setReason(reason);

    std::string reasonText;
    switch(reason) {
        case NACK_CONGESTION: reasonText = "Congestion"; break;
        case NACK_DUPLICATE: reasonText = "Duplicate"; break;
        case NACK_NO_ROUTE: reasonText = "NoRoute"; break;
        default: reasonText = "Unknown"; break;
    }
    nack->setReasonText(reasonText.c_str());

    int gateIndex = getGateForFace(outFace);
    if (gateIndex != -1 && gateIndex < gateSize("ndnOut")) {
        send(nack, "ndnOut", gateIndex);
    } else {
        delete nack;
    }
}

int NDNProcessor::registerFace(int gateIndex) {
    int faceId = nextFaceId++;
    faceToGate[faceId] = gateIndex;
    gateToFace[gateIndex] = faceId;

    // DEBUG: Log face registration with gate destination
    cGate *gate = this->gate("ndnOut", gateIndex);
    std::string destination = "UNKNOWN";
    if (gate->isConnected()) {
        cModule *dest = gate->getNextGate()->getOwnerModule();
        destination = dest->getFullPath();
    }

    EV_DEBUG << "*** REGISTERED FACE " << faceId << " for gate " << gateIndex
            << " --> " << destination << " ***" << endl;

    return faceId;
}

int NDNProcessor::getFaceForGate(int gateIndex) {
    auto it = gateToFace.find(gateIndex);
    return (it != gateToFace.end()) ? it->second : -1;
}

int NDNProcessor::getGateForFace(int faceId) {
    auto it = faceToGate.find(faceId);
    return (it != faceToGate.end()) ? it->second : -1;
}

bool NDNProcessor::shouldCacheData(DataPacket *data) {
    if (!enableCaching || !data->isCacheable())
        return false;
    return true;
}

bool NDNProcessor::verifySignature(DataPacket *data) {
    // Simplified signature verification
    std::string sig = data->getSignature();
    return data->isSigned() && !sig.empty();
}

InterestPacket* NDNProcessor::createInterest(const std::string &name) {
    InterestPacket *interest = new InterestPacket();
    interest->setName(name.c_str());
    interest->setNonce(intuniform(0, INT_MAX));
    interest->setInterestLifetime(SimTime(4.0));
    interest->setTimestamp(simTime());
    interest->setHopCount(0);
    // mustBeFresh is set via selectors
    InterestSelector selectors;
    selectors.mustBeFresh = true;
    interest->setSelectors(selectors);
    interest->setPriority(0);  // Normal priority
    return interest;
}

DataPacket* NDNProcessor::createData(const std::string &name, const std::string &content) {
    DataPacket *data = new DataPacket();
    data->setName(name.c_str());
    data->setContent(content.c_str());
    data->setContentLength(content.length());
    data->setTimestamp(simTime());
    data->setFreshnessPeriod(10.0);  // 10 seconds freshness
    data->setNominalFreshness(10.0);
    data->setIsCacheable(true);
    data->setIsSigned(true);
    std::string sig = "SIG_" + std::to_string(nodeId) + "_" + std::to_string(simTime().inUnit(SIMTIME_MS));
    data->setSignature(sig.c_str());
    data->setSignerId(nodeIdentifier.c_str());
    data->setSignatureTime(simTime());
    return data;
}

} // namespace ndniot
