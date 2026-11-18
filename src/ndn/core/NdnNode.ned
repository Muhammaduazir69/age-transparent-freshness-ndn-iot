//
// NDN-IoT-RL - Base NDN Node Definition
// Compound module with NDNProcessor + PIT/FIB/CS
//

package ndniot.ndn.core;

import ndniot.ndn.core.INdnProcessor;
import ndniot.ndn.tables.PIT;
import ndniot.ndn.tables.FIB;
import ndniot.ndn.tables.INdnCs;

module NdnNode
{
    parameters:
        // Node identification
        int nodeId;
        string nodeType = default("ndn");

        // NDN configuration
        int pitMaxSize = default(1000);
        int fibMaxSize = default(500);
        int csMaxSize = default(100);
        int csMaxEntries = default(0);   // item-count cap (0 = unlimited)
        string forwardingStrategy = default("BestRoute");

        // Cache configuration
        string cacheReplacementPolicy = default("LRU");
        bool enableCaching = default(true);

        // Security configuration
        bool enableSignatureVerification = default(false);
        double signatureVerificationDelay @unit(ms) = default(1ms);

        // IDS integration flag (passed from parent)
        bool hasIDSModule = default(false);

        // swap-in points for the RL-aware variants
        string processorTypename = default("ndniot.ndn.core.NDNProcessor");
        string csTypename = default("ndniot.ndn.tables.CS");

        @display("i=abstract/router;bgb=700,500");

    gates:
        input ndnIn[] @loose;
        output ndnOut[] @loose;

    submodules:
        processor: <processorTypename> like INdnProcessor {
            nodeId = parent.nodeId;
            nodeType = parent.nodeType;
            enableCaching = parent.enableCaching;
            enableSignatureVerification = parent.enableSignatureVerification;
            signatureVerificationDelay = parent.signatureVerificationDelay;
            forwardingStrategy = parent.forwardingStrategy;
            @display("p=350,250");
        }

        pit: PIT {
            maxSize = parent.pitMaxSize;
            @display("p=200,150");
        }

        fib: FIB {
            maxSize = parent.fibMaxSize;
            @display("p=350,150");
        }

        cs: <csTypename> like INdnCs {
            maxSize = parent.csMaxSize;
            maxEntries = parent.csMaxEntries;
            enabled = parent.enableCaching;
            replacementPolicy = parent.cacheReplacementPolicy;
            @display("p=500,150");
        }

    connections allowunconnected:
        // Connect processor to PIT/FIB/CS
        processor.pitOut --> pit.processorIn;
        pit.processorOut --> processor.pitIn;

        processor.fibOut --> fib.processorIn;
        fib.processorOut --> processor.fibIn;

        processor.csOut --> cs.processorIn;
        cs.processorOut --> processor.csIn;

        // Connect external gates to processor
        for i=0..sizeof(ndnIn)-1 {
            ndnIn[i] --> processor.ndnIn++;
        }

        for i=0..sizeof(ndnOut)-1 {
            processor.ndnOut++ --> ndnOut[i];
        }
}
