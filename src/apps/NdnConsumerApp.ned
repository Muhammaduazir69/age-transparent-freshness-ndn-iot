//
// NDN-IoT-RL - Consumer application (APP face).
// Issues Interests with Zipf-distributed content popularity over a catalog,
// measures retrieval latency, hop count, freshness satisfaction, staleness.
//
package ndniot.apps;

simple NdnConsumerApp like INdnAppLogic
{
    parameters:
        @class(ndniot::NdnConsumerApp);
        bool enabled = default(true);
        string contentPrefix = default("/wsn/area0");  // catalog prefix requested
        int catalogSize = default(50);                  // items per prefix
        double zipfAlpha = default(0.8);                // Zipf skewness (thesis: 0.6-1.2)
        double requestRate = default(1.0);              // Interests per second (Poisson)
        double startTime @unit(s) = default(5s);
        double stopTime @unit(s) = default(-1s);        // -1 = run to sim end
        double interestLifetime @unit(s) = default(4s);
        int interestByteLength @unit(B) = default(40B);
        bool mustBeFresh = default(false);
        double ratePeriod @unit(s) = default(0s);       // >0: square-wave peak/idle rate (iCache dynamics)
        double peakRateMultiplier = default(1.0);
        double versionInterval @unit(s) = default(0s);
        int nameSpace = default(0);       // SHADE: size of the item name space (0 = catalogSize)
        // Distributed sensing field: when > 1, every reading is published by
        // one of `producerCount` sensors and its name carries that sensor's
        // component, contentPrefix/n<p>/item<k>. Requests still follow the
        // Zipf ranking over the whole catalog, so a node's store holds copies
        // fetched over a range of path lengths. 0 or 1 keeps the single
        // producer naming, contentPrefix/item<k>.
        int producerCount = default(0);
        // The lifetime the application needs, as a multiple of the lifetime
        // the publisher declared to the network. 1.0 means they coincide,
        // which is the normal case. A publisher that under-declares in order
        // to force earlier expiry sets the declared period to 1/k of nominal
        // and this to k, so the staleness test still uses the nominal value.
        double validityScale = default(1.0);
        bool attacker = default(false);   // SHADE: issue junk Interests while the regime's attackRate > 0  // >0: content-name generations (type churn)

        @signal[retrievalLatency](type=simtime_t);
        @signal[dataHopCount](type=long);
        @signal[contentAge](type=simtime_t);
        @signal[staleDelivery](type=long);
        @signal[freshDelivery](type=long);
        @statistic[retrievalLatency](record=stats,vector);
        @statistic[dataHopCount](record=stats,vector);
        @statistic[contentAge](record=stats,vector);
        @signal[contentAgeRatio](type=double);
        @statistic[contentAgeRatio](record=stats,histogram);
        @signal[restamps](type=long);
        @statistic[restamps](record=stats,histogram);
        @statistic[staleDelivery](record=count);
        @statistic[freshDelivery](record=count);
        @display("i=block/source");
    gates:
        output toNdn;
        input fromNdn;
}
