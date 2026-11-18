//
// NDN-IoT-RL - RL bridge (ZeroMQ + POSIX shared memory) to the Python trainer.
// Add exactly one instance named "rlBridge" at network level.
//
package ndniot.rl;

simple RlBridge
{
    parameters:
        @class(ndniot::RlBridge);
        string mode = default("off");     // off | random | train | infer
        string endpoint = default("tcp://127.0.0.1:5555");  // env MAACO_PORT overrides port
        string shmName = default("/maaco");                 // env MAACO_SHM overrides
        int decisionTimeoutMs = default(30000);
        @display("i=block/control");
}
