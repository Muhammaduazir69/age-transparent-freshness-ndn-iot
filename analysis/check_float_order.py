#!/usr/bin/env python3
"""Fail when a float is numbered in a different order than it is first cited.

Springer: "Figures and tables should be numbered in the order in which they
are cited in the text." A float's number comes from where its \\label sits,
its citation order from where the first \\ref sits, and nothing in LaTeX ties
the two together. Reordering a section, or inserting a figure, silently
breaks it.

    python3 check_float_order.py sec_intro.tex sec_related.tex ...
"""
import re, sys

# Several files are concatenated in the order given, which must be the order
# the document \input{}s them: the MDPI master is a wrapper whose \input
# lines carry no floats at all, so reading it alone reported "0 numbered, 0
# cited" and passed while checking nothing.
if len(sys.argv) < 2:
    sys.exit("usage: check_float_order.py FILE [FILE ...]  (in body order)")


def expand(path, depth=0):
    """Splice in \\input{} the way the document does.

    Float numbering follows the order the typesetter meets the environments,
    and in this source most tables sit in their own files that the section
    files \\input. Reading the section files alone found one table out of
    eleven and reported the order wrong.
    """
    import os
    if depth > 8:
        return ""
    try:
        text = open(path, errors="replace").read()
    except OSError:
        return ""
    base = os.path.dirname(path) or "."

    def sub(m):
        name = m.group(1).strip()
        if not name.endswith(".tex"):
            name += ".tex"
        return expand(os.path.join(base, name), depth + 1)

    return re.sub(r"\\input\{([^}]*)\}", sub, text)


src = "\n".join(expand(f) for f in sys.argv[1:])
# strip comments so a commented-out reference does not count
src = re.sub(r"(?<!\\)%.*", "", src)

bad = 0
for kind, env in (("fig", "figure"), ("tab", "table")):
    # numbering order: the order the labels appear inside their environments
    labels = []
    for m in re.finditer(r"\\begin\{%s\*?\}(.*?)\\end\{%s\*?\}" % (env, env),
                         src, re.S):
        for lm in re.finditer(r"\\label\{(%s:[^}]+)\}" % kind, m.group(1)):
            labels.append(lm.group(1))
    # citation order: first \ref outside any float environment
    body = re.sub(r"\\begin\{%s\*?\}.*?\\end\{%s\*?\}" % (env, env), " ",
                  src, flags=re.S)
    first = {}
    for m in re.finditer(r"\\ref\{(%s:[^}]+)\}" % kind, body):
        first.setdefault(m.group(1), m.start())
    cited = [l for l in sorted(first, key=first.get)]
    uncited = [l for l in labels if l not in first]
    expected = [l for l in labels if l in first]
    print(f"{env}s: {len(labels)} numbered, {len(expected)} cited in the text")
    if uncited:
        bad += len(uncited)
        print("   never cited:", ", ".join(uncited))
    if cited != expected:
        bad += 1
        print("   numbered order:", " ".join(expected))
        print("   cited order:   ", " ".join(cited))
        for i, (a, b) in enumerate(zip(expected, cited), 1):
            if a != b:
                print(f"   first mismatch at position {i}: "
                      f"numbered {a}, cited {b}")
                break
    else:
        print("   cited in numbered order")
sys.exit(1 if bad else 0)
