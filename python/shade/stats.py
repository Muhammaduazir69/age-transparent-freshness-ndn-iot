#!/usr/bin/env python3
"""Paired statistics of one reference configuration against every other one.

    python3 -m shade.stats ../results/main/summary_runs.csv --ref SHADE \
        --metric valid_cache_service -o ../results/main/stats.csv

Runs are paired by seed (every configuration of a campaign uses the same
evaluation seeds, so a seed fixes the request sequences, attacker timing and
channel randomness up to the policy's own influence). For each regime and each
competitor it reports the mean paired difference with a 95% t-interval, the
two-sided Wilcoxon signed-rank p-value, the Holm-adjusted p-value within the
regime, and Cliff's delta.
"""
import argparse, collections, csv, math
import numpy as np
from scipy import stats


def cliffs_delta(a, b):
    a, b = np.asarray(a), np.asarray(b)
    gt = sum((x > b).sum() for x in a)
    lt = sum((x < b).sum() for x in a)
    return (gt - lt) / (len(a) * len(b))


def holm(pvals):
    order = np.argsort(pvals)
    m = len(pvals)
    adj = np.empty(m)
    running = 0.0
    for rank, i in enumerate(order):
        running = max(running, min(1.0, (m - rank) * pvals[i]))
        adj[i] = running
    return adj


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("runs")
    ap.add_argument("--ref", default="SHADE")
    ap.add_argument("--metric", default="valid_cache_service")
    ap.add_argument("--configs", nargs="*", default=None)
    ap.add_argument("-o", "--out", default=None)
    a = ap.parse_args()
    data = collections.defaultdict(dict)   # (cfg, regime) -> seed -> value
    for r in csv.DictReader(open(a.runs)):
        v = r.get(a.metric, "")
        if v not in ("", "nan"):
            data[(r["config"], r["regime"])][int(r["seed"])] = float(v)
    regimes = sorted({k[1] for k in data}, key=lambda x: ["stationary", "drift",
                     "pollution", "flash", "all"].index(x) if x in
                     ["stationary", "drift", "pollution", "flash", "all"] else 9)
    cfgs = a.configs or sorted({k[0] for k in data} - {a.ref})
    rows = []
    for reg in regimes:
        ref = data.get((a.ref, reg), {})
        block = []
        for c in cfgs:
            oth = data.get((c, reg), {})
            seeds = sorted(set(ref) & set(oth))
            if len(seeds) < 3:
                continue
            x = np.array([ref[s] for s in seeds])
            y = np.array([oth[s] for s in seeds])
            d = x - y
            n = len(d)
            mean = d.mean()
            ci = stats.t.ppf(0.975, n - 1) * d.std(ddof=1) / math.sqrt(n) if n > 1 else float("nan")
            try:
                p = stats.wilcoxon(x, y, zero_method="wilcox", alternative="two-sided").pvalue \
                    if np.any(d != 0) else 1.0
            except ValueError:
                p = 1.0
            block.append(dict(regime=reg, ref=a.ref, other=c, metric=a.metric, n=n,
                              ref_mean=x.mean(), other_mean=y.mean(), diff=mean, ci95=ci,
                              rel_pct=100 * mean / y.mean() if y.mean() else float("nan"),
                              p=p, cliffs_delta=cliffs_delta(x, y),
                              wins=int((d > 0).sum())))
        if block:
            adj = holm(np.array([b["p"] for b in block]))
            for b, q in zip(block, adj):
                b["p_holm"] = q
            rows += block
    keys = ["regime", "ref", "other", "metric", "n", "ref_mean", "other_mean", "diff",
            "ci95", "rel_pct", "p", "p_holm", "cliffs_delta", "wins"]
    if a.out:
        with open(a.out, "w", newline="") as fh:
            w = csv.DictWriter(fh, keys)
            w.writeheader()
            w.writerows(rows)
    for r in rows:
        print(f"{r['regime']:10s} {r['other']:18s} n={r['n']:2d} ref={r['ref_mean']:.4f} "
              f"oth={r['other_mean']:.4f} d={r['diff']:+.4f}±{r['ci95']:.4f} "
              f"({r['rel_pct']:+.1f}%) p={r['p']:.2g} holm={r['p_holm']:.2g} "
              f"delta={r['cliffs_delta']:+.2f} wins={r['wins']}/{r['n']}")


if __name__ == "__main__":
    main()
