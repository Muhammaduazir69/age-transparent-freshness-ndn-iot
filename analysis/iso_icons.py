"""Isometric vector icons for the NDN-IoT substrate figure.

Why these and not the stock set
-------------------------------
`netfig3d` ships satellites, gateways and handsets, which belong to a
non-terrestrial paper.  This study runs on battery-powered 802.15.4 motes in a
4x4 lattice, so every node icon here is drawn from what the simulated node
actually is: a printed circuit board carrying a microcontroller, a whip
antenna and a cell, plus the sensing transducer on the one node that publishes
content.

Everything is a true isometric solid: a top face and two side faces per box,
the sides shaded darker than the top by a fixed factor, so the icons read as
three-dimensional objects lit from the upper left rather than as flat clip art.
All drawing happens in inches on a transparent overlay with equal aspect, which
keeps the output vector and undistorted by the 3D projection underneath.

Every icon is a function ``icon(ax, x, y, s=..., z=...)`` so it can be
registered in ``netfig3d.ICONS`` and placed by ``Scene.node``.
"""
from matplotlib.patches import Circle, Ellipse, FancyBboxPatch, Polygon

# ---------------------------------------------------------------- palette
INK = "#1f2933"
INK2 = "#52606d"

PCB = "#6d7a85"          # neutral slate board: color is reserved for role
CHIP = "#333b42"         # the microcontroller package
GOLD = "#c9a227"         # pads and the antenna whip
CELL = "#c9ced4"         # coin cell
# These are the manuscript's validated hues (paper_theme.py), so Fig. 1 and the
# result plots speak one color language: blue is a request or the measured
# substrate, green is the content source, purple is the learning plane.
SOURCE = "#1a8f63"       # the producing transducer: where content comes from
SOURCE_D = "#146b4a"
LEARN = "#7c4fc4"        # the learning plane
LEARN_D = "#5f37a3"
RADIO = "#2a78d6"        # a request, and radio energy
DATA = "#a8690a"         # a Data packet coming back

# isometric basis: unit steps along the scene's x and y
EX = (0.866, 0.5)
EY = (-0.866, 0.5)


def _shade(hex_color, f):
    """Multiply a hex color toward black by f (f < 1 darkens)."""
    c = hex_color.lstrip("#")
    r, g, b = (int(c[i:i + 2], 16) for i in (0, 2, 4))
    return "#%02x%02x%02x" % (min(255, int(r * f)), min(255, int(g * f)), min(255, int(b * f)))


def iso_box(ax, x, y, w, d, h, color, ec=INK, lw=0.5, z=5):
    """An isometric cuboid whose TOP-face center sits at (x, y).

    w runs along EX, d along EY, h straight down.  The two visible side faces
    are shaded so the solid reads as lit from the upper left.
    """
    ax_, ay_ = EX[0] * w / 2, EX[1] * w / 2
    bx_, by_ = EY[0] * d / 2, EY[1] * d / 2
    A = (x + ax_ + bx_, y + ay_ + by_)      # back corner
    B = (x + ax_ - bx_, y + ay_ - by_)      # right corner
    C = (x - ax_ - bx_, y - ay_ - by_)      # front corner
    D = (x - ax_ + bx_, y - ay_ + by_)      # left corner
    if h > 0:
        ax.add_patch(Polygon([B, C, (C[0], C[1] - h), (B[0], B[1] - h)], closed=True,
                             fc=_shade(color, 0.68), ec=ec, lw=lw, zorder=z, joinstyle="miter"))
        ax.add_patch(Polygon([C, D, (D[0], D[1] - h), (C[0], C[1] - h)], closed=True,
                             fc=_shade(color, 0.84), ec=ec, lw=lw, zorder=z, joinstyle="miter"))
    ax.add_patch(Polygon([A, B, C, D], closed=True, fc=color, ec=ec, lw=lw, zorder=z + 1,
                         joinstyle="miter"))
    return dict(A=A, B=B, C=C, D=D)


def iso_disc(ax, x, y, rx, color, ec=INK, lw=0.5, z=5, h=0.0):
    """A flat isometric disc, optionally extruded into a short cylinder."""
    if h > 0:
        ax.add_patch(Ellipse((x, y - h), rx * 2, rx * 1.0, fc=_shade(color, 0.72), ec=ec, lw=lw, zorder=z))
        ax.add_patch(Polygon([(x - rx, y), (x + rx, y), (x + rx, y - h), (x - rx, y - h)],
                             fc=_shade(color, 0.72), ec="none", zorder=z))
    ax.add_patch(Ellipse((x, y), rx * 2, rx * 1.0, fc=color, ec=ec, lw=lw, zorder=z + 1))


def _antenna(ax, x, y, s, z, color=GOLD):
    """A whip antenna rising from the board with a small tip bead."""
    ax.plot([x, x], [y, y + s * 0.46], color=color, lw=1.15, solid_capstyle="round", zorder=z + 3)
    ax.add_patch(Circle((x, y + s * 0.46), s * 0.05, fc=color, ec=INK, lw=0.3, zorder=z + 4))


def _board(ax, x, y, s, z, board_color=PCB):
    """The common mote body: PCB, MCU, coin cell, three gold pads."""
    iso_box(ax, x, y, s * 1.02, s * 0.74, s * 0.075, board_color, z=z)
    iso_box(ax, x + s * 0.10, y + s * 0.045, s * 0.34, s * 0.30, s * 0.055, CHIP, z=z + 2)
    iso_disc(ax, x - s * 0.27, y - s * 0.045, s * 0.15, CELL, z=z + 2, h=s * 0.05)
    for k in (-1, 1):
        px = x + s * 0.30 + EY[0] * k * s * 0.14
        py = y - s * 0.115 + EY[1] * k * s * 0.14
        ax.add_patch(Ellipse((px, py), s * 0.08, s * 0.040, fc=GOLD, ec="none", zorder=z + 3))


def mote(ax, x, y, s=0.30, z=5, **_):
    """A battery-powered 802.15.4 caching node: board, MCU, cell, whip."""
    _board(ax, x, y, s, z)
    _antenna(ax, x + s * 0.34, y + s * 0.10, s, z)


def mote_producer(ax, x, y, s=0.30, z=5, **_):
    """The publishing node: the mote plus a sensing transducer.

    Shape, not color, carries the role.  At the printed size of this figure an
    icon is about 0.24 in across, small enough that a reader picks it out by
    silhouette before any hue registers, and a figure whose roles separate only
    by color fails for a color-blind reader and in grayscale print.  The
    producer is therefore a probe: a bulb at the foot of a stem under a vented
    head, the outline of a transducer sampling its environment.  The consumer
    opposite it is a flat panel.  The two stay apart in a thumbnail with the
    color removed.
    """
    _board(ax, x, y, s, z)
    _antenna(ax, x + s * 0.34, y + s * 0.10, s, z)
    sx, sy = x - s * 0.24, y + s * 0.10
    ax.plot([sx, sx], [sy + s * 0.10, sy + s * 0.46], color=SOURCE_D, lw=1.4,
            solid_capstyle="round", zorder=z + 3)
    ax.add_patch(Circle((sx, sy + s * 0.10), s * 0.105, fc=SOURCE, ec=INK, lw=0.5,
                        zorder=z + 4))
    for k in (-1, 0, 1):
        fy = sy + s * (0.40 + 0.055 * k)
        ax.plot([sx - s * 0.115, sx + s * 0.115], [fy, fy], color=SOURCE_D, lw=1.0,
                solid_capstyle="round", zorder=z + 5)
    ax.add_patch(Circle((sx - s * 0.035, sy + s * 0.075), s * 0.032, fc="#ffffff",
                        ec="none", alpha=0.85, zorder=z + 5))


def mote_consumer(ax, x, y, s=0.30, z=5, **_):
    """The requesting node: the mote plus a flat display panel and radio arcs.

    The counterpart silhouette to the producer's probe, a rectangle on a short
    post, so the two separate by outline alone.  The arcs say the request
    leaves over the radio.
    """
    _board(ax, x, y, s, z)
    _antenna(ax, x + s * 0.34, y + s * 0.10, s, z)
    dx, dy = x - s * 0.24, y + s * 0.12
    ax.plot([dx, dx], [dy, dy + s * 0.20], color=RADIO, lw=1.4,
            solid_capstyle="round", zorder=z + 3)
    pw, ph = s * 0.30, s * 0.22
    ax.add_patch(FancyBboxPatch((dx - pw / 2, dy + s * 0.20), pw, ph,
                                boxstyle="round,pad=0.0,rounding_size=%f" % (s * 0.035),
                                fc=RADIO, ec=INK, lw=0.5, zorder=z + 4))
    for frac, alpha, wfrac in ((0.62, 0.90, 0.28), (0.34, 0.70, 0.10)):
        ax.plot([dx - pw * 0.28, dx + pw * wfrac],
                [dy + s * 0.20 + ph * frac, dy + s * 0.20 + ph * frac],
                color="#ffffff", lw=0.7, alpha=alpha, solid_capstyle="round",
                zorder=z + 5)
    for k in (1, 2):
        ax.add_patch(Ellipse((dx, dy + s * 0.20 + ph), s * (0.34 + 0.22 * k),
                             s * (0.16 + 0.11 * k), fc="none", ec=RADIO, lw=0.6,
                             alpha=0.58 - 0.18 * k, zorder=z + 5))


def mote_hit(ax, x, y, s=0.30, z=5, **_):
    """The node that answered: a small filled Content Store badge."""
    _board(ax, x, y, s, z)
    _antenna(ax, x + s * 0.34, y + s * 0.10, s, z)
    bx, by = x - s * 0.26, y + s * 0.28
    for k in range(3):
        iso_box(ax, bx, by + k * s * 0.105, s * 0.38, s * 0.28, s * 0.045,
                RADIO if k == 2 else "#c3d9f4", ec=RADIO, lw=0.5, z=z + 4 + k)


def battery(ax, x, y, s=0.22, z=5, level=0.45, **_):
    """A cell with a charge bar, for the energy model the swap deletes."""
    iso_box(ax, x, y, s * 0.78, s * 0.50, s * 0.34, CELL, z=z)
    # terminal nub on the top face, toward the back corner
    iso_box(ax, x + s * 0.24, y + s * 0.14, s * 0.14, s * 0.12, s * 0.05, "#8e959c", z=z + 3)
    # charge remaining, drawn on the lit left-hand side face
    x0 = x - EX[0] * s * 0.39
    y0 = y - EX[1] * s * 0.39
    for k in range(4):
        fx = x0 + EY[0] * (-s * 0.20) + EX[0] * k * s * 0.18
        fy = y0 + EY[1] * (-s * 0.20) + EX[1] * k * s * 0.18 - s * 0.17
        ax.add_patch(Polygon([(fx, fy), (fx + EX[0] * s * 0.14, fy + EX[1] * s * 0.14),
                              (fx + EX[0] * s * 0.14, fy + EX[1] * s * 0.14 - s * 0.10),
                              (fx, fy - s * 0.10)],
                             fc="#1baf7a" if k < 4 * level + 0.5 else "#e3e6e9",
                             ec="none", zorder=z + 4))


def learner(ax, x, y, s=0.34, z=5, **_):
    """The off-path learning process: a package with a small policy network."""
    iso_box(ax, x, y, s * 1.05, s * 0.78, s * 0.14, LEARN, z=z)
    cols = [[(x - s * 0.26, y + s * 0.02 + k * s * 0.10) for k in (-1, 0, 1)],
            [(x, y + s * 0.02 + k * s * 0.13) for k in (-1.3, 0, 1.3)],
            [(x + s * 0.26, y + s * 0.02 + k * s * 0.10) for k in (-0.6, 0.6)]]
    for L0, L1 in zip(cols[:-1], cols[1:]):
        for a in L0:
            for b in L1:
                ax.plot([a[0], b[0]], [a[1], b[1]], color="#ffffff", lw=0.35, alpha=0.85, zorder=z + 3)
    for layer in cols:
        for p in layer:
            ax.add_patch(Circle(p, s * 0.042, fc="#ffffff", ec=LEARN_D, lw=0.4, zorder=z + 4))


def radio_rings(ax, x, y, s=0.30, z=3, n=3, color=RADIO, **_):
    """Concentric isometric rings: one radio's reach in the shared medium."""
    for k in range(1, n + 1):
        r = s * (0.55 + 0.42 * k)
        ax.add_patch(Ellipse((x, y), r * 2, r * 1.0, fc="none", ec=color, lw=0.55,
                             alpha=0.55 - 0.12 * k, zorder=z))


ICONS = dict(mote=mote, mote_producer=mote_producer, mote_consumer=mote_consumer,
             mote_hit=mote_hit, battery=battery, learner=learner,
             radio_rings=radio_rings)
