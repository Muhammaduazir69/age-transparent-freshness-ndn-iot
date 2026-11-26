#!/usr/bin/env python3
"""Check a response letter against the manuscript it describes, completely.

Three classes of claim are checkable mechanically and all three are checked for
every occurrence rather than sampled:

  1. every section reference the letter makes resolves to a real numbered
     section of the manuscript;
  2. every phrase the letter quotes from the manuscript appears in it;
  3. every number the letter attributes to the manuscript appears in it.

Numbers that belong to the revision's history rather than to the manuscript
(for instance what a superseded run measured) are listed in HISTORY_ONLY and
are reported separately instead of as failures.

    python3 check_letter.py <letter.md> <manuscript.pdf> <manuscript.aux>
"""
import re, subprocess, sys

HISTORY_ONLY = {"353.8", "354.8", "65.535"}

letter, pdf, aux = sys.argv[1], sys.argv[2], sys.argv[3]
_raw = open(letter).read()
# The letter is hard-wrapped, so a line break falls inside most sentences.
# Treating it as a sentence boundary cut clauses in half and hid the removal
# marker that tells a quotation of old wording from a quotation of new.
# Paragraph breaks are kept, single line breaks become spaces.
L = re.sub(r"(?<!\n)\n(?!\n)", " ", _raw)
P = re.sub(r"\s+", " ", subprocess.run(["pdftotext", pdf, "-"], capture_output=True,
                                       text=True).stdout)
A = open(aux, errors="replace").read()

# --- sections the manuscript actually has, from the aux ---
have = set()
for m in re.finditer(r"\\newlabel\{[^}]*\}\{\{([0-9]+(?:\.[0-9]+)*)\}", A):
    have.add(m.group(1))
for m in re.finditer(r"\\contentsline\s*\{[a-z]+\}\s*\{\\numberline\s*\{([0-9.]+)\}", A):
    have.add(m.group(1).rstrip("."))
fail = []

refs = sorted(set(re.findall(r"§\s*([0-9]+(?:\.[0-9]+)*)", L)))
bad_refs = [r for r in refs if r not in have]
print(f"section references: {len(refs)} distinct, {len(bad_refs)} unresolved")
if bad_refs:
    fail.append(("section", bad_refs))
    print("   unresolved:", bad_refs)

# --- phrases the letter quotes ---
# A response letter quotes two kinds of text: wording the manuscript now has,
# which must be present, and wording the revision removed, which must be
# absent. Treating both as "must be present" is what made an earlier run of
# this check report five false failures. The sentence around the quote says
# which kind it is.
REMOVED = re.compile(
    # Stems, not whole words: a trailing \b after "contradict" fails to match
    # "contradicted", which is how two quotations of removed wording were
    # misreported as missing from the manuscript.
    r"\b(previous\w*|former\w*|used to|no longer|remov\w*|delet\w*|replac\w*|"
    r"retract\w*|earlier draft|old version|had said|had written|first draft|"
    r"was wrong|is false|did not support|contradict\w*|overstat\w*|correct\w*|"
    r"now reads|it now says|instead reads)", re.I)
# the letter transliterates display math and config strings; those are not
# prose quotations and the manuscript renders them differently
SKIP = re.compile(r"<-|\\|^[A-Za-z_]+ = |experts =|ageMode|residualHeaderBytes")

def quoted_spans(text):
    """Double-quoted spans, plus backtick spans taken by splitting on backticks.

    Matching backtick pairs with a regex mis-pairs them when a sentence holds
    several: the span between one code fragment's closing tick and the next
    one's opening tick gets captured as if it were a quotation. Splitting and
    taking the odd pieces cannot make that mistake.
    """
    out = [(m.group(1), m.start(1)) for m in re.finditer(r'"([^"\n]{18,140})"', text)]
    pos, parts = 0, text.split("`")
    for i, part in enumerate(parts):
        if i % 2 == 1 and 18 <= len(part) <= 140 and "\n" not in part:
            out.append((part, pos))
        pos += len(part) + 1
    return out

def sentence_around(text, i):
    """The clause holding position i, plus the clause before it.

    A removal marker often sits in the preceding sentence ("The previous
    version argued that ... 'quote'"), so one clause is too narrow. Sentence
    boundaries are also found with a period that is followed by whitespace, or
    a semicolon: splitting on any period cuts a clause in half at a section
    number like 6.8.
    """
    bounds = [m.end() for m in re.finditer(r"(?:[.;]\s|\n)", text[:i])]
    a = bounds[-2] if len(bounds) >= 2 else (bounds[0] if bounds else 0)
    m = re.search(r"(?:[.;]\s|\n)", text[i:])
    b = i + m.start() if m else len(text)
    return text[a:b]

spans = quoted_spans(L)
missing_q, lingering_q, skipped_q = [], [], 0
for q, i in spans:
    qn = re.sub(r"\s+", " ", q).strip().strip(".,;:")
    if (not qn or not qn[0].isalnum() or re.fullmatch(r"[\w./\\{}()+\-]+", qn)
            or SKIP.search(qn)):
        skipped_q += 1
        continue
    should_be_gone = bool(REMOVED.search(sentence_around(L, i)))
    present = qn in P
    if should_be_gone and present:
        lingering_q.append(qn)
    elif not should_be_gone and not present:
        missing_q.append(qn)
print(f"quoted phrases: {len(spans)} found, {skipped_q} are identifiers or math,")
print(f"   {len(missing_q)} claimed-present but absent, "
      f"{len(lingering_q)} claimed-removed but still present")
for q in missing_q:
    print("   MISSING:", q[:110])
for q in lingering_q:
    print("   STILL THERE:", q[:110])
if missing_q:
    fail.append(("quote-missing", missing_q))
if lingering_q:
    fail.append(("quote-lingering", lingering_q))

# --- numbers ---
nums = sorted(set(re.findall(r"(?<![\w.])(\d+\.\d+)(?![\w])", L)))
missing_n = [n for n in nums if n not in P and n not in HISTORY_ONLY]
hist = [n for n in nums if n not in P and n in HISTORY_ONLY]
print(f"decimal values: {len(nums)} checked, {len(missing_n)} not in the manuscript, "
      f"{len(hist)} revision-history only")
if missing_n:
    print("   missing:", missing_n)
    fail.append(("number", missing_n))
if hist:
    print("   history only (expected):", hist)

print()
print("LETTER CHECK:", "FAIL" if fail else "PASS")
sys.exit(1 if fail else 0)
