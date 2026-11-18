#!/usr/bin/env python3
"""Report quantities the pipeline computed that no sentence uses.

Every major finding in the November 2026 methodology review was the same
shape: a number was computed, it qualified a claim the manuscript made, and it
never reached the text. The C = 40 eviction rate, the per-joule metric that
moves against the rule, the hand-off buckets where a trend reverses. Each sat
in numbers.tex, unused, while the prose stated the unqualified version.

This does not fail the build. Plenty of macros are intermediates. It prints the
list so that a human has to look at it once per round and decide, which is the
step that was missing.
"""
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
GENERATED = {"numbers.tex", "numbers_stats.tex"}


def main():
    defined = {}
    for name in GENERATED:
        p = ROOT / name
        if not p.exists():
            continue
        for m in re.finditer(r"\\(?:new|provide)command\{\\([A-Za-z]+)\}", p.read_text()):
            defined.setdefault(m.group(1), name)

    used = set()
    for p in sorted(ROOT.glob("*.tex")):
        if p.name in GENERATED:
            continue
        for m in re.finditer(r"\\([A-Za-z]+)", p.read_text()):
            used.add(m.group(1))

    unused = sorted(k for k in defined if k not in used)
    print(f"computed quantities: {len(defined)}   never used in the text: {len(unused)}")
    if unused:
        for i in range(0, len(unused), 4):
            print("   " + "  ".join(f"{k:<22}" for k in unused[i:i + 4]).rstrip())
        print("   (intermediates are fine; look for any that qualify a claim the text makes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
