#!/usr/bin/env python3
"""Figure 1: the system, the forwarding pipeline, and the packet.

Why this figure exists
----------------------
The manuscript previously opened on a timeline of one path, which shows the
defect but not the system it lives in. A reader arriving at Section 4 had no
picture of where in a node the rule acts, what a node keeps, or which part of
a Data packet may legally be written on the way. Three of the supervisor's
comments asked for exactly that: the packet structure, the forwarding
mechanism with PIT, FIB, Content Store and MustBeFresh, and how a per-hop
value avoids the producer's signature.

Drawn at the Springer text measure, 372 pt, so the figure is placed at scale
1.000 and a 7 pt label is 7 pt on paper. The previous figure was drawn at
482 pt and placed in a 372 pt measure, which shrank its smallest type to
5.4 pt.

Everything in it is taken from the implementation rather than from a generic
NDN diagram: the names are the ones the producer application builds, the
table contents are the fields those tables hold, and the two marked decision
points are the two call sites this paper changes.
"""
import sys
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, Circle, FancyArrowPatch

# regen.sh runs every figure script under python3 -I, which drops the script's
# own directory from sys.path, so the shared theme has to be put back by hand.
sys.path.insert(0, str(Path(__file__).resolve().parent))
import paper_theme as T
import iso_icons as I

W, H = 372 / 72.0, 6.26          # exact Springer measure

# --------------------------------------------------------------- palette
BOX_F, BOX_E = "#f7f9fb", "#8e99a4"
ACC = "#1a4f8f"                  # structural stroke, draw.io-like
OURS = T.P_OURS                  # what this paper changes
INT_C = "#2a78d6"                # Interest
DAT_C = "#a8690a"                # Data
SIGN_F, SIGN_E = "#eef4fb", "#5b7fa6"
LL_F, LL_E = "#eaf5ee", OURS
WARN = "#9a3b2f"


def rbox(ax, x, y, w, h, fc=BOX_F, ec=BOX_E, lw=0.7, r=0.035, z=4, ls="-"):
    ax.add_patch(FancyBboxPatch(
        (x, y), w, h, boxstyle=f"round,pad=0,rounding_size={r}",
        fc=fc, ec=ec, lw=lw, zorder=z, linestyle=ls))


def label(ax, x, y, s, pt=T.PT_MICRO, w="normal", c=T.INK, ha="center",
          va="center", z=9, style="normal", fam=None):
    ax.text(x, y, s, fontsize=pt, fontweight=w, color=c, ha=ha, va=va,
            zorder=z, style=style, family=fam)


def arrow(ax, p, q, c=ACC, lw=0.9, z=8, style="-|>", ms=6, ls="-"):
    ax.add_patch(FancyArrowPatch(p, q, arrowstyle=style, mutation_scale=ms,
                                 color=c, lw=lw, zorder=z, linestyle=ls,
                                 shrinkA=0, shrinkB=0,
                                 joinstyle="miter", capstyle="butt"))


def badge(ax, x, y, n, c=OURS, r=0.072, z=12):
    """A numbered marker for a point this paper changes."""
    ax.add_patch(Circle((x, y), r, fc=c, ec="white", lw=0.8, zorder=z))
    label(ax, x, y, str(n), pt=T.PT_MICRO - 1.0, w="bold", c="white", z=z + 1)


def panel(ax, x, y, w, h, letter, title):
    rbox(ax, x, y, w, h, fc="#ffffff", ec=T.RULE, lw=0.6, r=0.045, z=2)
    label(ax, x + 0.10, y + h - 0.15, f"({letter})", pt=T.PT_PANEL,
          w="bold", ha="left")
    label(ax, x + 0.36, y + h - 0.15, title, pt=T.PT_PANEL, ha="left")


def build():
    T.apply_rc(plt)
    fig = plt.figure(figsize=(W, H))
    ax = fig.add_axes([0, 0, 1, 1])
    ax.set_xlim(0, W); ax.set_ylim(0, H)
    ax.set_aspect("equal"); ax.axis("off")

    # ============================================================ panel (a)
    ay, ah = 4.60, 1.58
    panel(ax, 0.04, ay, W - 0.08, ah, "a", "Sensing field")

    row = ay + 0.74
    xs = [0.56, 1.46, 2.36, 3.26, 4.30]
    kinds = ["consumer", "cache", "cache", "cache", "producer"]
    names = ["Consumer", "Node 1", "Node 2", "Node 3", "Publisher"]
    for x, kind, nm in zip(xs, kinds, names):
        {"consumer": I.mote_consumer, "producer": I.mote_producer,
         "cache": I.mote_hit}[kind](ax, x, row, s=0.27, z=6)
        label(ax, x, row - 0.29, nm, pt=T.PT_MICRO, w="bold")

    yi = row + 0.42
    for a, b in zip(xs[:-1], xs[1:]):
        arrow(ax, (a + 0.20, yi), (b - 0.20, yi), c=INT_C, lw=0.9, ms=5.5,
              ls=(0, (2.6, 1.5)))
    label(ax, W / 2, yi + 0.125, "Interest   /campus/env/item17",
          pt=T.PT_MICRO, c=INT_C, fam="monospace")

    yd = row - 0.47
    for a, b in zip(xs[1:], xs[:-1]):
        arrow(ax, (a - 0.20, yd), (b + 0.20, yd), c=DAT_C, lw=0.9, ms=5.5)
    label(ax, W / 2, yd - 0.135,
          "Data: the reading, its declared lifetime $F$, and the "
          "publisher's signature", pt=T.PT_MICRO, c=DAT_C)


    # ============================================================ panel (b)
    by, bh = 2.20, 2.30
    panel(ax, 0.04, by, W - 0.08, bh, "b", "Inside a node")

    sw, sh, gap = 0.78, 0.27, 0.115
    x_first = 0.60
    py_i = by + 1.66
    py_d = by + 1.17

    def step(x, y, s, fc=BOX_F, ec=BOX_E, w="normal"):
        rbox(ax, x, y, sw, sh, fc=fc, ec=ec, lw=0.75, r=0.03, z=5)
        label(ax, x + sw / 2, y + sh / 2, s, pt=T.PT_MICRO, w=w)
        return x + sw

    label(ax, 0.14, py_i + sh / 2, "Interest", pt=T.PT_MICRO, w="bold",
          c=INT_C, ha="left")
    x = x_first
    for k, s in enumerate(["Face", "Content Store\nlookup",
                           "MustBeFresh\ncheck", "PIT insert",
                           "FIB lookup"]):
        if k == 1:
            x_cs = x
        nxt = step(x, py_i, s)
        if k < 4:
            arrow(ax, (nxt, py_i + sh / 2), (nxt + gap, py_i + sh / 2),
                  c=INT_C, lw=0.8, ms=5)
        x = nxt + gap
    label(ax, x - gap + 0.04, py_i + sh + 0.085, "then upstream",
          pt=T.PT_MICRO, c=INT_C, ha="right")

    label(ax, 0.14, py_d + sh / 2, "Data", pt=T.PT_MICRO, w="bold", c=DAT_C,
          ha="left")
    x = x_first
    marks = []
    for k, (s, fc, ec) in enumerate([
            ("Face", BOX_F, BOX_E), ("PIT match", BOX_F, BOX_E),
            ("admit to the\nContent Store", LL_F, OURS),
            ("attach the\nresidual", LL_F, OURS),
            ("send\ndownstream", BOX_F, BOX_E)]):
        marks.append(x)
        nxt = step(x, py_d, s, fc=fc, ec=ec,
                   w="bold" if ec == OURS else "normal")
        if k < 4:
            arrow(ax, (nxt, py_d + sh / 2), (nxt + gap, py_d + sh / 2),
                  c=DAT_C, lw=0.8, ms=5)
        x = nxt + gap
    badge(ax, marks[2] + 0.055, py_d + sh - 0.012, 1)
    badge(ax, marks[3] + 0.055, py_d + sh - 0.012, 2)

    # a fresh hit short-circuits the pipeline
    arrow(ax, (x_cs + sw / 2, py_i), (x_cs + sw / 2, py_d + sh), c=INT_C,
          lw=0.7, ms=5, ls=(0, (2, 1.4)))
    label(ax, x_cs + sw / 2 + 0.07, (py_i + py_d + sh) / 2,
          "fresh hit answered here", pt=T.PT_MICRO, c=T.LEAD, ha="left")

    yexp = by + 0.99
    badge(ax, 0.20, yexp, 1, r=0.062)
    label(ax, 0.30, yexp,
          "deadline set to $t_a + \\min\\{F,\\ \\max(0, r)\\}$, "
          "not $t_a + F$", pt=T.PT_MICRO, ha="left", c=OURS)
    badge(ax, 2.74, yexp, 2, r=0.062)
    label(ax, 2.84, yexp,
          "residual $r$ set to deadline $-$ now, one link only",
          pt=T.PT_MICRO, ha="left", c=OURS)

    # the three tables
    tw, th, ty = 1.54, 0.72, by + 0.17
    for x0, nm, fc, ec, key, val in [
        (0.14, "Content Store", T.CS_F, T.CS_E,
         "/campus/env/item17", "expires in 12.4 s"),
        (1.80, "Pending Interest Table", T.PIT_F, T.PIT_E,
         "/campus/env/item17", "awaited on face 2"),
        (3.46, "Forwarding Information Base", T.FIB_F, T.FIB_E,
         "/campus/env", "next hop is face 0")]:
        rbox(ax, x0, ty, tw, th, fc=fc, ec=ec, lw=0.8, r=0.03, z=4)
        label(ax, x0 + tw / 2, ty + th - 0.115, nm, pt=T.PT_MICRO, w="bold")
        ax.plot([x0 + 0.05, x0 + tw - 0.05], [ty + th - 0.205] * 2,
                color=ec, lw=0.5, zorder=6)
        label(ax, x0 + tw / 2, ty + th - 0.325, key, pt=T.PT_MICRO,
              fam="monospace")
        label(ax, x0 + tw / 2, ty + th - 0.505, val, pt=T.PT_MICRO, c=T.LEAD)

    # ============================================================ panel (c)
    cy, ch = 0.06, 1.98
    panel(ax, 0.04, cy, W - 0.08, ch, "c", "The Data packet")

    # the link-local header, on its own line at the top
    lx, lw_, ly, lh = 0.17, 1.74, cy + 1.40, 0.28
    rbox(ax, lx, ly, lw_, lh, fc=LL_F, ec=LL_E, lw=0.9, r=0.025, z=5)
    label(ax, lx + lw_ / 2, ly + 0.172, "NDNLPv2 link-local header",
          pt=T.PT_MICRO, w="bold", c=LL_E)
    label(ax, lx + lw_ / 2, ly + 0.072, "residual lifetime $r$, 8 B",
          pt=T.PT_MICRO, c=T.LEAD)
    label(ax, lx + lw_ + 0.14, ly + 0.14,
          "an ignorable type in [800, 959]", pt=T.PT_MICRO,
          ha="left", c=T.LEAD)

    # MetaInfo expansion, its own line
    px, py_, pbh = 0.17, cy + 0.46, 0.42
    blocks = [("Name", 0.76), ("MetaInfo", 0.94), ("Content", 0.80),
              ("Signature\nInfo", 0.80), ("Signature\nValue", 0.84)]
    xx, span = px, {}
    for nm, bw in blocks:
        span[nm] = (xx, bw)
        xx += bw
    mx, mw = span["MetaInfo"]
    ex0, exw = 1.12, 1.34
    rbox(ax, ex0, cy + 1.02, exw, 0.22, fc="#ffffff", ec=SIGN_E,
         lw=0.6, r=0.02, z=5)
    label(ax, ex0 + exw / 2, cy + 1.13, "ContentType, FreshnessPeriod $F$",
          pt=T.PT_MICRO)
    ax.plot([mx + mw / 2] * 2, [cy + 0.88, cy + 1.02], color=SIGN_E, lw=0.6,
            zorder=6)

    # the packet itself
    for nm, bw in blocks:
        x0 = span[nm][0]
        rbox(ax, x0, py_, bw, pbh, fc=SIGN_F, ec=SIGN_E, lw=0.8, r=0.025, z=5)
        label(ax, x0 + bw / 2, py_ + pbh / 2, nm, pt=T.PT_MICRO)

    # the signed portion runs Name through SignatureInfo; SignatureValue
    # carries the signature and is not itself covered by it
    x0 = span["Name"][0]
    x1 = span["Signature\nInfo"][0] + span["Signature\nInfo"][1]
    yb = py_ - 0.085
    ax.plot([x0, x0, x1, x1], [yb + 0.05, yb, yb, yb + 0.05], color=SIGN_E,
            lw=0.9, zorder=7)
    label(ax, (x0 + x1) / 2, yb - 0.115,
          "signed portion: Name through SignatureInfo, covered by "
          "SignatureValue", pt=T.PT_MICRO, c=SIGN_E)

    # the annotation rides beside the packet, never inside it
    axx = lx + 0.07
    arrow(ax, (axx, ly), (axx, py_ + pbh + 0.03), c=LL_E, lw=0.85, ms=5.5,
          ls=(0, (2, 1.4)))
    label(ax, axx + 0.09, cy + 1.14,
          "carried beside\nthe packet",
          pt=T.PT_MICRO, ha="left", c=LL_E)

    return fig


if __name__ == "__main__":
    import sys
    out = sys.argv[1] if len(sys.argv) > 1 else "fig_system.pdf"
    fig = build()
    fig.savefig(out, format="pdf")
    fig.savefig(out.replace(".pdf", ".png"), dpi=200)
    print(f"wrote {out} at {W*72:.1f} x {H*72:.1f} pt (scale 1.000 in a 372 pt measure)")
