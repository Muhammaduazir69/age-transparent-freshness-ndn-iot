"""Reward-weight sweep Pareto figure (thesis 4.6): cache hit ratio vs
freshness satisfaction over the reward-design pilots, plus the retrained
original-reward control as a like-for-like reference mark.

Every point on this figure comes from the SAME three validation seeds
(100--102) and the same 45-episode pilot budget, so the points are
comparable to each other and to the control.  Test-seed baseline values are
deliberately NOT overlaid: they come from a different seed set and would
make the comparison look tighter or looser than it is.

Usage:
  python -m maaco.analysis.pareto ../results/final/pilot_reward_design.csv \
      ../results/final/validation_original_reward.csv -o ../results/figures
"""

import argparse
import pathlib

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

from .plots import (HILITE, GRID, INK, INK_MUTED, MUTED, ci95, Scale,
                    _legend, _save, _wrap)

# pilot tag -> (label, is the swept insertion-cost front?)
SWEEP_LABELS = {
    "B0":  (r"$w_{ins}$=0",    True),
    "B2":  (r"$w_{ins}$=0.2",  True),
    "C35": (r"$w_{ins}$=0.35", True),
    "C50": (r"$w_{ins}$=0.5",  True),
    "C70": (r"$w_{ins}$=0.7",  True),
    "BN":  ("coordination off", False),
    "BG":  ("5 s admission grace", False),
}
CANONICAL = "B2"           # the design carried into the full campaign

# hand-tuned label placement: these points sit close together, so the
# offsets steer each label into free space instead of onto a neighbour
OFFSET = {                 # tag -> (dx pt, dy pt, ha, va)
    "B0":  (0, 10, "center", "bottom"),
    "B2":  (10, 3, "left", "bottom"),
    "C35": (0, -11, "center", "top"),
    "C50": (10, -3, "left", "top"),
    "C70": (0, 10, "center", "bottom"),
    "BN":  (0, 10, "center", "bottom"),
    "BG":  (10, 0, "left", "center"),
    "orig": (-10, 0, "right", "center"),
}


def _final_rows(path, suffix="~final"):
    """Mean hit / freshness per pilot tag, from the last saved checkpoint."""
    df = pd.read_csv(path)
    out = {}
    for cfg, sub in df.groupby("config"):
        if not str(cfg).endswith(suffix):
            continue
        tag = str(cfg)[: -len(suffix)]
        out[tag] = {
            "hit": sub.cacheHitRatio.mean(),
            "hit_ci": ci95(sub.cacheHitRatio),
            "fresh": sub.freshnessSatisfactionRatio.mean(),
            "fresh_ci": ci95(sub.freshnessSatisfactionRatio),
            "seeds": len(sub),
        }
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("pilot_csv")
    ap.add_argument("control_csv", nargs="?", default=None)
    ap.add_argument("-o", "--outdir", default="../results/figures")
    args = ap.parse_args()

    outdir = pathlib.Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    pilots = _final_rows(args.pilot_csv)
    rows = []
    for tag, (label, on_front) in SWEEP_LABELS.items():
        if tag not in pilots:
            continue
        rows.append({"point": tag, "label": label, "kind":
                     "insert_cost_sweep" if on_front else "structural_variant",
                     **pilots[tag]})
    control = _final_rows(args.control_csv) if args.control_csv else {}
    for tag, r in control.items():
        rows.append({"point": "original reward", "label": "original reward",
                     "kind": "control", **r})
    sweep = pd.DataFrame(rows)
    sweep.to_csv(outdir / "pareto_sweep.csv", index=False)

    fig_w, print_w = 9.0, 5.67     # chapter includes this at 0.9\textwidth
    sc = Scale(fig_w, print_w)
    fig, ax = plt.subplots(figsize=(fig_w, fig_w * 0.70))

    # No connecting line: at three validation seeds and a 45-episode budget
    # these points do not trace a monotone front, and joining them would
    # assert an ordering the measurements do not support.
    front = sweep[sweep.kind == "insert_cost_sweep"]
    ax.errorbar(front.hit, front.fresh, xerr=front.hit_ci, yerr=front.fresh_ci,
                color=HILITE, ls="none", marker="o", ms=sc.ms,
                capsize=sc.capsize, elinewidth=sc.axlw, capthick=sc.axlw,
                mec="white", mew=sc.pts(0.9), zorder=5,
                label="Insertion-cost weight sweep")

    other = sweep[sweep.kind == "structural_variant"]
    ax.scatter(other.hit, other.fresh, marker="^", s=sc.pts(6.5) ** 2,
               color=MUTED, edgecolor=INK_MUTED, linewidth=sc.pts(0.8),
               zorder=4, label="Structural variant")

    ctl = sweep[sweep.kind == "control"]
    if not ctl.empty:
        ax.scatter(ctl.hit, ctl.fresh, marker="s", s=sc.pts(6.5) ** 2,
                   color="white", edgecolor=INK, linewidth=sc.pts(1.1),
                   zorder=6, label="Original reward design (control)")

    for _, r in sweep.iterrows():
        key = "orig" if r.kind == "control" else r.point
        dx, dy, ha, va = OFFSET.get(key, (7, 5, "left", "bottom"))
        bold = (r.point == CANONICAL)
        ax.annotate(r.label + (" (selected)" if bold else ""),
                    (r.hit, r.fresh), xytext=(sc.pts(dx), sc.pts(dy)),
                    textcoords="offset points", fontsize=sc.annot,
                    ha=ha, va=va, color=INK if bold or r.kind == "control"
                    else INK_MUTED,
                    fontweight="bold" if bold else "normal")

    # axis labels state the quantity only; the better direction is carried by
    # the caption and the prose, never by the chart
    ax.set_xlabel("Cache hit ratio")
    ax.set_ylabel(_wrap("Freshness satisfaction ratio"))
    sc.apply(ax)
    # the front sweeps corner to corner and the labels claim what is left, so
    # the legend goes outside the axes rather than over any of it
    _legend(ax, sc, loc="lower left", bbox_to_anchor=(0.0, 1.005),
            borderaxespad=0.0, framealpha=1.0, ncol=2)
    ax.margins(x=0.14, y=0.12)
    fig.tight_layout()
    _save(fig, outdir, "pareto_hit_freshness")
    plt.close(fig)
    sweep.to_csv(outdir / "pareto_hit_freshness_data.csv", index=False)
    print(sweep.round(4).to_string(index=False))
    print(f"pareto figure + pareto_sweep.csv written to {outdir}")


if __name__ == "__main__":
    main()
