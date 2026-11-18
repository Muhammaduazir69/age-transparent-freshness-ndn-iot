//
// NDN-IoT-SHADE - ShadeNDNProcessor
// Per-router online selection among K caching experts (admission rule +
// eviction rule). Every expert runs in a metadata-only ghost cache fed with
// the router's own Interest and Data stream, so at the end of each epoch the
// router knows the validity-weighted hit utility every expert *would* have
// earned (full-information feedback) and makes the discounted leader the
// active policy of the real Content Store. A two-sided Page-Hinkley test on
// the active expert's utility detects a change of operating regime and
// discards the evidence gathered before it.
//
// selector = "shade"  : ghosts + discounted leader + hysteresis + Page-Hinkley
//            "fixed"  : expert `fixedExpert` only (ghosts still logged)
//            "exp3"   : bandit feedback only (active expert's real utility),
//                       EXP3 (Auer et al. 2002); ablation of the ghosts
//            "random" : a uniformly random expert each epoch
//

#ifndef __NDNIOT_SHADENDNPROCESSOR_H
#define __NDNIOT_SHADENDNPROCESSOR_H

#include "rlndn/RlNDNProcessor.h"
#include "shade/GhostCache.h"
#include <deque>
#include <unordered_map>
#include <vector>

namespace ndniot {

class ShadeCS;
class RegimeController;

class ShadeNDNProcessor : public RlNDNProcessor {
  protected:
    struct Expert {
        std::string name;
        Admission adm;
        Eviction ev;
        GhostCache ghost;
        // epoch counters (ghost)
        long eLookups = 0, eValid = 0, eStale = 0;
        double score = 0;           // discounted cumulative utility G_k
        double lastUtility = 0;
        // per-regime counters (ghost) for the fidelity analysis
        std::vector<long> rValid, rStale;
        std::vector<long> rActiveEpochs;
    };
    std::vector<Expert> experts;
    int active = 0;
    std::string selector = "shade";
    int fixedExpert = 0;

    // selector parameters
    simtime_t epoch;
    double discount = 0.9;          // rho
    double hysteresis = 0.01;       // switch margin on normalised score
    double staleWeight = 1.0;       // beta
    bool usePageHinkley = true;
    double phDelta = 0.005, phLambda = 0.5;
    double phMean = 0, phCumUp = 0, phMinUp = 0, phCumDown = 0, phMaxDown = 0;
    long phCount = 0;
    int minLookups = 1;

    // EXP3 ablation
    double exp3Gamma = 0.1;
    std::vector<double> exp3W;

    // admission parameters
    double freshFrac = 0.5;          // FRESH: admit if remaining origin life > freshFrac * FP
    simtime_t doorWindow;            // DOOR: admit on the >= doorMin-th request in the window
    int doorMin = 2;
    std::unordered_map<std::string, std::deque<simtime_t>> recent;   // router request log

    // Q-learning admission expert (one-step, tabular)
    static const int QL_STATES = 16;
    double qTable[QL_STATES][2] = {};
    double qAlpha = 0.1, qEpsilon = 0.05, qSlotCost = 0.5;
    simtime_t qSkipHorizon;
    struct SkipRec { int state; simtime_t when, validUntil; int missed = 0; };
    std::unordered_map<std::string, SkipRec> skipped;
    long qUpdates = 0;

    // real-cache accounting
    long eLookups = 0, eValid = 0, eStale = 0;
    std::vector<long> rLookups, rValid, rStale, rIssuedHere;
    long switches = 0, phAlarms = 0, epochsRun = 0;
    double scoreWeight = 0;                  // W, discounted number of scored epochs
    int shadowActive = -1;                   // shuffle control: SHADE's own choice
    std::vector<long> rAlarms;
    // bandit baselines
    double ducbGamma = 0.9, ducbXi = 0.6;
    std::vector<double> ducbN, ducbS;
    double uMin = 1e18, uMax = -1e18;
    double scaledReward(double u);
    int banditHold = 1;                      // epochs a bandit/hedge pull is held
    bool banditSkipFirst = false, banditFixedScale = false;
    long pullEpochs = 0, pullN = 0;          // current pull: epochs elapsed, epochs rewarded
    double pullSum = 0;
    double hedgeEta = 50;
    // fixed comparators outside the portfolio
    double probAdmit = 0.5;
    int lcdHops = 1;
    int tlfuSample = 1000;
    long tlfuSeen = 0;
    std::unordered_map<std::string, int> tlfuCount;   // TinyLFU frequency (exact counts, halved)
    int tlfuFreq(const std::string &name) const;
    bool needTlfu = false;
    // instrumentation
    static const int DIV_BUCKETS = 7;
    double divSum[DIV_BUCKETS] = {};
    double divSigned[DIV_BUCKETS] = {};   // store minus ghost, signed
    long divCount[DIV_BUCKETS] = {};
    long epochsSinceSwitch = 0;
    long peakLogNames = 0, peakLogEntries = 0, peakSkipped = 0;
    std::vector<int> pendingAdmit;   // decision per expert for the Data being processed
    bool havePending = false;

    ShadeCS *shadeCs = nullptr;
    bool needTara = false;           // a TARA or pCASTING expert reads the real store
    double pcastWb = 1.0, pcastWo = 1.0, pcastWf = 1.0;   // pCASTING weights
    RegimeController *regimes = nullptr;
    cMessage *epochTimer = nullptr;

    simsignal_t activeExpertSignal, switchSignal, phAlarmSignal, realUtilitySignal;
    simsignal_t epochLookupsSignal, epochValidSignal, epochStaleSignal;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual void processInterest(InterestPacket *interest, int inFace) override;
    virtual void handleCSResponse(cMessage *msg) override;
    virtual void processData(DataPacket *data, int inFace) override;
    virtual bool shouldCacheData(DataPacket *data) override;

    void parseExperts(const std::string &spec, int capacity, double tau);
    void observeData(const DataPacket *data, bool decideForReal);
    int recentCount(const std::string &name, simtime_t now);
    int qState(const std::string &name, const DataPacket *data, simtime_t now);
    bool admitBy(const Expert &x, const std::string &name, const DataPacket *data,
                 simtime_t now, int &qs);
    void qLearn(int s, int a, double r);
    void endEpoch();
    void activate(int k);
    bool pageHinkley(double x);
    int regimeNow() const;

  public:
    virtual ~ShadeNDNProcessor();
};

} // namespace ndniot

#endif
