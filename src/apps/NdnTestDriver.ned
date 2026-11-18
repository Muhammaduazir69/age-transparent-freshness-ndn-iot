//
// NDN-IoT-RL - Standalone smoke-test driver (M1). Not used in INET scenarios.
//
package ndniot.apps;

simple NdnTestDriver
{
    parameters:
        @class(ndniot::NdnTestDriver);
        string role = default("consumer");        // "consumer" | "producer"
        string prefix = default("/iot/temp");
        double requestInterval @unit(s) = default(1s);
        int numRequests = default(10);
        double freshnessPeriod @unit(s) = default(10s);
        double startTime @unit(s) = default(0.1s);

        @signal[retrievalLatency](type=simtime_t);
        @statistic[retrievalLatency](record=stats,vector);
        @display("i=block/app");
    gates:
        input fromNdn;
        output toNdn;
}
