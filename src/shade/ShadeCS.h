//
// NDN-IoT-SHADE - ShadeCS
// Content Store whose eviction rule can be switched at run time by the
// selector, plus LeCaR (Vietri et al., HotStorage 2018) as a comparison
// policy: regret-weighted choice between LRU and LFU victims, with a history
// of evicted names per expert.
//

#ifndef __NDNIOT_SHADECS_H
#define __NDNIOT_SHADECS_H

#include "ndn/tables/CS.h"
#include <deque>
#include <unordered_map>

namespace ndniot {

class ShadeCS : public CS {
  protected:
    bool lecar = false;
    double lecarLearningRate = 0.45;
    double lecarDiscount = 0;          // d = 0.005^(1/N), set in initialize
    double wLru = 0.5, wLfu = 0.5;
    long lecarTick = 0;
    struct Hist { std::deque<std::pair<std::string, long>> order; std::unordered_map<std::string, long> when; };
    Hist histLru, histLfu;
    long lecarLruChoices = 0, lecarLfuChoices = 0;

    virtual void initialize() override;
    virtual void evictEntry() override;
    virtual void handleLookupRequest(CSLookupRequest *request) override;
    virtual void finish() override;
    void remember(Hist &h, const std::string &name);
    std::string lfuVictim() const;

  public:
    void setEviction(ReplacementPolicy p) { policy = p; }
    ReplacementPolicy eviction() const { return policy; }
};

} // namespace ndniot

#endif
