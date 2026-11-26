#!/usr/bin/env python3
"""Generate the result tables from the recorded campaign."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent))
import campaign as C
from make_results_figs import POLICY_PAIRS, POLICY_CITE

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
         r"\begin{tabularx}{\textwidth}{L{4.25cm} c c c c}",
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
        key = POLICY_CITE.get(name)
        name = f"{name}~\\cite{{{key}}}" if key else name
        L.append(f"{name} & {100*ps[0]:.2f} & {100*ps[1]:.2f} & "
                 f"{ps[5]-ps[4]}/{ps[5]} & {100*pv[0]:.2f} & {100*pv[1]:.2f} & "
                 f"{pv[4]}/{pv[5]}\\\\")
    L += [r"\bottomrule", r"\end{tabularx}", r"\end{adjustwidth}", r"\end{table}", ""]
    Path(out).write_text("\n".join(L))
    print("wrote", out, f"({n_ok} policy pairs)")


FDC_ROWS = [
    # label, config, admission half, expiry half
    (r"Standard NDN", "Fixed-LCE-LRU", "off", "off"),
    (r"Expiry rule only (this paper, timestamp)", "AT-LCE-LRU", "off", "on"),
    (r"FDC admission only", "Fixed-FDCA-LRU", "on", "off"),
    (r"\textbf{FDC as published}", "AT-FDCA-LRU", "on", "on"),
    (r"FDC admission, clock-free expiry", "ATD-FDCA-LRU", "on", r"on$^{*}$"),
]


def tab_fdc(d, out):
    """The 2x2 that attributes FDC's two halves, plus the clock-free composition."""
    L = [r"\begin{table}[t]",
         r"\caption{Freshness-Driven Caching taken apart. Its probabilistic "
         r"admission rule and its residual-lifetime expiry rule are independent, "
         r"so each is run alone and both together, paired by seed over \NSeeds{} "
         r"seeds with everything else fixed. The last row replaces FDC's expiry "
         r"realization with the clock-free one of "
         r"Section~\ref{sec:method:clockfree}, marked $^{*}$. The asterisk governs "
         r"the expiry column only: that half then needs no clock and leaves the "
         r"signed Data packet untouched, while FDC's admission rule still reads "
         r"the publisher's timestamp, so the row as a whole is not clock-free}",
         r"\label{tab:fdc}",
         r"\begin{adjustwidth}{-\extralength}{0cm}",
         r"\begin{tabularx}{\fulllength}{L{4.0cm} c c c c c c c}",
         r"\toprule",
         r"& \multicolumn{2}{c}{\textbf{Rule in force}} & "
         r"\multicolumn{5}{c}{\textbf{Measured}}\\",
         r"\cmidrule(lr){2-3}\cmidrule(lr){4-8}",
         r"\textbf{Configuration} & adm. & exp. & stale (\%) & valid (\%) & "
         r"cache (\%) & energy (J) & lat. (ms)\\",
         r"\midrule"]
    n_ok = 0
    import statistics as st
    for label, cfg, adm, exp in FDC_ROWS:
        m = cm(d, cfg)
        if not m:
            continue
        n_ok += 1
        f = lambda k: st.mean(v[k] for v in m.values())
        L.append(f"{label} & {adm} & {exp} & {100*f('stale_delivery'):.2f} & "
                 f"{100*f('valid_cache_service'):.2f} & {100*f('cache_service'):.2f} & "
                 f"{f('energy_J'):.1f} & {f('latency_ms'):.1f}\\\\")
    L += [r"\bottomrule", r"\end{tabularx}", r"\end{adjustwidth}", r"\end{table}", ""]
    Path(out).write_text("\n".join(L))
    print("wrote", out, f"({n_ok} of {len(FDC_ROWS)} rows)")


VER_ROWS = [
    (r"Standard NDN, one name per item", "Fixed-LCE-LRU", "--"),
    (r"Versioned names, 30~s interval", "Ver30-LCE-LRU", "30~s"),
    (r"Versioned names, 90~s interval", "Ver90-LCE-LRU", "90~s"),
    (r"\textbf{Expiry rule, one name per item}", "AT-LCE-LRU", "--"),
]


def tab_versioned(d, out):
    """Versioned naming: the alternative that changes names instead of the protocol."""
    L = [r"\begin{table}[t]",
         r"\caption{Versioned naming against the expiry rule. Consumers derive the "
         r"version component from a schedule they share, so a cached copy is always "
         r"the right generation. It can still be older than the publisher's declared "
         r"lifetime, because a version interval and a freshness period are different "
         r"quantities, and the flash column is where they part company: publishers "
         r"there declare 15\% of their nominal lifetimes, which is narrower than "
         r"either version interval. Paired by seed over \NSeeds{} seeds}",
         r"\label{tab:versioned}",
         r"\begin{tabularx}{\textwidth}{L{3.95cm} c c c c c c}",
         r"\toprule",
         r"& & \multicolumn{2}{c}{\textbf{stale (\%)}} & & &\\",
         r"\cmidrule(lr){3-4}",
         r"\textbf{Configuration} & \textbf{Version} & all & flash & "
         r"\textbf{valid (\%)} & \textbf{cache (\%)} & \textbf{lat. (ms)}\\",
         r"\midrule"]
    n_ok = 0
    import statistics as st
    for label, cfg, iv in VER_ROWS:
        m = cm(d, cfg)
        if not m:
            continue
        n_ok += 1
        f = lambda k: st.mean(v[k] for v in m.values())
        flash = [C.run_metrics(p_, regimes=(3,)) for p_ in C.sca_files(d, cfg)]
        flash = [v for v in flash if v]
        fl = 100 * st.mean(v["stale_delivery"] for v in flash) if flash else float("nan")
        L.append(f"{label} & {iv} & {100*f('stale_delivery'):.2f} & {fl:.2f} & "
                 f"{100*f('valid_cache_service'):.2f} & {100*f('cache_service'):.2f} & "
                 f"{f('latency_ms'):.1f}\\\\")
    L += [r"\bottomrule", r"\end{tabularx}", r"\end{table}", ""]
    Path(out).write_text("\n".join(L))
    print("wrote", out, f"({n_ok} of {len(VER_ROWS)} rows)")


if __name__ == "__main__":
    d = sys.argv[1]
    tab_main(d, OUT / "tab_main.tex")
    tab_policies(d, OUT / "tab_policies.tex")
    tab_fdc(d, OUT / "tab_fdc.tex")
    tab_versioned(d, OUT / "tab_versioned.tex")
