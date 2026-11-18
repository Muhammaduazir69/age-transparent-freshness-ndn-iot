"""Collect OMNeT++ .sca results into a tidy pandas DataFrame.

Parses scalar files directly (no scavetool dependency) and computes the
thesis's per-run network-level metrics (3.9):
  cacheHitRatio, freshnessSatisfactionRatio, staleDeliveryRate,
  meanRetrievalLatency (s), meanHopCount, meanContentAge (s),
  satisfactionRatio, energyConsumedJ (battery delta), energyPerDeliveryJ,
  energyDelayProduct, coordBytes, cacheEnergyJ

Usage:
  python -m maaco.analysis.collect ../results/eval -o ../results/eval/summary.csv
"""

import argparse
import math
import pathlib
import re
import sys

import pandas as pd

SCALAR_RE = re.compile(r'^scalar (\S+) (\S+) ([-+0-9.eEinfna]+)\s*$')
STAT_FIELD_RE = re.compile(r'^field (\S+) ([-+0-9.eEinfna]+)\s*$')
STAT_RE = re.compile(r'^statistic (\S+) (\S+)\s*$')
FILE_RE = re.compile(r'(?P<config>.+?)-(?:s|ep)(?P<seed>\d+)\.sca$')


def parse_sca(path: pathlib.Path):
    scalars = []   # (module, name, value)
    stats = {}     # (module, statname) -> {field: value}
    cur = None
    with open(path) as f:
        for line in f:
            m = SCALAR_RE.match(line)
            if m:
                try:
                    scalars.append((m.group(1), m.group(2), float(m.group(3))))
                except ValueError:
                    pass
                cur = None
                continue
            m = STAT_RE.match(line)
            if m:
                cur = (m.group(1), m.group(2))
                stats[cur] = {}
                continue
            m = STAT_FIELD_RE.match(line)
            if m and cur is not None:
                try:
                    stats[cur][m.group(1)] = float(m.group(2))
                except ValueError:
                    pass
    return scalars, stats


def summarize_run(path: pathlib.Path) -> dict:
    scalars, stats = parse_sca(path)
    acc = {}

    def add(key, v):
        acc.setdefault(key, []).append(v)

    for mod, name, v in scalars:
        if name == "totalHits": add("hits", v)
        elif name == "totalMisses": add("misses", v)
        elif name == "freshDeliveries": add("fresh", v)
        elif name == "staleDeliveries": add("stale", v)
        elif name == "interestsIssued": add("interests", v)
        elif name == "dataReceived" and "consumer" in mod: add("delivered", v)
        elif name == "servedFromCacheCount": add("servedFromCache", v)
        elif name == "coordBytesSent": add("coordBytes", v)
        elif name == "cacheEnergyTotalJ": add("cacheEnergyJ", v)
        elif name == "redundancyVetoes": add("redundancyVetoes", v)
        elif name == "admissionsOffered": add("admissionsOffered", v)
        elif name == "admissionsAccepted": add("admissionsAccepted", v)
        elif name == "energyGuardVetoes": add("energyGuardVetoes", v)
        elif name == "totalInsertions" and mod.endswith(".cs"): add("cacheInsertions", v)
        elif name == "zeroHitEvictions": add("zeroHitEvictions", v)
        elif name == "evictedCount": add("evictedCount", v)
        elif name == "graceProtections": add("graceProtections", v)
        elif name == "admitVetoes": add("admitVetoes", v)
        elif name == "coordBytesReceived": add("coordBytesReceived", v)
        elif name == "rlFallbacks": acc["rlFallbacks"] = [v]
        elif name.startswith("residualEnergyCapacity"): add("residualJ", v)
        elif name == "rlDegraded": acc["rlDegraded"] = [v]
        # --- corrections campaign (2026-08-13) ---
        elif name == "totalAggregated": add("pitAggregated", v)
        elif name == "peakPITSize": add("peakPIT", v)
        elif name == "pitFullEvents": add("pitFullEvents", v)
        elif name == "expiredFirstEvictions": add("expiredFirstEvictions", v)
        elif name == "csLookupsDisabled": add("csLookupsDisabled", v)
        elif name == "inferenceCount": add("inferenceCount", v)
        elif name == "inferenceEnergyJ": add("inferenceEnergyJ", v)
        elif name == "inferenceMeanDelayS": add("inferenceDelay", v)
        elif name == "inferenceEnergyDrawnJ": add("inferenceEnergyDrawnJ", v)
        elif name == "ctrlDecisionBytes": acc["ctrlDecisionBytes"] = [v]
        elif name == "ctrlActionBytes": acc["ctrlActionBytes"] = [v]
        elif name == "ctrlRewardBytes": acc["ctrlRewardBytes"] = [v]
        elif name == "ctrlTotalBytes": acc["ctrlTotalBytes"] = [v]

    lat_mean_w, lat_n = 0.0, 0.0
    hop_mean_w, hop_n = 0.0, 0.0
    age_mean_w, age_n = 0.0, 0.0
    for (mod, sname), f in stats.items():
        n, mean = f.get("count", 0), f.get("mean", float("nan"))
        if n <= 0 or mean != mean:
            continue
        if sname.startswith("retrievalLatency"):
            lat_mean_w += mean * n; lat_n += n
        elif sname.startswith("dataHopCount"):
            hop_mean_w += mean * n; hop_n += n
        elif sname.startswith("contentAge"):
            age_mean_w += mean * n; age_n += n

    hits = sum(acc.get("hits", [0]))
    misses = sum(acc.get("misses", [0]))
    fresh = sum(acc.get("fresh", [0]))
    stale = sum(acc.get("stale", [0]))
    interests = sum(acc.get("interests", [0]))
    delivered = sum(acc.get("delivered", [0]))
    residual = acc.get("residualJ", [])

    # battery energy consumed = sum(nominal - residual); nominal from config (50 J default)
    # more robust: report mean residual; consumed derived downstream if needed
    row = {
        "cacheHitRatio": hits / max(1.0, hits + misses),
        "freshnessSatisfactionRatio": fresh / max(1.0, fresh + stale),
        "staleDeliveryRate": stale / max(1.0, fresh + stale),
        "satisfactionRatio": delivered / max(1.0, interests),
        "meanRetrievalLatency": lat_mean_w / lat_n if lat_n else float("nan"),
        "meanHopCount": hop_mean_w / hop_n if hop_n else float("nan"),
        "meanContentAge": age_mean_w / age_n if age_n else float("nan"),
        "meanResidualJ": sum(residual) / len(residual) if residual else float("nan"),
        "deliveredCount": delivered,
        "coordBytes": sum(acc.get("coordBytes", [0])),
        "cacheEnergyJ": sum(acc.get("cacheEnergyJ", [0])),
        "redundancyVetoes": sum(acc.get("redundancyVetoes", [0])),
        "interestsIssued": interests,
    }
    offered = sum(acc.get("admissionsOffered", [0]))
    accepted = sum(acc.get("admissionsAccepted", [0]))
    ins = sum(acc.get("cacheInsertions", [0]))
    ev = sum(acc.get("evictedCount", [0]))
    row.update({
        # interest-level view of caching (the headline cacheHitRatio is
        # LOOKUP-weighted: a 4-hop miss registers 4 misses but a 1-hop hit
        # registers 1 hit, so it understates end-to-end caching benefit)
        "consumerCacheHitRatio": (sum(acc.get("servedFromCache", [0])) /
                                  delivered if delivered else float("nan")),
        "admissionsOffered": offered,
        "admissionsAccepted": accepted,
        "admissionRate": accepted / offered if offered else float("nan"),
        "cacheInsertions": ins,
        "hitsPerInsertion": hits / ins if ins else float("nan"),
        "zeroHitEvictions": sum(acc.get("zeroHitEvictions", [0])),
        "zeroHitEvictionRate": (sum(acc.get("zeroHitEvictions", [0])) / ev
                                if ev else float("nan")),
        "graceProtections": sum(acc.get("graceProtections", [0])),
        "admitVetoes": sum(acc.get("admitVetoes", [0])),
        "energyGuardVetoes": sum(acc.get("energyGuardVetoes", [0])),
        "coordBytesReceived": sum(acc.get("coordBytesReceived", [0])),
        "rlDegraded": acc.get("rlDegraded", [float("nan")])[0],
        "rlFallbacks": acc.get("rlFallbacks", [float("nan")])[0],
    })

    # --- corrections campaign additions ---
    aggregated = sum(acc.get("pitAggregated", [0]))
    nodes = len(acc.get("residualJ", [])) or 1
    residual_sum = sum(acc.get("residualJ", []))
    row.update({
        # Corrected cache hit ratio. The Content Store is queried BEFORE the
        # PIT (correct NDN order), so an Interest that will be aggregated onto
        # an existing PIT entry still books a full CS miss even though it
        # generates no forwarded traffic and no fetch. Those lookups are
        # removed from the denominator here. The interest-level ratio is
        # unaffected by this and is the cross-check.
        "pitAggregated": aggregated,
        "cacheHitRatioCorrected": (
            hits / max(1.0, hits + misses - aggregated)),
        "peakPITSize": max(acc.get("peakPIT", [0])) if acc.get("peakPIT") else float("nan"),
        "pitFullEvents": sum(acc.get("pitFullEvents", [0])),
        "expiredFirstEvictions": sum(acc.get("expiredFirstEvictions", [0])),
        "csLookupsDisabled": sum(acc.get("csLookupsDisabled", [0])),
        # energy, reported per policy instead of only asserted
        "energyConsumedJ": (nominal_capacity(acc) - residual_sum
                            if residual_sum else float("nan")),
        "meanEnergyConsumedPerNodeJ": (
            (nominal_capacity(acc) - residual_sum) / nodes
            if residual_sum else float("nan")),
        "nodesWithBattery": nodes if residual_sum else 0,
        "energyPerDeliveryJ": ((nominal_capacity(acc) - residual_sum) / delivered
                               if residual_sum and delivered else float("nan")),
        # inference cost (item i)
        "inferenceCount": sum(acc.get("inferenceCount", [0])),
        "inferenceEnergyJ": sum(acc.get("inferenceEnergyJ", [0])),
        "inferenceMeanDelayS": (sum(acc.get("inferenceDelay", [0]))
                                / len(acc["inferenceDelay"])
                                if acc.get("inferenceDelay") else float("nan")),
        # learning control plane (item j)
        "ctrlDecisionBytes": acc.get("ctrlDecisionBytes", [0])[0],
        "ctrlActionBytes": acc.get("ctrlActionBytes", [0])[0],
        "ctrlRewardBytes": acc.get("ctrlRewardBytes", [0])[0],
        "ctrlTotalBytes": acc.get("ctrlTotalBytes", [0])[0],
    })
    return row


# Nominal battery capacity per node, inferred from the run: SmartFarm uses
# 50 J, the energy-binding variant 6 J, the idealized substrate an ideal
# (infinite) storage. Residual is recorded per node; the nominal is not, so it
# is recovered as the smallest standard capacity that bounds every residual.
_NOMINALS = (3.0, 6.0, 8.0, 50.0, 100.0)


def nominal_capacity(acc):
    res = acc.get("residualJ", [])
    if not res:
        return 0.0
    hi = max(res)
    # IdealEpEnergyStorage (the idealized substrate) reports an unbounded
    # residual: there is no battery to draw down, so there is no energy number
    # to report and NaN is the honest answer.
    if not math.isfinite(hi) or hi > 1e6 or hi <= 0 or min(res) < 0:
        # IdealEpEnergyStorage (the idealized substrate) has no battery to draw
        # down and reports meaningless residuals; there is no energy number to
        # report and NaN is the honest answer.
        return float("nan")
    for n in _NOMINALS:
        if hi <= n + 1e-9:
            return n * len(res)
    return hi * len(res)


def collect(result_dir: str) -> pd.DataFrame:
    rows = []
    for path in sorted(pathlib.Path(result_dir).glob("*.sca")):
        m = FILE_RE.match(path.name)
        if not m:
            continue
        row = summarize_run(path)
        if row["interestsIssued"] <= 0:
            # truncated/crashed run (no application scalars recorded):
            # skip it loudly rather than silently imputing zero metrics
            print(f"WARNING: skipping {path.name}: no application scalars "
                  f"(truncated or failed run)", file=sys.stderr)
            continue
        row["config"] = m.group("config")
        row["seed"] = int(m.group("seed"))
        rows.append(row)
    return pd.DataFrame(rows)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("result_dir")
    ap.add_argument("-o", "--output", default=None)
    args = ap.parse_args()

    df = collect(args.result_dir)
    if df.empty:
        print("no .sca files matched", file=sys.stderr)
        sys.exit(1)

    out = args.output or str(pathlib.Path(args.result_dir) / "summary.csv")
    df.to_csv(out, index=False)

    g = df.groupby("config").agg(
        runs=("seed", "count"),
        hit=("cacheHitRatio", "mean"),
        fresh=("freshnessSatisfactionRatio", "mean"),
        stale=("staleDeliveryRate", "mean"),
        lat_ms=("meanRetrievalLatency", lambda s: 1000 * s.mean()),
        hops=("meanHopCount", "mean"),
    ).round(4)
    print(g.to_string())
    print(f"\nwritten: {out} ({len(df)} runs)")


if __name__ == "__main__":
    main()
