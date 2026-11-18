#!/usr/bin/env python3
"""Network-wide time series from the epoch vectors of ShadeCampus runs.

    python3 -m shade.timeseries ../results/main -o ../results/main/timeseries.csv \
        --configs SHADE Fixed-LCE-LRU ...

For every run it sums, per 10 s epoch, the real-store lookups, valid hits and
stale hits of all nodes, and counts how many nodes have each expert active.
Output columns: config, seed, t, lookups, valid, stale, share_E0..share_E4.
"""
import argparse, collections, csv, os, re, subprocess, tempfile
from pathlib import Path

OPP = os.environ.get("OPP_SCAVETOOL", "opp_scavetool")


def export(vec, names):
    with tempfile.NamedTemporaryFile(suffix=".csv", delete=False) as tf:
        out = tf.name
    flt = " OR ".join(f'name=~"{n}:vector"' for n in names)
    subprocess.run([OPP, "export", "-f", flt, "-F", "CSV-R", "-o", out, str(vec)],
                   check=True, capture_output=True)
    rows = list(csv.DictReader(open(out)))
    os.unlink(out)
    return rows


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dir")
    ap.add_argument("-o", "--out", required=True)
    ap.add_argument("--configs", nargs="*")
    a = ap.parse_args()
    res = []
    for vec in sorted(Path(a.dir).glob("*.vec")):
        m = re.match(r"(.+)-s?(\d+)\.vec$", vec.name)
        if not m or (a.configs and m.group(1) not in a.configs):
            continue
        cfg, seed = m.group(1), int(m.group(2))
        rows = export(vec, ["epochLookups", "epochValid", "epochStale", "activeExpert"])
        agg = collections.defaultdict(lambda: collections.defaultdict(float))
        for r in rows:
            if r["type"] != "vector" or not r["vectime"]:
                continue
            name = r["name"].split(":")[0]
            ts = [float(x) for x in r["vectime"].split()]
            vs = [float(x) for x in r["vecvalue"].split()]
            for t, v in zip(ts, vs):
                tb = round(t / 10.0) * 10.0
                if name == "activeExpert":
                    agg[tb][f"share_E{int(v)}"] += 1
                    agg[tb]["nodes"] += 1
                else:
                    agg[tb][name] += v
        if not agg:
            continue
        for t in sorted(agg):
            d = agg[t]
            n = d.get("nodes", 0) or 1
            res.append(dict(config=cfg, seed=seed, t=t, lookups=d.get("epochLookups", 0),
                            valid=d.get("epochValid", 0), stale=d.get("epochStale", 0),
                            **{f"share_E{k}": d.get(f"share_E{k}", 0) / n for k in range(5)}))
        print("ok", vec.name, flush=True)
    keys = ["config", "seed", "t", "lookups", "valid", "stale"] + [f"share_E{k}" for k in range(5)]
    with open(a.out, "w", newline="") as fh:
        w = csv.DictWriter(fh, keys)
        w.writeheader()
        w.writerows(res)


if __name__ == "__main__":
    main()
