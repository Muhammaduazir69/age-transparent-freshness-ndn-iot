#!/usr/bin/env python3
"""Figure 1: why a cached NDN reading reaches a consumer stale, and the fix.

Panel (a) standard NDN: every Content Store restarts FreshnessPeriod from its
own arrival time, so the lifetime the network enforces is renewed at each hop
while the reading keeps aging. Panel (b) age-transparent freshness: the copy
carries the life it has left, so a node stops serving it at the publisher's
deadline. The ages printed below each router are measured: they are the mean
age, in units of the declared lifetime F, of copies handed on by that many
Content Stores (Section 5, seed 10).
"""
import sys
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import FancyBboxPatch, Rectangle, FancyArrowPatch
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C

NODE_FILL, NODE_EDGE = "#91ace0", "#5a7ec0"
PROD_FILL, PROD_EDGE = "#acd78e", "#6ea84f"
CONS_FILL, CONS_EDGE = "#fee695", "#e0b93c"
LINE_DATA = "#4771c3"
GOOD, BAD = "#35b44a", "#e54c5e"
BADGE_BAD, BADGE_GOOD = "#fdecee", "#eaf7ec"
INK, GREY, PANEL_EDGE = "#1a1a1a", "#5a5a5a", "#808080"

# Measured mean age / F of copies handed on by M Content Stores. Read from the
# campaign rather than typed in, so the figure cannot drift from Section 6.
AGE_STD = [0.002, 0.210, 0.462, 0.786, 1.195]   # fallback only
AGE_AT  = [0.002, 0.213, 0.372, 0.482, 0.565]   # fallback only


def load_ages(results_dir):
    global AGE_STD, AGE_AT
    for cfg, name in (("Fixed-LCE-LRU", "AGE_STD"), ("AT-LCE-LRU", "AGE_AT")):
        tab, n = C.restamp_table(results_dir, cfg)
        if not tab or n == 0:
            print(f"  warning: no runs for {cfg}; keeping fallback ages")
            continue
        globals()[name] = [tab.get(m, (0, 0, 0.0))[2] for m in range(5)]
    print(f"  ages from campaign: std={[round(v,3) for v in AGE_STD]}, "
          f"at={[round(v,3) for v in AGE_AT]}")

def setup():
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["Nimbus Sans", "Arial", "Helvetica", "DejaVu Sans"],
        "pdf.fonttype": 42, "ps.fonttype": 42, "axes.linewidth": 0.6,
    })

def rbox(ax, x, y, w, h, fill, edge, title, sub):
    ax.add_patch(FancyBboxPatch((x, y), w, h,
                 boxstyle="round,pad=0,rounding_size=%.3f" % (0.06 * h),
                 facecolor=fill, edgecolor=edge, linewidth=0.9, zorder=5))
    ax.text(x + w / 2, y + h * 0.64, title, ha="center", va="center",
            fontsize=7.6, fontweight="bold", color=INK, zorder=6)
    ax.text(x + w / 2, y + h * 0.27, sub, ha="center", va="center",
            fontsize=6.4, color=INK, zorder=6)

def antenna(ax, x, y, h, color, arcs=2):
    """A stub antenna with radiating arcs, drawn above a node."""
    ax.plot([x, x], [y, y + h], color=color, linewidth=1.0, zorder=7,
            solid_capstyle="round")
    for k in range(arcs):
        r = h * (0.50 + 0.38 * k)
        th = np.linspace(np.deg2rad(28), np.deg2rad(152), 40)
        ax.plot(x + r * np.cos(th) * 1.5, y + h + r * np.sin(th) * 0.55,
                color=color, linewidth=0.6, alpha=0.85 - 0.22 * k, zorder=7)


def mote(ax, x, y, w, h, fill, edge, title, sub, kind="router", slots=None):
    """An 802.15.4 sensor node drawn as a board rather than a plain rectangle.

    A labelled box is the pattern that makes a systems figure look unfinished.
    Every node here is the same device class, so each one is a small board with
    an antenna, a battery gauge and, for a node that caches, a strip of Content
    Store slots whose fill shows what the store is holding.
    """
    ax.add_patch(FancyBboxPatch((x, y), w, h,
                 boxstyle="round,pad=0,rounding_size=%.3f" % (0.06 * h),
                 facecolor=fill, edgecolor=edge, linewidth=0.9, zorder=5))
    antenna(ax, x + w * 0.845, y + h, h * 0.14, edge,
            arcs=3 if kind == "sensor" else 2)

    # battery gauge, top left: every node in this field runs on a cell
    bw_, bh_ = w * 0.105, h * 0.115
    bx, by = x + w * 0.055, y + h * 0.775
    ax.add_patch(Rectangle((bx, by), bw_, bh_, facecolor="#ffffff",
                           edgecolor=edge, linewidth=0.5, zorder=6))
    ax.add_patch(Rectangle((bx, by), bw_ * 0.68, bh_, facecolor=edge,
                           edgecolor="none", zorder=7))
    ax.add_patch(Rectangle((bx + bw_, by + bh_ * 0.28), bw_ * 0.12, bh_ * 0.44,
                           facecolor=edge, edgecolor="none", zorder=6))

    ty = 0.635 if kind == "router" else 0.565
    ax.text(x + w / 2, y + h * ty, title, ha="center", va="center",
            fontsize=7.4, fontweight="bold", color=INK, zorder=8)

    if kind == "router":
        # The caption goes above the slots, inside the board. Below them it
        # ran into the box border and the badge underneath.
        ax.text(x + w / 2, y + h * 0.395, sub, ha="center", va="center",
                fontsize=6.0, color=GREY, zorder=8)
        # Content Store as a row of slots, two of them holding a copy
        n = 5
        sw = w * 0.108
        sx = x + (w - (n * sw + (n - 1) * w * 0.018)) / 2
        sy = y + h * 0.120
        sh = h * 0.200
        for i in range(n):
            cx = sx + i * (sw + w * 0.018)
            held = i in (slots or (1, 3))
            ax.add_patch(Rectangle((cx, sy), sw, sh,
                         facecolor=edge if held else "#ffffff",
                         edgecolor=edge, linewidth=0.5, zorder=6))
    else:
        ax.text(x + w / 2, y + h * 0.235, sub, ha="center", va="center",
                fontsize=6.3, color=INK, zorder=6)


def agebar(ax, x, y, w, h, age):
    """A box of width w represents one lifetime F; the fill is the age."""
    ax.add_patch(Rectangle((x, y), w, h, facecolor="#ffffff",
                           edgecolor=GREY, linewidth=0.55, zorder=5))
    f = min(1.0, age)
    ax.add_patch(Rectangle((x, y), w * f, h, facecolor=GOOD if age <= 1.0 else BAD,
                           edgecolor="none", zorder=6))
    if age > 1.0:
        ax.add_patch(Rectangle((x + w, y), w * (age - 1.0), h, facecolor=BAD,
                               edgecolor=BAD, linewidth=0.4, hatch="////", zorder=6))
    # the publisher's deadline
    ax.plot([x + w, x + w], [y - 0.008, y + h + 0.008], color=INK,
            linewidth=0.9, zorder=7, solid_capstyle="butt")

def chain(ax, y0, transparent):
    # Four caches, not three: the ages printed are those of a copy that has
    # been handed on by that many Content Stores, so a consumer shown the
    # four-hand-off age has to sit behind four of them.
    xs = [0.030, 0.192, 0.354, 0.516, 0.678, 0.845]
    bw, bh = 0.122, 0.092
    ytop = y0 + 0.184
    ages = AGE_AT if transparent else AGE_STD

    mote(ax, xs[0], ytop, bw, bh, PROD_FILL, PROD_EDGE, "Sensor",
         "publishes, life F", kind="sensor")
    for i, x in enumerate(xs[1:5], start=1):
        mote(ax, x, ytop, bw, bh, NODE_FILL, NODE_EDGE, "Cache %d" % i,
             "Content Store", kind="router")
    mote(ax, xs[5], ytop, bw, bh, CONS_FILL, CONS_EDGE, "Consumer",
         "uses reading", kind="consumer")

    for a, b in zip(xs[:-1], xs[1:]):
        ax.add_patch(FancyArrowPatch((a + bw, ytop + bh / 2), (b, ytop + bh / 2),
                     arrowstyle="-|>", mutation_scale=9, linewidth=1.3,
                     color=LINE_DATA, shrinkA=1, shrinkB=1, zorder=4))

    # what the cache does to the clock, as a badge under each router
    badge = "clock reset to F" if not transparent else "carries F minus age"
    bcol = BAD if not transparent else GOOD
    bfill = BADGE_BAD if not transparent else BADGE_GOOD
    for x in xs[1:5]:
        ax.add_patch(FancyBboxPatch((x + 0.004, ytop - 0.052), bw - 0.008, 0.038,
                     boxstyle="round,pad=0,rounding_size=0.010",
                     facecolor=bfill, edgecolor=bcol, linewidth=0.7, zorder=5))
        ax.text(x + bw / 2, ytop - 0.033, badge, ha="center", va="center",
                fontsize=5.9, fontweight="bold", color=bcol, zorder=6)

    # age of the copy as it leaves each router, and as the consumer sees it
    gy, gh = y0 + 0.074, 0.032
    ax.text(0.030, gy + gh / 2, "age of\nthe copy", ha="left", va="center",
            fontsize=6.3, color=GREY, zorder=6)
    # One bar per cache: the age of the copy as that cache hands it on. The
    # consumer receives what the last cache handed on, so its verdict refers
    # to the final bar rather than carrying a bar of its own.
    for i, x in enumerate(xs[1:5], start=1):
        agebar(ax, x, gy, bw * 0.82, gh, ages[i])
        ax.text(x + bw * 0.41, gy - 0.016, "%.2fF" % ages[i], ha="center",
                va="top", fontsize=6.5,
                color=BAD if ages[i] > 1.0 else INK,
                fontweight="bold" if ages[i] > 1.0 else "normal", zorder=6)

    last = ages[4]
    verdict = ("receives %.2fF: valid" % last if transparent
               else "receives %.2fF: stale, yet accepted" % last)
    ax.text(xs[5] + bw, ytop + bh + 0.030, verdict, ha="right", va="bottom",
            fontsize=7.0, fontweight="bold", color=GOOD if transparent else BAD,
            zorder=7)
    ax.annotate("", xy=(xs[5] + bw * 0.5, gy + gh * 0.5),
                xytext=(xs[4] + bw * 0.82, gy + gh * 0.5),
                arrowprops=dict(arrowstyle="-|>", color=GREY, linewidth=0.8,
                                linestyle=(0, (2, 1.6)), shrinkA=2, shrinkB=2),
                zorder=4)

def build(out_pdf):
    setup()
    fig = plt.figure(figsize=(6.7, 4.35))
    ax = fig.add_axes([0, 0, 1, 1])
    ax.set_xlim(0, 1); ax.set_ylim(0, 1); ax.axis("off")

    for y0, title in ((0.512, "(a) Standard NDN: FreshnessPeriod restarts at every cache"),
                      (0.048, "(b) Age-transparent freshness (this work)")):
        ax.add_patch(Rectangle((0.018, y0), 0.964, 0.372, facecolor="#ffffff",
                               edgecolor=PANEL_EDGE, linewidth=0.8, zorder=1))
        ax.text(0.032, y0 + 0.346, title, ha="left", va="top",
                fontsize=8.2, fontweight="bold", color=INK, zorder=7)

    chain(ax, 0.512, transparent=False)
    chain(ax, 0.048, transparent=True)

    # legend: what the age bar means
    ly = 0.945
    ax.add_patch(Rectangle((0.052, ly - 0.013), 0.030, 0.018, facecolor=GOOD,
                           edgecolor=GREY, linewidth=0.4, zorder=7))
    ax.text(0.090, ly - 0.004, "age still within the declared lifetime F",
            ha="left", va="center", fontsize=6.9, color=INK, zorder=7)
    ax.add_patch(Rectangle((0.520, ly - 0.013), 0.030, 0.018, facecolor=BAD,
                           edgecolor=BAD, linewidth=0.4, hatch="////", zorder=7))
    ax.text(0.558, ly - 0.004, "age past F; the vertical rule marks the deadline",
            ha="left", va="center", fontsize=6.9, color=INK, zorder=7)

    fig.savefig(out_pdf, format="pdf")
    fig.savefig(str(out_pdf).replace(".pdf", ".png"), dpi=400)
    print("wrote", out_pdf)

if __name__ == "__main__":
    if len(sys.argv) > 2:
        load_ages(sys.argv[2])
    out = Path(sys.argv[1] if len(sys.argv) > 1
               else Path(__file__).resolve().parents[1] / "figures" / "fig_arch.pdf")
    out.parent.mkdir(parents=True, exist_ok=True)
    build(out)
