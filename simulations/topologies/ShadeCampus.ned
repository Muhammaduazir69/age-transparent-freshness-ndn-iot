//
// NDN-IoT-SHADE - ShadeCampus
// Smart-campus sensing field: rows x cols IEEE 802.15.4 SensorNodes (INET),
// NDN as a UDP overlay on every node, 4-connected lattice adjacency, one
// sink/producer at a corner, and a network-wide RegimeController that drives
// the four operating regimes (stationary, popularity drift, cache pollution,
// flash crowd with short-lived data).
//
package ndniot.simulations.topologies;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.SensorNode;
import inet.physicallayer.wireless.ieee802154.packetlevel.Ieee802154NarrowbandScalarRadioMedium;
import ndniot.apps.NdnStaticConfigurator;
import ndniot.rl.RlBridge;
import ndniot.shade.RegimeController;

network ShadeCampus
{
    parameters:
        int cols = default(6);
        int rows = default(6);
        double spacing @unit(m) = default(30m);
        @display("bgb=330,250,white");

    submodules:
        radioMedium: Ieee802154NarrowbandScalarRadioMedium {
            @display("p=275,60");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=275,100");
        }
        rlBridge: RlBridge {
            @display("p=275,140");
        }
        ndnConfigurator: NdnStaticConfigurator {
            topology = "grid";
            gridCols = parent.cols;
            @display("p=275,180");
        }
        regimeController: RegimeController {
            @display("p=275,220");
        }
        sensor[rows * cols]: SensorNode {
            @display("p=150,50");
        }
}
