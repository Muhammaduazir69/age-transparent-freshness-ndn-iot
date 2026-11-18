"""Statistics for the reviewer-response campaign.

Two corrections relative to `analysis.stats`:

1. **t-distribution confidence intervals.** The existing `ci95` multiplies the
   standard error by 1.96, the normal quantile. With n = 10 seeds the correct
   multiplier is t(0.975, 9) = 2.262, so every published interval was about
   15% too narrow. Intervals here use the t quantile for the actual n.

2. **Holm-Bonferroni across the whole family of pairwise tests.** The campaign
   runs dozens of Welch tests on the same data; reporting each at alpha = 0.05
   inflates the family-wise error rate. `holm()` returns adjusted p-values and
   the survival decision for every comparison in a family.

Also provides ranking tables (`rank_table`) that report, for one metric, every
policy's mean with its t-based CI, its rank, and whether its interval overlaps
the leader's, which is what an "is it really fifth of six" question needs.
"""

import argparse
import itertools
import pathlib

import numpy as np
import pandas as pd
from scipy import stats as sps

METRICS = [
    ("cacheHitRatio", "higher"),
    ("cacheHitRatioCorrected", "higher"),
    ("consumerCacheHitRatio", "higher"),
    ("freshnessSatisfactionRatio", "higher"),
    ("staleDeliveryRate", "lower"),
    ("meanRetrievalLatency", "lower"),
    ("meanEnergyConsumedPerNodeJ", "lower"),
    ("energyPerDeliveryJ", "lower"),
    ("meanHopCount", "lower"),
    ("satisfactionRatio", "higher"),
]


def ci_t(x, conf=0.95):
    """Half-width of the `conf` confidence interval using the t quantile."""
    x = np.asarray(x, dtype=float)
    x = x[np.isfinite(x)]
    n = len(x)
    if n < 2:
        return float("nan")
    return float(sps.t.ppf(0.5 + conf / 2.0, n - 1) * x.std(ddof=1) / np.sqrt(n))


def ci_z(x, conf=0.95):
    """The interval the previous campaign reported (normal quantile)."""
    x = np.asarray(x, dtype=float)
    x = x[np.isfinite(x)]
    if len(x) < 2:
        return float("nan")
    return float(1.96 * x.std(ddof=1) / np.sqrt(len(x)))


def holm(pvals, alpha=0.05):
    """Holm-Bonferroni step-down. Returns (adjusted_p, reject) arrays."""
    p = np.asarray(pvals, dtype=float)
    m = len(p)
    order = np.argsort(p)
    adj = np.empty(m, dtype=float)
    running = 0.0
    for rank, idx in enumerate(order):
        val = (m - rank) * p[idx]
        running = max(running, val)          # enforce monotonicity
        adj[idx] = min(1.0, running)
    return adj, adj < alpha


def summarize(df, metric, configs=None):
    """Per-config mean / SD / CI table for one metric."""
    configs = configs or sorted(df["config"].unique())
    rows = []
    for c in configs:
        x = df.loc[df["config"] == c, metric].astype(float).values
        x = x[np.isfinite(x)]
        if len(x) == 0:
            continue
        rows.append({"config": c, "n": len(x), "mean": x.mean(),
                     "sd": x.std(ddof=1) if len(x) > 1 else 0.0,
                     "ci95_t": ci_t(x), "ci95_z_legacy": ci_z(x),
                     "min": x.min(), "max": x.max()})
    return pd.DataFrame(rows)


def rank_table(df, metric, configs=None, better="higher"):
    """Ranking with t-based CIs and overlap-with-leader flags."""
    s = summarize(df, metric, configs)
    if s.empty:
        return s
    s = s.sort_values("mean", ascending=(better == "lower")).reset_index(drop=True)
    s["rank"] = np.arange(1, len(s) + 1)
    lead_lo = s.loc[0, "mean"] - s.loc[0, "ci95_t"]
    lead_hi = s.loc[0, "mean"] + s.loc[0, "ci95_t"]
    s["ci_lo"] = s["mean"] - s["ci95_t"]
    s["ci_hi"] = s["mean"] + s["ci95_t"]
    s["overlapsLeaderCI"] = (s["ci_hi"] >= lead_lo) & (s["ci_lo"] <= lead_hi)
    return s


def pairwise(df, metric, configs, better="higher", alpha=0.05):
    """All pairwise Welch tests within one family, Holm-corrected."""
    rows = []
    for a, b in itertools.combinations(configs, 2):
        xa = df.loc[df["config"] == a, metric].astype(float).values
        xb = df.loc[df["config"] == b, metric].astype(float).values
        xa, xb = xa[np.isfinite(xa)], xb[np.isfinite(xb)]
        if len(xa) < 2 or len(xb) < 2:
            continue
        t, p = sps.ttest_ind(xa, xb, equal_var=False)
        diff = xa.mean() - xb.mean()
        rows.append({"metric": metric, "a": a, "b": b,
                     "a_mean": xa.mean(), "a_ci95_t": ci_t(xa),
                     "b_mean": xb.mean(), "b_ci95_t": ci_t(xb),
                     "diff": diff,
                     "rel_pct": 100.0 * diff / abs(xb.mean()) if xb.mean() else np.nan,
                     "t": t, "p_raw": p})
    out = pd.DataFrame(rows)
    if out.empty:
        return out
    adj, rej = holm(out["p_raw"].values, alpha)
    out["p_holm"] = adj
    out["significant_raw"] = out["p_raw"] < alpha
    out["significant_holm"] = rej
    out["family_size"] = len(out)
    out["holm_threshold_smallest"] = alpha / len(out)
    return out


def versus(df, focus, others, metrics=None, alpha=0.05):
    """Focus-vs-each-comparator table across metrics, Holm-corrected over the
    whole (metric x comparator) family, which is the family a reader of the
    results chapter is actually looking at."""
    metrics = metrics or METRICS
    rows = []
    for key, better in metrics:
        a = df.loc[df["config"] == focus, key].astype(float).values
        a = a[np.isfinite(a)]
        if len(a) < 2:
            continue
        for other in others:
            b = df.loc[df["config"] == other, key].astype(float).values
            b = b[np.isfinite(b)]
            if len(b) < 2:
                continue
            t, p = sps.ttest_ind(a, b, equal_var=False)
            diff = a.mean() - b.mean()
            win = (diff > 0) if better == "higher" else (diff < 0)
            rows.append({"metric": key, "better": better, "focus": focus,
                         "comparator": other,
                         "focus_mean": a.mean(), "focus_ci95_t": ci_t(a),
                         "comp_mean": b.mean(), "comp_ci95_t": ci_t(b),
                         "diff": diff,
                         "rel_pct": 100.0 * diff / abs(b.mean()) if b.mean() else np.nan,
                         "t": t, "p_raw": p, "direction": "win" if win else "loss"})
    out = pd.DataFrame(rows)
    if out.empty:
        return out
    adj, rej = holm(out["p_raw"].values, alpha)
    out["p_holm"] = adj
    out["significant_raw"] = out["p_raw"] < alpha
    out["significant_holm"] = rej
    out["verdict"] = np.where(rej, out["direction"],
                              np.where(out["significant_raw"],
                                       out["direction"] + " (fails Holm)", "tie"))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("summary")
    ap.add_argument("--metric", default="cacheHitRatio")
    ap.add_argument("--focus")
    ap.add_argument("--configs", nargs="*")
    ap.add_argument("-o", "--outdir")
    args = ap.parse_args()

    df = pd.read_csv(args.summary)
    configs = args.configs or sorted(df["config"].unique())
    pd.set_option("display.width", 250)

    r = rank_table(df, args.metric, configs)
    print(r.to_string(index=False))
    pw = pairwise(df, args.metric, configs)
    if args.outdir:
        d = pathlib.Path(args.outdir)
        d.mkdir(parents=True, exist_ok=True)
        r.to_csv(d / f"rank_{args.metric}.csv", index=False)
        pw.to_csv(d / f"pairwise_{args.metric}.csv", index=False)
        if args.focus:
            versus(df, args.focus,
                   [c for c in configs if c != args.focus]).to_csv(
                d / f"versus_{args.focus}.csv", index=False)


if __name__ == "__main__":
    main()
