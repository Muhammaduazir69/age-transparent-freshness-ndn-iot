//
// NDN-IoT-SHADE - RegimeController (see RegimeController.ned)
//
#ifndef __NDNIOT_REGIMECONTROLLER_H
#define __NDNIOT_REGIMECONTROLLER_H

#include <omnetpp.h>
#include <string>
#include <vector>

using namespace omnetpp;

namespace ndniot {

class RegimeController : public cSimpleModule {
  protected:
    std::vector<std::string> names;
    std::vector<double> starts, alpha, rateScale, driftPeriod, driftStep,
                        attackRate, freshScale;
    std::vector<cMessage *> timers;
    simsignal_t regimeSignal;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    std::vector<double> parseList(const char *par, size_t n);

  public:
    virtual ~RegimeController();
    static RegimeController *find(cModule *from);

    int count() const { return (int)starts.size(); }
    int regimeAt(simtime_t t) const;
    int current() const { return regimeAt(simTime()); }
    double regimeStart(int r) const { return starts.at(r); }
    const std::string &regimeName(int r) const { return names.at(r); }

    double alphaNow() const { return alpha[current()]; }
    double rateScaleNow() const { return rateScale[current()]; }
    double driftPeriodNow() const { return driftPeriod[current()]; }
    double driftStepNow() const { return driftStep[current()]; }
    double attackRateNow() const { return attackRate[current()]; }
    double freshScaleNow() const { return freshScale[current()]; }
    const std::vector<double> &alphas() const { return alpha; }
    double driftPeriodOf(int r) const { return driftPeriod.at(r); }
    double driftStepOf(int r) const { return driftStep.at(r); }
};

} // namespace ndniot

#endif
