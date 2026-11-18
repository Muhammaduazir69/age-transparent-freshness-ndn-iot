//
// NDN-IoT-RL - replacement-policy-aware Content Store (A2C hook, thesis RO2)
// replacementPolicy: LRU | LFU | FIFO | LFF | RL
//
package ndniot.rlndn;

import ndniot.ndn.tables.CS;

simple RlCS extends CS
{
    parameters:
        @class(ndniot::RlCS);
        bool admitVsRetain = default(true);   // thesis 3.5.4 tier-3 conflict resolution
        // Admission grace: a freshly admitted entry is shielded from eviction
        // for this long, so an admission is never wasted by being displaced
        // before it can serve anything (0s = off, original behavior). The
        // shield is skipped when every resident is inside its grace window.
        double admissionGrace @unit(s) = default(0s);
        // Replacement-network inference cost (charged via the processor):
        //   actor  (8+16)x128 + 128x128 + 128x1 = 19 584 MAC per candidate item
        //   critic (2*128+16)x128 + 128x1       = 34 944 MAC per decision
        double replacementMacsPerItem = default(19584);
        double replacementMacsFixed = default(34944);
}
