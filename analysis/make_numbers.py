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
                "DRL", "Fixed-FDCA-LRU"]
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
                             ("Fixed-FRESH-LFFO", "capGridFresh"),
                             ("Fixed-FDCA-LRU", "capGridFdc")):
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
    def _equiv(sub, cfg, seed=10):
        a = d / f"{cfg}-{seed}.sca"
        b = d.parent / sub / f"{cfg}-{seed}.sca"
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

    # Build C adds the FDC admission rule and moves the clock-free realization's
    # residual lifetime out of the signed FreshnessPeriod and into its own
    # hop-by-hop annotation. Both additions default to build B's behavior on
    # every configuration that does not name them, which is a claim about the
    # recorded values rather than about the diff, so it is tested the same way.
    _eqc = [_equiv("equiv5", _c, _s)
            for _c in ("Fixed-LCE-LRU", "AT-LCE-LRU") for _s in (10, 11)]
    _eqc = [r for r in _eqc if r]
    if _eqc:
        put("equivNFdc", f"{sum(r[0] for r in _eqc):,}".replace(",", "{,}"))
        put("equivRunsFdc", str(len(_eqc)))
        put("equivDiffFdc", str(sum(r[1] for r in _eqc)))
    # Build D changes only how a reading's freshness class is read out of its
    # name, which is the same parse for every name reported before the
    # versioned arm. The FDC configuration is included because build C's
    # additions are reachable only from it.
    _eqd = [_equiv("equiv6", _c, _s)
            for _c in ("Fixed-LCE-LRU", "AT-LCE-LRU", "Fixed-FDCA-LRU")
            for _s in (10, 11)]
    _eqd = [r for r in _eqd if r]
    if _eqd:
        put("equivNVer", f"{sum(r[0] for r in _eqd):,}".replace(",", "{,}"))
        put("equivRunsVer", str(len(_eqd)))
        put("equivDiffVer", str(sum(r[1] for r in _eqd)))
    # Build E made the annotation link-local, so a node that does not implement
    # it neither honors nor relays it. Build F gave the annotation its true
    # eight-byte size and added two counters per node, so its comparison has
    # new scalars on one side; what matters there is that no shared value
    # moved. Build G corrected comments only.
    _eqe = [_equiv("equiv7", _c, _s)
            for _c in ("Fixed-LCE-LRU", "AT-LCE-LRU", "ATD-LCE-LRU", "Fixed-FDCA-LRU")
            for _s in (10, 11)]
    _eqe = [r for r in _eqe if r]
    if _eqe:
        put("equivNLink", f"{sum(r[0] for r in _eqe):,}".replace(",", "{,}"))
        put("equivRunsLink", str(len(_eqe)))
        put("equivDiffLink", str(sum(r[1] for r in _eqe)))
    # For the builds that add scalars, compare only the names both sides hold.
    def _equiv_shared(sub, cfg, seed=10):
        a = d / f"{cfg}-{seed}.sca"
        b = d.parent / sub / f"{cfg}-{seed}.sca"
        if not (a.exists() and b.exists()):
            return None
        def vals(path):
            return {(m, n): v for m, n, v in C.read_scalars(path)}
        va, vb = vals(a), vals(b)
        shared = set(va) & set(vb)
        if not shared:
            return None
        return len(shared), sum(1 for k in shared if va[k] != vb[k])
    _eqf = [_equiv_shared(_s2, _c, _s3)
            for _s2 in ("equiv8", "equiv9")
            for _c in ("Fixed-LCE-LRU", "AT-LCE-LRU")
            for _s3 in (10, 11)]
    _eqf = [r for r in _eqf if r]
    if _eqf:
        put("equivNTlv", f"{sum(r[0] for r in _eqf):,}".replace(",", "{,}"))
        put("equivRunsTlv", str(len(_eqf)))
        put("equivDiffTlv", str(sum(r[1] for r in _eqf)))

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

    # The on-wire cost of the hop-by-hop annotation, measured rather than
    # computed from the TLV size. The bytes are charged where the annotation
    # is first attached, so what matters is how often that happens over a run.
    def _bytes(cfg):
        tot = []
        for _p in C.sca_files(d, cfg):
            acc = 0.0
            for _mod, _nm, _v in C.read_scalars(_p):
                if _nm == "ndnBytesSent":
                    try:
                        acc += float(_v)
                    except ValueError:
                        pass
            tot.append(acc)
        import statistics as _sb
        return _sb.mean(tot) if tot else None
    _bo, _bd = _bytes("AT-LCE-LRU"), _bytes("ATD-LCE-LRU")
    if _bo and _bd:
        put("DecBytesOrigin", f"{_bo/1e6:.2f}")
        put("DecBytesDecrement", f"{_bd/1e6:.2f}")
        put("DecBytesPct", f"{100*(_bd/_bo - 1):.2f}")
    # How many annotations were actually written, from the counter the Content
    # Store records. This used to be derived by dividing the byte difference
    # between the two arms by the TLV size, which is wrong twice over: the two
    # arms are independent runs whose traffic differs by a couple of hundred
    # packets, so the difference is not purely annotation bytes, and the
    # quotient came out below the number of cache hits, which is a lower bound
    # on the number of annotations.
    def _counter(cfg, name):
        tot = []
        for _p in C.sca_files(d, cfg):
            acc = 0.0
            for _mod, _nm, _v in C.read_scalars(_p):
                if _nm == name:
                    try:
                        acc += float(_v)
                    except ValueError:
                        pass
            tot.append(acc)
        import statistics as _sc
        return _sc.mean(tot) if tot else None
    _att = _counter("ATD-LCE-LRU", "annotationsAttached")
    _chg = _counter("ATD-LCE-LRU", "annotationsCharged")
    if _att:
        put("DecAnnotated", f"{_att:,.0f}".replace(",", "{,}"))
    if _chg:
        put("DecAnnotatedCharged", f"{_chg:,.0f}".replace(",", "{,}"))

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
                 ("Field-pCASTING", "Field-AT-pCASTING", "FieldPcast"),
                 ("Field-FDCA-LRU", "Field-AT-FDCA-LRU", "FieldFdc"))
        gains, worst_at, stds = [], 0.0, []
        NICE = {"FieldLce": "leave-copy-everywhere", "FieldLcd": "leave-copy-down",
                "FieldLfua": "aged LFU", "FieldPcast": "pCASTING",
                "FieldFdc": "FDC admission"}
        for base, treat, tag in pairs:
            mb, mt = cm(fieldd, base), cm(fieldd, treat)
            if not (mb and mt):
                continue
            for m, suff in ((mb, "Std"), (mt, "At")):
                put(tag + suff + "Stale",
                    pct(_s7.mean(v["stale_delivery"] for v in m.values())))
                put(tag + suff + "Valid",
                    pct(_s7.mean(v["valid_cache_service"] for v in m.values())))
            stds.append((NICE.get(tag, tag),
                         100 * _s7.mean(v["stale_delivery"] for v in mb.values())))
            g = 100 * (_s7.mean(v["valid_cache_service"] for v in mt.values())
                       - _s7.mean(v["valid_cache_service"] for v in mb.values()))
            put(tag + "Gain", f"{g:.2f}")
            gains.append(g)
            worst_at = max(worst_at,
                           100 * max(v["stale_delivery"] for v in mt.values()))
        if gains:
            put("fieldGainMin", f"{min(gains):.2f}")
            put("fieldGainMax", f"{max(gains):.2f}")
            # The range of standard-NDN staleness over the families in the
            # table. Quoting two hand-picked arms excluded a third that sits
            # below both of them.
            if stds:
                put("fieldStdStaleMin", f"{min(v for _, v in stds):.2f}")
                put("fieldStdStaleMax", f"{max(v for _, v in stds):.2f}")
                put("fieldStdStaleMinTag", min(stds, key=lambda kv: kv[1])[0])
                put("fieldStdStaleMaxTag", max(stds, key=lambda kv: kv[1])[0])
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

    # --- FDC: the two halves run apart -------------------------------------
    # Amadeo et al. (2020) fasten a probabilistic admission rule to the
    # residual-lifetime expiry rule and evaluate the pair. The four cells here
    # separate them: standard NDN, the expiry rule alone, the admission rule
    # alone, and both together, which is FDC as published.
    import statistics as _s10
    _FDC = (("Std", "Fixed-LCE-LRU"), ("At", "AT-LCE-LRU"),
            ("Adm", "Fixed-FDCA-LRU"), ("Full", "AT-FDCA-LRU"),
            ("Dec", "ATD-FDCA-LRU"))
    _cells = {}
    for _tag, _cfg in _FDC:
        _m = cm(d, _cfg)
        if not _m:
            continue
        _cells[_tag] = _m
        put("fdc" + _tag + "Stale", pct(_s10.mean(v["stale_delivery"] for v in _m.values())))
        put("fdc" + _tag + "Valid", pct(_s10.mean(v["valid_cache_service"] for v in _m.values())))
        put("fdc" + _tag + "Hit", pct(_s10.mean(v["cache_service"] for v in _m.values())))
        put("fdc" + _tag + "Energy", one(_s10.mean(v["energy_J"] for v in _m.values())))
        put("fdc" + _tag + "Lat", one(_s10.mean(v["latency_ms"] for v in _m.values())))
        put("fdc" + _tag + "Seeds", str(len(_m)))
    # Attribution. Of the staleness FDC removes against standard NDN, what
    # share does each half account for when run on its own?
    if {"Std", "Adm", "Full"} <= set(_cells):
        def _sm(t):
            return _s10.mean(v["stale_delivery"] for v in _cells[t].values())
        _tot = _sm("Std") - _sm("Full")
        if _tot > 0:
            put("fdcAdmShare", f"{100*(_sm('Std') - _sm('Adm'))/_tot:.0f}\\%")
            put("fdcExpShare", f"{100*(_sm('Adm') - _sm('Full'))/_tot:.0f}\\%")
    # Paired comparisons, same seeds, one factor apart in each case.
    for _a, _b, _tag in (("Fixed-LCE-LRU", "Fixed-FDCA-LRU", "fdcAdmVsStd"),
                         ("Fixed-FDCA-LRU", "AT-FDCA-LRU", "fdcFullVsAdm"),
                         ("AT-LCE-LRU", "AT-FDCA-LRU", "fdcFullVsAt"),
                         ("AT-FDCA-LRU", "ATD-FDCA-LRU", "fdcDecVsFull")):
        for _metric, _mt in (("stale_delivery", "Stale"),
                             ("valid_cache_service", "Valid"),
                             ("cache_service", "Hit"),
                             ("energy_J", "Energy")):
            _q = pr(d, _a, _b, _metric)
            if _q:
                _sc = 100 if _metric != "energy_J" else 1
                put(_tag + _mt + "Delta", f"{_sc*_q[2]:+.2f}")
                put(_tag + _mt + "Wins", f"{_q[4]}/{_q[5]}")

    # Satisfaction, the admission acceptance rate and the transmitted-frame
    # count, which is what the energy difference between the cells is made of.
    def _sumscalars(cfg, names):
        acc = {n: [] for n in names}
        for _p in C.sca_files(d, cfg):
            tot = {n: 0.0 for n in names}
            for _mod, _nm, _v in C.read_scalars(_p):
                if _nm in tot:
                    try:
                        tot[_nm] += float(_v)
                    except ValueError:
                        pass
            for n in names:
                acc[n].append(tot[n])
        return {n: (_s10.mean(v) if v else None) for n, v in acc.items()}

    _FR = ("nbTxFrames", "admissionsOffered", "admissionsAccepted",
           "ndnPacketsSent", "pitInsertionEvent:count", "csMiss:count",
           "csHit:count", "totalInsertions", "totalEvictions")
    _frames = {}
    for _tag, _cfg in _FDC:
        if _tag not in _cells:
            continue
        _t = _sumscalars(_cfg, _FR)
        _frames[_tag] = _t
        put("fdc" + _tag + "Sat",
            pct(_s10.mean(v["satisfaction"] for v in _cells[_tag].values())))
        if _t.get("admissionsOffered"):
            put("fdc" + _tag + "Admit",
                one(100 * _t["admissionsAccepted"] / _t["admissionsOffered"]))
    # Where the energy difference between the cells comes from, measured rather
    # than reasoned about. Each quantity is reported as a percentage change
    # against standard NDN, which is the cell the energy baseline belongs to.
    _COUNTER = {"nbTxFrames": "FramesPct", "ndnPacketsSent": "PktsPct",
                "pitInsertionEvent:count": "PitPct", "csMiss:count": "MissPct",
                "csHit:count": "HitsPct", "totalInsertions": "InsPct",
                "totalEvictions": "EvictPct"}
    if "Std" in _frames:
        for _tag in _frames:
            for _nm, _suf in _COUNTER.items():
                _b = _frames["Std"].get(_nm)
                _v = _frames[_tag].get(_nm)
                if _b and _v:
                    put("fdc" + _tag + _suf, f"{100*(_v/_b - 1):+.2f}")
            # hits each admitted copy goes on to serve, which is what the
            # admission rule is trying to raise
            _i, _h = _frames[_tag].get("totalInsertions"), _frames[_tag].get("csHit:count")
            _bi, _bh = _frames["Std"].get("totalInsertions"), _frames["Std"].get("csHit:count")
            if _i and _h and _bi and _bh:
                put("fdc" + _tag + "HitsPerInsPct",
                    f"{100*((_h/_i)/(_bh/_bi) - 1):+.1f}")
    # the acceptance rate the FDC admission arm actually ran at
    if "Adm" in _frames and _frames["Adm"].get("admissionsOffered"):
        put("fdcAdmitRate",
            one(100 * _frames["Adm"]["admissionsAccepted"]
                / _frames["Adm"]["admissionsOffered"]))

    # --- the two premises of the capacity hypothesis, in their mean form ----
    # Hypothesis 1 needs the mean residence time and the hand-off count both
    # to rise with capacity. Neither is assumed: the hand-off count is the
    # mean of the recorded M distribution, and the residence mean is read off
    # the one-hand-off deliveries exactly as the calibration does.
    for _cap, _cfg in ((5, "C5-LCE-LRU"), (10, "Fixed-LCE-LRU"),
                       (20, "C20-LCE-LRU"), (40, "C40-LCE-LRU")):
        _t = C.restamp_table(d, _cfg)[0]
        if not _t:
            continue
        _tot = sum(v[0] for v in _t.values())
        if _tot <= 0:
            continue
        _w = {5: "Five", 10: "Ten", 20: "Twenty", 40: "Forty"}[_cap]
        put("capMeanM" + _w, two(sum(k * v[0] for k, v in _t.items()) / _tot))
        if 1 in _t:
            put("capMeanW" + _w, two(_t[1][2]))
        # The conclusion of Hypothesis 1 is about the mean delivered age, which
        # is a different quantity from the stale-delivery rate the operational
        # part of the section reports. restamp_table carries the mean age in
        # units of the declared lifetime per hand-off bucket, so pooling it by
        # delivery count gives E[A]/F at this capacity.
        put("capMeanAge" + _w,
            f"{sum(v[2] * v[0] for v in _t.values()) / _tot:.3f}")

    # (5, 6) The field scenario's standard-NDN staleness range and the least
    # affected family, over the pairs that are actually in the table rather
    # than over two hand-picked ones.

    # --- versioned naming: the second no-protocol-change alternative --------
    # Consumers derive the version component from a schedule they share, which
    # is the first of the two name-discovery routes of Sec. 5.3.
    for _cfg, _tag in (("Ver30-LCE-LRU", "VerThirty"),
                       ("Ver90-LCE-LRU", "VerNinety"),
                       ("Fixed-LCE-LRU", "VerBase"),
                       ("AT-LCE-LRU", "VerRule")):
        _m = cm(d, _cfg)
        if not _m:
            continue
        put(_tag + "Stale", pct(_s10.mean(v["stale_delivery"] for v in _m.values())))
        put(_tag + "Valid", pct(_s10.mean(v["valid_cache_service"] for v in _m.values())))
        put(_tag + "Hit", pct(_s10.mean(v["cache_service"] for v in _m.values())))
        put(_tag + "Energy", one(_s10.mean(v["energy_J"] for v in _m.values())))
        put(_tag + "Lat", one(_s10.mean(v["latency_ms"] for v in _m.values())))
        put(_tag + "Seeds", str(len(_m)))
        # Where the residue sits. Versioned naming bounds delivered age by the
        # version interval rather than by the declared lifetime, so it should
        # hold in the three regimes whose lifetimes are at least the interval
        # and fail in the flash regime, which shortens them. Measured per
        # regime rather than argued.
        for _r, _nm in ((3, "Flash"),):
            _vals = [C.run_metrics(_p, regimes=(_r,)) for _p in C.sca_files(d, _cfg)]
            _vals = [v for v in _vals if v]
            if _vals:
                put(_tag + _nm,
                    pct(_s10.mean(v["stale_delivery"] for v in _vals)))
        _q = [C.run_metrics(_p, regimes=(0, 1, 2)) for _p in C.sca_files(d, _cfg)]
        _q = [v for v in _q if v]
        if _q:
            put(_tag + "Quiet", pct(_s10.mean(v["stale_delivery"] for v in _q)))
    for _cfg, _tag in (("Ver30-LCE-LRU", "VerThirty"), ("Ver90-LCE-LRU", "VerNinety")):
        for _metric, _mt in (("stale_delivery", "Stale"),
                             ("valid_cache_service", "Valid"),
                             ("cache_service", "Hit"),
                             ("energy_J", "Energy"),
                             ("latency_ms", "Lat")):
            _r2 = pr(d, "Fixed-LCE-LRU", _cfg, _metric)
            if _r2:
                _sc = 100 if _metric in ("stale_delivery", "valid_cache_service",
                                         "cache_service") else 1
                put(_tag + _mt + "Delta", f"{_sc*_r2[2]:+.2f}")
                put(_tag + _mt + "Wins", f"{_r2[4]}/{_r2[5]}")
                if _metric == "energy_J" and _r2[0]:
                    put(_tag + "EnergyPct", f"{100*_r2[2]/_r2[0]:+.1f}")
                if _metric == "latency_ms" and _r2[0]:
                    put(_tag + "LatPct", f"{100*_r2[2]/_r2[0]:+.1f}")

    # The size of the published dataset, counted from the result files rather
    # than typed into the Data Availability statement, where it had gone stale
    # by 180 runs.
    _total = 0
    for _sub in ("v2_main", "v2_cap5", "v2_cap20", "v2_field"):
        _dd = d.parent / _sub
        if _dd.is_dir():
            _total += len(list(_dd.glob("*.sca"))) + len(list(_dd.glob("*.sca.gz")))
    if _total:
        put("datasetRuns", f"{_total:,}".replace(",", "{,}"))

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
