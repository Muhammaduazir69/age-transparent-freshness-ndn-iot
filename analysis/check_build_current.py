#!/usr/bin/env python3
"""Fail if the Springer build has drifted from the MDPI master.

The Springer manuscript is generated from ../sensors_v2 by convert.py, so it is
a derived artifact and can silently fall behind. That happened once: the master
was corrected from four bytes to eight, the package had been built before the
correction, and both shipped PDFs then contradicted their own derivation two
paragraphs earlier. A clean compile log and a page count cannot see that.

Two checks, neither of which passes on a stale build:

  1. regenerate the source into a temporary file and diff it against the
     committed main.tex;
  2. compare what the two rendered PDFs assert: every measured value, and a
     list of watched phrases whose wording the revision changed. Both builds
     take their numbers from the same generated macros, so any disagreement is
     a stale artifact rather than a typesetting difference.

    python3 check_build_current.py
"""
import collections, pathlib, re, subprocess, sys, tempfile

HERE = pathlib.Path(__file__).resolve().parent
MASTER = HERE.parent / "sensors_v2"
fail = []

# --- 1. is the committed source what convert.py produces now? ---
with tempfile.NamedTemporaryFile("w+", suffix=".tex", delete=False) as tf:
    tmp = pathlib.Path(tf.name)
r = subprocess.run([sys.executable, "-I", "convert.py", str(tmp)], cwd=HERE,
                   capture_output=True, text=True)
if not tmp.exists() or tmp.stat().st_size == 0:
    fail.append("convert.py produced nothing: " + (r.stderr or "")[-300:])
else:
    a, b = (HERE / "main.tex").read_text(), tmp.read_text()
    if a == b:
        print("source: the committed main.tex is what convert.py produces now")
    else:
        import difflib
        d = [l for l in difflib.unified_diff(a.splitlines(), b.splitlines(),
                                             "committed", "regenerated",
                                             n=0, lineterm="")][:12]
        fail.append("the committed main.tex is STALE:\n    " + "\n    ".join(d))
tmp.unlink(missing_ok=True)

# --- 2. do the two renders assert the same things? ---
# MDPI's submission mode prints a line-number gutter, so bare digit runs have
# to go before anything is compared.
def text_of(pdf):
    t = subprocess.run(["pdftotext", str(pdf), "-"], capture_output=True,
                       text=True).stdout
    t = re.sub(r"\s+", " ", t)
    t = re.sub(r"\b\d{1,4}(?:\s+\d{1,4}){3,}\b", " ", t)   # the gutter
    return t

WATCHED = [
    "eight bytes", "four bytes",
    "not clock-free", "needs no clock",
    "ignores the annotation and strips it", "decays across the unupgraded",
    "no stale readings at all",
    "six builds", "four builds",
    "least affected", "both halves help",
]

m_pdf, d_pdf = MASTER / "main.pdf", HERE / "main.pdf"
if not (m_pdf.exists() and d_pdf.exists()):
    fail.append("one of the two PDFs is missing")
else:
    M, D = text_of(m_pdf), text_of(d_pdf)
    # Every measured value, as a SET. Counts legitimately differ: Springer asks
    # for a Code Availability heading that MDPI folds elsewhere, and it repeats
    # the two tool version numbers. What a stale build looks like is a value
    # present in one render and absent from the other, which is what the
    # pre-revision Springer abstract looked like when it was still claiming
    # the old design.
    def vals(t):
        return set(re.findall(r"(?<![\w.])\d+\.\d{1,3}(?![\w])", t))
    vm, vd = vals(M), vals(D)
    only_m, only_d = sorted(vm - vd), sorted(vd - vm)
    print(f"measured values: {len(vm)} distinct in MDPI, {len(vd)} in Springer")
    if only_m or only_d:
        print("   only in MDPI:  ", only_m[:14])
        print("   only in Springer:", only_d[:14])
        fail.append(f"{len(only_m)} values appear only in MDPI and {len(only_d)} "
                    f"only in Springer")
    else:
        print("   every value in one render appears in the other")
    # watched phrases must appear in both or in neither
    bad = []
    for w in WATCHED:
        if (w in M) != (w in D):
            bad.append(f"{w!r}: MDPI={w in M}, Springer={w in D}")
    print(f"watched phrases: {len(WATCHED)} checked, {len(bad)} disagree")
    for b in bad:
        print("   DISAGREE:", b)
    if bad:
        fail.append("watched phrases disagree between the builds")

print()
print("BUILD CURRENCY:", "FAIL" if fail else "PASS")
for f in fail:
    print(" -", f)
sys.exit(1 if fail else 0)
