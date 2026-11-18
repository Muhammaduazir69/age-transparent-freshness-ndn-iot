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
                continue
            # exact test where the sample allows it
            mode = "exact" if len(seeds) <= 25 else "approx"
            try:
                st, p = wilcoxon(diff, alternative="two-sided", method=mode)
            except TypeError:
                st, p = wilcoxon(diff, alternative="two-sided")
            rec[key] = (st, p)
        rows.append(rec)

    lines = [r"\begin{table}[t]",
             r"\caption{Exact Wilcoxon signed-rank test on the paired per-seed "
             r"differences produced by switching the rule on, with Holm's "
             r"step-down correction applied across the \NPolicyPoints{} policies "
             r"for each metric. A dash marks a comparison in which every seed "
             r"gave an identical difference of zero.}",
             r"\label{tab:stats}",
             r"\begin{tabularx}{\textwidth}{L{4.2cm} c c c c c}",
             r"\toprule",
             r"& & \multicolumn{2}{c}{\textbf{Stale deliveries}} & "
             r"\multicolumn{2}{c}{\textbf{Valid cache service}}\\",
             r"\cmidrule(lr){3-4}\cmidrule(lr){5-6}",
             r"\textbf{Caching policy} & \textbf{Seeds} & $p$ & Holm $p$ & $p$ & Holm $p$\\",
             r"\midrule"]
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

    for r in rows:
        lines.append(
            f"{r['name']} & {r['n']} & "
            f"{fmt(r['stale_delivery'][1])} & {fmt(r['holm']['stale_delivery'])} & "
            f"{fmt(r['valid_cache_service'][1])} & {fmt(r['holm']['valid_cache_service'])}\\\\")
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
    frag = [f"\\newcommand{{\\holmWorstValid}}{{{ptex(worst_v)}}}",
            f"\\newcommand{{\\holmWorstStale}}{{{ptex(worst_s)}}}",
            f"\\newcommand{{\\NStatPolicies}}{{{len(rows)}}}"]
    (OUT / "numbers_stats.tex").write_text("\n".join(frag) + "\n")
    print(f"largest Holm-adjusted p: valid {worst_v:.3g}, stale {worst_s:.3g}")


if __name__ == "__main__":
    main(sys.argv[1])
