//
// NDN-IoT-RL - NdnStaticConfigurator
// Network-level module that, at t=0 (after all initialize() stages, so IP
// addresses and mobility positions are final):
//   1. discovers every NdnOverlayApp in the network,
//   2. derives radio-neighbor adjacency from node positions and commRange,
//   3. binds each neighbor pair to UDP wire faces on both transports,
//   4. runs BFS from every producer prefix and installs one shortest-path
//      FIB route per (node, prefix): prefix -> face toward the producer.
// Producers additionally get prefix -> face 1 (their local producer app).
//

#ifndef __NDNIOT_NDNSTATICCONFIGURATOR_H
#define __NDNIOT_NDNSTATICCONFIGURATOR_H

#include <omnetpp.h>
#include "inet/networklayer/common/L3Address.h"

using namespace omnetpp;

namespace ndniot {

class NdnUdpTransport;

class NdnStaticConfigurator : public cSimpleModule {
protected:
    double commRange = 60;
    std::string topology = "range";   // "range" | "grid" | "tree"
    std::string extraLinks;           // "hostA=hostB;..." adjacency beyond the base topology
    int gridCols = 0;                 // for topology="grid"
    int treeFanout = 2;               // for topology="tree" (heap indexing over discovery order)
    cMessage *setupTimer = nullptr;

    struct IfaceAddr {
        inet::L3Address addr;
        uint32_t subnet = 0;   // network address (addr & netmask), IPv4 only
        uint32_t mask = 0;
    };
    struct AppInfo {
        cModule *app = nullptr;          // NdnOverlayApp
        cModule *host = nullptr;         // SensorNode / NrUe / StandardHost
        NdnUdpTransport *transport = nullptr;
        cModule *fib = nullptr;
        inet::L3Address addr;            // primary (resolver) address
        std::vector<IfaceAddr> ifaceAddrs;
        double x = 0, y = 0;
        std::string producedPrefix;
        // neighborAppIndex -> NDN face id on THIS node pointing at that neighbor
        std::map<int, int> faceToNeighbor;
    };
    std::vector<AppInfo> apps;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    void discoverApps();
    void buildAdjacency();
    bool areAdjacent(int i, int j) const;
    void bindPair(int i, int j);
    inet::L3Address pickSendAddress(int from, int to) const;
    void applyExtraLinks();
    void installRoutes();
    // Give each node the icon and label of the role it was actually assigned,
    // so a Qtenv capture shows the experiment rather than 16 identical motes.
    void annotateRoles();

public:
    virtual ~NdnStaticConfigurator() { cancelAndDelete(setupTimer); }
    static const int APP_FACES = 2;  // face 0 consumer, face 1 producer
};

} // namespace ndniot

#endif
