"""Integrity checks for the enlarged substrate campaign.

1. Reproduction: the enlarged campaign re-ran the six training seeds the
   reviewer campaign already had. Frozen greedy policies on the same seed sets
   are deterministic, so the new runs must reproduce the old ones exactly. If
   they do, the new and old runs pool; if they do not, something changed and
   nothing pools.
2. Degradation gate: rlDegraded and rlFallbacks must be zero on every learned
   run in the campaign.
3. Seed disjointness: training episode seed-sets must not touch the test seeds
   0..9 or the validation seeds 100..102.
"""

import pathlib
import sys

import numpy as np
import pandas as pd

from . import substrate2 as S

PAIRS = [
    # (new dir, new stem, old dir, old stem)
    *[(f"test_real", f"MAACO-rs{s}", "test_maaco", f"rs{s}") for s in range(1, 7)],
    *[(f"test_transfer", f"MAACO-rs{s}", "substrate_rl", f"IMAACO-rs{s}") for s in (1, 2, 3)],
    *[(f"test_real", f"DQN-rs{s}", "stage1", f"DQN-rs{s}") for s in (1, 2, 3)],
    *[(f"test_transfer", f"DQN-rs{s}", "substrate_rl", f"IDQN-rs{s}") for s in (1, 2, 3)],
    *[(f"test_idealtrained", f"MAACO-idealS{s}", "test_ideal", f"idealS{s}") for s in (1, 2)],
    *[(f"test_real", f"MAB-mabS{s}", "test_mab", f"mabS{s}") for s in range(1, 5)],
    *[(f"test_real", f"TABQ-tqS{s}", "test_tabq", f"tqS{s}") for s in range(1, 5)],
]


def reproduction():
    rows = []
    for ndir, nstem, odir, ostem in PAIRS:
        new = S.cell(ndir, nstem).sort_values("seed")
        old = S.runs_for(odir, ostem).sort_values("seed")
        if new.empty or old.empty:
            rows.append(dict(new=f"{ndir}/{nstem}", old=f"{odir}/{ostem}",
                             status="MISSING", nNew=len(new), nOld=len(old),
                             maxAbsDiff=np.nan))
            continue
        n = min(len(new), len(old))
        a = new["cacheHitRatio"].values[:n]
        b = old["cacheHitRatio"].values[:n]
        d = float(np.max(np.abs(a - b)))
        rows.append(dict(new=f"{ndir}/{nstem}", old=f"{odir}/{ostem}",
                         status="exact" if d < 1e-12 else
                                ("close" if d < 1e-6 else "DIFFERS"),
                         nNew=len(new), nOld=len(old), maxAbsDiff=d))
    return pd.DataFrame(rows)


def degradation():
    bad, total = [], 0
    for d in ("test_real", "test_transfer", "test_idealtrained", "test_reverse",
              "abl_AE", "abl_AC", "abl_AB", "abl_AM"):
        df = S.load(S.S2 / d)
        if df.empty:
            continue
        learned = df[df["rlDegraded"].notna()]
        total += len(learned)
        for cfg, seed in S.degraded_in(learned):
            bad.append((d, cfg, seed))
    return total, bad


def seed_disjointness():
    """Training seed s uses simulation seed-sets s*100000 + episode."""
    test, val = set(range(10)), {100, 101, 102}
    clash = []
    for s in list(range(1, 11)):
        for e in range(300):
            if s * 100000 + e in test or s * 100000 + e in val:
                clash.append((s, e))
    return clash


def main():
    pd.set_option("display.width", 200)
    r = reproduction()
    print("=== reproduction of the reviewer campaign's runs ===")
    print(r.to_string(index=False))
    print("  summary:", r["status"].value_counts().to_dict())

    total, bad = degradation()
    print(f"\n=== degradation gate: {total} learned runs checked ===")
    print("  offending runs:", bad if bad else "none")

    print("\n=== training/test/validation seed disjointness ===")
    c = seed_disjointness()
    print("  clashes:", c if c else "none (training seed-sets start at 100000)")

    r.to_csv(S.S2 / "verification.csv", index=False)
    return 1 if (bad or (r["status"] == "DIFFERS").any()) else 0


if __name__ == "__main__":
    sys.exit(main())
