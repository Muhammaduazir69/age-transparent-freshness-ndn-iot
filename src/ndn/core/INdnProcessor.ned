//
// NDN-IoT-RL - interface for the NDN forwarder (allows RlNDNProcessor swap).
// Declares every parameter that NdnNode assigns on its processor submodule
// (NED interface rule: assignments must resolve against the interface).
//
package ndniot.ndn.core;

moduleinterface INdnProcessor
{
    parameters:
        int nodeId;
        string nodeType;
        bool enableCaching;
        bool enableSignatureVerification;
        double signatureVerificationDelay @unit(ms);
        string forwardingStrategy;
    gates:
        input pitIn;   output pitOut;
        input fibIn;   output fibOut;
        input csIn;    output csOut;
        input ndnIn[]; output ndnOut[];
}
