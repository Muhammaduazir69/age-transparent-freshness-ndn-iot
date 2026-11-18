#include "NdnStaticConfigurator.h"
#include "NdnUdpTransport.h"
#include "ndn/tables/FIB.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/mobility/contract/IMobility.h"

#include <queue>
#include <sstream>

using namespace inet;

namespace ndniot {

Define_Module(NdnStaticConfigurator);

void NdnStaticConfigurator::initialize() {
    commRange = par("commRange");
    topology = par("topology").stdstringValue();
    gridCols = par("gridCols");
    treeFanout = par("treeFanout");
    extraLinks = par("extraLinks").stdstringValue();
    // run after every module finished all init stages (addresses + positions final)
    annotateRoles();   // icons must be right at t=0, before anything runs
    setupTimer = new cMessage("ndnTopologySetup");
    scheduleAt(SIMTIME_ZERO, setupTimer);
}

void NdnStaticConfigurator::handleMessage(cMessage *msg) {
    ASSERT(msg == setupTimer);
    discoverApps();
    buildAdjacency();
    installRoutes();
    cancelAndDelete(setupTimer);
    setupTimer = nullptr;
}

void NdnStaticConfigurator::discoverApps() {
    cModule *network = getSystemModule();
    for (cModule::SubmoduleIterator hostIt(network); !hostIt.end(); ++hostIt) {
        cModule *host = *hostIt;
        // find NdnOverlayApp instances among the host's app[] vector (any depth-1 submodule)
        for (cModule::SubmoduleIterator appIt(host); !appIt.end(); ++appIt) {
            cModule *app = *appIt;
            if (strcmp(app->getComponentType()->getName(), "NdnOverlayApp") != 0)
                continue;

            AppInfo info;
            info.app = app;
            info.host = host;
            info.transport = check_and_cast<NdnUdpTransport *>(app->getSubmodule("transport"));
            info.fib = app->getSubmodule("ndn")->getSubmodule("fib");
            info.producedPrefix = app->getSubmodule("producer")->par("producedPrefix").stdstringValue();

            info.addr = L3AddressResolver().addressOf(host);
            // collect every IPv4 interface address (multi-homed gateways)
            if (auto *ift = dynamic_cast<IInterfaceTable *>(host->getSubmodule("interfaceTable"))) {
                for (int k = 0; k < ift->getNumInterfaces(); k++) {
                    NetworkInterface *ni = ift->getInterface(k);
                    if (ni->isLoopback())
                        continue;
                    Ipv4Address a = ni->getIpv4Address();
                    if (a.isUnspecified())
                        continue;
                    IfaceAddr ia;
                    ia.addr = L3Address(a);
                    ia.mask = ni->getIpv4Netmask().getInt();
                    ia.subnet = a.getInt() & ia.mask;
                    info.ifaceAddrs.push_back(ia);
                }
            }

            cModule *mob = host->getSubmodule("mobility");
            if (mob) {
                auto *mobility = check_and_cast<IMobility *>(mob);
                auto pos = mobility->getCurrentPosition();
                info.x = pos.x;
                info.y = pos.y;
            }
            apps.push_back(info);
        }
    }
    EV_INFO << "NdnStaticConfigurator: discovered " << apps.size() << " overlay apps" << endl;
    if (apps.empty())
        throw cRuntimeError("NdnStaticConfigurator found no NdnOverlayApp instances");
}

void NdnStaticConfigurator::annotateRoles() {
    // Visualization only: this touches display strings, never state that any
    // measurement depends on.  Every node is the same SensorNode type, so
    // without this a capture shows sixteen identical icons and the reader
    // cannot see which node produces, which consume, or which only forward.
    cModule *network = getSystemModule();
    for (cModule::SubmoduleIterator hostIt(network); !hostIt.end(); ++hostIt) {
        cModule *host = *hostIt;
        cModule *app = nullptr;
        for (cModule::SubmoduleIterator appIt(host); !appIt.end(); ++appIt) {
            if (strcmp((*appIt)->getComponentType()->getName(), "NdnOverlayApp") == 0) {
                app = *appIt;
                break;
            }
        }
        if (!app)
            continue;   // radioMedium, configurator, rlBridge: not NDN nodes

        const std::string producedPrefix = app->par("producedPrefix").stdstringValue();
        const bool isProducer = !producedPrefix.empty();
        const bool isConsumer = app->par("isConsumer").boolValue();

        const char *icon;
        const char *tint;
        std::string label;
        if (isProducer) {
            // publishes measurements for a prefix: a sensing instrument
            icon = "misc/thermometer";
            tint = "#1a8f63";
            label = "producer " + producedPrefix;
        }
        else if (isConsumer) {
            icon = "misc/sensor2";
            tint = "#2a78d6";
            label = "consumer + cache";
        }
        else {
            icon = "misc/sensor3";
            tint = "#9aa5b1";
            label = "cache only";
        }

        cDisplayString &ds = host->getDisplayString();
        ds.setTagArg("i", 0, icon);
        ds.setTagArg("i", 1, tint);
        ds.setTagArg("i", 2, "40");     // tint strength, keeps the icon readable
        // Label only what is distinctive.  In a 16-node grid fifteen identical
        // "consumer" labels overlap into noise and hide the one node that is
        // different, so the majority role is carried by the icon and its tint
        // alone and only the exceptions are named.
        if (!isConsumer || isProducer) {
            ds.setTagArg("t", 0, label.c_str());
            ds.setTagArg("t", 1, "t");      // above the icon
            ds.setTagArg("t", 2, tint);
        }
    }
}

void NdnStaticConfigurator::buildAdjacency() {
    long links = 0;
    for (size_t i = 0; i < apps.size(); i++) {
        for (size_t j = i + 1; j < apps.size(); j++) {
            if (!areAdjacent((int)i, (int)j))
                continue;
            bindPair((int)i, (int)j);
            links++;
        }
    }
    applyExtraLinks();
    // one-shot audit dump (EV_WARN: visible at default log level)
    for (size_t i = 0; i < apps.size(); i++) {
        std::stringstream ss;
        ss << apps[i].host->getFullName() << " (" << (int)apps[i].x << "," << (int)apps[i].y << ") [" << apps[i].addr << "]:";
        for (const auto &kv : apps[i].faceToNeighbor)
            ss << " face" << kv.second << "->" << apps[kv.first].host->getFullName()
               << "(" << pickSendAddress((int)i, kv.first) << ")";
        EV_WARN << "NDNTOPO " << ss.str() << endl;
    }
    EV_INFO << "NdnStaticConfigurator: " << links << " base links bound" << endl;
    recordScalar("ndnOverlayLinks", links);
}

void NdnStaticConfigurator::bindPair(int i, int j) {
    if (apps[i].faceToNeighbor.count(j))
        return;  // already bound
    std::vector<L3Address> addrsI, addrsJ;
    for (const auto &ia : apps[i].ifaceAddrs) addrsI.push_back(ia.addr);
    for (const auto &ia : apps[j].ifaceAddrs) addrsJ.push_back(ia.addr);
    int wireI = apps[i].transport->addNeighbor(pickSendAddress(i, j), addrsJ);
    int wireJ = apps[j].transport->addNeighbor(pickSendAddress(j, i), addrsI);
    if (wireI < 0 || wireJ < 0)
        throw cRuntimeError("Out of wire faces on %s or %s (increase maxWireFaces)",
                            apps[i].host->getFullName(), apps[j].host->getFullName());
    apps[i].faceToNeighbor[j] = wireI + APP_FACES;
    apps[j].faceToNeighbor[i] = wireJ + APP_FACES;
}

inet::L3Address NdnStaticConfigurator::pickSendAddress(int from, int to) const {
    // prefer a 'to' address on a subnet shared with 'from' (on-link unicast);
    // otherwise the resolver's primary address (IP-routed path, e.g. via 5G core)
    for (const auto &ta : apps[to].ifaceAddrs)
        for (const auto &fa : apps[from].ifaceAddrs)
            if (ta.subnet == fa.subnet && ta.mask == fa.mask && ta.mask != 0)
                return ta.addr;
    return apps[to].addr;
}

void NdnStaticConfigurator::applyExtraLinks() {
    if (extraLinks.empty())
        return;
    std::map<cModule *, int> hostToIdx;
    for (size_t k = 0; k < apps.size(); k++)
        hostToIdx[apps[k].host] = (int)k;

    std::stringstream ss(extraLinks);
    std::string item;
    while (std::getline(ss, item, ';')) {
        if (item.empty()) continue;
        size_t eq = item.find('=');
        if (eq == std::string::npos)
            throw cRuntimeError("extraLinks entry '%s' is not of the form hostA=hostB", item.c_str());
        std::string na = item.substr(0, eq), nb = item.substr(eq + 1);
        cModule *ha = getSystemModule()->getModuleByPath(na.c_str());
        cModule *hb = getSystemModule()->getModuleByPath(nb.c_str());
        if (!ha || !hb || !hostToIdx.count(ha) || !hostToIdx.count(hb))
            throw cRuntimeError("extraLinks: host '%s' or '%s' not found / has no NdnOverlayApp",
                                na.c_str(), nb.c_str());
        bindPair(hostToIdx[ha], hostToIdx[hb]);
        EV_INFO << "extra overlay link: " << na << " <-> " << nb << endl;
    }
}

bool NdnStaticConfigurator::areAdjacent(int i, int j) const {
    if (topology == "grid") {
        // 4-connected lattice over discovery order (row-major, gridCols wide)
        int ri = i / gridCols, ci = i % gridCols;
        int rj = j / gridCols, cj = j % gridCols;
        return std::abs(ri - rj) + std::abs(ci - cj) == 1;
    }
    if (topology == "tree") {
        // heap indexing: children of k are k*f+1 .. k*f+f
        int lo = std::min(i, j), hi = std::max(i, j);
        return hi >= lo * treeFanout + 1 && hi <= lo * treeFanout + treeFanout;
    }
    // default: geometric range
    double dx = apps[i].x - apps[j].x, dy = apps[i].y - apps[j].y;
    return std::sqrt(dx * dx + dy * dy) <= commRange;
}

void NdnStaticConfigurator::installRoutes() {
    // Topological depth of every node, recorded so that the analysis uses the
    // measured hop distance to the publisher rather than a proxy. With several
    // publishers, depthSum/depthCnt is the mean over them and depthMin the
    // nearest one.
    std::vector<long> depthSum(apps.size(), 0), depthCnt(apps.size(), 0);
    std::vector<long> depthMin(apps.size(), -1);
    // BFS from each producer; install prefix -> next-hop-face at every node
    for (size_t p = 0; p < apps.size(); p++) {
        const std::string &prefix = apps[p].producedPrefix;
        if (prefix.empty())
            continue;

        // producer's own FIB: prefix -> producer app face (face 1)
        check_and_cast<FIB *>(apps[p].fib)->addRoute(prefix, 1, 0);

        // BFS over adjacency
        std::vector<int> dist(apps.size(), -1);
        std::vector<int> parent(apps.size(), -1);
        std::queue<int> q;
        dist[p] = 0;
        q.push((int)p);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (const auto &kv : apps[u].faceToNeighbor) {
                int v = kv.first;
                if (dist[v] < 0) {
                    dist[v] = dist[u] + 1;
                    parent[v] = u;
                    q.push(v);
                }
            }
        }
        for (size_t n = 0; n < apps.size(); n++) {
            if (n == p || dist[n] < 0)
                continue;  // unreachable or the producer itself
            int towards = parent[n];  // next hop toward producer along BFS tree
            int face = apps[n].faceToNeighbor.at(towards);
            check_and_cast<FIB *>(apps[n].fib)->addRoute(prefix, face, dist[n]);
        }
        for (size_t n = 0; n < apps.size(); n++) {
            if (dist[n] < 0)
                continue;
            depthSum[n] += dist[n];
            depthCnt[n]++;
            if (depthMin[n] < 0 || dist[n] < depthMin[n])
                depthMin[n] = dist[n];
        }
        EV_INFO << "Routes installed for prefix " << prefix
                << " (producer: " << apps[p].host->getFullName() << ")" << endl;
    }
    for (size_t n = 0; n < apps.size(); n++) {
        if (depthCnt[n] <= 0)
            continue;
        recordScalar(("depthMean_n" + std::to_string(n)).c_str(),
                     (double)depthSum[n] / (double)depthCnt[n]);
        recordScalar(("depthMin_n" + std::to_string(n)).c_str(), (double)depthMin[n]);
    }
}

} // namespace ndniot
