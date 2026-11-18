//
// NDN-IoT-RL - energy cost of Content Store operations, drawn from the host
// battery (default energies from flash/SRAM access literature, ini-tunable).
//
package ndniot.energy;

import inet.power.contract.IEpEnergyConsumer;

simple CacheEpEnergyConsumer like IEpEnergyConsumer
{
    parameters:
        @class(ndniot::CacheEpEnergyConsumer);
        string energySourceModule = default("^.^.energyStorage");  // host battery
        double insertEnergy @unit(J) = default(0.00005J);   // CS write (admission)
        double hitEnergy @unit(J) = default(0.00001J);      // CS read (hit)
        double evictEnergy @unit(J) = default(0.000005J);     // eviction bookkeeping
        double pulseInterval @unit(s) = default(100ms); // power-presentation window
        @display("i=block/plug");
}
