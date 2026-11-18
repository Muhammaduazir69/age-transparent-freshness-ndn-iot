"""Per-metric means, 95% CIs, and Welch t-tests between an RL config and each
baseline config, from a collected summary CSV (analysis.collect schema).

Usage:
  python -m maaco.analysis.stats summary.csv --rl SmartFarm-B-MAACO \
      --baselines SmartFarm-B-LCE-LRU SmartFarm-B-Prob-LRU [-o out.csv]
"""

import argparse

import numpy as np
import pandas as pd
from scipy import stats as sps

METRICS = [
    ("cacheHitRatio", "higher"),
    ("freshnessSatisfactionRatio", "higher"),
    ("staleDeliveryRate", "lower"),
    ("meanRetrievalLatency", "lower"),
    ("meanHopCount", "lower"),
    ("satisfactionRatio", "higher"),
    ("meanContentAge", "lower"),
]


def ci95(x):
    x = np.asarray(x, dtype=float)
    x = x[~np.isnan(x)]
    if len(x) < 2:
        return 0.0
    return 1.96 * x.std(ddof=1) / np.sqrt(len(x))


def welch_table(df, rl_config, baselines, metrics=None):
    """Return a tidy DataFrame: metric x baseline with means, CIs, p-values,
    and win/loss from the RL config's perspective."""
    metrics = metrics or METRICS
    rows = []
    rl = df[df["config"] == rl_config]
    for key, better in metrics:
        a = rl[key].dropna().values
        for b_cfg in baselines:
            b = df.loc[df["config"] == b_cfg, key].dropna().values
            if len(a) < 2 or len(b) < 2:
                continue
            t, p = sps.ttest_ind(a, b, equal_var=False)
            diff = a.mean() - b.mean()
            if better == "higher":
                verdict = "win" if diff > 0 else "loss"
            else:
                verdict = "win" if diff < 0 else "loss"
            if p >= 0.05:
                verdict = "tie(ns)"
            rows.append({
                "metric": key, "better": better, "baseline": b_cfg,
                "rl_mean": a.mean(), "rl_ci95": ci95(a),
                "base_mean": b.mean(), "base_ci95": ci95(b),
                "diff": diff,
                "rel_pct": 100.0 * diff / abs(b.mean()) if b.mean() else np.nan,
                "t": t, "p": p, "verdict": verdict,
            })
    return pd.DataFrame(rows)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("summary")
    ap.add_argument("--rl", required=True)
    ap.add_argument("--baselines", nargs="+", required=True)
    ap.add_argument("-o", "--output")
    args = ap.parse_args()

    df = pd.read_csv(args.summary)
    out = welch_table(df, args.rl, args.baselines)
    if args.output:
        out.to_csv(args.output, index=False)
    pd.set_option("display.width", 200)
    print(out.to_string(index=False))


if __name__ == "__main__":
    main()
