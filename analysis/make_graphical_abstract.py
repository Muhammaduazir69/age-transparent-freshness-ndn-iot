#!/usr/bin/env python3
"""Graphical abstract: the problem in one row, the fix and its effect below."""
import sys
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Rectangle, FancyArrowPatch
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C

NODE, NODE_E = "#91ace0", "#5a7ec0"
PROD, PROD_E = "#acd78e", "#6ea84f"
CONS, CONS_E = "#fee695", "#e0b93c"
GOOD, BAD, LINE = "#35b44a", "#e54c5e", "#4771c3"
INK, GREY = "#1a1a1a", "#5a5a5a"

def setup():
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["Nimbus Sans", "Arial", "Helvetica", "DejaVu Sans"],
        "pdf.fonttype": 42, "ps.fonttype": 42, "axes.linewidth": 0.6,
    })

def chain(ax, y, ages, bad_last, caption, badge, badge_col):
    xs = [0.055, 0.225, 0.395, 0.565, 0.760]
    bw, bh = 0.125, 0.075
    for i, x in enumerate(xs):
        if i == 0:
            f, e, t = PROD, PROD_E, "sensor"
        elif i == len(xs) - 1:
            f, e, t = CONS, CONS_E, "consumer"
        else:
            f, e, t = NODE, NODE_E, "cache"
        ax.add_patch(FancyBboxPatch((x, y), bw, bh,
                     boxstyle="round,pad=0,rounding_size=0.006",
                     facecolor=f, edgecolor=e, linewidth=0.9, zorder=5))
        ax.text(x + bw/2, y + bh/2, t, ha="center", va="center",
                fontsize=7.4, fontweight="bold", color=INK, zorder=6)
    for a, b in zip(xs[:-1], xs[1:]):
        ax.add_patch(FancyArrowPatch((a+bw, y+bh/2), (b, y+bh/2), arrowstyle="-|>",
                     mutation_scale=8, linewidth=1.2, color=LINE,
                     shrinkA=1, shrinkB=1, zorder=4))
    # age bars under the caches and the consumer
    gw, gh = 0.100, 0.026
    for i, x in enumerate(xs[1:], start=1):
        a = ages[i]
        gx = x + (bw - gw) / 2
        ax.add_patch(Rectangle((gx, y-0.050), gw, gh, facecolor="#ffffff",
                               edgecolor=GREY, linewidth=0.5, zorder=5))
        col = BAD if a > 1.0 else GOOD
        ax.add_patch(Rectangle((gx, y-0.050), gw*min(1.0, a), gh, facecolor=col,
                               edgecolor="none", zorder=6))
        if a > 1.0:
            ax.add_patch(Rectangle((gx+gw, y-0.050), gw*(a-1.0), gh, facecolor=BAD,
                                   edgecolor=BAD, linewidth=0.4, hatch="////", zorder=6))
        ax.plot([gx+gw, gx+gw], [y-0.056, y-0.018], color=INK, linewidth=0.8, zorder=7)
        ax.text(gx+gw/2, y-0.062, f"{a:.2f}F", ha="center", va="top",
                fontsize=6.4, color=BAD if a > 1.0 else INK,
                fontweight="bold" if a > 1.0 else "normal", zorder=6)
    ax.add_patch(FancyBboxPatch((0.760, y+bh+0.014), 0.125, 0.034,
                 boxstyle="round,pad=0,rounding_size=0.008",
                 facecolor="#ffffff", edgecolor=badge_col, linewidth=0.9, zorder=6))
    ax.text(0.8225, y+bh+0.031, badge, ha="center", va="center",
            fontsize=7.0, fontweight="bold", color=badge_col, zorder=7)
    ax.text(0.055, y+bh+0.031, caption, ha="left", va="center",
            fontsize=8.4, fontweight="bold", color=INK, zorder=7)

def build(d, out):
    std, _ = C.restamp_table(d, "Fixed-LCE-LRU")
    at, _ = C.restamp_table(d, "AT-LCE-LRU")
    a_std = [std.get(m, (0, 0, 0))[2] for m in range(5)]
    a_at = [at.get(m, (0, 0, 0))[2] for m in range(5)]
    A = C.config_metrics(d, "Fixed-LCE-LRU"); B = C.config_metrics(d, "AT-LCE-LRU")
    ps = C.paired_runs(A, B, "stale_delivery")
    pv = C.paired_runs(A, B, "valid_cache_service")

    setup()
    fig = plt.figure(figsize=(5.5, 3.6))
    ax = fig.add_axes([0, 0, 1, 1]); ax.set_xlim(0, 1); ax.set_ylim(0, 1); ax.axis("off")
    ax.text(0.5, 0.965, "Cached sensor readings age without the network noticing",
            ha="center", va="top", fontsize=9.6, fontweight="bold", color=INK)
    ax.text(0.5, 0.915, "NDN restarts FreshnessPeriod at every cache, so the age already spent is forgotten",
            ha="center", va="top", fontsize=7.4, color=GREY)
    chain(ax, 0.700, a_std, True, "Standard NDN", "stale", BAD)
    chain(ax, 0.420, a_at, False, "Age-transparent freshness", "valid", GOOD)

    # result strip
    ax.add_patch(Rectangle((0.055, 0.075), 0.830, 0.215, facecolor="#f5f7f9",
                           edgecolor="#c9ced3", linewidth=0.8, zorder=1))
    if ps and pv:
        # both age-transparent values are the improvement, so both are green
        cells = [("stale deliveries", f"{100*ps[0]:.2f}%", f"{100*ps[1]:.2f}%", GOOD),
                 ("valid cache service", f"{100*pv[0]:.2f}%", f"{100*pv[1]:.2f}%", GOOD)]
        for i, (lab, a, b, col) in enumerate(cells):
            x = 0.105 + i * 0.420
            ax.text(x, 0.248, lab, ha="left", va="center", fontsize=7.6,
                    fontweight="bold", color=INK, zorder=5)
            ax.text(x, 0.188, "standard", ha="left", va="center", fontsize=7.0, color=GREY, zorder=5)
            ax.text(x + 0.230, 0.188, a, ha="right", va="center", fontsize=8.4, color=INK, zorder=5)
            ax.text(x, 0.128, "age-transparent", ha="left", va="center", fontsize=7.0, color=GREY, zorder=5)
            ax.text(x + 0.230, 0.128, b, ha="right", va="center", fontsize=8.4,
                    fontweight="bold", color=col, zorder=5)
    ax.text(0.5, 0.040, "36 IEEE 802.15.4 sensors, OMNeT++ and INET, 20 seeds",
            ha="center", va="center", fontsize=6.8, color=GREY)
    out = Path(out); out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, dpi=400, bbox_inches="tight")
    fig.savefig(str(out).replace(".png", ".pdf"), bbox_inches="tight")
    print("wrote", out)

if __name__ == "__main__":
    d = sys.argv[1]
    out = sys.argv[2] if len(sys.argv) > 2 else Path(__file__).resolve().parents[1] / "figures" / "graphical_abstract.png"
    build(d, out)
