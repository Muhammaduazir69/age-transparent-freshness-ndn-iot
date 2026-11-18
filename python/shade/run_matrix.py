#!/usr/bin/env python3
"""Run a configuration x seed matrix of the ShadeCampus scenario in parallel.

Example:
    python3 -m shade.run_matrix --configs Fixed-LCE-LRU SHADE --seeds 0-9 \
        --out ../results/main --jobs 10 [--set key=value ...]

Every run writes <out>/<Config>-<seed>.sca; runs whose .sca already exists
are skipped, so an interrupted campaign can simply be restarted.
"""
import argparse, concurrent.futures as cf, os, subprocess, sys, time
from pathlib import Path

PROJ = Path(__file__).resolve().parents[2]
RUN = PROJ / "simulations" / "run"


def seeds(spec):
    out = []
    for part in spec.split(","):
        if "-" in part:
            a, b = part.split("-")
            out += list(range(int(a), int(b) + 1))
        else:
            out.append(int(part))
    return out


def complete(sca):
    """A finished run, judged by content rather than by byte count.

    A run killed partway through still leaves a header-only .sca, and one of
    those came to 4096 bytes, which sailed past an earlier size>1000 test. Ten
    such stubs were then skipped on every resume and silently halved the seed
    count of one configuration in the published table. Recorded values are what
    makes a run usable, so count those.
    """
    if not sca.exists() or sca.stat().st_size < 1000:
        return False
    n = 0
    with open(sca, errors="replace") as fh:
        for line in fh:
            if line.startswith(("scalar ", "statistic ")):
                n += 1
                if n >= 100:
                    return True
    return False


def one(cfg, seed, outdir, extra, ini):
    sca = outdir / f"{cfg}-{seed}.sca"
    if complete(sca):                                # completed earlier
        return cfg, seed, "skip", 0.0
    t0 = time.time()
    cmd = [str(RUN), "-c", cfg, "-r", str(seed), "-f", ini,
           f"--result-dir={outdir}"] + [f"--{kv}" for kv in extra]
    p = subprocess.run(cmd, capture_output=True, text=True)
    ok = p.returncode == 0 and complete(sca)
    if not ok:
        (outdir / f"{cfg}-{seed}.err").write_text(p.stdout[-4000:] + p.stderr[-4000:])
    return cfg, seed, "ok" if ok else "FAIL", time.time() - t0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--configs", nargs="+", required=True)
    ap.add_argument("--seeds", default="0-9")
    ap.add_argument("--out", required=True)
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 2) - 1))
    ap.add_argument("--ini", default="omnetpp.ini")
    ap.add_argument("--set", nargs="*", default=[],
                    help="extra ini assignments, e.g. **.csMaxEntries=20")
    a = ap.parse_args()
    outdir = Path(a.out).resolve()
    outdir.mkdir(parents=True, exist_ok=True)
    jobs = [(c, s) for c in a.configs for s in seeds(a.seeds)]
    print(f"{len(jobs)} runs -> {outdir}", flush=True)
    fails = 0
    with cf.ThreadPoolExecutor(a.jobs) as ex:
        futs = [ex.submit(one, c, s, outdir, a.set, a.ini) for c, s in jobs]
        for f in cf.as_completed(futs):
            cfg, seed, st, dt = f.result()
            fails += st == "FAIL"
            print(f"{st:4s} {cfg}-{seed} {dt:6.1f}s", flush=True)
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
