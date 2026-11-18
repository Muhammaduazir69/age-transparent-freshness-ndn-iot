#!/usr/bin/env python3
"""Result figures. Every value is read from the recorded .sca files."""
import sys, collections
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C

_CACHE = {}


def cm(d, cfg):
    if cfg not in _CACHE:
        _CACHE[cfg] = C.config_metrics(d, cfg)
    return _CACHE[cfg]


def pr(d, a, b, m):
    x, y = cm(d, a), cm(d, b)
    return C.paired_runs(x, y, m) if x and y else None

W_FULL, W_HALF = 6.3, 3.05
STD, ATC = "#c0392b", "#1a8f63"          # standard NDN, age-transparent
MODEL = "#5a6673"
REG_COL = {"stationary": "#2a78d6", "drift": "#7c4fc4",
           "pollution": "#a8690a", "flash": "#c0392b"}
INK, GRID = "#1f2933", "#d7dbdf"

def setup():
    plt.rcParams.update({
        "font.family": "serif",
        "font.serif": ["Palatino", "TeX Gyre Pagella", "DejaVu Serif"],
        "pdf.fonttype": 42, "ps.fonttype": 42,
        "axes.linewidth": 0.6, "axes.edgecolor": INK,
        "xtick.labelsize": 7.5, "ytick.labelsize": 7.5,
        "axes.labelsize": 8.0, "legend.fontsize": 7.2,
        "axes.grid": True, "grid.color": GRID, "grid.linewidth": 0.4,
        "axes.axisbelow": True, "figure.dpi": 150,
    })

def pending(out, why):
    """A figure whose data has not been produced yet.

    Written rather than skipped so the document still builds, and marked with
    the word PLACEHOLDER so check_placeholders.py fails while it is in the PDF.
    """
    setup()
    fig, ax = plt.subplots(figsize=(W_FULL, 1.6))
    ax.axis("off")
    ax.text(0.5, 0.5, "PLACEHOLDER: %s" % why, ha="center", va="center",
            fontsize=10, color=STD, transform=ax.transAxes)
    out = Path(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, format="pdf", bbox_inches="tight")
    fig.savefig(str(out).replace(".pdf", ".png"), dpi=200, bbox_inches="tight")
    plt.close(fig)
    print("wrote PLACEHOLDER", out)


def save(fig, out):
    out = Path(out)
    out.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out, format="pdf", bbox_inches="tight")
    fig.savefig(str(out).replace(".pdf", ".png"), dpi=400, bbox_inches="tight")
    plt.close(fig)
    print("wrote", out)


def fig_mechanism(d, outdir):
    """Staleness and age against the number of cache-to-cache hand-offs."""
    std, _ = C.restamp_table(d, "Fixed-LCE-LRU")
    at, _ = C.restamp_table(d, "AT-LCE-LRU")
    setup()
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(W_FULL, 2.45))

    for tab, col, lab, mk in ((std, STD, "standard NDN", "o"), (at, ATC, "age-transparent", "s")):
        ms = sorted(tab)
        a1.plot(ms, [tab[m][1] for m in ms], marker=mk, color=col, label=lab,
                linewidth=1.2, markersize=4)
        a2.plot(ms, [tab[m][2] for m in ms], marker=mk, color=col, label=lab,
                linewidth=1.2, markersize=4)

    # model lower bound: M * E[W_1], with E[W_1] read from the M = 1 bucket
    if 1 in std:
        ew = std[1][2]
        ms = sorted(std)
        a2.plot(ms, [m * ew for m in ms], linestyle="--", color=MODEL, linewidth=1.0,
                label=r"$M\,\mathbb{E}[W_1]$, model")
    a2.axhline(1.0, color=INK, linewidth=0.8, linestyle=":")
    a2.text(0.15, 1.04, "declared lifetime $F$", fontsize=6.6, color=INK)

    a1.set_xlabel("hand-offs $M$ between Content Stores")
    a1.set_ylabel("stale deliveries (%)")
    a1.set_title("(a) staleness by hand-off count", fontsize=8.2, pad=4)
    a1.legend(frameon=False, loc="upper left")

    a2.set_xlabel("hand-offs $M$ between Content Stores")
    a2.set_ylabel("mean delivered age / $F$")
    a2.set_title("(b) age by hand-off count", fontsize=8.2, pad=4)
    a2.legend(frameon=False, loc="upper left")
    save(fig, Path(outdir) / "fig_mechanism.pdf")


def fig_depth(d, outdir):
    """Staleness against the consumer's topological distance to the publisher."""
    std, _ = C.depth_table(d, "Fixed-LCE-LRU")
    at, _ = C.depth_table(d, "AT-LCE-LRU")
    setup()
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(W_FULL, 2.45))
    for r, name in C.REGIMES.items():
        if r in std:
            bs = sorted(std[r])
            a1.plot(bs, [std[r][b][0] for b in bs], marker="o", markersize=3.4,
                    linewidth=1.15, color=REG_COL[name], label=name)
        if r in at:
            bs = sorted(at[r])
            a2.plot(bs, [at[r][b][0] for b in bs], marker="s", markersize=3.4,
                    linewidth=1.15, color=REG_COL[name], label=name)
    a1.set_title("(a) standard NDN", fontsize=8.2, pad=4)
    a2.set_title("(b) age-transparent freshness", fontsize=8.2, pad=4)
    for a in (a1, a2):
        a.set_xlabel("hops from the publisher")
    a1.set_ylabel("stale deliveries (%)")
    a2.set_ylabel("stale deliveries (%)")
    a1.legend(frameon=False, loc="upper left", ncol=2)
    a2.legend(frameon=False, loc="upper left", ncol=2)
    # Panel (b) keeps its own scale: the residual is three orders of magnitude
    # smaller and would be invisible on the scale of panel (a).
    save(fig, Path(outdir) / "fig_depth.pdf")


def fig_capacity(d, outdir):
    caps = [(5, "C5-LCE-LRU", "C5-AT-LCE-LRU"), (10, "Fixed-LCE-LRU", "AT-LCE-LRU"),
            (20, "C20-LCE-LRU", "C20-AT-LCE-LRU"), (40, "C40-LCE-LRU", "C40-AT-LCE-LRU")]
    xs, s_std, s_at, v_std, v_at = [], [], [], [], []
    for cap, b, t in caps:
        p = pr(d, b, t, "stale_delivery")
        q = pr(d, b, t, "valid_cache_service")
        if not p or not q:
            continue
        xs.append(cap)
        s_std.append(100 * p[0]); s_at.append(100 * p[1])
        v_std.append(100 * q[0]); v_at.append(100 * q[1])
    if not xs:
        pending(Path(outdir) / "fig_capacity.pdf",
                "capacity sweep not yet complete"); return
    setup()
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(W_FULL, 2.45))
    a1.plot(xs, s_std, marker="o", color=STD, linewidth=1.2, markersize=4, label="standard NDN")
    a1.plot(xs, s_at, marker="s", color=ATC, linewidth=1.2, markersize=4, label="age-transparent")
    a1.set_ylabel("stale deliveries (%)")
    a1.set_title("(a) staleness grows with capacity", fontsize=8.2, pad=4)
    a2.plot(xs, v_std, marker="o", color=STD, linewidth=1.2, markersize=4, label="standard NDN")
    a2.plot(xs, v_at, marker="s", color=ATC, linewidth=1.2, markersize=4, label="age-transparent")
    a2.set_ylabel("valid cache service (%)")
    a2.set_title("(b) service that is actually usable", fontsize=8.2, pad=4)
    for a in (a1, a2):
        a.set_xlabel("Content Store capacity (entries)")
        a.set_xscale("log", base=2); a.set_xticks(xs)
        a.set_xticklabels([str(x) for x in xs])
        a.legend(frameon=False, loc="best")
    save(fig, Path(outdir) / "fig_capacity.pdf")


POLICY_PAIRS = [
    ("LCE + LRU", "Fixed-LCE-LRU", "AT-LCE-LRU"),
    ("LCD + LRU", "Fixed-LCD-LRU", "AT-LCD-LRU"),
    ("LCE + aged LFU", "Fixed-LCE-LFUA", "AT-LCE-LFUA"),
    ("LCE + LFF-origin", "Base-LCE-LFFO", "AT-LCE-LFFO"),
    ("fresh-gated + LFF", "Fixed-FRESH-LFFO", "AT-FRESH-LFFO"),
    ("TinyLFU + LRU", "Fixed-TLFU-LRU", "AT-TLFU-LRU"),
    ("door-gated + LRU", "Fixed-DOOR-LRU", "AT-DOOR-LRU"),
    ("LeCaR", "LeCaR", "AT-LeCaR"),
    ("pCASTING", "Base-pCASTING", "AT-pCASTING"),
    ("Q-learning admission", "Fixed-QL-LRU", "AT-QL-LRU"),
    ("DQN + A2C", "DRL", "AT-DRL"),
]

# Eleven standard-NDN points fall close together between 6% and 14% stale, and
# a single fixed offset overprints three of the labels. These are placed by
# hand against the rendered figure: (dx pt, dy pt, horizontal anchor).
LABEL_NUDGE = {
    "LCE + LRU":            (6, -9, "left"),
    "LeCaR":                (-6, -9, "right"),
    "LCD + LRU":            (-7, -2, "right"),
    "LCE + aged LFU":       (-6, -9, "right"),
    "LCE + LFF-origin":     (7, 5, "left"),
    "fresh-gated + LFF":    (7, -9, "left"),
    "TinyLFU + LRU":        (2, 7, "left"),
    "door-gated + LRU":     (-4, -10, "right"),
    "pCASTING":             (-6, 4, "right"),
    "Q-learning admission": (6, 4, "left"),
    "DQN + A2C":            (6, -9, "left"),
}


def fig_policies(d, outdir):
    """Each policy as an arrow from its standard point to its age-transparent one."""
    setup()
    fig, ax = plt.subplots(figsize=(W_FULL, 3.5))
    got = 0
    for name, b, t in POLICY_PAIRS:
        ps = pr(d, b, t, "stale_delivery")
        pv = pr(d, b, t, "valid_cache_service")
        if not ps or not pv:
            continue
        got += 1
        x0, x1 = 100 * ps[0], 100 * ps[1]
        y0, y1 = 100 * pv[0], 100 * pv[1]
        ax.annotate("", xy=(x1, y1), xytext=(x0, y0),
                    arrowprops=dict(arrowstyle="-|>", color="#8a9199", linewidth=0.9,
                                    shrinkA=2.5, shrinkB=2.5))
        ax.plot([x0], [y0], marker="o", color=STD, markersize=4.6, zorder=5)
        ax.plot([x1], [y1], marker="s", color=ATC, markersize=4.6, zorder=5)
        dx, dy, ha = LABEL_NUDGE.get(name, (5, -7, "left"))
        ax.annotate(name, (x0, y0), textcoords="offset points", xytext=(dx, dy),
                    fontsize=6.6, color=INK, ha=ha)
    if not got:
        plt.close(fig)
        pending(Path(outdir) / "fig_policies.pdf",
                "policy pairs not yet complete"); return
    ax.plot([], [], marker="o", color=STD, linestyle="none", label="standard NDN")
    ax.plot([], [], marker="s", color=ATC, linestyle="none", label="age-transparent")
    ax.set_xlabel("stale deliveries (%)")
    ax.set_ylabel("valid cache service (%)")
    # Room on the right for the labels that sit at the far end of the sweep.
    x0_, x1_ = ax.get_xlim()
    ax.set_xlim(x0_ - 0.4, x1_ + 2.6)
    ax.legend(frameon=False, loc="lower right")
    save(fig, Path(outdir) / "fig_policies.pdf")


def fig_frontier(d, outdir):
    """The under-declaration frontier against the rule.

    A publisher can blunt the per-hop rule by declaring a shorter lifetime than
    the application needs, which forces copies to expire sooner. That traces a
    frontier. The question is whether changing the clock reaches a point the
    frontier cannot.
    """
    pts = []
    for lab, cfg in (("declared = needed", "Fixed-LCE-LRU"),
                     ("declared = 1/2", "Short2-LCE-LRU"),
                     ("declared = 1/3", "Short3-LCE-LRU"),
                     ("declared = 1/4", "Short4-LCE-LRU")):
        m = cm(d, cfg)
        if not m:
            continue
        xs = [v["stale_delivery"] for v in m.values()]
        ys = [v["valid_cache_service"] for v in m.values()]
        pts.append((lab, 100 * sum(xs) / len(xs), 100 * sum(ys) / len(ys)))
    at = cm(d, "AT-LCE-LRU")
    if len(pts) < 2 or not at:
        pending(Path(outdir) / "fig_frontier.pdf",
                "under-declaration runs not yet complete")
        return
    axs = [v["stale_delivery"] for v in at.values()]
    ays = [v["valid_cache_service"] for v in at.values()]
    ax_, ay_ = 100 * sum(axs) / len(axs), 100 * sum(ays) / len(ays)

    setup()
    fig, ax = plt.subplots(figsize=(W_FULL, 3.1))
    ax.plot([p[1] for p in pts], [p[2] for p in pts], marker="o", color=STD,
            linewidth=1.2, markersize=5, label="under-declared lifetime, per-hop rule")
    for i, (lab, x, y) in enumerate(pts):
        # The k=1 point sits at the far right, where a label drawn to the
        # right of it leaves the axes and lands on the legend.
        off = (-7, -3) if i == 0 else (6, -3)
        ha = "right" if i == 0 else "left"
        ax.annotate(lab, (x, y), textcoords="offset points", xytext=off,
                    fontsize=6.8, color=INK, ha=ha)
    ax.plot([ax_], [ay_], marker="*", color=ATC, markersize=13, linestyle="none",
            label="age-transparent, lifetime left alone", zorder=6)
    ax.annotate("age-transparent", (ax_, ay_), textcoords="offset points",
                xytext=(9, 2), fontsize=7.0, color=ATC, fontweight="bold")
    ax.set_xlabel("stale deliveries (%)")
    ax.set_ylabel("valid cache service (%)")
    # Upper right is the only region the curve and its labels leave empty.
    ax.legend(frameon=False, loc="upper right", fontsize=6.8)
    save(fig, Path(outdir) / "fig_frontier.pdf")


if __name__ == "__main__":
    d = Path(sys.argv[1])
    outdir = Path(sys.argv[2]) if len(sys.argv) > 2 else Path(__file__).resolve().parents[1] / "figures"
    fig_mechanism(d, outdir)
    fig_depth(d, outdir)
    fig_capacity(d, outdir)
    fig_policies(d, outdir)
    fig_frontier(d, outdir)
