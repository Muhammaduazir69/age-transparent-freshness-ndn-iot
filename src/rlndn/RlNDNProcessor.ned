//
// NDN-IoT-RL - placement-policy-aware NDN processor (DQN hook, thesis RO1)
//
package ndniot.rlndn;

import ndniot.ndn.core.NDNProcessor;

simple RlNDNProcessor extends NDNProcessor
{
    parameters:
        @class(ndniot::RlNDNProcessor);
        string placementPolicy = default("lce");  // lce | lcd | prob | freshCheck | rl
        double probCacheP = default(0.5);
        double freshThreshold @unit(s) = default(5s);
        double energyGuardFraction = default(0.2); // below this residual fraction: never cache
        // Control for the reviewer question "is state[9] carrying residual
        // energy, or just an elapsed-time clock?".  On the realistic substrate
        // the battery drains monotonically, so the two are ~90% correlated and
        // the observed advantage cannot be attributed from the main runs alone.
        //   residual : the measured battery fraction (default; what the paper runs)
        //   clock    : an exact normalised elapsed-time clock, no energy at all
        //   constant : pinned at 1.0, the coordinate carries no information
        // The guard in shouldCache always uses the TRUE residual fraction, so
        // this switch changes only what the agent observes, never the physics.
        string energyCoordMode = default("residual");
        // Coordination trustworthiness (NDNComm'26 "trustworthiness in
        // multi-agent systems"): degrade ONLY the neighbor-digest input the
        // agent observes.  honest = unchanged; lossy = drop a fraction of
        // received digests; stale = accept each face's first digest and never
        // refresh it; misreport = node 0 advertises an all-ones filter, the
        // coordination-layer analogue of a cache-pollution advertisement.
        string digestTrustMode = default("honest");
        double digestLossProb = default(0.5);   // used by digestTrustMode="lossy"
        double energyCoordHorizon @unit(s) = default(600s); // clock normaliser
        // LCD (Laoutaris et al.): cache one hop BELOW the serving node. The
        // serving point stamps lcdDepth = 0 and forwardData increments on the
        // way out, after the local admission decision, so the node that is one
        // hop downstream sees 1. The original `== 0` therefore cached only at
        // the serving node itself, i.e. "cache at origin only", and never
        // moved a copy down the path at all. 1 = correct LCD, 0 = the legacy
        // (broken) behaviour, kept only so the defect can be re-measured.
        int lcdCacheDepth = default(1);
        // Coordination as a true ACTION MASK rather than a pre-policy veto.
        // With a pre-policy veto the agent is never queried in vetoed states,
        // so those states never enter replay and state feature 10
        // (neighborCached) can never reach 1.0 during training. With the mask
        // the policy is queried, the state is observed and learned from, and
        // the digest constrains the action afterwards.
        bool coordAsActionMask = default(true);

        // coordination layer (thesis RO3)
        bool enableDigests = default(false);       // Bloom-filter cache digests to neighbors
        double digestInterval @unit(s) = default(60s);   // thesis: 30-120 s
        bool redundancyVeto = default(true);       // hard veto; false = digests only enter the state (soft coordination)
        int redundancyThreshold = default(2);      // >= this many caching neighbors -> don't cache
        int digestBits = default(0);               // filter size m; 0 = sized to csMaxEntries for digestFalsePositive
        double digestFalsePositive = default(0.01); // target false-positive rate of a sized filter
        bool piggybackDigests = default(true);     // carry digests on outgoing Interest/Data; standalone only on idle faces
        double energyReportInterval @unit(s) = default(10s);  // EV_ENERGY_DELTA cadence (0 = off)

        // ---- inference cost (see docs: derivation from the actual nets) ----
        // Placement QNet: 16x256 + 256x256 + 256x2 = 70 144 multiply-accumulates
        double placementMacs = default(70144);
        // Energy and time per MAC. 0 = inference charged nothing, which is the
        // legacy behaviour every previously published number was produced with.
        // Cortex-M4F @80 MHz, 112 uA/MHz @3.0 V, 1 MAC/cycle:
        //     time 1.25e-8 s/MAC, energy 4.2e-10 J/MAC
        // ATmega256RFR2 @16 MHz, 4.1 mA @3.6 V, ~10 cycles per Q15 MAC:
        //     time 6.25e-7 s/MAC, energy 9.225e-9 J/MAC
        double inferenceEnergyPerMacJ = default(0);
        double inferenceTimePerMacS = default(0);

        @signal[rlInferenceEnergy](type=double);
        @statistic[rlInferenceEnergy](record=sum,count);
}
