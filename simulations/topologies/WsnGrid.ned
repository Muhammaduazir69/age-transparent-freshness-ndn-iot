//
// NDN-IoT-RL - WSN grid topology (thesis 3.8.1: agriculture/environmental
// monitoring, 4x4 .. 20x20). Real INET IoT devices: SensorNode with IEEE
// 802.15.4 narrowband radio and (via ini) SimpleEpEnergyStorage battery.
// NDN runs as a UDP overlay app on every node; adjacency is the 4-connected
// lattice (configurator topology="grid").
//
package ndniot.simulations.topologies;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.SensorNode;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import ndniot.apps.NdnStaticConfigurator;
import ndniot.rl.RlBridge;

network WsnGrid
{
    parameters:
        int cols = default(4);
        int rows = default(4);
        double spacing @unit(m) = default(30m);
        @display("bgb=,,white");

    submodules:
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=50,50");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,120");
        }
        rlBridge: RlBridge {
            @display("p=50,260");
        }
        ndnConfigurator: NdnStaticConfigurator {
            topology = "grid";
            gridCols = parent.cols;
            @display("p=50,190");
        }
        sensor[rows * cols]: SensorNode {
            @display("p=150,50");
        }
}
