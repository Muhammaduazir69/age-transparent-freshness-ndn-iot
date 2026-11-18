#include "RegimeController.h"
#include <sstream>

namespace ndniot {

Define_Module(RegimeController);

RegimeController::~RegimeController() {
    for (auto *t : timers)
        cancelAndDelete(t);
}

std::vector<double> RegimeController::parseList(const char *parName, size_t n) {
    std::istringstream in(par(parName).stdstringValue());
    std::vector<double> v;
    double x;
    while (in >> x)
        v.push_back(x);
    if (n > 0 && v.size() != n)
        throw cRuntimeError("RegimeController: '%s' has %d values, expected %d",
                            parName, (int)v.size(), (int)n);
    return v;
}

void RegimeController::initialize() {
    std::istringstream in(par("regimeNames").stdstringValue());
    std::string s;
    while (in >> s)
        names.push_back(s);
    starts = parseList("regimeStarts", 0);
    if (starts.empty() || starts.size() != names.size())
        throw cRuntimeError("RegimeController: regimeNames and regimeStarts differ in length");
    for (size_t i = 1; i < starts.size(); i++)
        if (starts[i] <= starts[i - 1])
            throw cRuntimeError("RegimeController: regimeStarts must increase");
    size_t n = starts.size();
    alpha = parseList("zipfAlpha", n);
    rateScale = parseList("rateScale", n);
    driftPeriod = parseList("driftPeriod", n);
    driftStep = parseList("driftStep", n);
    attackRate = parseList("attackRate", n);
    freshScale = parseList("freshScale", n);

    regimeSignal = registerSignal("regime");
    for (size_t i = 0; i < n; i++) {
        auto *t = new cMessage("regimeStart");
        t->setKind((short)i);
        timers.push_back(t);
        scheduleAt(SimTime(starts[i]), t);
    }
}

void RegimeController::handleMessage(cMessage *msg) {
    emit(regimeSignal, (long)msg->getKind());
    EV_INFO << "Regime " << msg->getKind() << " (" << names[msg->getKind()]
            << ") begins" << endl;
}

int RegimeController::regimeAt(simtime_t t) const {
    int r = 0;
    for (int i = 0; i < (int)starts.size(); i++)
        if (t.dbl() >= starts[i])
            r = i;
    return r;
}

RegimeController *RegimeController::find(cModule *from) {
    for (cModule *m = from; m; m = m->getParentModule())
        if (cModule *rc = m->getSubmodule("regimeController"))
            return dynamic_cast<RegimeController *>(rc);
    return nullptr;
}

} // namespace ndniot
