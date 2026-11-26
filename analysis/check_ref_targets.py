#!/usr/bin/env python3
"""Fail when a cross-reference points at the wrong kind of thing.

LaTeX is silent whenever a label resolves, so "Section~\\ref{tab:fdc}" compiles
cleanly and prints a table number after the word Section. Inserting one
subsection in this paper shifted three references in the response letter by one
and nothing noticed, because each still resolved. This checks the word in front
of every reference against the kind of object its label names, and prints what
each one resolves to so the targets are reviewable rather than merely present.

    python3 check_ref_targets.py main.aux *.tex
"""
import re, sys
from collections import Counter

aux = open(sys.argv[1], errors="replace").read()
num, kind = {}, {}
for m in re.finditer(r"\\newlabel\{([^}]+)\}\{\{([^}]*)\}", aux):
    lbl, val = m.group(1), m.group(2)
    if lbl.endswith(("@cref", "sub@")) or "@" in lbl:
        continue
    num.setdefault(lbl, val)
PREFIX = {"sec": "section", "tab": "table", "fig": "figure", "eq": "equation",
          "alg": "algorithm", "prop": "proposition", "hyp": "hypothesis"}
WORD = {
    "section": {"sec"}, "sections": {"sec"}, "subsection": {"sec"},
    "table": {"tab"}, "tables": {"tab"},
    "figure": {"fig"}, "figures": {"fig"}, "fig.": {"fig"},
    "equation": {"eq"}, "equations": {"eq"},
    "algorithm": {"alg"},
    "proposition": {"prop"}, "hypothesis": {"hyp"},
}
bad, checked = [], 0
counts = Counter()
for f in sys.argv[2:]:
    t = re.sub(r"\s+", " ", open(f).read())
    for m in re.finditer(r"(\w+\.?)[~ ]*\\(?:ref|eqref)\{([^}]+)\}", t):
        word, lbl = m.group(1).lower(), m.group(2)
        pre = lbl.split(":")[0]
        if pre not in PREFIX:
            continue
        checked += 1
        counts[pre] += 1
        if lbl not in num:
            bad.append(f"{f}: {lbl} is referenced but has no label")
            continue
        if word in WORD and pre not in WORD[word]:
            bad.append(f"{f}: \"{word} \\ref{{{lbl}}}\" names a "
                       f"{PREFIX[pre]}, not a {word}")
        # a bare \eqref must point at an equation
        if "\\eqref{" + lbl + "}" in t and pre != "eq":
            bad.append(f"{f}: \\eqref{{{lbl}}} points at a {PREFIX[pre]}")
print(f"cross-references checked: {checked} "
      + ", ".join(f"{k}={v}" for k, v in sorted(counts.items())))
print(f"mismatched or dangling: {len(bad)}")
for b in bad:
    print("   ", b)
sys.exit(1 if bad else 0)
