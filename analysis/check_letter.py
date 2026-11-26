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
for m in re.finditer(r"\\contentsline\s*\{(?:sub){0,2}section\}\s*\{\\numberline\s*\{([0-9.]+)\}", A):
    have.add(m.group(1).rstrip("."))
fail = []

refs = sorted(set(re.findall(r"§\s*([0-9]+(?:\.[0-9]+)*)", L)),
              key=lambda r: [int(x) for x in r.split(".")])
bad_refs = [r for r in refs if r not in have]
# Titles, from the table of contents in the .aux. A reference can resolve and
# still point at the wrong section: inserting one subsection shifted three of
# this letter's references by one, and "it resolves" did not notice.
titles = {}
for m in re.finditer(r"\\contentsline\s*\{(?:sub){0,2}section\}\s*\{\\numberline\s*\{([0-9.]+)\}"
                     r"([^}]*)\}", A):
    titles.setdefault(m.group(1).rstrip("."), m.group(2).strip())
print(f"section references: {len(refs)} distinct, {len(bad_refs)} unresolved")
for r in refs:
    print(f"   §{r:<7s} {titles.get(r, '(title not in the contents listing)')}")
if bad_refs:
    fail.append(("section", bad_refs))
    print("   unresolved:", bad_refs)

# --- phrases the letter quotes ---
# A response letter quotes two kinds of text: wording the manuscript now has,
# which must be present, and wording the revision removed, which must be
# absent. Treating both as "must be present" is what made an earlier run of
# this check report five false failures. The sentence around the quote says
# which kind it is.
# A response letter quotes three kinds of text and they are checked three ways.
# Inferring the kind from the surrounding prose was tried and abandoned: a
# marker-word heuristic classified "all of them \"the first knob...\"" as a
# removal and could not handle one sentence quoting both the old wording and
# the new. The kind is therefore declared here. The lists are short, they live
# next to the letter, and a quotation that leaves one of them changes the
# check's verdict, which is the point.
#
# OLD_WORDING  wording the revision removed: must be ABSENT from the manuscript
# EXTERNAL     quoted from another paper or a specification: not our text
# everything else                                    must be PRESENT
OLD_WORDING = [
    "raw hit ratio can only fall",
    "least affected policy",
    "receive no stale readings at all",
    "the admission half carries its own cost in cache service while the expiry half does not",
    "Sections 6.7 to 6.5",
    "5 and twenty entries",
    "fresh-gated + LFF",
    "the first measurement of staleness amplification",
    "to our knowledge the first NDN Content Store rule that enforces a validity bound on the publisher's clock",
]
# quoted from another source, or a formulation coined in this letter rather
# than taken from the manuscript
EXTERNAL = [
    "is static, i.e., it is not decreased by caching nodes when answering requests",
    "strict synchronization with the Coordinated Universal Time",
    "all vehicles maintain strict synchronization with the Coordinated Universal Time (UTC) that can be acquired from the Global Navigation Support System",
    "based in low-income countries",
    "prefer the journal's own printing to an index",
]
# identifiers, config strings and transliterated display math are not prose
SKIP = re.compile(r"<-|\\|^[A-Za-z_]+ = |experts =|ageMode|residualHeaderBytes|"
                  r"RFP = |e <- ")

def quoted_spans(text):
    """Quotations, found by splitting on each delimiter and taking odd pieces.

    Pairing delimiters with a regex mis-pairs adjacent quotations: the text
    between one closing mark and the next opening mark is captured as though it
    were itself a quotation.
    """
    out = []
    for delim in ('"', "`"):
        pos, parts = 0, text.split(delim)
        for k, part in enumerate(parts):
            if k % 2 == 1 and 18 <= len(part) <= 160 and "\n" not in part:
                out.append((part, pos))
            pos += len(part) + len(delim)
    return out

def listed(q, lst):
    """Exact match after whitespace normalization.

    Substring matching in either direction conflates a renamed label with its
    old form: "fresh-gated + LFF" sits inside "fresh-gated + LFF-origin", so a
    containment test declared the new name removed.
    """
    qn = re.sub(r"\s+", " ", q).strip().strip(".,;:").lower()
    return any(qn == re.sub(r"\s+", " ", x).strip().lower() for x in lst)

spans = quoted_spans(L)
missing_q, lingering_q, skipped_q, external_q = [], [], 0, 0
for q, i in spans:
    qn = re.sub(r"\s+", " ", q).strip().strip(".,;:")
    if not qn or not qn[0].isalnum() or re.fullmatch(r"[\w./\\{}()+\-]+", qn) \
            or SKIP.search(qn):
        skipped_q += 1
        continue
    if listed(qn, EXTERNAL):
        external_q += 1
        continue
    present = qn in P or qn.lower() in P.lower()
    if listed(qn, OLD_WORDING):
        if present:
            lingering_q.append(qn)
    elif not present:
        missing_q.append(qn)
print(f"quoted phrases: {len(spans)} found, {skipped_q} identifiers or math, "
      f"{external_q} quoted from other sources")
print(f"   {len(missing_q)} claimed-present but absent, "
      f"{len(lingering_q)} declared-removed but still present")
for q in missing_q:
    print("   MISSING:", q[:120])
for q in lingering_q:
    print("   STILL THERE:", q[:120])
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
