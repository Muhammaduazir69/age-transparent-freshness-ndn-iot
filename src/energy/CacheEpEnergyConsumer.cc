#include "CacheEpEnergyConsumer.h"

using namespace inet;
using namespace inet::units::values;

namespace ndniot {

Define_Module(CacheEpEnergyConsumer);

void CacheEpEnergyConsumer::initialize(int stage) {
    if (stage == 0) {
        insertEnergyJ = par("insertEnergy").doubleValue();
        hitEnergyJ = par("hitEnergy").doubleValue();
        evictEnergyJ = par("evictEnergy").doubleValue();
        pulseInterval = par("pulseInterval");

        csInsertSig = registerSignal("csInsertionEvent");
        csHitSig = registerSignal("csHit");
        csEvictSig = registerSignal("csEvictionEvent");
        rlInferSig = registerSignal("rlInferenceEnergy");

        // catch the Content Store's events as they propagate up to our parent
        getParentModule()->subscribe(csInsertSig, this);
        getParentModule()->subscribe(csHitSig, this);
        getParentModule()->subscribe(csEvictSig, this);
        getParentModule()->subscribe(rlInferSig, this);

        pulseTimer = new cMessage("cacheEnergyPulse");
    }
    else if (stage == 1) {
        const char *path = par("energySourceModule");
        cModule *mod = getModuleByPath(path);
        if (mod) {
            energySource = dynamic_cast<power::IEpEnergySource *>(mod);
            if (energySource)
                energySource->addEnergyConsumer(this);
        }
        if (!energySource)
            EV_WARN << "CacheEpEnergyConsumer: no energy source at '" << path
                    << "' — cache energy not drawn from any battery" << endl;
    }
}

void CacheEpEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signalID,
                                          long value, cObject *details) {
    Enter_Method_Silent();
    double j = 0;
    if (signalID == csInsertSig) j = insertEnergyJ;
    else if (signalID == csHitSig) j = hitEnergyJ;
    else if (signalID == csEvictSig) j = evictEnergyJ;
    else return;

    pendingJ += j;
    totalJ += j;
    totalOps++;
    if (energySource && !pulseTimer->isScheduled())
        scheduleAt(simTime() + pulseInterval, pulseTimer);
}

void CacheEpEnergyConsumer::receiveSignal(cComponent *source, simsignal_t signalID,
                                          double value, cObject *details) {
    Enter_Method_Silent();
    if (signalID != rlInferSig || value <= 0)
        return;
    pendingJ += value;
    totalJ += value;
    inferenceJ += value;
    totalOps++;
    if (energySource && !pulseTimer->isScheduled())
        scheduleAt(simTime() + pulseInterval, pulseTimer);
}

void CacheEpEnergyConsumer::handleMessage(cMessage *msg) {
    ASSERT(msg == pulseTimer);
    if (pendingJ > 0) {
        // present the accumulated energy as power over the next pulse
        setPower(W(pendingJ / pulseInterval.dbl()));
        pendingJ = 0;
        scheduleAt(simTime() + pulseInterval, pulseTimer);
    } else {
        setPower(W(0));
    }
}

void CacheEpEnergyConsumer::setPower(W p) {
    if (p == powerConsumption)
        return;
    powerConsumption = p;
    emit(power::IEpEnergyConsumer::powerConsumptionChangedSignal, p.get());
}

void CacheEpEnergyConsumer::finish() {
    recordScalar("cacheEnergyOps", totalOps);
    recordScalar("cacheEnergyTotalJ", totalJ);
    recordScalar("inferenceEnergyDrawnJ", inferenceJ);
}

} // namespace ndniot
