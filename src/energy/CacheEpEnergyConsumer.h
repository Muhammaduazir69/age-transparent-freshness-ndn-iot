//
// NDN-IoT-RL - CacheEpEnergyConsumer
// Models the energy cost of caching operations (thesis 3.9.3): CS writes
// (admission), CS reads (hits), evictions, and digest transmissions, drawn
// from the host's IEpEnergyStorage battery alongside the radio's
// SensorStateBasedEpEnergyConsumer.
//
// Discrete per-operation energies (J) are accumulated and presented to the
// (power-integrating) storage as an average power over the next accounting
// pulse: P = pendingJ / pulseInterval.
// Subscribes to the Content Store's csInsertionEvent/csHit/csEvictionEvent
// signals at the overlay-app level (signals propagate up the module tree).
//

#ifndef __NDNIOT_CACHEEPENERGYCONSUMER_H
#define __NDNIOT_CACHEEPENERGYCONSUMER_H

#include <omnetpp.h>
#include "inet/power/contract/IEpEnergyConsumer.h"
#include "inet/power/contract/IEpEnergySource.h"

using namespace omnetpp;

namespace ndniot {

class CacheEpEnergyConsumer : public cSimpleModule,
                              public inet::power::IEpEnergyConsumer,
                              public cListener {
protected:
    double insertEnergyJ = 50e-6;   // per CS write
    double hitEnergyJ = 10e-6;      // per CS read (hit serving)
    double evictEnergyJ = 5e-6;     // per eviction
    simtime_t pulseInterval;

    double pendingJ = 0;
    inet::units::values::W powerConsumption = inet::units::values::W(0);
    cMessage *pulseTimer = nullptr;
    inet::power::IEpEnergySource *energySource = nullptr;

    simsignal_t csInsertSig, csHitSig, csEvictSig, rlInferSig;
    double inferenceJ = 0;
    long totalOps = 0;
    double totalJ = 0;

    virtual int numInitStages() const override { return 2; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    void setPower(inet::units::values::W p);

public:
    virtual ~CacheEpEnergyConsumer() { cancelAndDelete(pulseTimer); }

    // IEnergyConsumer / IEpEnergyConsumer
    virtual inet::power::IEnergySource *getEnergySource() const override { return energySource; }
    virtual inet::units::values::W getPowerConsumption() const override { return powerConsumption; }

    // cListener (cache event signals)
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
                               long value, cObject *details) override;
    // RL inference energy arrives as Joules on a double-valued signal
    virtual void receiveSignal(cComponent *source, simsignal_t signalID,
                               double value, cObject *details) override;
    using cListener::receiveSignal;
};

} // namespace ndniot

#endif
