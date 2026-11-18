//
// NDN-IoT-SHADE - ShadeCS: run-time switchable eviction rule, plus LeCaR.
//
package ndniot.shade;

import ndniot.ndn.tables.CS;

simple ShadeCS extends CS
{
    parameters:
        @class(ndniot::ShadeCS);
        // replacementPolicy additionally accepts "LECAR"
        double lecarLearningRate = default(0.45);   // lambda in Vietri et al. (2018)
}
