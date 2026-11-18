//
// NDN-IoT-RL - static topology/route configurator for the NDN overlay.
// Place one instance at network level. commRange must match (or conservatively
// under-estimate) the actual radio range of the PHY configuration.
//
package ndniot.apps;

simple NdnStaticConfigurator
{
    parameters:
        @class(ndniot::NdnStaticConfigurator);
        double commRange @unit(m) = default(60m);
        string topology = default("range");   // "range" | "grid" (4-connected) | "tree" (heap-indexed)
        int gridCols = default(4);
        int treeFanout = default(2);
        string extraLinks = default("");      // "hostA=hostB;..." overlay links beyond the base topology
        @display("i=block/cogwheel");
}
