# Reference verification

Run 26 November 2026 on the 53 entries of `refs.bib`, all of which are cited.
The script is `figures_src/verify_refs.py` in this repository's tooling; the
raw output is reproducible by rerunning it.

## Why three registries rather than one

An earlier paper of this group was rejected in part because a co-author was
printed as first author, an error a single index had and the publisher did not.
A DOI resolving proves nothing about the metadata behind it, and publishers
answer scripted requests with 202 or 403 whether or not the DOI is good, so
resolution through `doi.org` is useless as evidence either way. This check
therefore compares each entry against:

- **Crossref**, the DOI registry, which returns what the publisher deposited;
- **Semantic Scholar**, whose corpus is assembled from several sources
  including DBLP, so it is not simply a mirror of the deposit;
- **OpenAlex**, as a third opinion where the first two disagree.

DBLP itself was the first choice and is unavailable to scripts: it now answers
with a bot interstitial.

Compared per entry: title (best match across registries, since a truncated
deposit in one is a publisher's error and not ours), year, **first author**,
author count, and page range.

## Coverage

| | count |
|---|---|
| entries, all cited, none uncited | 53 |
| found in Crossref | 45 |
| found in Semantic Scholar | 42 |
| found in OpenAlex | 50 |
| found in at least one | 50 |
| carrying both a DOI and a page range, page range checked | 43 |

The three entries in no registry are not indexed works, and each was verified
by fetching it directly:

- `ndnspec2023`, the NDN Packet Format Specification v0.3. Fetched;
  `docs.named-data.net` returns the document, and the signed-portion definition
  the paper quotes is in it verbatim.
- `ndnlpv2`, the NDN link protocol wiki page. Fetched; the ignorable-field rule
  for types in [800, 959] and the `CachePolicy` precedent that
  Section 4.3 rests on are both quoted from it verbatim.
- `zhang2024sieve`, a USENIX NSDI paper. USENIX assigns no DOI; its
  presentation page returns HTTP 200 and the entry carries that URL.

## One error found and corrected

**`chan2024lora`, the page range.** The entry said 61--71. The article itself
says 60--70: the PDF's running head reads
"Asia-Pacific Journal of Information Technology and Multimedia 13(1) 2024:
60 - 70" and its first page is numbered 60, and Crossref's deposit agrees.
The 61--71 figure came from the journal website's issue listing, which is a
transcription rather than the printing. Corrected to 60--70.

This refines a rule we had been applying: prefer the journal's own printing to
an index, and the printing is the article, not a table on the website.

## Four flags inspected and resolved

**`chan2024lora`, the author list.** Our entry carries five authors; all three
registries say three. The registries are not independent here, because all
three trace to the same publisher deposit. The article PDF is decisive and
agrees with us, printing in our exact order: Chan Jian Siang, Azana Hafizah
Mohd Aman, Mohd Syazwan Baharuddin, Zainab S. Attarbashi, Mohamad Sirajuddin
Jalil. Entry kept as it stands.

**`einziger2017tinylfu`, the title.** Crossref's deposit is the bare word
"TinyLFU". Semantic Scholar carries the full title, "TinyLFU: A Highly
Efficient Cache Admission Policy", which is ours. Crossref confirms everything
else: ACM Transactions on Storage 13(4), 2017, Einziger, Friedman and Manes in
that order. Entry correct; the deposit is incomplete.

**`traverso2013temporal`, the title.** Same pattern: Crossref holds
"Temporal locality in today's content caching" and the published title
continues ": why it matters and how to model it". Entry correct.

**`mnih2015dqn`, the author count.** Nineteen authors in Crossref against four
in our entry, which ends in `and others` so that BibTeX renders "et al.". That
is deliberate and is the convention for a nineteen-author paper. Entry correct.

## Page ranges

Two of the 43 differ from Crossref in form rather than in substance, and both
of ours are the form the publisher prints:

- `einziger2017tinylfu`: ours `35:1--35:31`, ACM's article-number style for
  Article 35; Crossref deposited `1-31`. Same 31 pages.
- `breslau1999zipf`: ours `126--134`; Crossref `126-134 vol.1`, which appends
  the proceedings volume. Same pages.

## First authors

No entry's first author disagrees with any registry. One apparent mismatch,
`meszaros2019inet`, is an artifact of the comparison stripping LaTeX accent
macros from `M{\'e}sz{\'a}ros`; Semantic Scholar's unaccented "Meszaros"
confirms it.

## Entries without a DOI

Eight, each checked individually. Six are USENIX, PMLR or workshop papers that
were never assigned one and each carries a URL that resolves:
`vietri2018lecar`, `rodriguez2021cacheus`, `megiddo2003arc`, `mnih2016a3c`,
`zhang2024sieve`. The sixth, `ari2002acme`, is a 2002 workshop paper with no
online record under a stable URL; it is cited with full bibliographic detail
(venue, series, volume, pages, month) and is indexed in OpenAlex. The remaining
two are the specifications above.
