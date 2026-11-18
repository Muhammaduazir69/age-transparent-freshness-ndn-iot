#include "NdnUdpTransport.h"

#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/cPacketChunk.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressTag_m.h"

using namespace inet;

namespace ndniot {

Define_Module(NdnUdpTransport);

void NdnUdpTransport::initialize(int stage) {
    ApplicationBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        localPort = par("localPort");
        destPort = par("destPort");
        numWireFaces = gateSize("fromNdn");
        packetSentSignal_ = registerSignal("packetSent");
        packetReceivedSignal_ = registerSignal("packetReceived");
    }
}

void NdnUdpTransport::handleStartOperation(LifecycleOperation *operation) {
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    socket.setCallback(this);
    socketReady = true;
    EV_DEBUG << "NdnUdpTransport up on port " << localPort << endl;
}

void NdnUdpTransport::handleStopOperation(LifecycleOperation *operation) {
    socketReady = false;
    socket.close();
}

void NdnUdpTransport::handleCrashOperation(LifecycleOperation *operation) {
    socketReady = false;
    socket.destroy();
}

void NdnUdpTransport::handleMessageWhenUp(cMessage *msg) {
    if (msg->arrivedOn("fromNdn")) {
        // NDN stack wants to send this packet out on wire face = gate index
        int wireIdx = msg->getArrivalGate()->getIndex();
        NdnPacket *ndn = dynamic_cast<NdnPacket *>(msg);
        if (!ndn) { delete msg; return; }
        sendToNeighbor(ndn, wireIdx);
    }
    else if (socket.belongsToSocket(msg)) {
        socket.processMessage(msg);  // dispatches to socketDataArrived()
    }
    else {
        delete msg;
    }
}

void NdnUdpTransport::sendToNeighbor(NdnPacket *ndn, int wireFaceIndex) {
    auto it = faceToAddr.find(wireFaceIndex);
    if (!socketReady || it == faceToAddr.end()) {
        EV_WARN << "No neighbor bound to wire face " << wireFaceIndex << ", dropping "
                << ndn->getName() << endl;
        delete ndn;
        return;
    }
    if (ndn->getByteLength() == 0)
        ndn->setByteLength(64);  // safety: never send zero-length chunks

    auto *pkt = new Packet(ndn->getClassName());
    ndnBytesSent += ndn->getByteLength();
    ndnPacketsSent++;
    pkt->insertAtBack(makeShared<cPacketChunk>(ndn));  // chunk takes ownership
    emit(packetSentSignal_, pkt);
    socket.sendTo(pkt, it->second, destPort);
}

void NdnUdpTransport::socketDataArrived(UdpSocket *sock, Packet *packet) {
    auto chunk = packet->popAtFront<cPacketChunk>();
    cPacket *inner = chunk->getPacket()->dup();
    NdnPacket *ndn = dynamic_cast<NdnPacket *>(inner);
    if (!ndn) {
        delete inner;
        delete packet;
        return;
    }

    // Identify sender -> wire face (needed so the forwarder knows the in-face)
    L3Address srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
    int wireIdx = faceForAddress(srcAddr);
    emit(packetReceivedSignal_, packet);
    delete packet;

    ndnBytesReceived += ndn->getByteLength();
    ndnPacketsReceived++;

    if (wireIdx < 0) {
        EV_WARN << "NDN packet from unknown neighbor " << srcAddr << ", dropping" << endl;
        delete ndn;
        return;
    }
    // Deliver into the NDN stack on the matching wire face gate
    send(ndn, "toNdn", wireIdx);
}

void NdnUdpTransport::socketErrorArrived(UdpSocket *sock, Indication *indication) {
    EV_WARN << "UDP socket error: " << indication->getName() << endl;
    delete indication;
}

void NdnUdpTransport::socketClosed(UdpSocket *sock) {
}

int NdnUdpTransport::addNeighbor(const L3Address &sendAddr,
                                 const std::vector<L3Address> &allAddrs) {
    Enter_Method_Silent();
    int idx = (int)faceToAddr.size();
    if (idx >= numWireFaces) {
        EV_ERROR << "All " << numWireFaces << " wire faces in use; increase maxWireFaces" << endl;
        return -1;
    }
    faceToAddr[idx] = sendAddr;
    for (const auto &a : allAddrs)
        addrToFace[a.str()] = idx;
    addrToFace[sendAddr.str()] = idx;
    EV_DEBUG << "Neighbor " << sendAddr << " (+" << allAddrs.size()
            << " aliases) bound to wire face " << idx << endl;
    return idx;
}

int NdnUdpTransport::faceForAddress(const L3Address &addr) const {
    auto it = addrToFace.find(addr.str());
    return it != addrToFace.end() ? it->second : -1;
}

void NdnUdpTransport::finish() {
    recordScalar("ndnBytesSent", ndnBytesSent);
    recordScalar("ndnBytesReceived", ndnBytesReceived);
    recordScalar("ndnPacketsSent", ndnPacketsSent);
    recordScalar("ndnPacketsReceived", ndnPacketsReceived);
    ApplicationBase::finish();
}

} // namespace ndniot
