#!/usr/bin/env python3
"""Shared readers for the age-transparency campaign.

Everything the paper reports is derived here from the recorded .sca files, so
there is one source of truth per fact.
"""
import collections, csv, gzip, math, re, statistics as st
from pathlib import Path

# The compromised sensors issue junk Interests that are always answered by the
# publisher and are never stale. They dilute their own per-consumer rates and
# distort their mean path length, so they are not ordinary consumers.
ATTACKERS = {0, 5, 30}
REGIMES = {0: "stationary", 1: "drift", 2: "pollution", 3: "flash"}
RESTAMP_BINS = 8


def sca_files(d, cfg):
    d = Path(d)
    pat = re.compile(rf"^{re.escape(cfg)}-\d+\.sca(\.gz)?$")
    return sorted(f for f in d.iterdir() if pat.match(f.name))


def read_scalars(path):
    op = gzip.open if str(path).endswith(".gz") else open
    with op(path, "rt") as f:
        for line in f:
            if line.startswith("scalar "):
                p = line.split()
                if len(p) >= 4:
                    yield p[1], p[2], p[3]


def _f(v):
    try:
        return float(v)
    except ValueError:
        return None


def restamp_table(d, cfg):
    """Pooled over runs: M -> (delivered, stale, mean age/F)."""
    acc = collections.defaultdict(lambda: [0.0, 0.0, 0.0])
    files = sca_files(d, cfg)
    for path in files:
        for mod, name, val in read_scalars(path):
            if not mod.endswith(".consumer"):
                continue
            n = re.search(r"sensor\[(\d+)\]", mod)
            if n and int(n.group(1)) in ATTACKERS:
                continue
            m = re.match(r"M(\d+):(delivered|stale|ageRatioSum)$", name)
            if not m:
                continue
            v = _f(val)
            if v is None:
                continue
            b = int(m.group(1))
            acc[b][{"delivered": 0, "stale": 1, "ageRatioSum": 2}[m.group(2)]] += v
    out = {}
    for b, (dl, stl, ags) in sorted(acc.items()):
        if dl > 0:
            out[b] = (dl, 100.0 * stl / dl, ags / dl)
    return out, len(files)


def depth_table(d, cfg, per_regime=True):
    """Topological depth -> stale% , using the BFS depth the run recorded."""
    agg = collections.defaultdict(lambda: collections.defaultdict(lambda: [0.0, 0.0]))
    files = sca_files(d, cfg)
    for path in files:
        depth, cons = {}, collections.defaultdict(lambda: collections.defaultdict(float))
        for mod, name, val in read_scalars(path):
            m = re.match(r"depthMean_n(\d+)$", name)
            if m:
                v = _f(val)
                if v is not None:
                    depth[int(m.group(1))] = v
                continue
            if not mod.endswith(".consumer"):
                continue
            n = re.search(r"sensor\[(\d+)\]", mod)
            if not n:
                continue
            ni = int(n.group(1))
            if ni in ATTACKERS:
                continue
            mm = re.match(r"R(\d+):(stale|fresh)$", name)
            if mm:
                v = _f(val)
                if v is not None:
                    cons[ni][(int(mm.group(1)), mm.group(2))] += v
        for ni, d_ in cons.items():
            if ni not in depth:
                continue
            b = int(round(depth[ni]))
            for (r, kind), v in d_.items():
                key = r if per_regime else "all"
                agg[key][b][0] += v if kind == "stale" else 0.0
                agg[key][b][1] += v
    out = {}
    for key, per_b in agg.items():
        out[key] = {b: (100.0 * s / tot, tot) for b, (s, tot) in sorted(per_b.items()) if tot > 0}
    return out, len(files)


def load_runs(csv_path):
    rows = [r for r in csv.DictReader(open(csv_path))]
    idx = collections.defaultdict(dict)
    for r in rows:
        if r["regime"] == "all":
            idx[r["config"]][r["seed"]] = r
    return rows, idx


def paired(idx, base, treat, metric):
    """-> (mean_base, mean_treat, mean_diff, t, wins, n) or None."""
    if base not in idx or treat not in idx:
        return None
    seeds = sorted(set(idx[base]) & set(idx[treat]), key=int)
    b, t_ = [], []
    for s in seeds:
        vb, vt = _f(idx[base][s].get(metric, "")), _f(idx[treat][s].get(metric, ""))
        if vb is None or vt is None:
            continue
        b.append(vb); t_.append(vt)
    if len(b) < 2:
        return None
    d = [x - y for x, y in zip(t_, b)]
    md = st.mean(d)
    sd = st.stdev(d)
    tv = md / (sd / math.sqrt(len(d))) if sd > 0 else (math.inf if md else 0.0)
    return st.mean(b), st.mean(t_), md, tv, sum(1 for x in d if x > 0), len(d)


def regime_means(rows, cfg, metric):
    out = {}
    for reg in ["stationary", "drift", "pollution", "flash", "all"]:
        vals = [_f(r.get(metric, "")) for r in rows if r["config"] == cfg and r["regime"] == reg]
        vals = [v for v in vals if v is not None]
        if vals:
            out[reg] = st.mean(vals)
    return out


# ---------------------------------------------------------------------------
# Per-run metrics computed here rather than taken from the shared collector.
# The collector divides valid cache service by Interests ISSUED while dividing
# cache service and stale delivery by Interests SATISFIED, so the three cannot
# be compared with each other. Everything below uses one denominator: the
# number of Data packets a consumer actually received. Satisfaction is the one
# per-issued quantity and is labeled as such.
#
# Consumer-side rates are taken over legitimate consumers only. The three
# compromised sensors request names that are never requested twice, so their
# traffic is always fetched from the publisher and is never stale; including it
# dilutes every delivery rate in the pollution regime without telling us
# anything about caching. Energy is summed over all nodes, since every node
# spends it.
# ---------------------------------------------------------------------------
CONS_KEYS = ("issued", "satisfied", "fromCache", "freshFromCache",
             "stale", "fresh", "latencySum", "hopSum")


def run_metrics(path, regimes=(0, 1, 2, 3), include_attackers=False):
    c = collections.defaultdict(float)
    energy_residual, nodes = 0.0, 0
    for mod, name, val in read_scalars(path):
        v = _f(val)
        if v is None:
            continue
        if name == "residualEnergyCapacity:last":
            energy_residual += v
            nodes += 1
            continue
        if not mod.endswith(".consumer"):
            continue
        n = re.search(r"sensor\[(\d+)\]", mod)
        if n and (not include_attackers) and int(n.group(1)) in ATTACKERS:
            continue
        m = re.match(r"R(\d+):(\w+)$", name)
        if m and int(m.group(1)) in regimes and m.group(2) in CONS_KEYS:
            c[m.group(2)] += v
    iss, sat = c["issued"], c["satisfied"]
    if sat <= 0:
        return None
    return {
        "issued": iss,
        "satisfied": sat,
        "satisfaction": sat / iss if iss else float("nan"),
        "cache_service": c["fromCache"] / sat,
        "valid_cache_service": c["freshFromCache"] / sat,
        "stale_delivery": c["stale"] / sat,
        "stale_cache_hits": (c["fromCache"] - c["freshFromCache"]) / sat,
        "latency_ms": 1000 * c["latencySum"] / sat,
        "hops": c["hopSum"] / sat,
        "energy_J": (50.0 * nodes - energy_residual) if nodes else float("nan"),
    }


def config_metrics(d, cfg, regimes=(0, 1, 2, 3)):
    """-> {seed: metrics} for one configuration."""
    out = {}
    for path in sca_files(d, cfg):
        m = re.search(r"-(\d+)\.sca", path.name)
        if not m:
            continue
        r = run_metrics(path, regimes)
        if r:
            out[m.group(1)] = r
    return out


def paired_runs(a, b, metric):
    """Paired comparison between two {seed: metrics} dicts."""
    seeds = sorted(set(a) & set(b), key=int)
    xs = [a[s][metric] for s in seeds if metric in a[s] and metric in b[s]]
    ys = [b[s][metric] for s in seeds if metric in a[s] and metric in b[s]]
    if len(xs) < 2:
        return None
    d = [y - x for x, y in zip(xs, ys)]
    md = st.mean(d); sd = st.stdev(d)
    tv = md / (sd / math.sqrt(len(d))) if sd > 0 else (math.inf if md else 0.0)
    return st.mean(xs), st.mean(ys), md, tv, sum(1 for x in d if x > 0), len(d)


def cs_pressure(d, cfg):
    """Insertions, evictions and the eviction rate of the Content Stores.

    An eviction rate near one says the store is full essentially all the time,
    so its capacity is a binding constraint rather than a number in a config
    file. Worth checking before reporting anything as a function of capacity.
    """
    ins = ev = 0.0
    files = sca_files(d, cfg)
    for path in files:
        for mod, name, val in read_scalars(path):
            if not mod.endswith(".cs"):
                continue
            v = _f(val)
            if v is None:
                continue
            if name == "totalInsertions":
                ins += v
            elif name == "totalEvictions":
                ev += v
    if ins <= 0:
        return None
    return {"insertions": ins, "evictions": ev, "evict_rate": ev / ins,
            "runs": len(files)}
