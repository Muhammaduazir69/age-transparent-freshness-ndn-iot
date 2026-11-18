//
// NDN-IoT-RL - NdnOverlayApp
// The complete NDN overlay packaged as an INET application (like IApp), so it
// drops into any INET/Simu5G host's app[] vector (SensorNode, NrUe,
// StandardHost) without touching the host NED.
//
// Face layout (NDN face id = ndnIn/ndnOut gate index, registered in
// connection order):
//   face 0        = consumer logic (Interests originate here)
//   face 1        = producer logic (Interests for producedPrefix served here)
//   face 2..N+1   = wire faces (UDP unicast to radio neighbors; wire face i
//                   on the transport = NDN face i+2)
// The NdnStaticConfigurator binds neighbors to wire faces and installs FIB
// routes accordingly.
//
package ndniot.apps;

import inet.applications.contract.IApp;
import ndniot.ndn.core.NdnNode;
import ndniot.energy.CacheEpEnergyConsumer;

module NdnOverlayApp like IApp
{
    parameters:
        int nodeId = default(0);
        int maxWireFaces = default(6);        // >= node degree in the topology

        // consumer knobs (forwarded)
        bool isConsumer = default(true);
        string contentPrefix = default("/wsn/area0");
        double zipfAlpha = default(0.8);
        double requestRate = default(1.0);
        int catalogSize = default(50);
        int producerCount = default(0);       // >1: distributed sensing field
        double validityScale = default(1.0);  // app lifetime / declared lifetime

        // producer knobs (forwarded)
        string producedPrefix = default("");   // empty = not a producer
        double freshnessPeriod @unit(s) = default(60s);
        string freshnessClasses = default("");  // heterogeneous TTL classes (see NdnProducerApp)

        // cache knobs (forwarded to NdnNode)
        int csMaxEntries = default(100);
        string cacheReplacementPolicy = default("LRU");
        bool enableCaching = default(true);
        bool cacheEnergyAccounting = default(false);  // draw CS op energy from host battery

        @display("i=block/layer");
    gates:
        input socketIn;
        output socketOut;

    submodules:
        consumer: NdnConsumerApp {
            enabled = parent.isConsumer;
            contentPrefix = parent.contentPrefix;
            zipfAlpha = parent.zipfAlpha;
            requestRate = parent.requestRate;
            catalogSize = parent.catalogSize;
            producerCount = parent.producerCount;
            // default(): a plain NED assignment is final and would silently
            // swallow an ini assignment made at the consumer instead of the app.
            validityScale = default(parent.validityScale);
            @display("p=80,60");
        }
        producer: NdnProducerApp {
            producedPrefix = parent.producedPrefix;
            freshnessPeriod = parent.freshnessPeriod;
            freshnessClasses = parent.freshnessClasses;
            @display("p=200,60");
        }
        ndn: NdnNode {
            nodeId = parent.nodeId;
            csMaxEntries = parent.csMaxEntries;
            cacheReplacementPolicy = parent.cacheReplacementPolicy;
            enableCaching = parent.enableCaching;
            @display("p=140,150");
        }
        transport: NdnUdpTransport {
            @display("p=140,240");
        }
        cacheEnergy: CacheEpEnergyConsumer if cacheEnergyAccounting {
            @display("p=260,240");
        }

    connections:
        // face 0: consumer
        consumer.toNdn --> ndn.ndnIn++;
        ndn.ndnOut++ --> consumer.fromNdn;
        // face 1: producer
        producer.toNdn --> ndn.ndnIn++;
        ndn.ndnOut++ --> producer.fromNdn;
        // faces 2..maxWireFaces+1: UDP wire faces
        for i=0..maxWireFaces-1 {
            transport.toNdn++ --> ndn.ndnIn++;
            ndn.ndnOut++ --> transport.fromNdn++;
        }
        // INET socket plumbing
        socketIn --> transport.socketIn;
        transport.socketOut --> socketOut;
}
