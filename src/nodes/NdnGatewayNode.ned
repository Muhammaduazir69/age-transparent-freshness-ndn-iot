//
// NDN-IoT-RL - NdnGatewayNode
// Dual-homed IoT gateway: a Simu5G NR UE (cellular NIC toward gNodeB) that
// additionally carries an IEEE 802.15.4 narrowband interface on the WSN
// plane. The added NIC is wired into the network-layer MessageDispatcher
// exactly like LteUe wires its optional eth[] interfaces.
//
// The NDN overlay app on this node bridges the two tiers: wire faces toward
// WSN sensors go over wsnNic (on-link), the face toward the cloud repo goes
// over the NR NIC through gNB/UPF (IP-routed) — bound by NdnStaticConfigurator
// via its extraLinks parameter.
//
package ndniot.nodes;

import simu5g.nodes.NrUe;
import inet.linklayer.ieee802154.Ieee802154NarrowbandInterface;

module NdnGatewayNode extends NrUe
{
    parameters:
        @display("i=device/accesspoint");
        wsnNic.interfaceTableModule = absPath(".interfaceTable");
        wsnNic.energySourceModule = "";   // gateway assumed mains-powered
        wsnNic.**.mobilityModule = absPath(".mobility");
    submodules:
        wsnNic: Ieee802154NarrowbandInterface {
            @display("p=200,589");
        }
    connections:
        wsnNic.upperLayerOut --> nl.in++;
        wsnNic.upperLayerIn <-- nl.out++;
}
