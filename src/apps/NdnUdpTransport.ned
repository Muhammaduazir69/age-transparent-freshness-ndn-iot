//
// NDN-IoT-RL - UDP transport for the NDN overlay.
// fromNdn[i]/toNdn[i] pair = wire face i (NDN face i+1); each is bound to one
// UDP neighbor by the NdnStaticConfigurator.
//
package ndniot.apps;

simple NdnUdpTransport
{
    parameters:
        @class(ndniot::NdnUdpTransport);
        int localPort = default(5885);
        int destPort = default(5885);

        @signal[packetSent](type=inet::Packet);
        @signal[packetReceived](type=inet::Packet);
        @statistic[packetSent](title="NDN packets sent over UDP"; record=count,"sum(packetBytes)");
        @statistic[packetReceived](title="NDN packets received over UDP"; record=count,"sum(packetBytes)");
        @display("i=block/dispatch");
    gates:
        input socketIn;
        output socketOut;
        input fromNdn[];   // from NdnNode.ndnOut (wire faces)
        output toNdn[];    // to NdnNode.ndnIn (wire faces)
}
