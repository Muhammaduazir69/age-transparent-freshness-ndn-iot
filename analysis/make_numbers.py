#!/usr/bin/env python3
"""Write numbers.tex from the recorded campaign.

One macro per fact. Nothing in the manuscript is typed by hand, so a rerun of
the campaign updates every sentence that quotes a measurement.

    python3 make_numbers.py <results_dir> <summary_runs.csv> [out.tex]
"""
import re, sys, collections
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C
from make_results_figs import POLICY_PAIRS

def pct(x):  return f"{100*x:.2f}"
def one(x):  return f"{x:.1f}"
def two(x):  return f"{x:.2f}"

_CACHE = {}


def cm(d, cfg):
    """Per-seed metrics for one configuration, computed with one denominator.

    The cache key has to include the directory. Keyed on the configuration
    name alone, a lookup in the capacity-sweep campaign silently returned the
    main campaign's numbers for the same name, which is how a grid of eleven
    policies at two capacities came back reporting zero of them rising.
    """
    key = (str(d), cfg)
    if key not in _CACHE:
        _CACHE[key] = C.config_metrics(d, cfg)
    return _CACHE[key]


def pr(d, base, treat, metric):
    a, b = cm(d, base), cm(d, treat)
    if not a or not b:
        return None
    return C.paired_runs(a, b, metric)


def main(d, out):
    # Everything is derived from the recorded .sca files. The shared
    # collector's CSV is deliberately not read: its valid-cache-service field
    # uses a different denominator from its cache-service field.
    d = Path(d)
    M = {}

    def put(name, val):
        M[name] = val

    # --- seeds and breadth -------------------------------------------------
    put("NSeeds", str(len(cm(d, "Fixed-LCE-LRU"))))
    npol = sum(1 for _, b, t in POLICY_PAIRS if cm(d, b) and cm(d, t))
    put("NPolicies", str(npol))

    # --- headline pair -----------------------------------------------------
    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "valid_cache_service")
    if p:
        put("validStd", pct(p[0])); put("validAt", pct(p[1]))
        put("validGain", two(100*p[2])); put("validWins", f"{p[4]}/{p[5]}")
        put("relGain", one(100*p[2]/p[0]))
    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "stale_delivery")
    if p:
        put("staleLceAll", pct(p[0])); put("staleAtAll", pct(p[1]))
    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "cache_service")
    if p:
        put("hitStd", pct(p[0])); put("hitAt", pct(p[1])); put("hitDrop", two(-100*p[2]))
    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "energy_J")
    if p:
        put("energyStd", one(p[0])); put("energyAt", one(p[1]))
        put("energyPct", one(100*p[2]/p[0]))
    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "latency_ms")
    if p:
        put("latStd", one(p[0])); put("latAt", one(p[1]))
        put("latencyPct", one(100*p[2]/p[0]))
    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "satisfaction")
    if p:
        put("satDrop", two(-100*p[2]))

    # how much the exclusion of the compromised sensors moves the headline
    import statistics as _st2
    _all = [C.run_metrics(x, include_attackers=True) for x in C.sca_files(d, "Fixed-LCE-LRU")]
    _leg = [C.run_metrics(x, include_attackers=False) for x in C.sca_files(d, "Fixed-LCE-LRU")]
    _all = [x["stale_delivery"] for x in _all if x]
    _leg = [x["stale_delivery"] for x in _leg if x]
    if _all and _leg:
        put("attackerDelta", two(100 * (_st2.mean(_all) - _st2.mean(_leg))))

    p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", "stale_cache_hits")
    if p:
        put("staleHitsStd", pct(p[0])); put("staleHitsAt", pct(p[1]))

    # --- leave-copy-down ---------------------------------------------------
    p = pr(d, "Fixed-LCD-LRU", "AT-LCD-LRU", "stale_delivery")
    if p:
        put("staleLcdAll", pct(p[0]))

    # --- depth -------------------------------------------------------------
    for cfg, prefix in (("Fixed-LCE-LRU", "staleLce"), ("Fixed-LCD-LRU", "staleLcd")):
        tbl, _ = C.depth_table(d, cfg, per_regime=False)
        allb = tbl.get("all", {})
        if allb:
            deep = max(allb)
            put(prefix + "Deep", two(allb[deep][0]))
            put(prefix + "DeepHops", str(deep))
    tbl, _ = C.depth_table(d, "Fixed-LCE-LRU")
    if 3 in tbl and tbl[3]:
        deep = max(tbl[3]); put("staleLceFlashDeep", two(tbl[3][deep][0]))
    tbl, _ = C.depth_table(d, "AT-LCE-LRU")
    worst = 0.0
    zero_n = 0
    zero_regimes = []
    for r, per_b in tbl.items():
        rmax = max((s for s, _n in per_b.values()), default=0.0)
        worst = max(worst, rmax)
        if rmax == 0.0:
            zero_n += int(sum(n for _s, n in per_b.values()))
            zero_regimes.append(C.REGIMES.get(r, str(r)))
    put("staleAtWorstDepth", f"{worst:.3f}")
    put("zeroStaleDeliveries", f"{zero_n:,}".replace(",", "\\,"))
    put("NZeroRegimes", {0: "none", 1: "one", 2: "two", 3: "three",
                         4: "all four"}.get(len(zero_regimes), str(len(zero_regimes))))

    # --- mechanism ---------------------------------------------------------
    tab, _ = C.restamp_table(d, "Fixed-LCE-LRU")
    if tab:
        mmax = max(tab)
        put("staleWorstM", two(tab[mmax][1])); put("worstM", str(mmax))
        WORD = {1: "one", 2: "two", 3: "three", 4: "four"}
        for m, w in WORD.items():
            if m in tab:
                put("staleM" + w, two(tab[m][1]))
                put("ageM" + w, f"{tab[m][2]:.3f}")
        if 1 in tab:
            put("meanWone", f"{tab[1][2]:.3f}")
    tabat, _ = C.restamp_table(d, "AT-LCE-LRU")
    if tabat:
        put("ageAtMax", f"{max(v[2] for v in tabat.values()):.3f}")
        put("staleAtWorstM", two(max(v[1] for v in tabat.values())))

    # --- capacity ----------------------------------------------------------
    caps = {5: ("C5-LCE-LRU", "C5-AT-LCE-LRU"), 10: ("Fixed-LCE-LRU", "AT-LCE-LRU"),
            20: ("C20-LCE-LRU", "C20-AT-LCE-LRU"), 40: ("C40-LCE-LRU", "C40-AT-LCE-LRU")}
    names = {5: "Five", 10: "Ten", 20: "Twenty", 40: "Forty"}
    for cap, (b, t) in caps.items():
        p = pr(d, b, t, "stale_delivery")
        if p:
            put("staleC" + names[cap], pct(p[0]))
            put("staleAtC" + names[cap], pct(p[1]))
        q = pr(d, b, t, "valid_cache_service")
        if q:
            put("validC" + names[cap], pct(q[0]))
            put("validAtC" + names[cap], pct(q[1]))

    # the capacity range actually measured, so the text cannot drift from it
    done_caps = [c for c, (b, t) in caps.items() if pr(d, b, t, "stale_delivery")]
    if done_caps:
        put("capMin", str(min(done_caps)))
        put("capMax", str(max(done_caps)))
        put("capSpan", f"{max(done_caps) // min(done_caps)}")
        put("NCapacities", str(len(done_caps)))

    # Does capacity actually bind? An eviction rate near one says the store is
    # full essentially always, which is what makes a capacity sweep meaningful.
    for cap, (b, _t) in caps.items():
        pres = C.cs_pressure(d, b)
        if pres:
            put("evictRateC" + names[cap], f"{100*pres['evict_rate']:.1f}")

    # --- learned comparators ----------------------------------------------
    import statistics as _st
    for cfg, mac in (("DRL", "staleDrlAll"), ("Fixed-QL-LRU", "staleQlAll")):
        mm = cm(d, cfg)
        if mm:
            put(mac, pct(_st.mean(v["stale_delivery"] for v in mm.values())))

    # --- do the re-implemented baselines behave as their authors claim? ----
    # Every baseline lives in our harness, so a reader is entitled to ask
    # whether they are faithful. Each one's own paper claims a specific
    # qualitative effect; these macros let the setup section check each claim
    # against our implementation rather than asserting fidelity.
    import statistics as _stB
    _ref = cm(d, "Fixed-LCE-LRU")
    if _ref:
        _rs = 100 * _stB.mean(v["stale_delivery"] for v in _ref.values())
        _rh = 100 * _stB.mean(v["cache_service"] for v in _ref.values())
        _re_ = _stB.mean(v["energy_J"] for v in _ref.values())
        for _c, _tag in (("Base-LCE-LFFO", "bLff"), ("Base-pCASTING", "bPcast"),
                         ("Fixed-TLFU-LRU", "bTlfu"), ("Fixed-LCD-LRU", "bLcd")):
            _m = cm(d, _c)
            if not _m:
                continue
            put(_tag + "Stale", pct(_stB.mean(v["stale_delivery"] for v in _m.values())))
            put(_tag + "Hit", pct(_stB.mean(v["cache_service"] for v in _m.values())))
            put(_tag + "Energy", f"{_stB.mean(v['energy_J'] for v in _m.values()):.1f}")
        put("bRefStale", f"{_rs:.2f}")
        put("bRefHit", f"{_rh:.2f}")
        put("bRefEnergy", f"{_re_:.1f}")
        _lff = cm(d, "Base-LCE-LFFO")
        if _lff:
            _ls = 100 * _stB.mean(v["stale_delivery"] for v in _lff.values())
            if _ls > 0:
                put("bLffFold", f"{_rs / _ls:.0f}")

    # --- does capacity raise staleness under EVERY policy, or just one? ----
    # The abstract used to assert "under every caching policy we tested" from a
    # sweep run on leave-copy-everywhere with LRU alone. The grid now exists,
    # so the claim is computed: how many policies were swept, how many rose,
    # and the smallest and largest rise.
    import statistics as _st9
    _cap5, _cap20 = d.parent / "v2_cap5", d.parent / "v2_cap20"
    if _cap5.is_dir() and _cap20.is_dir():
        _POL = ["Fixed-LCE-LRU", "Fixed-LCD-LRU", "Fixed-LCE-LFUA",
                "Base-LCE-LFFO", "Fixed-FRESH-LFFO", "Fixed-TLFU-LRU",
                "Fixed-DOOR-LRU", "LeCaR", "Base-pCASTING", "Fixed-QL-LRU",
                "DRL"]
        _rises, _ratios = [], []
        for _c in _POL:
            _a, _b = cm(_cap5, _c), cm(_cap20, _c)
            if not (_a and _b and len(_a) >= 20 and len(_b) >= 20):
                continue
            _sa = 100 * _st9.mean(v["stale_delivery"] for v in _a.values())
            _sb = 100 * _st9.mean(v["stale_delivery"] for v in _b.values())
            _rises.append((_c, _sa, _sb))
            if _sa > 0:
                _ratios.append(_sb / _sa)
        if _rises:
            _up = [r for r in _rises if r[2] > r[1]]
            put("capGridPolicies", str(len(_rises)))
            put("capGridRising", str(len(_up)))
            put("capGridMinFive", f"{min(r[1] for r in _rises):.2f}")
            put("capGridMaxTwenty", f"{max(r[2] for r in _rises):.2f}")
            _d = [(r[2] - r[1]) for r in _rises]
            put("capGridMinRise", f"{min(_d):.2f}")
            put("capGridMaxRise", f"{max(_d):.2f}")
            if _ratios:
                put("capGridMedianRatio", f"{_st9.median(_ratios):.1f}")
            # the named freshness-aware policy's own pair, so the sentence that
            # cites it is not quoting a minimum taken over a different arm
            for _c, _tag in (("Base-LCE-LFFO", "capGridLffo"),
                             ("Fixed-FRESH-LFFO", "capGridFresh")):
                _r = [r for r in _rises if r[0] == _c]
                if _r:
                    put(_tag + "Five", f"{_r[0][1]:.2f}")
                    put(_tag + "Twenty", f"{_r[0][2]:.2f}")

    # --- how much of "radio energy" responds to traffic at all? ------------
    # A share of a budget is only meaningful if the budget is the right one.
    # An earlier version of this block guessed the traffic-independent part
    # from the lowest total observed and reported a 79% idle floor, which was
    # wrong by two orders of magnitude: the radios idle in receive mode at
    # 0.005 mW, so the floor is a few joules, not hundreds. Measure it instead
    # of guessing it, by regressing total field energy on frames exchanged
    # across the capacity sweep, where the offered load is identical and only
    # the radio traffic differs.
    import statistics as _st8

    def _frames(path):
        tot = 0.0
        for mod, name, val in C.read_scalars(path):
            if name in ("nbTxFrames", "nbRxFrames"):
                v = C._f(val)
                if v is not None:
                    tot += v
        return tot

    _pairs = []
    for _d, _c in ((d.parent / "v2_cap5", "C5-LCE-LRU"), (d, "Fixed-LCE-LRU"),
                   (d, "C20-LCE-LRU"), (d, "C40-LCE-LRU")):
        if not _d.is_dir():
            continue
        _m = cm(_d, _c)
        if not _m:
            continue
        for _p in C.sca_files(_d, _c):
            _s = re.search(r"-(\d+)\.sca", _p.name)
            if _s and _s.group(1) in _m:
                _fr = _frames(_p)
                if _fr > 0:
                    _pairs.append((_fr, _m[_s.group(1)]["energy_J"]))
    if len(_pairs) >= 20:
        _n = len(_pairs)
        _sx = sum(a for a, _ in _pairs); _sy = sum(b for _, b in _pairs)
        _sxx = sum(a * a for a, _ in _pairs); _sxy = sum(a * b for a, b in _pairs)
        _b = (_n * _sxy - _sx * _sy) / (_n * _sxx - _sx * _sx)
        _a = (_sy - _b * _sx) / _n
        _ss = sum((bb - (_a + _b * aa)) ** 2 for aa, bb in _pairs)
        _tt = sum((bb - _sy / _n) ** 2 for _, bb in _pairs)
        put("energyFitRuns", str(_n))
        put("energyFitIntercept", f"{_a:.1f}")
        put("energyFitSlope", f"{1000 * _b:.2f}")
        put("energyFitRsq", f"{1 - _ss / _tt:.4f}")
        _base = _st8.mean(v["energy_J"] for v in cm(d, "Fixed-LCE-LRU").values())
        put("energyFitInterceptPct", f"{100 * _a / _base:.0f}")

    # --- the top hand-off bucket is a tail bucket, not a count -------------
    # NdnConsumerApp clamps at RESTAMP_BINS-1, so the last bucket is "M or
    # more". It is also thin, and the paper used to quote a mean age from it
    # without saying how few deliveries it holds.
    _rt = C.restamp_table(d, "Fixed-LCE-LRU")[0]
    _rta = C.restamp_table(d, "AT-LCE-LRU")[0]
    if _rt:
        _top = max(_rt)
        put("worstM", str(_top))
        put("worstMN", f"{_rt[_top][0]:,.0f}".replace(",", "{,}"))
        if _top in _rta:
            put("ageAtMaxN", f"{_rta[_top][0]:,.0f}".replace(",", "{,}"))
        # the deepest bucket with a sample that can carry a rate
        _solid = [b for b in sorted(_rt) if _rt[b][0] >= 2000]
        if _solid:
            _b = max(_solid)
            put("solidM", str(_b))
            put("solidMN", f"{_rt[_b][0]:,.0f}".replace(",", "{,}"))
            put("solidMStale", f"{_rt[_b][1]:.2f}")
            put("solidMAge", f"{_rt[_b][2]:.3f}")

    # --- may the two builds be pooled? -------------------------------------
    # The campaign spans two binaries. The claim that results may be combined
    # is testable, so the setup section states the test rather than asserting
    # the usual "built once" guarantee, which was not true of this campaign.
    # These macros count the recorded values that matched, so the sentence
    # cannot drift away from the check.
    def _equiv(sub, cfg):
        a = d / f"{cfg}-10.sca"
        b = d.parent / sub / f"{cfg}-10.sca"
        if not (a.exists() and b.exists()):
            return None
        def vals(path):
            out = {}
            for mod, name, val in C.read_scalars(path):
                out[(mod, name)] = val
            return out
        va, vb = vals(a), vals(b)
        if not va or set(va) != set(vb):
            return None
        diff = sum(1 for k in va if va[k] != vb[k])
        return len(va), diff
    _eq = []
    for _sub, _cfg, _tag in (("equiv2", "Fixed-LCE-LRU", "Lce"),
                             ("equiv3", "Fixed-DOOR-LRU", "Door")):
        _r = _equiv(_sub, _cfg)
        if _r:
            put("equivN" + _tag, f"{_r[0]:,}".replace(",", "{,}"))
            put("equivDiff" + _tag, str(_r[1]))
            _eq.append(_r)
    if _eq:
        put("equivDiffTotal", str(sum(r[1] for r in _eq)))

    # --- is the depth gradient really monotone? ----------------------------
    # It was claimed to be "monotone in all four regimes". It is not: three of
    # the four contain exactly one small reversal. Report the rank correlation,
    # which is what the claim was reaching for and which the data supports.
    _dt0 = C.depth_table(d, "Fixed-LCE-LRU")
    if isinstance(_dt0, tuple):
        _dt0 = _dt0[0]
    _rhos, _viol = [], 0
    for _reg, _byhop in _dt0.items():
        _h = sorted(_byhop)
        _v = [_byhop[k][0] for k in _h]
        _n = len(_v)
        if _n < 3:
            continue
        _viol += sum(1 for i in range(_n - 1) if _v[i + 1] < _v[i])
        _order = sorted(range(_n), key=lambda i: _v[i])
        _rank = [0] * _n
        for _r, _i in enumerate(_order):
            _rank[_i] = _r
        _mx = (_n - 1) / 2
        _my = sum(_rank) / _n
        _num = sum((i - _mx) * (_rank[i] - _my) for i in range(_n))
        _den = (sum((i - _mx) ** 2 for i in range(_n))
                * sum((_rank[i] - _my) ** 2 for i in range(_n))) ** 0.5
        if _den:
            _rhos.append(_num / _den)
    if _rhos:
        put("depthRhoMin", f"{min(_rhos):.2f}")
        put("depthReversals", str(_viol))
        put("depthRegimes", str(len(_rhos)))

    # --- what a consumer beside the publisher actually sees ----------------
    # The abstract used to say such a consumer receives "no" expired readings.
    # Pooled over the four regimes that is not quite true: the flash regime,
    # where lifetimes are shortest, puts a little staleness even at one hop.
    # Report the number instead of the absolute.
    _dt = C.depth_table(d, "Fixed-LCE-LRU")
    if isinstance(_dt, tuple):
        _dt = _dt[0]
    _dl = _stale = 0.0
    for _reg, _byhop in _dt.items():
        if 1 in _byhop:
            _p, _n = _byhop[1]
            _dl += _n
            _stale += _n * _p / 100.0
    if _dl:
        put("staleOneHop", f"{100 * _stale / _dl:.3f}")
        put("staleOneHopDeliveries", f"{_dl:,.0f}".replace(",", "{,}"))

    # --- useful work per joule ---------------------------------------------
    # Energy rises, so the question a sensing deployment actually asks is what
    # the extra energy buys. Deliveries that were already expired are energy
    # spent for nothing, and the rule converts some of them into useful ones.
    import statistics as _st4
    _Ae, _Be = cm(d, "Fixed-LCE-LRU"), cm(d, "AT-LCE-LRU")
    if _Ae and _Be:
        _s = sorted(set(_Ae) & set(_Be), key=int)
        def _pj(m, kind):
            if kind == "cache":
                num = m["valid_cache_service"] * m["satisfied"]
            elif kind == "valid":
                num = (1 - m["stale_delivery"]) * m["satisfied"]
            else:
                num = m["satisfied"]
            return num / m["energy_J"]
        for kind, tag in (("cache", "CacheJ"), ("valid", "ValidJ"), ("all", "AllJ")):
            xa = [_pj(_Ae[x], kind) for x in _s]
            xb = [_pj(_Be[x], kind) for x in _s]
            dd = [y - x for x, y in zip(xa, xb)]
            put("perJ" + tag + "Std", f"{_st4.mean(xa):.2f}")
            put("perJ" + tag + "At", f"{_st4.mean(xb):.2f}")
            put("perJ" + tag + "Pct", f"{100 * _st4.mean(dd) / _st4.mean(xa):+.1f}")
            put("perJ" + tag + "Wins", f"{sum(1 for v in dd if v > 0)}/{len(dd)}")

    # --- the gain as an absolute count, not a ratio ------------------------
    # A reviewer can reasonably ask whether a ratio moved because its
    # denominator did. It did not: the paired design issues the same Interests
    # in both arms, and the count of cache-served deliveries that were still
    # valid rises on every seed.
    import statistics as _st3
    _A, _B = cm(d, "Fixed-LCE-LRU"), cm(d, "AT-LCE-LRU")
    if _A and _B:
        _sd = sorted(set(_A) & set(_B), key=int)
        if _sd:
            va = [_A[x]["valid_cache_service"] * _A[x]["satisfied"] for x in _sd]
            vb = [_B[x]["valid_cache_service"] * _B[x]["satisfied"] for x in _sd]
            diff = [y - x for x, y in zip(va, vb)]
            put("validCountStd", f"{_st3.mean(va):,.0f}".replace(",", "\\,"))
            put("validCountAt", f"{_st3.mean(vb):,.0f}".replace(",", "\\,"))
            put("validCountGain", f"{_st3.mean(diff):,.0f}".replace(",", "\\,"))
            put("validCountWins", f"{sum(1 for v in diff if v > 0)}/{len(diff)}")
            ia = _st3.mean(_A[x]["issued"] for x in _sd)
            ib = _st3.mean(_B[x]["issued"] for x in _sd)
            put("issuedPerRun", f"{ia:,.0f}".replace(",", "\\,"))
            put("issuedIdentical", "yes" if abs(ia - ib) < 0.5 else "no")
            # the same gain measured per Interest issued rather than per delivery
            pa = [_A[x]["valid_cache_service"] * _A[x]["satisfied"] / _A[x]["issued"] for x in _sd]
            pb = [_B[x]["valid_cache_service"] * _B[x]["satisfied"] / _B[x]["issued"] for x in _sd]
            put("validIssuedStd", pct(_st3.mean(pa)))
            put("validIssuedAt", pct(_st3.mean(pb)))
            put("validIssuedGain", two(100 * (_st3.mean(pb) - _st3.mean(pa))))

    # --- how the gain tracks the staleness each policy had -----------------
    # One point per caching policy: staleness removed against valid service
    # gained. A slope near one says the freed slots are exactly the ones that
    # had been holding copies that could not be served validly.
    pts = []
    for name, b, t in POLICY_PAIRS:
        ps = pr(d, b, t, "stale_delivery")
        pv = pr(d, b, t, "valid_cache_service")
        if ps and pv:
            pts.append((100 * (ps[0] - ps[1]), 100 * pv[2]))
    if len(pts) >= 3:
        n = len(pts)
        sx = sum(x for x, _ in pts); sy = sum(y for _, y in pts)
        sxx = sum(x * x for x, _ in pts); sxy = sum(x * y for x, y in pts)
        syy = sum(y * y for _, y in pts)
        den = n * sxx - sx * sx
        if den:
            slope = (n * sxy - sx * sy) / den
            r2num = (n * sxy - sx * sy) ** 2
            r2den = den * (n * syy - sy * sy)
            put("gainSlope", f"{slope:.2f}")
            if r2den > 0:
                put("gainRsq", f"{r2num / r2den:.3f}")
        put("NPolicyPoints", str(n))
        put("staleRemovedMax", one(max(x for x, _ in pts)))
        put("staleRemovedMin", two(min(x for x, _ in pts)))

    # --- the clock-free realization against the timestamp one --------------
    # The claim is that they agree. Agreement is the absence of a difference,
    # so we report the difference and its paired spread rather than a p-value
    # that would only say we failed to reject.
    for metric, tag in (("stale_delivery", "Dec Stale"), ("valid_cache_service", "Dec Valid"),
                        ("cache_service", "Dec Hit"), ("energy_J", "Dec Energy")):
        q = pr(d, "AT-LCE-LRU", "ATD-LCE-LRU", metric)
        if q:
            key = tag.replace(" ", "")
            scale = 100 if metric != "energy_J" else 1
            put(key + "Origin", f"{scale*q[0]:.2f}")
            put(key + "Decrement", f"{scale*q[1]:.2f}")
            put(key + "Delta", f"{scale*q[2]:+.3f}")

    # --- where the clock-free realization loses its small residue ----------
    # It cannot subtract the time a copy spends in flight, only the time it
    # spends resident, so one transit of error accrues per hand-off. If that
    # is the right explanation the excess must vanish when there are no
    # hand-offs at all, and grow with their number. Both are checkable.
    _ta = C.restamp_table(d, "AT-LCE-LRU")[0]
    _td = C.restamp_table(d, "ATD-LCE-LRU")[0]
    WORD = {0: "Mzero", 1: "Mone", 2: "Mtwo", 3: "Mthree", 4: "Mfour",
            5: "Mfive", 6: "Msix", 7: "Mseven"}
    for bucket, word in WORD.items():
        if bucket in _ta and bucket in _td:
            put("DecExcess" + word, f"{_td[bucket][1] - _ta[bucket][1]:.3f}")
            put("DecExcessN" + word, f"{_td[bucket][0]:,.0f}".replace(",", "{,}"))
    # Where does the sample stop supporting the trend? The top buckets hold
    # too few deliveries to resolve a rate of around one percent, so the claim
    # is stated over the range that can carry it rather than over all eight.
    _supported = [b for b in sorted(_ta) if _ta[b][0] >= 500]
    if _supported:
        put("DecExcessMaxM", str(max(_supported)))
        put("DecExcessMinN", f"{min(_td[b][0] for b in _supported):,.0f}".replace(",", "{,}"))

    # --- partial deployment -------------------------------------------------
    import statistics as _s5
    _base = cm(d, "Fixed-LCE-LRU")
    _basestale = _s5.mean(v["stale_delivery"] for v in _base.values()) if _base else None
    for cfg, tag in (("Half-AT-LCE-LRU", "HalfOrigin"),
                     ("Half-ATD-LCE-LRU", "HalfDecrement"),
                     ("HalfNear-AT-LCE-LRU", "HalfNear")):
        m = cm(d, cfg)
        if m:
            st_ = _s5.mean(v["stale_delivery"] for v in m.values())
            put(tag + "Stale", pct(st_))
            put(tag + "Valid", pct(_s5.mean(v["valid_cache_service"] for v in m.values())))
            # share of the baseline's staleness that half a deployment removes
            if _basestale:
                put(tag + "Drop", f"{100*(1 - st_/_basestale):.0f}")
    put("halfNodes", "18")
    put("halfTotal", "36")
    # under-declaration: what the worst point on the curve costs in energy
    _m2, _m4 = cm(d, "Fixed-LCE-LRU"), cm(d, "Short4-LCE-LRU")
    if _m2 and _m4:
        e1 = _s5.mean(v["energy_J"] for v in _m2.values())
        e4 = _s5.mean(v["energy_J"] for v in _m4.values())
        put("ShortFourEnergyPct", f"{100*(e4-e1)/e1:.1f}")
        l1 = _s5.mean(v["latency_ms"] for v in _m2.values())
        l4 = _s5.mean(v["latency_ms"] for v in _m4.values())
        put("ShortFourLatPct", f"{100*(l4-l1)/l1:.1f}")

    # --- under-declaring the lifetime --------------------------------------
    import statistics as _s6
    for cfg, tag in (("Fixed-LCE-LRU", "ShortOne"), ("Short2-LCE-LRU", "ShortTwo"),
                     ("Short3-LCE-LRU", "ShortThree"), ("Short4-LCE-LRU", "ShortFour")):
        m = cm(d, cfg)
        if m:
            put(tag + "Stale", pct(_s6.mean(v["stale_delivery"] for v in m.values())))
            put(tag + "Valid", pct(_s6.mean(v["valid_cache_service"] for v in m.values())))
            # Latency and energy are the axes under-declaring actually loses on,
            # and in a battery-powered sensing network they are the ones that
            # decide whether a mechanism is usable at all.
            put(tag + "Lat", f"{_s6.mean(v['latency_ms'] for v in m.values()):.1f}")
            put(tag + "Energy", f"{_s6.mean(v['energy_J'] for v in m.values()):.1f}")

    # --- the distributed sensing field (scenario 2) -------------------------
    fieldd = d.parent / "v2_field"
    if fieldd.is_dir():
        import statistics as _s7
        pairs = (("Field-LCE-LRU", "Field-AT-LCE-LRU", "FieldLce"),
                 ("Field-LCD-LRU", "Field-AT-LCD-LRU", "FieldLcd"),
                 ("Field-LCE-LFUA", "Field-AT-LCE-LFUA", "FieldLfua"),
                 ("Field-pCASTING", "Field-AT-pCASTING", "FieldPcast"))
        gains, worst_at = [], 0.0
        for base, treat, tag in pairs:
            mb, mt = cm(fieldd, base), cm(fieldd, treat)
            if not (mb and mt):
                continue
            for m, suff in ((mb, "Std"), (mt, "At")):
                put(tag + suff + "Stale",
                    pct(_s7.mean(v["stale_delivery"] for v in m.values())))
                put(tag + suff + "Valid",
                    pct(_s7.mean(v["valid_cache_service"] for v in m.values())))
            g = 100 * (_s7.mean(v["valid_cache_service"] for v in mt.values())
                       - _s7.mean(v["valid_cache_service"] for v in mb.values()))
            put(tag + "Gain", f"{g:.2f}")
            gains.append(g)
            worst_at = max(worst_at,
                           100 * max(v["stale_delivery"] for v in mt.values()))
        if gains:
            put("fieldGainMin", f"{min(gains):.2f}")
            put("fieldGainMax", f"{max(gains):.2f}")
            put("fieldWorstAtStale", f"{worst_at:.3f}")
            put("fieldPairs", str(len(gains)))
            nf = cm(fieldd, "Field-LCE-LRU")
            put("fieldSeeds", str(len(nf)) if nf else "0")
            put("fieldNodes", "36")

    # --- the discarded expected-utility design (development seeds) ---------
    # Read from the recorded development campaign like everything else. These
    # two were hand-typed literals until 19 Nov 2026 and had gone stale by one
    # campaign (31.17/0.71 against a recorded 31.09/0.63), in a paper whose
    # method section promises that no number is typed into a sentence.
    _tara = d.parent / "tara_dev"
    _tm = cm(_tara, "TARA") if _tara.is_dir() else None
    if _tm:
        import statistics as _st7
        put("taraService", pct(_st7.mean(v["cache_service"] for v in _tm.values())))
        put("taraStale", pct(_st7.mean(v["stale_delivery"] for v in _tm.values())))
        put("taraSeeds", str(len(_tm)))

    # Any macro the manuscript references but the campaign cannot yet supply
    # gets a loud sentinel rather than being omitted, so the document still
    # builds and check_placeholders.py still fails on it.
    import re as _re
    src = Path(__file__).resolve().parents[1]
    used = set()
    for f in list(src.glob("*.tex")):
        for m in _re.finditer(r"\\([A-Za-z]+)\{\}", f.read_text()):
            used.add(m.group(1))
    pending = sorted(used - set(M))

    txt = ["% Generated by figures_src/make_numbers.py from the recorded campaign.",
           "% Do not edit by hand: a rerun overwrites this file.",
           "% Macros set to a 99.9 sentinel are not yet computable from the",
           "% campaign; check_placeholders.py fails while any reaches the PDF.",
           ""]
    for k in sorted(M):
        txt.append(f"\\newcommand{{\\{k}}}{{{M[k]}}}")
    # Sentinels for macros the manuscript references that the campaign cannot
    # yet supply. \providecommand leaves anything the class already defines
    # (\TeX, \PublishersNote, ...) untouched, so only genuinely missing names
    # pick up the sentinel.
    # Names the scanner picks up from the sources that are LaTeX or class
    # commands rather than campaign quantities. Emitting a sentinel for these
    # is harmless only as long as the class defines them first; one class
    # change and \TeX would print 99.9 in the back matter.
    NOT_OURS = {"TeX", "LaTeX", "PublishersNote", "NoteAdded", "AbbrevName",
                "sl", "it", "bf", "rm", "em", "today", "and"}
    pending = [k for k in pending if k not in NOT_OURS]
    if pending:
        txt.append("")
        txt.append("% pending: not yet computable from the campaign")
        for k in pending:
            txt.append(f"\\providecommand{{\\{k}}}{{99.9}}")
    Path(out).write_text("\n".join(txt) + "\n")
    print(f"wrote {out} with {len(M)} macros")
    missing = [k for k in ("validStd", "validAt", "staleLceAll", "staleAtAll",
                           "energyPct", "latencyPct", "NSeeds") if k not in M]
    if missing:
        print("WARNING missing:", missing)

if __name__ == "__main__":
    out = sys.argv[2] if len(sys.argv) > 2 else str(Path(__file__).resolve().parents[1] / "numbers.tex")
    main(sys.argv[1], out)
