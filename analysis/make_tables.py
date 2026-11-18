#!/usr/bin/env python3
"""Generate the result tables from the recorded campaign."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C
from make_results_figs import POLICY_PAIRS

_CACHE = {}


def cm(d, cfg):
    if cfg not in _CACHE:
        _CACHE[cfg] = C.config_metrics(d, cfg)
    return _CACHE[cfg]


def pr(d, a, b, m):
    x, y = cm(d, a), cm(d, b)
    return C.paired_runs(x, y, m) if x and y else None

OUT = Path(__file__).resolve().parents[1]

MAIN_ROWS = [
    ("Stale deliveries (\\%)", "stale_delivery", 100, 2, "lower"),
    ("Valid cache service (\\%)", "valid_cache_service", 100, 2, "higher"),
    ("Cache service (\\%)", "cache_service", 100, 2, "higher"),
    ("Interests satisfied (\\%)", "satisfaction", 100, 2, "higher"),
    ("Mean latency (ms)", "latency_ms", 1, 1, "lower"),
    ("Radio energy (J)", "energy_J", 1, 1, "lower"),
]


def fmt(v, dec):
    return f"{v:.{dec}f}"


def tab_main(d, out):
    L = [r"\begin{table}[t]",
         r"\caption{Standard NDN against age-transparent freshness on the default "
         r"configuration (leave-copy-everywhere, LRU, ten entries per store), "
         r"paired by seed over \NSeeds{} seeds. The last column counts the seeds on "
         r"which the change moves the metric in the direction the arrow marks as "
         r"better.}",
         r"\label{tab:main}",
         r"\begin{tabularx}{\textwidth}{L{5.0cm} c c c c}",
         r"\toprule",
         r"\textbf{Metric} & \textbf{Standard NDN} & \textbf{Age-transparent} & "
         r"\textbf{Change} & \textbf{Seeds}\\",
         r"\midrule"]
    for label, metric, scale, dec, better in MAIN_ROWS:
        p = pr(d, "Fixed-LCE-LRU", "AT-LCE-LRU", metric)
        if not p:
            continue
        b, t, md, tv, wins, n = p
        good = wins if better == "higher" else n - wins
        arrow = r"$\uparrow$" if better == "higher" else r"$\downarrow$"
        L.append(f"{label} {arrow} & {fmt(b*scale,dec)} & {fmt(t*scale,dec)} & "
                 f"${md*scale:+.{dec}f}$ & {good}/{n}\\\\")
    L += [r"\bottomrule", r"\end{tabularx}", r"\end{table}", ""]
    Path(out).write_text("\n".join(L))
    print("wrote", out)


def tab_policies(d, out):
    L = [r"\begin{table}[t]",
         r"\caption{The rule switched on underneath each caching policy, paired by "
         r"seed over \NSeeds{} seeds. Every policy keeps its own admission and "
         r"replacement rules; only the expiry of a stored copy changes.}",
         r"\label{tab:policies}",
         r"\begin{adjustwidth}{-\extralength}{0cm}",
         r"\begin{tabularx}{\fulllength}{L{3.3cm} c c c c c c}",
         r"\toprule",
         r"& \multicolumn{3}{c}{\textbf{Stale deliveries (\%)}} & "
         r"\multicolumn{3}{c}{\textbf{Valid cache service (\%)}}\\",
         r"\cmidrule(lr){2-4}\cmidrule(lr){5-7}",
         r"\textbf{Caching policy} & std. & age-tr. & seeds & std. & age-tr. & seeds\\",
         r"\midrule"]
    n_ok = 0
    for name, base, treat in POLICY_PAIRS:
        ps = pr(d, base, treat, "stale_delivery")
        pv = pr(d, base, treat, "valid_cache_service")
        if not ps or not pv:
            continue
        n_ok += 1
        L.append(f"{name} & {100*ps[0]:.2f} & {100*ps[1]:.2f} & "
                 f"{ps[5]-ps[4]}/{ps[5]} & {100*pv[0]:.2f} & {100*pv[1]:.2f} & "
                 f"{pv[4]}/{pv[5]}\\\\")
    L += [r"\bottomrule", r"\end{tabularx}", r"\end{adjustwidth}", r"\end{table}", ""]
    Path(out).write_text("\n".join(L))
    print("wrote", out, f"({n_ok} policy pairs)")


if __name__ == "__main__":
    d = sys.argv[1]
    tab_main(d, OUT / "tab_main.tex")
    tab_policies(d, OUT / "tab_policies.tex")
