//
// NDN-IoT-SHADE - ShadeNDNProcessor (see ShadeNDNProcessor.h)
//
package ndniot.shade;

import ndniot.rlndn.RlNDNProcessor;

simple ShadeNDNProcessor extends RlNDNProcessor
{
    parameters:
        @class(ndniot::ShadeNDNProcessor);
        string selector = default("shade");      // shade | fixed | exp3 | ducb | hedge | random | static | shuffle | assigned
        string assignment = default("");           // selector = assigned: expert index per nodeId
        int fixedExpert = default(0);              // index into `experts` when selector = fixed
        // portfolio: ADMISSION-EVICTION, admission in {LCE, FRESH, DOOR, QL},
        // eviction in {LRU, LFUA, LFFO}; the fixed comparators may also use the
        // admission rules LCD, PROB and TLFU, which are not in the portfolio
        string experts = default("LCE-LRU LCE-LFUA FRESH-LFFO DOOR-LRU QL-LRU");
        double epoch @unit(s) = default(10s);      // decision epoch
        double discount = default(0.9);            // rho, discounted cumulative utility
        double hysteresis = default(0.01);         // switch margin on the normalised score
        double staleWeight = default(1.0);         // beta, penalty per stale hit
        bool pageHinkley = default(true);          // change-point forgetting
        double phDelta = default(0.005);           // PH tolerance
        double phLambda = default(0.5);            // PH alarm threshold
        int minLookups = default(1);               // skip the update in empty epochs
        double exp3Gamma = default(0.1);           // EXP3 exploration (ablation)
        double ducbGamma = default(0.9);           // discounted UCB discount
        double ducbXi = default(0.6);              // discounted UCB exploration constant
        int banditHold = default(1);               // exp3/ducb/hedge: epochs each pull is held
        bool banditSkipFirst = default(false);     // reward a held pull without its first epoch
        bool banditFixedScale = default(false);    // reward (u+beta)/(1+beta) instead of running range
        double hedgeEta = default(50);             // hedge: learning rate on G_k/W
        double probAdmit = default(0.5);           // PROB admission probability (Prob(p))
        int lcdDepth = default(1);                 // LCD: admit at this many hops below the hit
        int tlfuSample = default(1000);            // TLFU: Interests between halvings of the counts
        double pcastWb = default(1.0);             // pCASTING: battery weight
        double pcastWo = default(1.0);             // pCASTING: cache-occupancy weight
        double pcastWf = default(1.0);             // pCASTING: residual-freshness weight
        double freshFrac = default(0.5);           // FRESH admission: remaining life > freshFrac * FP
        double doorWindow @unit(s) = default(60s); // DOOR admission window
        int doorMin = default(2);                  // DOOR: requests needed within the window
        double qAlpha = default(0.1);              // Q-learning step size
        double qEpsilon = default(0.05);           // Q-learning exploration
        double qSlotCost = default(0.5);           // kappa: opportunity cost of one admitted slot
        double qSkipHorizon @unit(s) = default(30s); // how long a skipped copy is tracked
        @signal[activeExpert](type=long);
        @signal[expertSwitch](type=long);
        @signal[phAlarm](type=long);
        @signal[realUtility](type=double);
        @statistic[activeExpert](record=vector);
        @statistic[expertSwitch](record=count,vector);
        @statistic[phAlarm](record=count,vector);
        @statistic[realUtility](record=vector,stats);
        @signal[epochLookups](type=long);
        @signal[epochValid](type=long);
        @signal[epochStale](type=long);
        @statistic[epochLookups](record=vector);
        @statistic[epochValid](record=vector);
        @statistic[epochStale](record=vector);
}
