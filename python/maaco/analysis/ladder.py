#!/usr/bin/env python3
"""Substrate ladder: separate the contention effect from the energy effect.

The published contrast moves two things at once, an idealized link and a
removed energy model, so a shift between them cannot be attributed to either.
Four rungs make it a 2x2:

    rung 1  I    ideal radio,     no energy     (published)
    rung 2  IE   ideal radio,     energy ON     (new)
    rung 3  BN   802.15.4 radio,  energy OFF    (new)
    rung 4  B    802.15.4 radio,  energy ON     (published)

Then, for any metric:
    contention effect  = (rung 4 - rung 2)   holding energy ON
    energy effect      = (rung 4 - rung 3)   holding the radio real
    interaction        = (4 - 2) - (3 - 1)

Reads rungs 1 and 4 from results/reviewer/ALL_RUNS.csv and rungs 2 and 3 from
results/ladder/, so the new runs are never mixed with the published ones in a
single mean.
"""
import csv
import math
import pathlib
import statistics as st
import sys

HERE = pathlib.Path(__file__).resolve().parents[3]
ALL_RUNS = HERE / "results" / "reviewer" / "ALL_RUNS.csv"
LADDER = HERE / "results" / "ladder"

POLICIES = ["LCE-LRU", "LCD-LRU", "Prob-LRU", "ProbTuned-LRU", "LCE-LFU",
            "LCE-LFUA", "LCE-LFF", "LCE-LFFO", "LCE-FIFO", "Fresh-LRU"]

_T = {2: 12.71, 3: 4.303, 4: 3.182, 5: 2.776, 6: 2.571, 7: 2.447, 8: 2.365,
      9: 2.306, 10: 2.262}


def ci95(v):
    if len(v) < 2:
        return 0.0
    return _T.get(len(v), 2.262) * st.stdev(v) / math.sqrt(len(v))


def from_all_runs(metric):
    out = {}
    if not ALL_RUNS.exists():
        return out
    for r in csv.DictReader(open(ALL_RUNS)):
        cfg = r.get("config", "")
        if "~" in cfg or cfg.endswith(("-Legacy", "-NoSweep")):
            continue
        try:
            val = float(r[metric])
        except (KeyError, ValueError, TypeError):
            continue
        out.setdefault(cfg, []).append(val)
    return out


def from_sca(metric):
    """Per-seed metric from the new ladder runs, via the project collector."""
    sys.path.insert(0, str(HERE / "python"))
    from maaco.analysis.collect import collect
    df = collect(str(LADDER))
    out = {}
    if df is None or len(df) == 0:
        return out
    for _, row in df.iterrows():
        if metric in row and row[metric] == row[metric]:
            out.setdefault(row["config"], []).append(float(row[metric]))
    return out


def main(metric="cacheHitRatio"):
    pub = from_all_runs(metric)
    new = from_sca(metric)
    rung = {"I": pub, "B": pub, "IE": new, "BN": new}

    print(f"\nSUBSTRATE LADDER   metric = {metric}\n")
    print(f"{'policy':<15}{'1 ideal':>12}{'2 ideal+E':>12}"
          f"{'3 real-E':>12}{'4 real':>12}   {'contention':>11}{'energy':>9}")
    print("-" * 86)
    rows = []
    for p in POLICIES:
        vals, cis = {}, {}
        ok = True
        for tag in ("I", "IE", "BN", "B"):
            v = rung[tag].get(f"SmartFarm-{tag}-{p}", [])
            if not v:
                ok = False
                break
            vals[tag], cis[tag] = st.mean(v), ci95(v)
        if not ok:
            print(f"{p:<15}{'(incomplete)':>12}")
            continue
        cont = (vals["B"] - vals["IE"]) / vals["IE"] * 100
        ener = (vals["B"] - vals["BN"]) / vals["BN"] * 100
        rows.append((p, vals, cont, ener))
        print(f"{p:<15}{vals['I']:>12.4f}{vals['IE']:>12.4f}"
              f"{vals['BN']:>12.4f}{vals['B']:>12.4f}   "
              f"{cont:>10.2f}%{ener:>8.2f}%")
    if rows:
        print("-" * 86)
        mc = st.mean([abs(r[2]) for r in rows])
        me = st.mean([abs(r[3]) for r in rows])
        print(f"{'mean |effect|':<15}{'':>48}   {mc:>10.2f}%{me:>8.2f}%")
        print(f"\nContention moves the classical field {mc:.2f}% on average; "
              f"removing the energy model moves it {me:.2f}%.")
    return rows


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else "cacheHitRatio")
