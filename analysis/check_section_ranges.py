#!/usr/bin/env python3
"""Fail on a cross-reference range whose endpoints are out of order.

Moving a subsection silently inverts every "Sections X to Y" that spans it, and
the result reads "Sections 6.7 to 6.5". LaTeX cannot see it: both labels
resolve. This reads the resolved numbers out of the .aux and checks each range.

    python3 check_section_ranges.py main.aux *.tex
"""
import re, sys

aux = open(sys.argv[1], errors="replace").read()
num = {}
for m in re.finditer(r"\\newlabel\{([^}]+)\}\{\{([0-9]+(?:\.[0-9]+)*)\}", aux):
    num.setdefault(m.group(1), m.group(2))

def key(lbl):
    v = num.get(lbl)
    return tuple(int(x) for x in v.split(".")) if v else None

bad = []
pats = [
    re.compile(r"\\ref\{([^}]+)\}\s*(?:to|through|--)~?\\ref\{([^}]+)\}"),
    re.compile(r"\\ref\{([^}]+)\}~?(?:to|through)~\\ref\{([^}]+)\}"),
]
n = 0
for f in sys.argv[2:]:
    t = re.sub(r"\s+", " ", open(f).read())
    for pat in pats:
        for m in pat.finditer(t):
            a, b = key(m.group(1)), key(m.group(2))
            n += 1
            if a and b and a >= b:
                bad.append(f"{f}: {m.group(1)} ({num[m.group(1)]}) to "
                           f"{m.group(2)} ({num[m.group(2)]}) is not increasing")
print(f"cross-reference ranges: {n} checked, {len(bad)} out of order")
for b in bad:
    print("   ", b)
sys.exit(1 if bad else 0)
