//
// NDN-IoT-RL - interface for the Content Store (allows RlCS swap).
// Declares every parameter that NdnNode assigns on its cs submodule.
//
package ndniot.ndn.tables;

moduleinterface INdnCs
{
    parameters:
        int maxSize;
        int maxEntries;
        bool enabled;
        string replacementPolicy;
    gates:
        input in @loose;            output out @loose;
        input processorIn @loose;   output processorOut @loose;
}
