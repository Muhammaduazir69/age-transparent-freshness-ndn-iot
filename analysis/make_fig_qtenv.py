#!/usr/bin/env python3
"""Figure: the scenario as the simulator actually runs it.

Two Qtenv captures of the running model. The point of including them is
evidence rather than decoration: a reader can see that the field is the one
the setup section describes, that every node is an INET SensorNode on a
battery, and that the NDN forwarder is a real one with its own PIT, FIB and
Content Store rather than an abstraction. Provenance, including the binary
md5 and the zoom, is pinned in capture/PROVENANCE.
"""
import sys
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

HERE = Path(__file__).resolve().parent
CAP = HERE / "capture"
INK = "#1a1a1a"


def trim(img, box):
    return img[box[1]:box[3], box[0]:box[2]]


def build(out):
    plt.rcParams.update({
        "font.family": "sans-serif",
        "font.sans-serif": ["Nimbus Sans", "Arial", "Helvetica", "DejaVu Sans"],
        "pdf.fonttype": 42, "ps.fonttype": 42,
    })
    field = mpimg.imread(CAP / "qtenv_field_raw.png")
    ndn = mpimg.imread(CAP / "qtenv_ndn_raw.png")

    # crop away the dead canvas; keep the status line, it carries the evidence
    field = trim(field, (10, 120, 1800, 1700))
    ndn = trim(ndn, (10, 120, 1500, 900))

    fh, fw = field.shape[0], field.shape[1]
    nh, nw = ndn.shape[0], ndn.shape[1]
    scale = fw / nw
    total_h = fh + nh * scale
    fig_w = 6.6
    fig_h = fig_w * total_h / fw + 0.30

    fig = plt.figure(figsize=(fig_w, fig_h))
    top_frac = fh / total_h
    pad = 0.30 / fig_h

    ax1 = fig.add_axes([0, (1 - pad) * (1 - top_frac) + pad, 1, (1 - pad) * top_frac])
    ax1.imshow(field); ax1.axis("off")
    ax1.set_title("(a) the 36-node field: every node an INET SensorNode on a 50 J cell, "
                  "sensor[35] publishing /campus/env",
                  fontsize=7.0, color=INK, pad=3, loc="left")

    ax2 = fig.add_axes([0, pad, 1, (1 - pad) * (1 - top_frac) - pad])
    ax2.imshow(ndn); ax2.axis("off")
    ax2.set_title("(b) inside one node's forwarder: the Content Store this paper changes, "
                  "beside an unmodified PIT and FIB",
                  fontsize=7.0, color=INK, pad=3, loc="left")

    fig.savefig(out, format="pdf", dpi=300, bbox_inches="tight")
    fig.savefig(str(out).replace(".pdf", ".png"), dpi=200, bbox_inches="tight")
    print("wrote", out)


if __name__ == "__main__":
    out = Path(sys.argv[1] if len(sys.argv) > 1
               else HERE.parent / "figures" / "fig_qtenv.pdf")
    out.parent.mkdir(parents=True, exist_ok=True)
    build(out)
