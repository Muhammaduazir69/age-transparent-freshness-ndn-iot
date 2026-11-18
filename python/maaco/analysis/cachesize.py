"""Merge the cache-size parameter study results into one tidy CSV.

Layout: results/cachesize/CS{5,20,50}/<config>-s<seed>.sca hold the swept
cells; the CS=10 cells reuse the stage-evaluation runs in results/eval/
(csMaxEntries=10 is the thesis default for every SmartFarm-B-* config).

Usage:
  python -m maaco.analysis.cachesize ../results -o ../results/cachesize/cachesize_summary.csv
"""

import argparse
import pathlib

import pandas as pd

from .collect import collect

POLICIES = ["SmartFarm-B-MAACO", "SmartFarm-B-DQN-LRU", "SmartFarm-B-LCE-LRU",
            "SmartFarm-B-Prob-LRU", "SmartFarm-B-LCE-LFF"]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("results_root")
    ap.add_argument("-o", "--output", required=True)
    args = ap.parse_args()

    root = pathlib.Path(args.results_root)
    frames = []
    for cs in (5, 20, 50):
        d = root / "cachesize" / f"CS{cs}"
        if not d.exists():
            continue
        df = collect(str(d))
        if df.empty:
            continue
        df["csSize"] = cs
        frames.append(df)

    # CS=10: the stage evaluation runs (default csMaxEntries=10)
    df10 = collect(str(root / "eval"))
    df10 = df10[df10["config"].isin(POLICIES)].copy()
    df10["csSize"] = 10
    frames.append(df10)

    out = pd.concat(frames, ignore_index=True)
    out = out[out["config"].isin(POLICIES)]
    out.to_csv(args.output, index=False)
    print(out.groupby(["config", "csSize"])
             .agg(runs=("seed", "count"), hit=("cacheHitRatio", "mean"),
                  fresh=("freshnessSatisfactionRatio", "mean")).round(4)
             .to_string())
    print(f"written: {args.output} ({len(out)} rows)")


if __name__ == "__main__":
    main()
