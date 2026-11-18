//
// NDN-IoT-RL - interface for local NDN application logic (APP face, face 0)
//
package ndniot.apps;

moduleinterface INdnAppLogic
{
    parameters:
        @display("i=block/app");
    gates:
        output toNdn;
        input fromNdn;
}
