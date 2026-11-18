"""Build every table for the enlarged substrate campaign.

    python -m maaco.analysis.substrate2_report [metric]

Writes CSVs into results/reviewer/substrate2/ and prints the tables. No number
in the output is typed by hand; everything derives from the .sca files.
"""

import json
import sys

import numpy as np
import pandas as pd
from scipy import stats as sps

from . import substrate2 as S
from .reviewer import ci_t

OUT = S.S2
pd.set_option("display.width", 300)
pd.set_option("display.max_columns", 60)

SHOW = ["policy", "cls", "trainedOn", "design", "nUnitReal", "nUnitIdeal",
        "real", "real_ci", "ideal", "ideal_ci", "relPct", "t", "p_raw",
        "p_holm", "significant_raw", "significant_holm", "effect"]


def rank_stability(per_seed, metric):
    """Is the 1st-versus-4th rank change an artifact of three training seeds?
    Rank every individual training seed's model against the nine fixed
    policies' means on each substrate."""
    fx = S.classical_rows(metric)
    fr = {r["policy"]: r["real"] for r in fx}
    fi = {r["policy"]: r["ideal"] for r in fx}
    rows = []
    for _, r in per_seed.iterrows():
        rr = 1 + sum(1 for v in fr.values() if v > r["real"])
        ri = 1 + sum(1 for v in fi.values() if v > r["ideal"])
        rows.append(dict(policy=r["policy"], trainSeed=r["trainSeed"],
                         real=r["real"], ideal=r["ideal"],
                         rankReal=rr, rankIdeal=ri, dropped=ri > rr))
    return pd.DataFrame(rows)


FIXED_STEM = {"LCE + LRU": "LCE-LRU", "LCE + LFU-Aged(60s)": "LCE-LFUA",
              "Prob(0.4, tuned) + LRU": "ProbTuned-LRU"}
FIXED_CFG_B = {"LCE + LRU": "SmartFarm-B-LCE-LRU",
               "LCE + LFU-Aged(60s)": "SmartFarm-B-LCE-LFUA",
               "Prob(0.4, tuned) + LRU": "SmartFarm-B-ProbTuned-LRU"}
FIXED_CFG_I = {"LCE + LRU": "SmartFarm-I-LCE-LRU",
               "LCE + LFU-Aged(60s)": "SmartFarm-I-LCE-LFUA",
               "Prob(0.4, tuned) + LRU": "SmartFarm-I-ProbTuned-LRU"}

ABL_SUBS = [
    ("B",  "B  full 802.15.4 + battery (reference)", "test_real",     "std"),
    ("AE", "AE energy model removed",                "abl_AE",        None),
    # ("AC", ...) withdrawn 2026-09-30: `ignoreInterference` is not read by the
    # 802.15.4 receiver chain, so this arm reproduced the reference bit for bit
    # and measured nothing.  See Revision_Plan/reviewer_proof_audit.md.
    ("AB", "AB zero contention window",              "abl_AB",        None),
    ("AM", "AM link idealized, battery kept",        "abl_AM",        None),
    ("I",  "I  fully idealized",                     "test_transfer", "ideal"),
]


def ablation_table(metric):
    rows = []
    for tag, label, d, fixed in ABL_SUBS:
        df = S.load(S.S2 / d)
        for name, pre, tags_r, _ in S.LEARNED:
            vals = []
            for tg in tags_r:
                c = df[df["config"] == pre + tg] if not df.empty else df
                if len(c):
                    vals.append(c[metric].astype(float).mean())
            if vals:
                v = np.array(vals)
                rows.append(dict(substrate=label, policy=name, cls="learned",
                                 unit="training seed", n=len(v),
                                 mean=v.mean(), ci=ci_t(v)))
        for pol, stem in FIXED_STEM.items():
            if fixed == "std":
                c = S.runs_for("std", FIXED_CFG_B[pol])
            elif fixed == "ideal":
                c = S.runs_for("ideal", FIXED_CFG_I[pol])
            else:
                c = df[df["config"] == stem] if not df.empty else df
            if len(c):
                v = c[metric].astype(float).values
                rows.append(dict(substrate=label, policy=pol, cls="classical",
                                 unit="test seed", n=len(v),
                                 mean=v.mean(), ci=ci_t(v)))
    return pd.DataFrame(rows)


def ablation_attribution(ab):
    """How much of each learned policy's full substrate shift does each single
    ingredient reproduce?"""
    rows = []
    ref = ab[ab.substrate.str.startswith("B ")].set_index("policy")["mean"]
    full = ab[ab.substrate.str.startswith("I ")].set_index("policy")["mean"]
    for tag, label, _, _ in ABL_SUBS:
        if tag in ("B", "I"):
            continue
        cur = ab[ab.substrate.str.startswith(tag + " ")].set_index("policy")["mean"]
        for pol in cur.index:
            if pol not in ref or pol not in full:
                continue
            tot = full[pol] - ref[pol]
            part = cur[pol] - ref[pol]
            rows.append(dict(ingredient=label, policy=pol,
                             shift=part, fullShift=tot,
                             shareOfFullPct=100.0 * part / tot if tot else np.nan))
    return pd.DataFrame(rows)


def dump(name, obj):
    json.dump(obj, open(OUT / name, "w"), indent=2, default=float)


def main():
    metric = sys.argv[1] if len(sys.argv) > 1 else S.METRIC
    OUT.mkdir(parents=True, exist_ok=True)
    suffix = "" if metric == "cacheHitRatio" else "_" + metric

    print(f"################ METRIC: {metric} ################")
    for arm in ("transfer", "persubstrate"):
        t, per_seed, deg, missing = S.main_table(arm, metric)
        t.to_csv(OUT / f"policy_substrate_table_{arm}{suffix}.csv", index=False)
        per_seed.to_csv(OUT / f"per_training_seed_{arm}{suffix}.csv", index=False)
        print(f"\n================ ARM: {arm} ================")
        if missing:
            print("  MISSING CELLS:", missing[:12], "..." if len(missing) > 12 else "")
        print("  degraded runs:", deg if deg else "none")
        print(t[SHOW].round(5).to_string(index=False))

        cc = S.class_contrast(t)
        ccs = S.class_contrast(t, value="relPct")
        classical_runs = {n: (S.runs_for("std", a)[metric].astype(float).values,
                              S.runs_for("ideal", b)[metric].astype(float).values)
                          for n, a, b in S.CLASSICAL}
        hb = (S.hierarchical_bootstrap(per_seed, classical_runs)
              if arm == "transfer" and not per_seed.empty and "relPct" in per_seed
              else None)
        dump(f"class_contrast_{arm}{suffix}.json",
             {"absolute": cc, "signed": ccs, "hierarchical_bootstrap": hb})
        print("\n  -- class contrast on |relative substrate delta| --")
        for k, v in cc.items():
            print(f"     {k}: {v}")
        print("  -- same test on the signed delta --")
        for k in ("learned_mean", "classical_mean", "diff", "p_perm", "p_welch"):
            print(f"     {k}: {ccs[k]}")
        if hb:
            print("  -- hierarchical bootstrap of the class difference --")
            for k, v in hb.items():
                print(f"     {k}: {v}")

        real = dict(zip(t["policy"], t["real"]))
        ideal = dict(zip(t["policy"], t["ideal"]))
        rk = S.rank_table(real, ideal)
        rk.to_csv(OUT / f"rank_table_{arm}{suffix}.csv", index=False)
        print("\n  -- ranking --")
        print(rk.round(5).to_string(index=False))
        print(f"  policies whose rank changes: {int(rk['changed'].sum())} of {len(rk)}")

        if arm == "transfer" and not per_seed.empty:
            rs = rank_stability(per_seed, metric)
            rs.to_csv(OUT / f"rank_stability{suffix}.csv", index=False)
            print("\n  -- rank stability, one model per training seed --")
            for pol in rs["policy"].unique():
                s = rs[rs["policy"] == pol]
                print(f"     {pol}: realistic ranks {sorted(s['rankReal'])}, "
                      f"idealized ranks {sorted(s['rankIdeal'])}, "
                      f"rank drops in {int(s['dropped'].sum())}/{len(s)} seeds")

    try:
        ab = ablation_table(metric)
        if not ab.empty:
            ab.to_csv(OUT / f"ablation_table{suffix}.csv", index=False)
            print("\n================ ABLATION ================")
            print(ab.round(5).to_string(index=False))
            at = ablation_attribution(ab)
            at.to_csv(OUT / f"ablation_attribution{suffix}.csv", index=False)
            print("\n  -- share of the full substrate shift reproduced --")
            print(at.round(4).to_string(index=False))
    except Exception as e:
        print(f"\n(ablation not available: {e})")


if __name__ == "__main__":
    main()
