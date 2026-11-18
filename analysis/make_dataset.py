#!/usr/bin/env python3
"""Export the dataset the manuscript is computed from.

The raw OMNeT++ scalar files are 5.4 GB, which is not a useful thing to hand a
reader. What a reader needs in order to check or reuse the results is the
per-run metrics behind every table and figure, on the denominators the paper
defines, plus the two breakdowns the mechanism section rests on. Those are a
few megabytes and they regenerate every reported number.

Usage: make_dataset.py <results_root> <out_dir>
"""
import csv
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C

CAMPAIGNS = {
    "main": "v2_main",
    "field": "v2_field",
    "cap5": "v2_cap5",
    "cap20": "v2_cap20",
}
METRICS = ["satisfaction", "cache_service", "valid_cache_service",
           "stale_delivery", "stale_cache_hits", "latency_ms", "energy_J"]


def configs_in(d):
    out = set()
    for p in d.glob("*.sca"):
        m = re.match(r"(.+)-\d+\.sca$", p.name)
        if m:
            out.add(m.group(1))
    return sorted(out)


def main(root, outdir):
    root, outdir = Path(root), Path(outdir)
    outdir.mkdir(parents=True, exist_ok=True)

    # 1. per-run metrics: the table every number in the paper is reduced from
    rows = 0
    with open(outdir / "per_run_metrics.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["campaign", "config", "seed"] + METRICS + ["satisfied", "issued"])
        for camp, sub in CAMPAIGNS.items():
            d = root / sub
            if not d.is_dir():
                continue
            for cfg in configs_in(d):
                m = C.config_metrics(d, cfg)
                for seed in sorted(m, key=int):
                    v = m[seed]
                    w.writerow([camp, cfg, seed]
                               + [f"{v[k]:.6g}" for k in METRICS]
                               + [f"{v.get('satisfied', ''):.6g}" if v.get('satisfied') else "",
                                  f"{v.get('issued', ''):.6g}" if v.get('issued') else ""])
                    rows += 1
    print(f"per_run_metrics.csv: {rows} runs")

    # 2. staleness by hand-off count, which is the paper's central mechanism
    n = 0
    with open(outdir / "handoff_breakdown.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["config", "handoffs_M", "delivered", "stale_pct", "mean_age_over_F",
                    "note"])
        for cfg in ("Fixed-LCE-LRU", "AT-LCE-LRU", "ATD-LCE-LRU"):
            t, _ = C.restamp_table(root / "v2_main", cfg)
            top = max(t) if t else None
            for mm in sorted(t):
                note = "top bucket is M or more" if mm == top else ""
                w.writerow([cfg, mm, f"{t[mm][0]:.0f}", f"{t[mm][1]:.4f}",
                            f"{t[mm][2]:.4f}", note])
                n += 1
    print(f"handoff_breakdown.csv: {n} rows")

    # 3. staleness by hop distance and regime
    n = 0
    with open(outdir / "depth_breakdown.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["config", "regime", "hops", "delivered", "stale_pct"])
        for cfg in ("Fixed-LCE-LRU", "AT-LCE-LRU", "Fixed-LCD-LRU"):
            t = C.depth_table(root / "v2_main", cfg)
            if isinstance(t, tuple):
                t = t[0]
            for reg in sorted(t):
                for h in sorted(t[reg]):
                    pct, cnt = t[reg][h]
                    w.writerow([cfg, reg, h, f"{cnt:.0f}", f"{pct:.4f}"])
                    n += 1
    print(f"depth_breakdown.csv: {n} rows")

    # 4. a machine-readable description, so the files are self-explaining
    (outdir / "README.md").write_text("""# Dataset: age-transparent freshness in NDN-based IoT networks

Every number in the manuscript is reduced from these files.

## per_run_metrics.csv
One row per simulation run. `campaign` is `main` (the single-sink field),
`field` (the distributed field), or `cap5` / `cap20` (the capacity grid).
`config` is the OMNeT++ configuration name; `seed` is the RNG seed, which also
fixes the workload, so a configuration and its age-transparent counterpart at
the same seed see exactly the same Interests.

All delivery-side rates share one denominator, the Data packets a consumer
received. `satisfaction` is the exception and is per Interest issued.
Compromised nodes (0, 5, 30) are excluded from every consumer-side rate.

| column | meaning |
|---|---|
| `cache_service` | share of deliveries answered by a Content Store |
| `valid_cache_service` | share answered by a Content Store and still inside the publisher's lifetime |
| `stale_delivery` | share delivered past the publisher's lifetime |
| `stale_cache_hits` | `cache_service` minus `valid_cache_service` |
| `latency_ms` | mean Interest-to-Data interval, over satisfied Interests |
| `energy_J` | field-wide radio energy drawn, 36 nodes |

## handoff_breakdown.csv
Deliveries grouped by how many Content Stores answered with the copy before it
arrived. This is the paper's independent variable. The counter saturates, so
the highest bucket means "that many hand-offs or more" and is flagged.

## depth_breakdown.csv
Deliveries grouped by the consumer's hop distance from the publisher, measured
from the forwarding state actually installed, and split by operating regime
(0 stationary, 1 drift, 2 pollution, 3 flash).

## Reproducing
`figures_src/regen.sh <results_dir>` recomputes every macro, table and figure
from the raw scalar files and rebuilds the manuscript. These CSVs are the same
reduction, exported for readers who do not want to run the simulator.
""")
    print("README.md written")


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
