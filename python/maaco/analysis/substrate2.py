"""Substrate-sensitivity analysis on the enlarged training-seed set.

The reviewer-response campaign estimated MAACO's substrate effect from three
training seeds, which put the headline at p = 0.058, and it estimated the
"learned policies are the movers" class claim from two rows that are really one
architecture counted twice (MAACO and its own placement-only ablation). This
module recomputes the whole substrate table from the enlarged campaign under
`results/reviewer/substrate2/`, over four genuinely different learners, and
adds the class-level contrast the paper's claim actually needs.

Design, stated once
-------------------
Two substrates: the packet-level IEEE 802.15.4 composition ("realistic") and
the idealized link ("idealized"). Two arms for every learned policy:

* TRANSFER arm -- one network, trained on 802.15.4, evaluated on both. The two
  substrate cells hold the SAME object, so the test is PAIRED by training seed.
  This answers "would an idealized evaluation have changed the ranking?".
* PER-SUBSTRATE arm -- one network trained on each substrate under the same
  recipe. The cells hold different objects, so the test is UNPAIRED (Welch).
  This answers "would an idealized study have reached different conclusions?",
  which is the question the paper's premise poses.

Unit of analysis: classical policies have no training seeds, so the replicate
is the test seed (10 per substrate) and the test is unpaired -- the two
networks consume the seed set differently, so a given seed index does not
reproduce the same request process on both substrates and pairing would be
unjustified. Learned policies have two nested levels; section 5 of the reviewer
response established that training-seed dispersion is three times the test-seed
dispersion, so the replicate is the TRAINING seed and each cell value is that
seed's mean over the same 10 test seeds.

Everything is read from .sca files; no CSV is written by hand.
"""

import itertools
import math
import pathlib

import numpy as np
import pandas as pd
from scipy import stats as sps

from .collect import collect
from .reviewer import ci_t, holm

PROJ = pathlib.Path(__file__).resolve().parents[3]
REV = PROJ / "results" / "reviewer"
# SUBSTRATE_ROOT selects the campaign directory.  The pinned-build campaign of
# October 2026 lives in substrate3, where every run, classical and learned,
# comes from one binary; substrate2 is the earlier mixed-build archive.
import os
S2 = REV / os.environ.get("SUBSTRATE_ROOT", "substrate2")

METRIC = "cacheHitRatio"

# --- policy inventory --------------------------------------------------------
CLASSICAL = [
    # The tuned 120 s aging window is the strongest fixed policy in Table II.
    # It had run on the realistic substrate since the benchmark campaign but was
    # never given an idealized counterpart, so it sat outside the substrate
    # contrast entirely and the rank headline was computed over a set that
    # excluded the leading opponent.  Both idealized twins were run 2026-09-30.
    ("LCE + LFU-Aged(120s, tuned)", "SmartFarm-B-LCE-LFUA120", "SmartFarm-I-LCE-LFUA120"),
    ("LCE + LFU-Aged(5s)",       "SmartFarm-B-LCE-LFUA5",     "SmartFarm-I-LCE-LFUA5"),
    ("LCE + LFU-Aged(60s)",      "SmartFarm-B-LCE-LFUA",      "SmartFarm-I-LCE-LFUA"),
    ("Prob(0.4, tuned) + LRU",   "SmartFarm-B-ProbTuned-LRU", "SmartFarm-I-ProbTuned-LRU"),
    ("Prob(0.5) + LRU",          "SmartFarm-B-Prob-LRU",      "SmartFarm-I-Prob-LRU"),
    ("LCE + LFU (un-aged)",      "SmartFarm-B-LCE-LFU",       "SmartFarm-I-LCE-LFU"),
    ("LCD + LRU (corrected)",    "SmartFarm-B-LCD-LRU",       "SmartFarm-I-LCD-LRU"),
    ("FreshCheck + LRU",         "SmartFarm-B-Fresh-LRU",     "SmartFarm-I-Fresh-LRU"),
    ("LCE + LRU",                "SmartFarm-B-LCE-LRU",       "SmartFarm-I-LCE-LRU"),
    ("LCE + FIFO",               "SmartFarm-B-LCE-FIFO",      "SmartFarm-I-LCE-FIFO"),
    ("LCE + LFF (origin clock)", "SmartFarm-B-LCE-LFFO",      "SmartFarm-I-LCE-LFFO"),
]

RS = [f"rs{s}" for s in range(1, 11)]
IDS = [f"idealS{s}" for s in range(1, 11)]
MABS = [f"mabS{s}" for s in range(1, 7)]
TQS = [f"tqS{s}" for s in range(1, 7)]

# name -> (stem prefix, realistic-trained tags, idealized-trained tags)
LEARNED = [
    ("MAACO",                  "MAACO-", RS,   IDS),
    ("DQN placement + LRU",    "DQN-",   RS,   IDS),
    ("Cache-MAB (contextual)", "MAB-",   MABS, MABS),
    ("Tabular Q (256 cells)",  "TABQ-",  TQS,  TQS),
]
LEARNED_NAMES = [x[0] for x in LEARNED]

# --- loading -----------------------------------------------------------------
_CACHE = {}


def load(dirname):
    d = str(dirname)
    if d not in _CACHE:
        p = pathlib.Path(d)
        _CACHE[d] = collect(d) if p.is_dir() else pd.DataFrame()
    return _CACHE[d]


def runs_for(rel_dir, stem):
    """Classical runs for one policy.

    Prefer the campaign's own copy, so that a self-contained campaign such as
    substrate3 draws its classical and learned halves from the same binary.
    Fall back to the shared archive for the older campaigns that never had one.
    """
    df = load(S2 / rel_dir)
    if df.empty:
        df = load(REV / rel_dir)
    if df.empty:
        return df
    return df[df["config"] == stem]


def cell(rel_dir, stem):
    """Per-run frame for one (policy, training seed, arm) cell."""
    df = load(S2 / rel_dir)
    if df.empty:
        return df
    return df[df["config"] == stem]


def degraded_in(df):
    """Integrity gate: every learned run must report rlDegraded = 0 and
    rlFallbacks = 0."""
    if df.empty:
        return []
    bad = df[(df["rlDegraded"].fillna(0) != 0) | (df["rlFallbacks"].fillna(0) != 0)]
    return [(r["config"], int(r["seed"])) for _, r in bad.iterrows()]


# --- effect sizes ------------------------------------------------------------
def _hedges_g(a, b):
    a, b = np.asarray(a, float), np.asarray(b, float)
    na, nb = len(a), len(b)
    if na < 2 or nb < 2:
        return float("nan")
    sp = math.sqrt(((na - 1) * a.var(ddof=1) + (nb - 1) * b.var(ddof=1)) / (na + nb - 2))
    if sp == 0:
        return float("nan")
    return (b.mean() - a.mean()) / sp * (1 - 3.0 / (4 * (na + nb) - 9))


def _dz(diffs):
    d = np.asarray(diffs, float)
    s = d.std(ddof=1)
    return float(d.mean() / s) if s else float("nan")


# --- per-policy substrate deltas --------------------------------------------
def classical_rows(metric=METRIC):
    rows = []
    for name, cr, ci in CLASSICAL:
        a = runs_for("std", cr)[metric].astype(float).values
        b = runs_for("ideal", ci)[metric].astype(float).values
        t, p = sps.ttest_ind(a, b, equal_var=False)
        rows.append(dict(
            policy=name, cls="classical", trainedOn="n/a (no training)",
            design="unpaired Welch, test seeds", nUnitReal=len(a), nUnitIdeal=len(b),
            real=a.mean(), real_ci=ci_t(a), ideal=b.mean(), ideal_ci=ci_t(b),
            delta=b.mean() - a.mean(),
            relPct=100.0 * (b.mean() - a.mean()) / a.mean(),
            t=t, p_raw=p, effect=_hedges_g(a, b), effect_kind="Hedges g"))
    return rows


def _seed_means(rel_dir, prefix, tags, metric):
    vals, used, deg, missing = [], [], [], []
    for tg in tags:
        d = cell(rel_dir, prefix + tg)
        deg += degraded_in(d)
        if len(d) == 0:
            missing.append(f"{rel_dir}/{prefix}{tg}")
            continue
        vals.append(d[metric].astype(float).mean())
        used.append((tg, len(d)))
    return np.array(vals), used, deg, missing


def learned_rows(arm="transfer", metric=METRIC):
    """arm = 'transfer' (paired) or 'persubstrate' (unpaired)."""
    ideal_dir = "test_transfer" if arm == "transfer" else "test_idealtrained"
    rows, per_seed, deg, missing = [], [], [], []
    for name, pre, tags_r, tags_i in LEARNED:
        ti = tags_r if arm == "transfer" else tags_i
        xr, ur, d1, m1 = _seed_means("test_real", pre, tags_r, metric)
        xi, ui, d2, m2 = _seed_means(ideal_dir, pre, ti, metric)
        deg += d1 + d2
        missing += m1 + m2
        if len(xr) == 0 or len(xi) == 0:
            continue
        if arm == "transfer":
            n = min(len(xr), len(xi))
            xr, xi = xr[:n], xi[:n]
            t, p = sps.ttest_rel(xi, xr)
            d = xi - xr
            design, eff, ek = ("paired t, training seeds", _dz(d), "Cohen dz")
            dci = ci_t(d)
            for k in range(n):
                per_seed.append(dict(policy=name, arm=arm, trainSeed=ur[k][0],
                                     nTestRuns=ur[k][1], real=xr[k], ideal=xi[k],
                                     delta=xi[k] - xr[k],
                                     relPct=100.0 * (xi[k] - xr[k]) / xr[k]))
        else:
            t, p = sps.ttest_ind(xi, xr, equal_var=False)
            design, eff, ek = ("unpaired Welch, training seeds",
                               _hedges_g(xr, xi), "Hedges g")
            dci = float("nan")
            for k, (tg, nr) in enumerate(ur):
                per_seed.append(dict(policy=name, arm=arm, trainSeed=tg,
                                     substrate="realistic", value=xr[k], nTestRuns=nr))
            for k, (tg, nr) in enumerate(ui):
                per_seed.append(dict(policy=name, arm=arm, trainSeed=tg,
                                     substrate="idealized", value=xi[k], nTestRuns=nr))
        rows.append(dict(
            policy=name, cls="learned",
            trainedOn=("802.15.4 only, transferred zero-shot" if arm == "transfer"
                       else "each substrate separately"),
            design=design, nUnitReal=len(xr), nUnitIdeal=len(xi),
            real=xr.mean(), real_ci=ci_t(xr), ideal=xi.mean(), ideal_ci=ci_t(xi),
            delta=xi.mean() - xr.mean(), delta_ci=dci,
            relPct=100.0 * (xi.mean() - xr.mean()) / xr.mean(),
            t=t, p_raw=p, effect=eff, effect_kind=ek))
    return rows, pd.DataFrame(per_seed), sorted(set(deg)), sorted(set(missing))


def main_table(arm="transfer", metric=METRIC):
    """The declared Holm family: every per-policy substrate test in the table."""
    cls = classical_rows(metric)
    lrn, per_seed, deg, missing = learned_rows(arm, metric)
    t = pd.DataFrame(cls + lrn)
    adj, rej = holm(t["p_raw"].values)
    t["p_holm"] = adj
    t["significant_raw"] = t["p_raw"] < 0.05
    t["significant_holm"] = rej
    t["familySize"] = len(t)
    return t, per_seed, deg, missing


# --- the class contrast (task 3) --------------------------------------------
def class_contrast(table, value="absRelPct"):
    """Exact permutation test of learned versus classical on a policy-level
    statistic, with Mann-Whitney and Welch cross-checks and Cliff's delta.

    Assumptions: the unit is the POLICY, which is the level the paper's claim
    is about and the only level at which the two groups are exchangeable under
    the null. The permutation null assumes only that a policy's substrate
    effect is exchangeable across the class label; it assumes NO normality and
    NO equal variance. It does not exploit within-policy precision, so it is
    conservative. It does assume policies are independent draws, which is only
    approximately true -- several classical policies share LRU and all share
    the topology and workload.
    """
    t = table.copy()
    t["absRelPct"] = t["relPct"].abs()
    a = t.loc[t["cls"] == "learned", value].values
    b = t.loc[t["cls"] == "classical", value].values
    obs = a.mean() - b.mean()
    pool = np.concatenate([a, b])
    n, k = len(pool), len(a)
    cnt = tot = 0
    for idx in itertools.combinations(range(n), k):
        m = np.zeros(n, dtype=bool)
        m[list(idx)] = True
        tot += 1
        if abs(pool[m].mean() - pool[~m].mean()) >= abs(obs) - 1e-12:
            cnt += 1
    U, p_mw = sps.mannwhitneyu(a, b, alternative="two-sided")
    tw, p_w = sps.ttest_ind(a, b, equal_var=False)
    gt = sum(1 for x in a for y in b if x > y)
    lt = sum(1 for x in a for y in b if x < y)
    return dict(value=value, learned_n=len(a), classical_n=len(b),
                learned_mean=float(a.mean()), classical_mean=float(b.mean()),
                learned_vals=[round(float(v), 4) for v in a],
                classical_vals=[round(float(v), 4) for v in b],
                diff=float(obs), p_perm=cnt / tot, perm_points=tot,
                U=float(U), p_mannwhitney=float(p_mw),
                t_welch=float(tw), p_welch=float(p_w),
                cliffs_delta=(gt - lt) / (len(a) * len(b)),
                rank_biserial=2 * U / (len(a) * len(b)) - 1)


def hierarchical_bootstrap(per_seed, classical_runs, n_boot=20000, seed=12345):
    """Bootstrap CI for the class difference in mean |relative substrate delta|.

    Resamples POLICIES with replacement within each class and, within each
    resampled policy, its replicate units (training seeds for learned policies,
    test seeds for classical ones). This respects the nesting instead of
    treating runs as independent, which a run-level ANOVA would.
    """
    rng = np.random.default_rng(seed)
    learned = {p: s["relPct"].values for p, s in per_seed.groupby("policy")}
    lpols, cpols = list(learned), list(classical_runs)
    out = np.empty(n_boot)
    for i in range(n_boot):
        lv = [abs(rng.choice(learned[p], size=len(learned[p]), replace=True).mean())
              for p in rng.choice(lpols, size=len(lpols), replace=True)]
        cv = []
        for p in rng.choice(cpols, size=len(cpols), replace=True):
            a, b = classical_runs[p]
            aa = rng.choice(a, size=len(a), replace=True)
            bb = rng.choice(b, size=len(b), replace=True)
            cv.append(abs(100.0 * (bb.mean() - aa.mean()) / aa.mean()))
        out[i] = np.mean(lv) - np.mean(cv)
    return dict(mean=float(out.mean()), lo=float(np.percentile(out, 2.5)),
                hi=float(np.percentile(out, 97.5)),
                p_two_sided=float(2 * min((out <= 0).mean(), (out >= 0).mean())),
                n_boot=n_boot)


def rank_table(realistic, idealized):
    r = sorted(realistic.items(), key=lambda kv: -kv[1])
    i = sorted(idealized.items(), key=lambda kv: -kv[1])
    rr = {k: n + 1 for n, (k, _) in enumerate(r)}
    ri = {k: n + 1 for n, (k, _) in enumerate(i)}
    return pd.DataFrame([dict(policy=k, real=realistic[k], ideal=idealized[k],
                              rankReal=rr[k], rankIdeal=ri[k],
                              changed=rr[k] != ri[k]) for k in realistic]
                        ).sort_values("rankReal")
