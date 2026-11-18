#!/usr/bin/env python3
"""Compare two .sca files by their recorded values only.

Used to test, rather than assert, that a rebuild left existing configurations
untouched. Run metadata (timestamps, run id, the processid attr) differs
between any two runs and is ignored; every scalar and every statistic field is
compared exactly.

    python3 compare_sca.py a.sca b.sca
"""
import gzip, sys
from pathlib import Path


def values(path):
    op = gzip.open if str(path).endswith(".gz") else open
    out = {}
    stat = None
    with op(path, "rt") as f:
        for line in f:
            p = line.split()
            if not p:
                continue
            if p[0] == "scalar" and len(p) >= 4:
                out[("scalar", p[1], p[2])] = p[3]
                stat = None
            elif p[0] == "statistic" and len(p) >= 3:
                stat = (p[1], p[2])
            elif p[0] == "field" and stat and len(p) >= 3:
                out[("field", stat[0], stat[1] + ":" + p[1])] = p[2]
            elif p[0] in ("bin", "attr", "param", "config", "run", "version"):
                continue
    return out


def main(a, b):
    va, vb = values(a), values(b)
    only_a = sorted(set(va) - set(vb))
    only_b = sorted(set(vb) - set(va))
    diff = sorted(k for k in set(va) & set(vb) if va[k] != vb[k])
    print(f"{Path(a).name}: {len(va)} recorded values")
    print(f"{Path(b).name}: {len(vb)} recorded values")
    print(f"only in first : {len(only_a)}")
    print(f"only in second: {len(only_b)}")
    print(f"differing     : {len(diff)}")
    for k in only_a[:5]:
        print("   missing in second:", k)
    for k in only_b[:5]:
        print("   missing in first :", k)
    for k in diff[:10]:
        print(f"   {k}: {va[k]} vs {vb[k]}")
    ok = not (only_a or only_b or diff)
    print("IDENTICAL" if ok else "NOT IDENTICAL")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2]))
