//
// NDN-IoT-RL - NdnUdpTransport
// Bridges the plain-OMNeT++ NDN stack (NdnNode) to INET UDP.
//
// Wire model: hop-by-hop unicast UDP between direct radio neighbors.
// Each neighbor is a logical NDN face: wire face i (transport gate index i)
// maps to NDN face i+1 (face 0 is the local application logic).
// NDN packets are wrapped in inet::Packet via cPacketChunk (recipe proven in
// the NDNPreEncryption project) and sent to the neighbor's UDP port.
//

#ifndef __NDNIOT_NDNUDPTRANSPORT_H
#define __NDNIOT_NDNUDPTRANSPORT_H

#include <omnetpp.h>
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "ndn/packets/NdnPackets_m.h"

using namespace omnetpp;

namespace ndniot {

class NdnUdpTransport : public inet::ApplicationBase, public inet::UdpSocket::ICallback {
protected:
    inet::UdpSocket socket;
    int localPort = 5885;
    int destPort = 5885;

    // wire face index (gate index) <-> neighbor address
    std::map<int, inet::L3Address> faceToAddr;
    std::map<std::string, int> addrToFace;  // L3Address::str() -> face index

    int numWireFaces = 0;
    bool socketReady = false;

    // stats
    long ndnBytesSent = 0, ndnBytesReceived = 0;
    long ndnPacketsSent = 0, ndnPacketsReceived = 0;
    simsignal_t packetSentSignal_;
    simsignal_t packetReceivedSignal_;

protected:
    virtual int numInitStages() const override { return inet::NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;

    virtual void handleStartOperation(inet::LifecycleOperation *operation) override;
    virtual void handleStopOperation(inet::LifecycleOperation *operation) override;
    virtual void handleCrashOperation(inet::LifecycleOperation *operation) override;

    // UdpSocket::ICallback
    virtual void socketDataArrived(inet::UdpSocket *socket, inet::Packet *packet) override;
    virtual void socketErrorArrived(inet::UdpSocket *socket, inet::Indication *indication) override;
    virtual void socketClosed(inet::UdpSocket *socket) override;

    void sendToNeighbor(NdnPacket *ndn, int wireFaceIndex);

public:
    // Called by NdnStaticConfigurator during topology setup.
    // sendAddr: address this node sends to; allAddrs: every address of the
    // neighbor host (matched against the source of incoming packets, so
    // multi-homed neighbors resolve to the same face).
    // Returns the assigned wire face index, or -1 if all faces are in use.
    int addNeighbor(const inet::L3Address &sendAddr,
                    const std::vector<inet::L3Address> &allAddrs);
    int getNumNeighbors() const { return (int)faceToAddr.size(); }
    int faceForAddress(const inet::L3Address &addr) const;
};

} // namespace ndniot

#endif
