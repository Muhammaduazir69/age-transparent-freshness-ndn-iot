"""Publication figures (400 DPI PNG + vector PDF) from collected results.

Design rules (dataviz method): one axis per chart; Okabe-Ito-derived CVD-safe
colors; visual hierarchy = the learned policies (MAACO, DQN, A2C) in one
saturated hue, every traditional baseline in a muted tint of the same hue;
bars sorted by value (best first) so ranking reads instantly; y-limits fitted
to the data range (~15% padding) so policy differences stay visible -- axis
truncation is declared in the figure captions in the thesis; 95% CI error bars
with caps; direct numeric value labels above every bar; recessive solid
hairline y-grid (horizontal rules only, never vertical); text in ink colors,
never series colors; CSV table exported next to every figure (table-view
relief).  No "higher/lower is better" markers are drawn: the better direction
belongs in the caption and the surrounding prose, not on the canvas.

One visual system.  Every figure in the thesis goes through the same four
knobs so the chapters read as one document:

  * FONT      one family (DejaVu Sans) at four fixed on-page sizes,
              PT_LABEL / PT_TICK / PT_VALUE / PT_LEGEND below;
  * GRID      horizontal hairlines only, `GRID` gray, set once in `Scale.apply`;
  * COLOR     `HILITE` for anything in `HIGHLIGHT`, `MUTED` for the rest;
  * GEOMETRY  one canvas + one print width per chart class (`BAR_*`, `LINE_*`,
              `TRAIN_*`), so the type scale is identical across the class.

Legibility model (thesis is delivered as PDF *and* Word): every figure is
drawn on a canvas wider than the width it is finally printed at (the
chapters include the single-panel charts at 0.78\\textwidth, i.e. 4.9 in once
pandoc has scaled them into the .docx page box).  All type and line weights
are multiplied by the canvas/print ratio in `Scale`, so the *on-page* sizes
are exactly the PT_* targets below while the surplus canvas pixels at 400 DPI
keep the raster embedded in Word crisp.  Never set a bare `fontsize=` in
this module; go through a `Scale` instance.

Usage:
  python -m maaco.analysis.plots ../results/eval/summary.csv -o ../results/figures
  python -m maaco.analysis.plots --training ../results/train/dqn_training.log \
                                 ../results/train/maaco_training.log -o ../results/figures
"""

import argparse
import pathlib
import re
import textwrap

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

# fixed categorical order — Okabe-Ito (validated CVD-safe); never cycled
PALETTE = ["#0072B2", "#E69F00", "#009E73", "#CC79A7", "#56B4E9", "#D55E00",
           "#F0E442", "#000000"]
# visual hierarchy: the learned policies in one saturated hue, baselines muted
HILITE = "#0072B2"        # Okabe-Ito blue, saturated — learned policies
MUTED = "#A8CBE0"         # light tint of the same blue — all baselines
INK = "#1a1a1a"
INK_MUTED = "#6b6b6b"
GRID = "#d9d9d9"

# Every learned policy carries the highlight hue, in every bar chart of the
# thesis: MAACO in the stage-3 matrix, its DQN half in stage 1, its A2C half
# in stage 2.  Anything not listed here is a traditional baseline and is drawn
# in MUTED.  Kept in one place so no caller can drift.
HIGHLIGHT = ("MAACO", "DQN+LRU", "LCE+A2C")

DPI = 400                 # raster export; the PDF twin stays vector

# On-page targets in points, measured at the printed width of the figure.
# The canvas sizes below are ~1.7-1.8x the printed width, so the equivalent
# matplotlib sizes are all comfortably above these floors; what these numbers
# fix is the size the reader actually sees next to 12 pt body text.
PT_LABEL = 13.0           # axis labels
PT_TICK = 11.0            # tick labels
PT_VALUE = 11.0           # numeric value labels on bars
PT_LEGEND = 11.0          # legend entries and legend titles
PT_ANNOT = 9.0            # dense in-plot point annotations (pareto only)
LW_AXIS = 1.1             # spines / ticks / error bars
LW_LINE = 1.5             # data lines
MS_MARK = 5.0             # line-plot markers

# Geometry per chart class: (canvas w, canvas h) inches, print width inches.
# Print widths follow the \includegraphics widths used in chapter 4 on a
# 16 cm (6.30 in) text block: 0.78\textwidth for the single-panel charts,
# 0.95\textwidth for the 2x2 training grid.
BAR_CANVAS = (8.8, 5.81)
BAR_PRINT = 4.91          # 0.78\textwidth
LINE_CANVAS = (8.8, 5.81)
LINE_PRINT = 4.91         # 0.78\textwidth — identical class to the bars
TRAIN_CANVAS = (10.4, 7.70)
TRAIN_PRINT = 5.99        # 0.95\textwidth

plt.rcParams.update({
    "font.family": "sans-serif", "font.sans-serif": ["DejaVu Sans"],
    "mathtext.fontset": "dejavusans",
    "font.size": 9, "axes.edgecolor": INK_MUTED, "axes.labelcolor": INK,
    "text.color": INK, "xtick.color": INK, "ytick.color": INK,
    # horizontal hairlines only, everywhere
    "axes.grid": True, "axes.grid.axis": "y",
    "grid.color": GRID, "grid.linewidth": 0.5, "grid.linestyle": "-",
    "axes.axisbelow": True, "figure.dpi": DPI, "savefig.dpi": DPI,
    "axes.spines.top": False, "axes.spines.right": False,
    "legend.frameon": True, "legend.framealpha": 0.96,
    "legend.edgecolor": GRID, "legend.facecolor": "white",
    "legend.fancybox": False,
})


class Scale:
    """Maps on-page point sizes onto an oversized drawing canvas.

    `fig_w` is the matplotlib figsize width, `print_w` the width the figure
    actually occupies on the page.  Everything typographic is multiplied by
    k = fig_w / print_w, so a PT_TICK of 11 really is 11 pt in the printed
    thesis and in Word, whatever canvas we chose.
    """

    def __init__(self, fig_w, print_w):
        self.k = k = fig_w / print_w
        self.label = PT_LABEL * k
        self.tick = PT_TICK * k
        self.value = PT_VALUE * k
        self.legend = PT_LEGEND * k
        self.annot = PT_ANNOT * k
        self.axlw = LW_AXIS * k
        self.linelw = LW_LINE * k
        self.ms = MS_MARK * k
        self.capsize = 4.0 * k

    def pts(self, v):
        """An arbitrary on-page length in points, on this canvas."""
        return v * self.k

    def apply(self, ax):
        """The one place axis typography and the grid are decided."""
        ax.tick_params(labelsize=self.tick, width=self.axlw,
                       length=self.pts(3.5), pad=self.pts(2.5))
        for sp in ax.spines.values():
            sp.set_linewidth(self.axlw)
        ax.xaxis.label.set_size(self.label)
        ax.yaxis.label.set_size(self.label)
        # light horizontal gridlines only — vertical rules stay off in every
        # figure of the thesis, including the log-x cache-size panels
        ax.grid(False)
        ax.grid(True, axis="y", linewidth=self.pts(0.55), color=GRID,
                linestyle="-")
        ax.set_axisbelow(True)


def _errkw(sc):
    """The single 95% CI error-bar style used by every chart."""
    return dict(lw=sc.axlw, ecolor=INK_MUTED, capthick=sc.axlw)


def _legend(ax, sc, handles=None, labels=None, **kw):
    """One legend style for the whole thesis: hairline box, white ground."""
    opts = dict(frameon=True, fontsize=sc.legend, framealpha=0.96,
                edgecolor=GRID, facecolor="white", borderpad=0.6,
                labelspacing=0.5, handlelength=2.8, handletextpad=0.7)
    opts.update(kw)
    leg = (ax.legend(handles, labels, **opts) if handles is not None
           else ax.legend(**opts))
    if leg.get_title().get_text():
        leg.get_title().set_fontsize(sc.legend)
        leg.get_title().set_color(INK)
    leg.get_frame().set_linewidth(sc.pts(0.6))
    return leg


def _wrap(text, width=22):
    """Break a long axis label onto two lines: a rotated y-label longer than
    the axes is what clips in tight-bbox exports."""
    if len(text) <= width:
        return text
    return "\n".join(textwrap.wrap(text, width))


def _save(fig, outdir, figname):
    """400 DPI PNG for Word + vector PDF for LaTeX, both tightly cropped."""
    for ext in ("pdf", "png"):
        fig.savefig(outdir / f"{figname}.{ext}", dpi=DPI,
                    bbox_inches="tight", pad_inches=0.06)

# Config -> display name. The scenario family was renamed WsnGrid -> SmartFarm
# mid-project; archived .sca/CSV files may carry either prefix, so both map.
_PRETTY_BASE = {
    "LCE-LRU": "LCE+LRU",
    "LCD-LRU": "LCD+LRU",
    "Prob-LRU": "Prob p=0.5",
    "ProbTuned-LRU": "Prob p=0.4",
    "LCE-LFU": "LCE+LFU",
    "LCE-FIFO": "LCE+FIFO",
    "Fresh-LRU": "Fresh+LRU",
    "LCE-LFF": "LCE+LFF",
    "DQN-LRU": "DQN+LRU",
    "LCE-A2C": "LCE+A2C",
    "MAACO": "MAACO",
}
PRETTY = {f"{fam}-B-{k}": v
          for fam in ("SmartFarm", "WsnGrid")
          for k, v in _PRETTY_BASE.items()}

METRICS = [
    ("cacheHitRatio", "Cache hit ratio", 1.0, "higher"),
    ("consumerCacheHitRatio", "Interest-level cache hit ratio", 1.0, "higher"),
    ("freshnessSatisfactionRatio", "Freshness satisfaction ratio", 1.0, "higher"),
    ("staleDeliveryRate", "Stale delivery rate", 1.0, "lower"),
    ("meanRetrievalLatency", "Mean retrieval latency (ms)", 1000.0, "lower"),
    ("meanHopCount", "Mean hop count", 1.0, "lower"),
    ("satisfactionRatio", "Interest satisfaction ratio", 1.0, "higher"),
]


def ci95(x):
    x = np.asarray(x, dtype=float)
    x = x[~np.isnan(x)]
    if len(x) < 2:
        return 0.0
    return 1.96 * x.std(ddof=1) / np.sqrt(len(x))


def _fmt_all(values):
    """Value labels for one chart, all at the SAME number of decimals.

    The decimal count is chosen from the largest magnitude in the chart, so a
    series that straddles 10 or 100 cannot end up with "9.876" next to "12.3".
    """
    m = max((abs(float(v)) for v in values), default=0.0)
    d = 0 if m >= 100 else (1 if m >= 10 else 3)
    return [f"{float(v):.{d}f}" for v in values]


def _bar_figure(df: pd.DataFrame, configs, key, title, scale, better,
                figname, outdir: pathlib.Path, highlight=HIGHLIGHT,
                print_w=BAR_PRINT):
    """One sorted bar chart for one metric over the given configs; writes
    <figname>.{pdf,png} and the underlying numbers to <figname>_data.csv.

    `better` only decides the sort direction; it is never drawn.  `print_w` is
    the width (inches) the figure is printed at in the thesis -- it only sets
    the type scale, never the data."""
    rows = []
    for c in configs:
        v = df.loc[df["config"] == c, key] * scale
        rows.append((PRETTY.get(c, c), v.mean(), ci95(v), len(v)))
    # sort so the BEST value comes first: descending when higher is
    # better, ascending when lower is better — ranking reads left→right
    rows.sort(key=lambda r: r[1], reverse=(better == "higher"))
    labels = [r[0] for r in rows]
    means = np.array([r[1] for r in rows])
    errs = np.array([r[2] for r in rows])

    # one canvas for the whole bar class, whatever the number of categories,
    # so type size and grid weight are identical across chapter 4
    fig_w, fig_h = BAR_CANVAS
    sc = Scale(fig_w, print_w)
    fig, ax = plt.subplots(figsize=(fig_w, fig_h))
    x = np.arange(len(rows))
    colors = [HILITE if l in highlight else MUTED for l in labels]
    bars = ax.bar(x, means, width=0.62, color=colors, yerr=errs,
                  capsize=sc.capsize, error_kw=_errkw(sc),
                  edgecolor="white", linewidth=sc.pts(0.8), zorder=3)
    # highlighted bar label in bold ink; every bar direct-labeled above its
    # error-bar cap (single series + small n keeps this readable).  When the
    # bars are near-equal every label sits at the same height, so shrink the
    # type just enough that neighbours keep a gap.
    texts = _fmt_all(means)
    slot_pt = (fig_w * 0.78 / len(rows)) * 72.0
    need_pt = max(len(t) for t in texts) * 0.60 * sc.value
    vsize = sc.value * min(1.0, 0.85 * slot_pt / need_pt)
    for rect, m, e, l, t in zip(bars, means, errs, labels, texts):
        ax.annotate(t, (rect.get_x() + rect.get_width() / 2, m + e),
                    xytext=(0, sc.pts(3.5)), textcoords="offset points",
                    ha="center", va="bottom", fontsize=vsize,
                    color=INK, fontweight="bold" if l in highlight else "normal")

    # y-limits fitted to the data variance (~15% padding around the
    # data+CI envelope), never the full 0..1 span — the axis truncation
    # is stated in the thesis captions.  Headroom above is larger than
    # below so the value labels clear the error-bar caps; with the former
    # better-direction note gone this is the only thing above the bars.
    lo = float((means - errs).min())
    hi = float((means + errs).max())
    span = max(hi - lo, 1e-9)
    ax.set_ylim(lo - 0.14 * span, hi + 0.24 * span)
    rot = 32 if max(len(l) for l in labels) > 5 else 0
    ax.set_xticks(x, labels, rotation=rot,
                  ha="right" if rot else "center",
                  rotation_mode="anchor" if rot else None)
    ax.set_ylabel(_wrap(title))
    sc.apply(ax)

    fig.tight_layout(pad=0.4)
    _save(fig, outdir, figname)
    plt.close(fig)
    pd.DataFrame({"strategy": labels, "mean": means, "ci95": errs,
                  "seeds": [r[3] for r in rows]}).to_csv(
        outdir / f"{figname}_data.csv", index=False)


# stage panels (client-required result order): figure name -> (metric, configs)
STAGE1 = ["SmartFarm-B-DQN-LRU", "SmartFarm-B-LCE-LRU",
          "SmartFarm-B-LCD-LRU", "SmartFarm-B-Prob-LRU",
          "SmartFarm-B-ProbTuned-LRU"]
STAGE2 = ["SmartFarm-B-LCE-A2C", "SmartFarm-B-LCE-LRU", "SmartFarm-B-LCE-LFU",
          "SmartFarm-B-LCE-FIFO", "SmartFarm-B-LCE-LFF", "SmartFarm-B-Fresh-LRU"]


def stage_bars(df: pd.DataFrame, outdir: pathlib.Path):
    have = set(df["config"])
    s1 = [c for c in STAGE1 if c in have]
    s2 = [c for c in STAGE2 if c in have]
    if len(s1) >= 2:
        _bar_figure(df, s1, "cacheHitRatio", "Cache hit ratio", 1.0, "higher",
                    "stage1_hit", outdir)
        _bar_figure(df, s1, "meanRetrievalLatency", "Mean retrieval latency (ms)",
                    1000.0, "lower", "stage1_latency", outdir)
    if len(s2) >= 2:
        _bar_figure(df, s2, "freshnessSatisfactionRatio",
                    "Freshness satisfaction ratio", 1.0, "higher",
                    "stage2_freshness", outdir)
        _bar_figure(df, s2, "staleDeliveryRate", "Stale delivery rate", 1.0,
                    "lower", "stage2_stale", outdir)
    print(f"stage figures written to {outdir}")


# cache-size study line plots: one line per policy, learned policies first.
# Every series carries BOTH a distinct dash pattern and a distinct marker so
# the panel survives grayscale printing and photocopying.  MAACO keeps the
# same saturated highlight blue it has in every bar chart; the baselines
# cannot share the muted tint here because they must stay separable, so they
# take the remaining CVD-safe hues at a lighter line weight.
CS_STYLE = {  # pretty name -> (color, linestyle, lw multiplier, marker, zorder)
    "MAACO": (HILITE, "-", 1.35, "o", 7),
    "DQN+LRU": ("#D55E00", (0, (6, 1.6)), 1.20, "s", 6),
    "Prob p=0.4": ("#009E73", (0, (1, 1.6)), 1.05, "D", 5),
    "Prob p=0.5": ("#E69F00", (0, (3, 1.2, 1, 1.2)), 1.00, "P", 4),
    "LCE+LRU": (INK_MUTED, (0, (4, 2)), 0.95, "^", 3),
    "LCE+LFF": ("#CC79A7", (0, (5, 1.5, 1, 1.5)), 0.95, "v", 2),
}
CS_ORDER = ["MAACO", "DQN+LRU", "Prob p=0.4", "Prob p=0.5", "LCE+LRU",
            "LCE+LFF"]


def cachesize_lines(df: pd.DataFrame, outdir: pathlib.Path):
    """df needs columns: config, seed, csSize, <metrics>."""
    fig_w, fig_h = LINE_CANVAS
    print_w = LINE_PRINT
    for key, title, figname in [
            ("cacheHitRatio", "Cache hit ratio", "cachesize_hit"),
            ("freshnessSatisfactionRatio", "Freshness satisfaction ratio",
             "cachesize_freshness")]:
        sc = Scale(fig_w, print_w)
        fig, ax = plt.subplots(figsize=(fig_w, fig_h))
        table = []
        cfgs = sorted(df["config"].unique(),
                      key=lambda c: (CS_ORDER.index(PRETTY.get(c, c))
                                     if PRETTY.get(c, c) in CS_ORDER else 99))
        for cfg in cfgs:
            name = PRETTY.get(cfg, cfg)
            sub = df[df["config"] == cfg]
            g = sub.groupby("csSize")[key]
            xs = np.array(sorted(g.groups.keys()))
            means = np.array([g.get_group(x).mean() for x in xs])
            cis = np.array([ci95(g.get_group(x)) for x in xs])
            color, ls, lwm, mk, z = CS_STYLE.get(
                name, (MUTED, (0, (2, 2)), 0.9, "x", 2))
            ax.plot(xs, means, color=color, ls=ls, lw=sc.linelw * lwm,
                    marker=mk, ms=sc.ms, mew=sc.pts(0.9), mfc=color,
                    mec="white", zorder=z, label=name,
                    solid_capstyle="round")
            # 95% CI drawn as a shaded band: the line-plot twin of the capped
            # error bars used on the bar charts
            ax.fill_between(xs, means - cis, means + cis, color=color,
                            alpha=0.15, lw=0, zorder=z - 1)
            for x, m, c in zip(xs, means, cis):
                table.append({"strategy": name, "csSize": int(x), "mean": m,
                              "ci95": c,
                              "seeds": int(g.get_group(x).count())})
        ax.set_xscale("log")
        xticks = sorted(df["csSize"].unique())
        ax.set_xticks(xticks, [str(int(t)) for t in xticks])
        ax.minorticks_off()
        ax.set_xlabel("Content Store capacity (entries)")
        ax.set_ylabel(_wrap(title))
        sc.apply(ax)
        # framed legend, dropped into whichever quadrant matplotlib finds
        # emptiest — these two panels fill opposite corners
        _legend(ax, sc, loc="best")
        fig.tight_layout(pad=0.4)
        _save(fig, outdir, figname)
        plt.close(fig)
        pd.DataFrame(table).to_csv(outdir / f"{figname}_data.csv", index=False)
    print(f"cache-size figures written to {outdir}")


def metric_bars(df: pd.DataFrame, outdir: pathlib.Path):
    # full-matrix (stage 3) charts: MAACO vs the traditional baselines only;
    # the single-agent ablation twins get their own stage-1/2 panels
    ablations = {"DQN+LRU", "LCE+A2C"}
    configs = [c for c in PRETTY
               if c in set(df["config"]) and PRETTY[c] not in ablations]

    for key, title, scale, better in METRICS:
        _bar_figure(df, configs, key, title, scale, better, key, outdir)

    # table view (relief + thesis appendix) — fixed strategy order
    rows = []
    for c in configs:
        sub = df[df["config"] == c]
        row = {"strategy": PRETTY[c], "seeds": len(sub)}
        for key, title, scale, _ in METRICS:
            row[key] = (sub[key] * scale).mean()
            row[key + "_ci95"] = ci95(sub[key] * scale)
        rows.append(row)
    pd.DataFrame(rows).to_csv(outdir / "metrics_table.csv", index=False)
    print(f"figures + metrics_table.csv written to {outdir}")


EP_RE = re.compile(
    r"\[ep\s+(\d+)\].*?hit=([\d.]+) fresh=([\d.]+) R=([+-][\d.]+)"
    r"(?:.*?eps=([\d.]+))?(?:.*?loss=([\d.a-z]+))?")

# training-log stem -> curve label (canonical operating point flagged)
LOG_LABELS = {
    "dqn_training": "DQN-only",
    "maaco_training": "MAACO (default)",
    "sweep_wB": "MAACO wB (canonical)",
    "liveaudit": "MAACO (live audit)",
    "dqnv2-smartfarm": "MAACO (SmartFarm)",
    "dqnv2-edgeiot": "MAACO (EdgeIoT)",
    "dqnv2-dqnonly": "DQN-only",
    "dqnv2-a2conly": "A2C-only",
    # v3 credit-assignment rebuild (the campaigns reported in chapter 4)
    "final-maaco": "MAACO (SmartFarm)",
    "final-dqn": "DQN-only (SmartFarm)",
    "final-a2c": "A2C-only (SmartFarm)",
    "final-edge": "MAACO (EdgeIoT)",
}


# one dash pattern + one marker per campaign: the curves stay separable in
# grayscale, and the markers survive the smoothing overlay
TRAIN_STYLE = [("-", "o"), ((0, (6, 1.6)), "s"), ((0, (4, 2)), "^"),
               ((0, (1, 1.6)), "D"), ((0, (5, 1.5, 1, 1.5)), "v")]


def training_curves(logs, outdir: pathlib.Path):
    # 2x2 rather than 1x3: printed at 0.95\textwidth a three-across strip
    # leaves each panel under 2 in wide, which is what made the axis text
    # unreadable in the .docx.  The fourth cell carries the legend, so no
    # legend sits on top of data.
    fig_w, fig_h = TRAIN_CANVAS
    sc = Scale(fig_w, TRAIN_PRINT)
    fig, axgrid = plt.subplots(2, 2, figsize=(fig_w, fig_h))
    axes = [axgrid[0][0], axgrid[0][1], axgrid[1][0]]
    legend_ax = axgrid[1][1]
    legend_ax.axis("off")
    table = []
    for i, log in enumerate(logs):
        stem = pathlib.Path(log).stem
        name = LOG_LABELS.get(stem, stem.replace("_training", "").upper())
        eps, hits, rewards, losses = [], [], [], []
        for line in open(log):
            m = EP_RE.search(line)
            if not m:
                continue
            eps.append(int(m.group(1)))
            hits.append(float(m.group(2)))
            rewards.append(float(m.group(4)))
            losses.append(float(m.group(6)) if m.group(6) and m.group(6) != "nan"
                          else np.nan)
        if not eps:
            continue
        for e, h, r, lo in zip(eps, hits, rewards, losses):
            table.append({"run": name, "episode": e, "hitRatio": h,
                          "meanTransitionReward": r, "loss": lo})
        color = PALETTE[i % len(PALETTE)]
        ls, mk = TRAIN_STYLE[i % len(TRAIN_STYLE)]
        # ~7 markers per curve, staggered per series so they never stack
        every = (i * 5 + 3, max(1, len(eps) // 7))

        def smooth(y, w=9):
            y = np.asarray(y, dtype=float)
            if len(y) < w:
                return y
            return pd.Series(y).rolling(w, min_periods=1, center=True).mean().values

        kw = dict(color=color, lw=sc.linelw, ls=ls, marker=mk,
                  ms=sc.pts(4.2), markevery=every, mew=sc.pts(0.8),
                  mfc=color, mec="white")
        axes[0].plot(eps, smooth(rewards), label=name, **kw)
        axes[0].plot(eps, rewards, color=color, lw=sc.pts(0.5), alpha=0.25)
        axes[1].plot(eps, smooth(hits), label=name, **kw)
        axes[1].plot(eps, hits, color=color, lw=sc.pts(0.5), alpha=0.25)
        axes[2].plot(eps, smooth(losses), label=name, **kw)

    for ax, ylab in zip(axes, ["Mean transition reward", "Cache hit ratio",
                               "TD loss (smoothed)"]):
        ax.set_xlabel("Episode")
        ax.set_ylabel(_wrap(ylab, 16))
        sc.apply(ax)
        ax.xaxis.set_major_locator(plt.MaxNLocator(4, integer=True))
    handles, labels = axes[0].get_legend_handles_labels()
    _legend(legend_ax, sc, handles, labels, loc="center", framealpha=1.0,
            title="Training campaign", borderpad=0.9, labelspacing=0.8,
            handlelength=3.2, handletextpad=0.8)
    fig.tight_layout(pad=0.4)
    _save(fig, outdir, "training_curves")
    plt.close(fig)
    pd.DataFrame(table).to_csv(outdir / "training_curves_data.csv", index=False)
    print(f"training_curves written to {outdir}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("inputs", nargs="+",
                    help="summary.csv for bars, or training logs with --training")
    ap.add_argument("--training", action="store_true")
    ap.add_argument("--stages", action="store_true",
                    help="stage-1/stage-2 comparison panels from summary.csv")
    ap.add_argument("--cachesize", action="store_true",
                    help="cache-size line plots from a csv with a csSize column")
    ap.add_argument("-o", "--outdir", default="../results/figures")
    args = ap.parse_args()

    outdir = pathlib.Path(args.outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    if args.training:
        training_curves(args.inputs, outdir)
    elif args.cachesize:
        cachesize_lines(pd.read_csv(args.inputs[0]), outdir)
    elif args.stages:
        stage_bars(pd.read_csv(args.inputs[0]), outdir)
    else:
        df = pd.read_csv(args.inputs[0])
        metric_bars(df, outdir)


if __name__ == "__main__":
    main()
