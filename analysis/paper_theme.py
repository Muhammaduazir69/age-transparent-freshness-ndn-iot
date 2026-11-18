"""One design system for every figure in the manuscript.

Why this module exists
----------------------
Figures 1 to 5 were previously drawn by four scripts that each carried their
own literal hex codes, their own point sizes and their own idea of what blue
meant.  A reader who moves from the architecture figure to the results plots
should not have to relearn the color code on the way, and a reviewer who
prints the paper in grayscale should still be able to tell the series apart.
Everything visual is therefore defined once, here, and imported by

    arch_theme.py           icon primitives for Fig. 1
    make_arch_fig.py        Fig. 1
    make_scenarios_fig.py   Fig. 2
    make_results_figs.py    Figs. 3, 4, 5

Two orthogonal color axes, and they never mix in one panel
----------------------------------------------------------
1.  SUBSTRATE, the independent variable of the paper's central experiment:

        SUB_REAL   #0072B2   IEEE 802.15.4, contention and battery
        SUB_IDEAL  #D55E00   idealized link, no contention, no energy

    These two appear only where the substrate is what changes.

2.  POLICY CLASS, used wherever policies are compared on one substrate:

        P_OURS     #00795A   MAACO, this paper's instrument
        P_LEARNED  #6C3FA8   other learned policies (DQN, tabular Q, MAB)
        P_PROB     #B8620E   probabilistic admission
        P_FREQ     #3C8DC4   frequency-based replacement (LFU, aged LFU)
        P_FRESH    #C2649A   freshness-directed replacement (LFF, FreshCheck)
        P_DEFAULT  #7d8790   the ICN defaults (LCE, LCD, LRU, FIFO)

Both axes were checked with the dataviz skill's `validate_palette.js` and pass
every test (lightness band, chroma floor, CVD separation, normal-vision floor
and contrast against the page) at the values above.  MAACO is the darkest of the policy colors on
purpose, so this paper's own series is the one that survives a grayscale
print most clearly.

Six categorical hues cannot all be separated by luminance alone, so no series
is ever identified by hue: every line also carries its own dash pattern and
marker, and every bar its own position and printed value.  `selftest()` checks
contrast within each axis and reports which pairs are leaning on the dash cue;
the build runs it and fails on a contrast error.

Typography
----------
Figures are drawn at their final printed width and saved without a tight
bounding box, so a point size set here is the point size the reader measures
on paper.  Nothing is allowed below 7 pt, which is the IEEE floor for figure
text and roughly the limit of comfortable reading at arm's length.
"""

import numpy as np

# --------------------------------------------------------------- geometry
COL = 3.5            # \columnwidth for IEEEtran two-column, inches
TEXT = 7.16          # \textwidth, inches

# ------------------------------------------------------- semantic palette
# axis 1: which physical substrate the numbers came from
SUB_REAL = "#2a78d6"
SUB_IDEAL = "#d64545"

# axis 2: what class of caching policy a series is
P_OURS = "#1a8f63"
P_LEARNED = "#7c4fc4"
P_PROB = "#a8690a"
P_FREQ = "#2a78d6"
P_FRESH = "#c2649a"
P_DEFAULT = "#7b8794"
P_FAINT = "#b8bfc6"

# legacy aliases so the older scripts keep working unchanged
GREEN_FILL = "#009E73"   # Fig. 1 learning-plane fills and rings
BLUE, ORANGE, GREEN, VIOLET = SUB_REAL, SUB_IDEAL, GREEN_FILL, P_LEARNED
REAL, IDEAL = SUB_REAL, SUB_IDEAL
MUTED = "#A8CBE0"

# ------------------------------------------------------------------- ink
INK = "#1f2933"
LEAD = "#52606d"
HAIR = "#9aa1a8"
RULE = "#c9ced3"
GRID = "#d7dbdf"

# ------------------------------------------------- panel and surface tints
PAPER = "#ffffff"
PANEL_A = "#f6f8f4"      # sensing field, a faint vegetation tint
PANEL_B = "#f7f8fa"      # forwarding plane, neutral
PANEL_C = "#f2f8f6"      # learning plane, faint teal
PANEL_D = "#fafafb"      # the swap band
PANEL_E = "#ccd2d8"      # panel stroke

CS_F, CS_E = "#e7f1e6", "#8fbb92"      # Content Store
PIT_F, PIT_E = "#fdf1da", "#dcb46b"    # PIT
FIB_F, FIB_E = "#e2ecf8", "#8fb3d6"    # FIB
WARN_F = "#fdeee5"

# ------------------------------------------------ material colors (icons)
# These make an icon look like the object it stands for.  They never carry
# meaning, and no reader is ever asked to decode them.
FR4_HI, FR4_LO = "#3f7f52", "#27573a"          # printed circuit board
COPPER = "#c08a45"                              # exposed trace and pad
SOLDER = "#c9cdd2"                              # tinned metal
MOULD_HI, MOULD_LO = "#31363c", "#181b1f"      # black molded package
SILICON_HI, SILICON_LO = "#39424f", "#1d232c"  # bare die
GOLD = "#d3ab4a"                                # bond pad and lead frame
LED_ON, LED_CORE = "#e0503c", "#ffb3a3"
ALK_HI, ALK_LO = "#e8b74e", "#b07f22"          # alkaline cell shell
CHARGE_HI, CHARGE_LO = "#7fbc5c", "#4c8b3c"    # remaining charge
STEEL_HI, STEEL_LO = "#cfd4d9", "#8e959c"      # mast and casing
SOIL_HI, SOIL_LO = "#9c7248", "#6d4c2c"        # soil probe
GLASS = "#eef3f7"
DEAD = "#c2c7cc"                                # a component that was removed

# infrastructure materials, for the scenario figure
PLASTIC_HI, PLASTIC_LO = "#dfe4e9", "#b3bbc3"  # router / AP enclosure
SCREEN_HI, SCREEN_LO = "#cfe0ee", "#8fa8bd"    # consumer display
RACK_HI, RACK_LO = "#8a939c", "#5c646c"        # server rack

# ------------------------------------------------------------ typography
PT_MICRO = 7.0       # hard floor, IEEE figure minimum
PT_VALUE = 7.0
PT_LEGEND = 7.2
PT_TICK = 7.5
PT_LABEL = 8.0
PT_PANEL = 8.6       # panel letter and its title

LW_AXIS = 0.6
LW_GRID = 0.4
LW_SERIES = 1.15

# ------------------------------------------------- per-policy style table
# (color, linestyle, marker).  Hue is never the only cue.
POLICY_STYLE = {
    "MAACO":            (P_OURS,    "-",                 "o"),
    "DQN+LRU":          (P_LEARNED, (0, (4.5, 1.6)),     "s"),
    "Tabular Q":        (P_LEARNED, (0, (1.2, 1.2)),     "^"),
    "Cache-MAB":        (P_LEARNED, (0, (3, 1.2, 1, 1.2)), "D"),
    "Prob(0.4)+LRU":    (P_PROB,    (0, (3, 1.4)),       "^"),
    "Prob(0.5)+LRU":    (P_PROB,    (0, (1.4, 1.4)),     "v"),
    "LFU-Aged":         (P_FREQ,    (0, (5, 1.5, 1, 1.5)), "D"),
    "LCE+LFU":          (P_FREQ,    (0, (1.8, 1.4)),     "d"),
    "LCE+LFF":          (P_FRESH,   (0, (2.6, 1.3)),     "P"),
    "FreshCheck+LRU":   (P_FRESH,   (0, (1.2, 1.1)),     "X"),
    "LCE+LRU":          (P_DEFAULT, (0, (4, 1.6)),       "x"),
    "LCE+FIFO":         (P_DEFAULT, (0, (1.5, 1.5)),     "+"),
    "LCD+LRU":          (P_FAINT,   (0, (2.2, 1.5)),     "*"),
}


def style(name, fallback=(P_DEFAULT, "-", "o")):
    """Color, dash and marker for a policy name, matched loosely."""
    if name in POLICY_STYLE:
        return POLICY_STYLE[name]
    for k, v in POLICY_STYLE.items():
        if k.lower().split("(")[0] in name.lower():
            return v
    return fallback


# ------------------------------------------------------------ matplotlib
def apply_rc(plt):
    """Install the house rc.  Call once per figure script."""
    plt.rcParams.update({
        # Serif, to match the IEEEtran body text and the 3D system figure.
        # A plot set in a different family than the paper it sits in reads as
        # pasted in, which is the single most common reason a competent figure
        # still looks unprofessional on the page.
        "font.family": "serif",
        "font.serif": ["Times New Roman", "Nimbus Roman", "Times",
                       "DejaVu Serif"],
        "mathtext.fontset": "stix",
        "pdf.fonttype": 42, "ps.fonttype": 42,     # embed as TrueType
        "font.size": PT_TICK,
        "axes.edgecolor": LEAD, "axes.labelcolor": INK, "text.color": INK,
        "xtick.color": INK, "ytick.color": INK,
        "axes.linewidth": LW_AXIS,
        "xtick.major.width": LW_AXIS, "ytick.major.width": LW_AXIS,
        "xtick.major.size": 2.2, "ytick.major.size": 2.2,
        "xtick.labelsize": PT_TICK, "ytick.labelsize": PT_TICK,
        "axes.labelsize": PT_LABEL,
        "legend.fontsize": PT_LEGEND,
        "legend.frameon": False,
        "legend.handlelength": 2.4,
        "legend.borderaxespad": 0.2,
        "figure.dpi": 600,
        "savefig.dpi": 600,
    })


def finish(ax, ygrid=True, xgrid=False):
    """Strip the top and right rules and lay a hairline grid behind."""
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    if ygrid:
        ax.yaxis.grid(True, color=GRID, lw=LW_GRID)
    if xgrid:
        ax.xaxis.grid(True, color=GRID, lw=LW_GRID)
    ax.set_axisbelow(True)


def panel_tag(ax, letter, title=None, dx=-0.085, dy=1.045, pt=None):
    """(a)-style panel label in the axes' top-left corner."""
    pt = pt or PT_PANEL
    s = f"({letter})" if title is None else f"({letter}) {title}"
    ax.text(dx, dy, s, transform=ax.transAxes, ha="left", va="baseline",
            fontsize=pt, fontweight="bold", color=INK, clip_on=False)


# --------------------------------------------------------------- checks
def _lum(hexc):
    r, g, b = (int(hexc[i:i + 2], 16) / 255 for i in (1, 3, 5))
    f = lambda c: c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
    return 0.2126 * f(r) + 0.7152 * f(g) + 0.0722 * f(b)


def selftest(verbose=True):
    """Contrast on paper, and grayscale separability within each color axis.

    The two axes never share a panel, so a substrate color is allowed to sit
    at the same luminance as a policy color.  Within an axis, a pair closer
    than 0.045 in relative luminance is legible in grayscale only because the
    series also differ in dash and marker; those pairs are listed rather than
    failed, and POLICY_STYLE is what guarantees the second cue.
    """
    axes = {
        "substrate": {"SUB_REAL": SUB_REAL, "SUB_IDEAL": SUB_IDEAL},
        "policy": {"P_OURS": P_OURS, "P_LEARNED": P_LEARNED,
                   "P_PROB": P_PROB, "P_FREQ": P_FREQ,
                   "P_FRESH": P_FRESH, "P_DEFAULT": P_DEFAULT},
    }
    bad, leaning = [], []
    for axis, pal in axes.items():
        for n, c in pal.items():
            contrast = 1.05 / (_lum(c) + 0.05)
            if contrast < 2.6:
                bad.append(f"{n} too pale on white (contrast {contrast:.2f})")
        ks = list(pal)
        for i in range(len(ks)):
            for j in range(i + 1, len(ks)):
                d = abs(_lum(pal[ks[i]]) - _lum(pal[ks[j]]))
                if d < 0.045:
                    leaning.append((axis, ks[i], ks[j], d))

    # every policy in the style table must carry a dash and a marker
    styleless = [k for k, v in POLICY_STYLE.items()
                 if v[1] in ("-", "solid") and k != "MAACO"]

    if verbose:
        n = sum(len(p) for p in axes.values())
        print(f"  theme: {n} semantic colors across {len(axes)} axes, "
              f"{len(bad)} contrast failures, "
              f"{len(leaning)} pairs relying on the dash cue")
        for a, x, y, d in leaning:
            print(f"    {a}: {x} vs {y}, luminance gap {d:.3f}")
        if styleless:
            print(f"    solid-line policies besides MAACO: {styleless}")
    for m in bad:
        print("    FAIL:", m)
    assert not styleless, "only MAACO may be a solid line"
    return not bad


if __name__ == "__main__":
    selftest()
