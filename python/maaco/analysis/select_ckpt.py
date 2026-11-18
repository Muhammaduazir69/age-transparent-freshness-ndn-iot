"""Apply the pre-registered checkpoint-selection rule to a validation sweep.

Rule (unchanged from the thesis campaign, applied identically to MAACO and to
every learned baseline so the comparison is like for like):

    score = w_hit * cacheHitRatio - w_stale * staleDeliveryRate

averaged over validation seeds 100, 101 and 102 only (disjoint from the test
seeds 0..9); ties within 0.005 of the best score are broken by the headline
cache hit ratio.

Usage:
  python -m maaco.analysis.select_ckpt <valDir> [--w-hit 1] [--w-stale 1]
      [--group-regex '(?P<run>.*)~(?P<ckpt>ep\\d+|final)']
Prints the winner per run group and writes selection.csv into <valDir>.
"""

import argparse
import pathlib
import re

import pandas as pd

from .collect import collect

DEFAULT_RE = r"^(?P<run>.+)~(?P<ckpt>ep\d+|final)$"


def select(val_dir, w_hit=1.0, w_stale=1.0, tie=0.005,
           group_regex=DEFAULT_RE):
    df = collect(val_dir)
    if df.empty:
        raise SystemExit(f"no results in {val_dir}")
    pat = re.compile(group_regex)
    runs, ckpts = [], []
    for c in df["config"]:
        m = pat.match(c)
        runs.append(m.group("run") if m else c)
        ckpts.append(m.group("ckpt") if m else "")
    df["run"] = runs
    df["ckpt"] = ckpts
    df["score"] = w_hit * df["cacheHitRatio"] - w_stale * df["staleDeliveryRate"]

    g = (df.groupby(["run", "ckpt"])
           .agg(seeds=("seed", "count"),
                score=("score", "mean"),
                hit=("cacheHitRatio", "mean"),
                hitCorrected=("cacheHitRatioCorrected", "mean"),
                ichr=("consumerCacheHitRatio", "mean"),
                fresh=("freshnessSatisfactionRatio", "mean"),
                stale=("staleDeliveryRate", "mean"),
                rlDegraded=("rlDegraded", "max"),
                rlFallbacks=("rlFallbacks", "max"))
           .reset_index())

    winners = []
    for run, sub in g.groupby("run"):
        best = sub["score"].max()
        cand = sub[sub["score"] >= best - tie]
        w = cand.sort_values("hit", ascending=False).iloc[0]
        winners.append({"run": run, "selected": w["ckpt"],
                        "valScore": w["score"], "valHit": w["hit"],
                        "valFresh": w["fresh"], "valStale": w["stale"],
                        "valIchr": w["ichr"],
                        "candidates": len(sub),
                        "tiedWithin": len(cand),
                        "rlDegradedMax": w["rlDegraded"],
                        "rlFallbacksMax": w["rlFallbacks"]})
    return g, pd.DataFrame(winners).sort_values("run")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("val_dir")
    ap.add_argument("--w-hit", type=float, default=1.0)
    ap.add_argument("--w-stale", type=float, default=1.0)
    ap.add_argument("--tie", type=float, default=0.005)
    ap.add_argument("--group-regex", default=DEFAULT_RE)
    args = ap.parse_args()

    g, win = select(args.val_dir, args.w_hit, args.w_stale, args.tie,
                    args.group_regex)
    d = pathlib.Path(args.val_dir)
    g.to_csv(d / "validation_all.csv", index=False)
    win.to_csv(d / "selection.csv", index=False)
    pd.set_option("display.width", 220)
    print(win.to_string(index=False))
    print(f"\nwritten: {d/'selection.csv'} and {d/'validation_all.csv'}")


if __name__ == "__main__":
    main()
