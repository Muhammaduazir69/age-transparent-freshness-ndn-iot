//
// NDN-IoT-RL - Producer application (APP face).
// Serves Data for its produced prefix; models an IoT sensor whose readings
// carry a publisher-declared freshness lifetime.
//
package ndniot.apps;

simple NdnProducerApp like INdnAppLogic
{
    parameters:
        @class(ndniot::NdnProducerApp);
        bool enabled = default(true);
        string producedPrefix = default("");             // e.g. "/wsn/area0"; empty = pure consumer node
        double freshnessPeriod @unit(s) = default(60s);  // thesis TTL classes: 1s / 60s / 1h / 24h
        // Heterogeneous TTL classes. Empty = every object gets freshnessPeriod
        // (the uniform workload, under which "least fresh first" eviction
        // degenerates to FIFO because arrival order IS expiry order). A
        // comma-separated list assigns content i the class i % n, so a
        // freshness-aware replacement policy has something real to sort on.
        string freshnessClasses = default("");
        int payloadBytes @unit(B) = default(32B);        // sensor reading payload
        int dataByteLength @unit(B) = default(80B);      // on-wire NDN Data size
        double processingDelay @unit(s) = default(2ms);  // sensing/response latency

        @signal[interestsServed](type=long);
        @statistic[interestsServed](record=count);
        @display("i=block/sink");
    gates:
        output toNdn;
        input fromNdn;
}
