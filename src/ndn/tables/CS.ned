//
// NDN-IoT-RL - Content Store NED Definition
//

package ndniot.ndn.tables;

simple CS like INdnCs
{
    parameters:
        @class(ndniot::CS);
        int maxSize = default(100);  // MB (byte budget)
        int maxEntries = default(0); // max cached items; 0 = no item limit (thesis sweeps 10-1000)
        bool enabled = default(true);
        // LRU | LFU | LFUA (aged LFU) | FIFO | LFF (hop clock) | LFFO (origin clock)
        string replacementPolicy = default("LRU");
        double cleanupInterval @unit(s) = default(5s);
        // Symmetric tier-1 expired sweep at eviction time. RlCS always did
        // this; the baseline CS did not, which was a non-policy advantage for
        // the learned model. Now shared, and switchable so the size of that
        // advantage can be measured.
        bool expiredFirst = default(true);
        // Expire a cached copy at the end of the producer's freshness lifetime
        // rather than one FreshnessPeriod after local arrival (age-transparent
        // freshness). Default false reproduces standard NDN behaviour.
        bool ageTransparent = default(false);
        // How age transparency is realized when it is on.
        //   "origin"    expire at the publisher's deadline, read from the
        //               packet's origin timestamp (needs a common time base)
        //   "decrement" subtract the residence time from FreshnessPeriod
        //               before forwarding, so the packet carries the residual
        //               lifetime and no clock has to be shared
        string ageMode = default("origin");
        double lfuAgingTau @unit(s) = default(60s);  // aged-LFU decay constant

        @signal[csSize](type=long);
        @signal[csHit](type=long);
        @signal[csMiss](type=long);
        @signal[csHitRatio](type=double);

        @statistic[csSize](record=vector,stats);
        @statistic[csHit](record=count,sum);
        @statistic[csMiss](record=count,sum);
        @statistic[csHitRatio](record=vector,stats);

        // DP-IDS monitoring signals
        @signal[csInsertionEvent](type=long);
        @signal[csEvictionEvent](type=long);

        @statistic[csInsertionEvent](record=count,vector);
        @statistic[csEvictionEvent](record=count,vector);

        @display("i=block/buffer2");  // block/cache does not exist in the OMNeT++ image set, so the store drew as a "?" placeholder

    gates:
        input in @loose;
        output out @loose;
        input processorIn @loose;   // Control messages from NDNProcessor
        output processorOut @loose;  // Responses to NDNProcessor
}
