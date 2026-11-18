//
// NDN-IoT-SHADE - RegimeController
// One network-wide timeline of operating regimes. Every parameter list holds
// one value per regime, in the order the regimes occur; regimeStarts holds the
// start time (s) of each regime. Applications read the current values through
// RegimeController::find(this). Nothing in a router reads the regime index:
// the selector has to discover the change from its own traffic.
//
package ndniot.shade;

simple RegimeController
{
    parameters:
        @class(ndniot::RegimeController);
        string regimeNames = default("stationary drift pollution flash");
        string regimeStarts = default("0 400 800 1200");   // s
        string zipfAlpha = default("0.9 0.9 0.9 1.2");     // consumer Zipf skew per regime
        string rateScale = default("1 1 1 2");             // consumer request-rate multiplier
        string driftPeriod = default("0 40 0 0");          // s; >0 rotates the popularity ranking
        string driftStep = default("0 10 0 0");            // ranks shifted per rotation
        string attackRate = default("0 0 3 0");            // junk Interests/s per attacker node
        string freshScale = default("1 1 1 0.15");         // producer FreshnessPeriod multiplier
        @signal[regime](type=long);
        @statistic[regime](record=vector);
        @display("i=block/timer");
}
