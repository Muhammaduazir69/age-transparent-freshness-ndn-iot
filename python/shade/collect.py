#!/usr/bin/env python3
"""Parse ShadeCampus .sca files into per-run, per-regime metrics.

    python3 -m shade.collect ../results/main -o ../results/main/summary

Writes <o>_runs.csv (one row per run x regime, regime "all" = whole run) and
<o>_summary.csv (mean, standard deviation and 95% t-interval per config x
regime x metric). Every number in the paper comes from these two files.
"""
import argparse, collections, csv, math, re, sys
from pathlib import Path

REGIMES = ["stationary", "drift", "pollution", "flash"]
REV_REGIMES = ["flash", "pollution", "drift", "stationary"]   # configs named Rev-*
T95 = {1: 12.706, 2: 4.303, 3: 3.182, 4: 2.776, 5: 2.571, 6: 2.447, 7: 2.365,
       8: 2.306, 9: 2.262, 10: 2.228, 11: 2.201, 12: 2.179, 14: 2.145,
       19: 2.093, 29: 2.045}

def t95(df):
    if df <= 0:
        return float("nan")
    keys = sorted(T95)
    for k in keys:
        if df <= k:
            return T95[k]
    return 1.96

def parse_sca(path):
    cons = collections.defaultdict(float)
    proc = collections.defaultdict(float)
    expert = collections.defaultdict(float)
    misc = collections.defaultdict(float)
    nproc = 0
    for line in open(path):
        if not line.startswith("scalar "):
            continue
        parts = line.split()
        if len(parts) < 4:
            continue
        mod, name, val = parts[1], parts[2], parts[3]
        try:
            v = float(val)
        except ValueError:
            continue
        if mod.endswith(".consumer"):
            if re.match(r"R\d+:", name):
                cons[name] += v
            elif name == "junkIssued":
                misc["junk"] += v
        elif mod.endswith(".ndn.processor"):
            m = re.match(r"R(\d+):E(\d+):(\w+)", name)
            if m:
                expert[(int(m.group(1)), int(m.group(2)), m.group(3))] += v
            elif re.match(r"R\d+:", name):
                proc[name] += v
            elif name in ("shadeSwitches", "shadePhAlarms", "ghostOperations",
                          "shadeEpochs", "admissionsOffered", "admissionsAccepted"):
                misc[name] += v
                if name == "shadeEpochs" and v > 0:
                    nproc += 1
        elif mod.endswith(".producer") and name == "interestsServedTotal":
            misc["producerServed"] += v
        elif name == "residualEnergyCapacity:last":
            misc["residualJ"] += v
            misc["nodes"] += 1
    misc["routers"] = nproc
    return cons, proc, expert, misc

def metrics(cons, proc, expert, misc, regimes, names=REGIMES):
    rows = {}
    def block(keys):
        g = lambda k: sum(cons.get(f"R{r}:{k}", 0.0) for r in keys)
        p = lambda k: sum(proc.get(f"R{r}:{k}", 0.0) for r in keys)
        iss, sat = g("issued"), g("satisfied")
        d = {
            "issued": iss,
            "satisfaction": sat / iss if iss else float("nan"),
            "valid_cache_service": g("freshFromCache") / iss if iss else float("nan"),
            "cache_service": g("fromCache") / sat if sat else float("nan"),
            "stale_delivery": g("stale") / sat if sat else float("nan"),
            "latency_ms": 1000 * g("latencySum") / sat if sat else float("nan"),
            "hops": g("hopSum") / sat if sat else float("nan"),
            "origin_fetch": (sat - g("fromCache")) / iss if iss else float("nan"),
            "router_valid_hit": p("validHits") / p("lookups") if p("lookups") else float("nan"),
            "router_stale_hit": p("staleHits") / p("lookups") if p("lookups") else float("nan"),
        }
        # time share of each expert as the active policy (router-epochs)
        tot = sum(v for (r, k, n), v in expert.items() if n == "activeEpochs" and r in keys)
        for k in range(8):
            a = sum(v for (r, kk, n), v in expert.items()
                    if n == "activeEpochs" and r in keys and kk == k)
            if tot:
                d[f"share_E{k}"] = a / tot
        # ghost-estimated valid hit ratio of each expert (counterfactual)
        if p("lookups"):
            for k in range(8):
                gv = sum(v for (r, kk, n), v in expert.items()
                         if n == "ghostValid" and r in keys and kk == k)
                if any(kk == k for (_, kk, _) in expert):
                    d[f"ghost_valid_E{k}"] = gv / p("lookups")
        return d
    for i, name in enumerate(names[:regimes]):
        rows[name] = block([i])
    rows["all"] = block(list(range(regimes)))
    rows["all"]["switches_per_router"] = misc["shadeSwitches"] / max(1, misc["routers"])
    rows["all"]["ph_alarms_per_router"] = misc["shadePhAlarms"] / max(1, misc["routers"])
    rows["all"]["energy_J"] = 50.0 * misc["nodes"] - misc["residualJ"]
    rows["all"]["ghost_ops"] = misc["ghostOperations"]
    rows["all"]["admit_rate"] = (misc["admissionsAccepted"] / misc["admissionsOffered"]
                                 if misc["admissionsOffered"] else float("nan"))
    rows["all"]["junk"] = misc["junk"]
    return rows

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("dirs", nargs="+")
    ap.add_argument("-o", "--out", required=True)
    a = ap.parse_args()
    runs = []
    for d in a.dirs:
        for f in sorted(Path(d).glob("*.sca")):
            m = re.match(r"(.+)-s?(\d+)\.sca$", f.name)
            if not m:
                continue
            cfg, seed = m.group(1), int(m.group(2))
            # a run that is still writing (or died) lacks the consumers' scalars
            if sum(1 for ln in open(f) if " R3:issued " in ln) < 36:
                print(f"skip incomplete {f.name}", file=sys.stderr)
                continue
            c, p, e, misc = parse_sca(f)
            nreg = len({int(k.split(":")[0][1:]) for k in c})
            names = REV_REGIMES if cfg.startswith("Rev-") else REGIMES
            for reg, d_ in metrics(c, p, e, misc, nreg or 4, names).items():
                runs.append({"config": cfg, "seed": seed, "regime": reg, **d_})
    if not runs:
        sys.exit("no .sca files found")
    keys = sorted({k for r in runs for k in r} - {"config", "seed", "regime"})
    with open(f"{a.out}_runs.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, ["config", "seed", "regime"] + keys)
        w.writeheader()
        w.writerows(runs)
    groups = collections.defaultdict(list)
    for r in runs:
        groups[(r["config"], r["regime"])].append(r)
    with open(f"{a.out}_summary.csv", "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["config", "regime", "metric", "n", "mean", "sd", "ci95"])
        for (cfg, reg), rs in sorted(groups.items()):
            for k in keys:
                xs = [r[k] for r in rs if k in r and r[k] == r[k]]
                if not xs:
                    continue
                n = len(xs)
                mu = sum(xs) / n
                sd = math.sqrt(sum((x - mu) ** 2 for x in xs) / (n - 1)) if n > 1 else 0.0
                ci = t95(n - 1) * sd / math.sqrt(n) if n > 1 else float("nan")
                w.writerow([cfg, reg, k, n, f"{mu:.6g}", f"{sd:.6g}", f"{ci:.6g}"])
    print(f"{len(runs)} rows -> {a.out}_runs.csv / {a.out}_summary.csv")

if __name__ == "__main__":
    main()
