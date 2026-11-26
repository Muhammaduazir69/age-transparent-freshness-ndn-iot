#!/usr/bin/env python3
"""Significance of the paired comparisons, with a family-wise correction.

Each policy pair is one hypothesis: does switching the rule on change the
metric? Twenty seeds is small enough that a distribution-free test is the
honest default, so we report the exact Wilcoxon signed-rank test on the paired
differences. Across the policy family the p-values are corrected by Holm's
step-down procedure, which controls the family-wise error rate without
assuming the tests are independent.

    python3 make_stats.py <results_dir>
"""
import sys
from pathlib import Path
from scipy.stats import wilcoxon
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C
from make_results_figs import POLICY_PAIRS

OUT = Path(__file__).resolve().parents[1]
METRICS = [("stale_delivery", "stale deliveries"),
           ("valid_cache_service", "valid cache service")]


def holm(pvals):
    """Holm step-down adjusted p-values, order preserved."""
    idx = sorted(range(len(pvals)), key=lambda i: pvals[i])
    m = len(pvals)
    adj = [0.0] * m
    running = 0.0
    for rank, i in enumerate(idx):
        val = (m - rank) * pvals[i]
        running = max(running, val)
        adj[i] = min(1.0, running)
    return adj


def _walsh(d):
    """Every pairwise average, including each value with itself."""
    n = len(d)
    return sorted((d[i] + d[j]) / 2.0 for i in range(n) for j in range(i, n))


def _signrank_cdf(n):
    """Exact null distribution of W+ over n non-zero pairs.

    counts[s] is the number of subsets of {1..n} summing to s, so the CDF is
    exact rather than normal-approximated. n is 20 here, so this is cheap.
    """
    M = n * (n + 1) // 2
    counts = [0] * (M + 1)
    counts[0] = 1
    for k in range(1, n + 1):
        for s in range(M, k - 1, -1):
            counts[s] += counts[s - k]
    total = float(2 ** n)
    cdf, run = [0.0] * (M + 1), 0
    for s in range(M + 1):
        run += counts[s]
        cdf[s] = run / total
    return cdf


def hodges_lehmann(diff, alpha=0.05):
    """Median paired difference and its distribution-free confidence interval.

    A signed-rank p value says a shift exists; it does not say how big it is.
    The estimator that belongs with that test is Hodges-Lehmann, the median of
    the Walsh averages, and its interval is read off the same exact null
    distribution rather than assuming normality (Hollander and Wolfe).
    Returns (estimate, low, high), or (estimate, None, None) when the sample
    is too small for a two-sided interval at this level.
    """
    d = [v for v in diff if v != 0.0]
    n = len(d)
    if n == 0:
        return 0.0, 0.0, 0.0
    w = _walsh(d)
    M = len(w)
    est = w[M // 2] if M % 2 else 0.5 * (w[M // 2 - 1] + w[M // 2])
    cdf = _signrank_cdf(n)
    c = -1
    for sv in range(len(cdf)):
        if cdf[sv] <= alpha / 2.0:
            c = sv
        else:
            break
    if c < 0 or 2 * (c + 1) >= M:
        return est, None, None
    return est, w[c], w[M - 1 - c]


def main(d):
    rows = []
    for name, base, treat in POLICY_PAIRS:
        A, B = C.config_metrics(d, base), C.config_metrics(d, treat)
        if not A or not B:
            continue
        seeds = sorted(set(A) & set(B), key=int)
        if len(seeds) < 6:
            continue
        rec = {"name": name, "n": len(seeds)}
        for key, _lab in METRICS:
            x = [A[s][key] for s in seeds]
            y = [B[s][key] for s in seeds]
            diff = [b - a for a, b in zip(x, y)]
            if all(abs(v) < 1e-15 for v in diff):
                rec[key] = (0.0, 1.0)
                rec.setdefault("hl", {})[key] = (0.0, 0.0, 0.0)
                continue
            # exact test where the sample allows it
            mode = "exact" if len(seeds) <= 25 else "approx"
            try:
                st, p = wilcoxon(diff, alternative="two-sided", method=mode)
            except TypeError:
                st, p = wilcoxon(diff, alternative="two-sided")
            rec[key] = (st, p)
            rec.setdefault("hl", {})[key] = hodges_lehmann(diff)
        rows.append(rec)

    for key, _lab in METRICS:
        ps = [r[key][1] for r in rows]
        adj = holm(ps)
        for r, a in zip(rows, adj):
            r.setdefault("holm", {})[key] = a

    def fmt(p):
        if p >= 1.0:
            return "--"
        if p < 1e-4:
            return "$<10^{-4}$"
        return f"{p:.4f}"

    def eff(hl):
        """Median paired difference with its interval, in percentage points.

        Both metrics are stored as fractions and every other table scales them
        by 100 for display, so this one has to as well: without it the column
        reads 0.062 where Table~\\ref{tab:policies} shows the same effect as a
        move from 6.18% to 0.01%.
        """
        est, lo, hi = hl
        if lo is None or (est == 0.0 and lo == 0.0 and hi == 0.0):
            return "--"
        m = lambda v: "$%.2f$" % (100.0 * v)   # math mode: hyphen is a minus
        return "%s [%s, %s]" % (m(est), m(lo), m(hi))

    nseeds = sorted({r["n"] for r in rows})
    seedtxt = str(nseeds[0]) if len(nseeds) == 1 else "%d to %d" % (nseeds[0], nseeds[-1])
    lines = [r"\begin{table}[t]",
             r"\caption{Effect of switching the rule on, per caching policy. "
             r"$\Delta$ is the Hodges-Lehmann median paired difference in "
             r"percentage points, treatment minus baseline, with its "
             r"distribution-free 95\% confidence interval read off the exact "
             r"signed-rank null distribution. Holm $p$ is the exact Wilcoxon "
             r"signed-rank $p$ after Holm's step-down correction across the "
             r"\NPolicyPoints{} policies within each metric. Every row is "
             + seedtxt + r" seeds, paired by seed. A dash marks a comparison "
             r"in which every seed gave an identical difference of zero}",
             r"\label{tab:stats}",
             r"\small",
             r"\begin{tabularx}{\textwidth}{L{2.75cm} r c r c}",
             r"\toprule",
             r"& \multicolumn{2}{c}{\textbf{Stale deliveries}} & "
             r"\multicolumn{2}{c}{\textbf{Valid cache service}}\\",
             r"\cmidrule(lr){2-3}\cmidrule(lr){4-5}",
             r"\textbf{Caching policy} & $\Delta$ [95\% CI] & Holm $p$ "
             r"& $\Delta$ [95\% CI] & Holm $p$\\",
             r"\midrule"]
    for r in rows:
        lines.append(
            f"{r['name']} & {eff(r['hl']['stale_delivery'])} & "
            f"{fmt(r['holm']['stale_delivery'])} & "
            f"{eff(r['hl']['valid_cache_service'])} & "
            f"{fmt(r['holm']['valid_cache_service'])}\\\\")
    lines += [r"\bottomrule", r"\end{tabularx}", r"\end{table}", ""]
    (OUT / "tab_stats.tex").write_text("\n".join(lines))
    print(f"wrote tab_stats.tex ({len(rows)} policies)")
    worst_v = max((r["holm"]["valid_cache_service"] for r in rows
                   if r["holm"]["valid_cache_service"] < 1.0), default=0.0)
    worst_s = max((r["holm"]["stale_delivery"] for r in rows
                   if r["holm"]["stale_delivery"] < 1.0), default=0.0)
    def ptex(v):
        """A p-value never prints as 0.0000. Below the resolution the exact
        test can express, say so with an inequality, as the table does."""
        return f"{v:.4f}" if v >= 1e-4 else "$<\\!10^{-4}$"
    # The span of the effect itself, so the prose can quote the range without
    # a hand-typed number going stale when the campaign is rerun.
    mags = sorted(abs(100.0 * r["hl"]["stale_delivery"][0]) for r in rows
                  if r["hl"]["stale_delivery"][1] is not None)
    smallest, largest = (mags[0], mags[-1]) if mags else (0.0, 0.0)
    nonzero = sum(1 for r in rows
                  if r["hl"]["stale_delivery"][1] is not None
                  and r["hl"]["stale_delivery"][2] is not None
                  and not (r["hl"]["stale_delivery"][1] <= 0.0
                           <= r["hl"]["stale_delivery"][2]))
    frag = [f"\\newcommand{{\\holmWorstValid}}{{{ptex(worst_v)}}}",
            f"\\newcommand{{\\holmWorstStale}}{{{ptex(worst_s)}}}",
            f"\\newcommand{{\\NStatPolicies}}{{{len(rows)}}}",
            f"\\newcommand{{\\effSmallest}}{{{smallest:.2f}}}",
            f"\\newcommand{{\\effLargest}}{{{largest:.2f}}}",
            f"\\newcommand{{\\effExcludeZero}}{{{nonzero}}}"]
    (OUT / "numbers_stats.tex").write_text("\n".join(frag) + "\n")
    print(f"largest Holm-adjusted p: valid {worst_v:.3g}, stale {worst_s:.3g}")


if __name__ == "__main__":
    main(sys.argv[1])
