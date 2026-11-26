# Reviewer-proof audit, supervisor revision round

Manuscript: *Age-Transparent Freshness*, for *Discover Computing*.
Signed off 26 November 2026 against the build that is in
`discover_computing/submission/`. Each gate below is marked PASS, PASS WITH
NOTE, N/A or FAIL, with the evidence that supports the mark. A gate marked PASS
on the strength of an assertion rather than a check is marked PASS WITH NOTE
instead, and says so.

## T1 Standards attribution

PASS. Four normative documents are cited for specific claims, and each was
fetched and the cited form compared against the text.

- NDN Packet Format Specification v0.3, cited for the signed portion. Fetched;
  it defines the signed portion as every element from `Name` up to but not
  including `SignatureValue`, which is what §4.3 quotes, and for the per-hop
  meaning of `FreshnessPeriod`, which §3.2 quotes.
- NDNLPv2, cited for where a per-hop value may live. Fetched; the ignorable
  range [800, 959] with the two least significant bits zero, and the
  `CachePolicy` precedent, are both in it verbatim. The type encoding claim
  (`CachePolicy` is 820, encoding `FD 03 34`) follows from NDN's VAR-NUMBER
  rule and the type's own value.
- RFC 9111, cited for the `Age` header accumulating residence time. The cited
  behavior is §4.2 of that RFC and matches the paper's one-sentence use.
- IEEE 802.15.4-2020, cited for the radio. No equation is attributed to it;
  the physical-layer values are INET's documented defaults and Table 4 marks
  each one's source.

## T2 Physical-quantity confusion

N/A. Every quantity in Equations (2) to (11) is a time or a count of times. No
equation mixes a kinematic quantity with a power ratio.

## T3 Abstract-adjective trace

PASS. Checked mechanically rather than by reading: `check_letter.py` confirms
every number in the response letter appears in the manuscript, and the abstract
itself was read against the body in the full read-through. The abstract makes
no claim the body does not carry, and after this round it credits prior work
for the expiry rule, which it previously did not.

Note on how this gate failed earlier in the round and how it is now held: the
Springer abstract was a restatement inside `convert.py` rather than the
master's text, and it kept the pre-revision claim after the master was
rewritten. `check_build_current.py` now compares the set of measured values and
a list of watched phrases between the two renders, which is what caught it.

## T4 Controlled-baseline framing

PASS. The headline comparison is against FDC, the closest prior work, not
against the weakest baseline, and FDC's advantages are reported: it beats the
expiry rule alone on valid cache service by 0.68 points on 20 of 20 seeds, and
its admission rule raises raw cache service by 0.74 points on 20 of 20. Two
metrics that move against the paper's mechanism are reported in the same
register as those that favor it: raw cache service falls by 0.40 points, and
total deliveries per joule fall 1.8% on 0 of 20 seeds.

## T5 Methodology completeness

PASS, after a change made by this audit. Every column of Tables 8, 9 and 11
traces to a specification. The learned comparator did not: it appeared in two
tables and carried a quantitative claim about its reward while the paper gave
only one sentence about it. §5.3 now specifies it, from the configuration the
shipped checkpoint recorded rather than from defaults in the code: two actions
over a 17-dimensional state with the features enumerated, nine per-item
features for eviction, the ten reward event kinds, the shipped weights, the
two-channel reward of Equation (12) and why stale delivery reaches only the
field-wide channel, the training schedule and both learning rates, and the
feature-distribution mismatch that holding the weights frozen causes.

## T6 Author name against bibliography key

PASS. `verification/references.md`: every entry's first author was compared
against Crossref, Semantic Scholar and OpenAlex, and none disagrees. One
apparent mismatch was a comparison artifact from stripping LaTeX accent macros
from `M{\'e}sz{\'a}ros`.

## T7 The mechanism actually runs

PASS. The expiry rule's two realizations are selected by recorded parameters,
and `check_effective_params.py` confirms over 50 configurations that no ini
assignment was silently ignored. The clock-free path is counted, not inferred:
the store records `annotationsAttached` (8337 per run) and
`annotationsCharged` (4803), and the timestamp arm records zero, which is what
an inert path should record. The FDC admission rule accepts 59.9% of offered
copies, so it is neither degenerate nor a no-op.

## T8 The geometry actually simulated

PASS. Hop distances come from the breadth-first search the configurator
recorded as a scalar, not from coordinates. The reported range, 1 to 10 across
the lattice and 1 to 9 across reported consumers, is consistent with the
excluded far-corner node being one of the three compromised sensors.

## T9 One source of truth per number

PASS WITH NOTE. Every number in the text and tables is a generated macro; 447
are computed and the build fails on a sentinel reaching the PDF. Three
quantities were hand-written at the start of this round and are now generated
because each had gone stale: the dataset size (1300 against 1480), the build
count (six against seven), and the capacity bounds in prose. The note is that
this gate has now failed three times in this paper's history in the same way,
which is why the generated-macro rule is applied to counts and not only to
measurements.

## T10 Mathematics

PASS. Proposition 1 carries its proof. The former Proposition 2 is now
Hypothesis 1, with the pathwise coupling it would need written out as Equation
(5), a proof of the implication, and an explicit statement that the coupling is
not claimed; both premises and the conclusion are measured, the conclusion on
the quantity the hypothesis names (mean delivered age: 0.101, 0.212, 0.347,
0.466 over the four capacities). The former Equation (4) is split into an exact
conditional identity, a labeled length-biasing hypothesis, and the prediction
that follows from it, with the falsification condition stated. Equation-to-code
correspondence for every equation is recorded in
`sensors_v2/verification/equation_code.md`, including FDC's Equations (2) to
(4) against the implementation.

## T11 Algorithm direction and specification

PASS. Algorithm 1 has no optimization step, so there is no update direction to
check. Its two substantive lines were checked against the code: the deadline is
`min{x.F, max{0, x.r}}` in both, and the hand-on value is `e - t_s` in both.
No hand-waving step remains: the clamp, the fallback when no annotation is
present, and the strip at a non-implementing node are all in the code and all
named in the algorithm or its caption.

## T12 Each mechanism takes effect, and the self-admission sweep

PASS. The expiry rule moves its target metric on 20 of 20 seeds under all 12
policies and clears a Holm correction over that family. The FDC baseline moves
staleness on 20 of 20. The versioned-naming arm moves it in the three regimes
where the interval binds and not in the fourth, which is the predicted pattern.

The self-admission sweep found two disclosures and both were acted on rather
than written up. The signature-verification routine is a stub: that is
disclosed in §5.2 with the direction it cuts, and it touches no core claim,
since the paper's claim is about when a copy expires. The clock-free path's
in-flight residue is disclosed and measured per hand-off count rather than
asserted negligible.

## T13 Feature provenance, causality, leakage

PASS. The learned comparator's training seeds are disjoint from the 20
evaluation seeds, which §5.3 states and the configuration records. Its state
features are node-local observables, enumerated in §5.3. No feature is derived
from a future quantity.

## T14 Baselines under equal selection

PASS. Every policy pair differs in exactly one parameter, the expiry rule, and
the paired design gives both arms identical traffic by seed. The FDC comparison
is a 2x2 in which each half is on or off, so neither half is given an
advantage of configuration. pCASTING's weights are the equal weighting the
original leaves unspecified, which §6.7 states, together with what a different
weighting would do.

## T15 Metric denominators

PASS. §5.7 fixes one denominator for every delivery-side quantity and labels
satisfaction as the one per-issued rate. One sentence that named a share of
deliveries as a share of hits was corrected in this round. The identity
relating the three cache-service quantities is stated as a reporting
convention, not as a check, because the third term is computed from the first
two.

## T16 Origin citations

PASS. Every named method is cited to its origin: DQN, A2C, LeCaR, ARC, TinyLFU,
ProbCache, LCD, pCASTING, least-fresh-first, FDC, and the Zipf and temporal
locality models. The comparison table's columns are sourced per row.

## T17 Letter and artifact consistency

PASS, and now gated. `check_letter.py` checks the response letter against the
shipped manuscript completely: 26 section references resolve, 27 quotations are
present where the letter claims present and absent where it declares them
removed, and 44 numbers appear in the manuscript bar three that belong to the
revision's history and are declared as such. `check_build_current.py` checks
that the Springer source is what the converter produces now and that the two
renders assert the same values. The code release is current with the submitted
manuscript and carries both checkers.

## Final end-to-end verification of the shipped artifact

Run after the last change, which is the point: the T5 fix added §5.3 and
shifted every section number after it, and three references in the response
letter went stale as a result. The state below is the state of the files in
`discover_computing/submission/`.

- **Cross-reference targets.** 200 references checked, 0 mismatched or
  dangling: 119 sections, 50 equations, 18 tables, 7 figures, 2 algorithms, 2
  propositions, 2 hypotheses. LaTeX cannot catch this class, because a label
  that resolves compiles cleanly whatever kind of object it names.
  `check_ref_targets.py`, wired into `regen.sh`, self-tested against three
  deliberately wrong references.
- **Reference ranges.** 0 out of order. `check_section_ranges.py`, self-tested
  against the "Sections 6.7 to 6.5" defect it was written for.
- **The response letter.** All 26 section references resolve, and the checker
  now prints the title each one resolves to, so the target is reviewable rather
  than merely present. 27 quotations and 43 numbers check out.
- **This audit's own citations.** The 15 section and table numbers cited above
  were compared against the final build's `.aux`: all correct.
- **The two builds agree.** 269 distinct measured values, every one in each
  render present in the other; 11 watched phrases agree.
- **The shipped source reproduces the shipped PDF**, compiled in an empty
  directory, text identical.
- **MDPI 47 pages, Springer 57 pages**, both with zero errors, zero undefined
  references and zero overfull boxes; abstract 226 words against the journal's
  under-250.
- The new §5.3 was read as rendered, not only as source. One ambiguity was
  removed: a feature index a reader could have counted from zero.

## Known residue

Three things are not clean and are recorded rather than hidden.

1. `manuscript_lint.py` reports one MAJOR, an acronym it reads out of the FRGS
   grant identifier `FRGS/1/2019/ICT03/UKM/02/1`. Verified a false positive by
   masking the grant number, which clears it to zero MAJOR. The grant number
   must stay verbatim.
2. Two MINOR lint findings are detex artifacts: a doubled token inside the
   algorithm's math and a sentence that begins with a `\texttt{}` the detexer
   removes.
3. The signature-verification stub of §5.2. Disclosed, scoped, and outside the
   paper's claims, but it is a limitation and not a clean pass.

## What this audit does not establish

That the manuscript is free of defects. Four review rounds each found
something, the per-round count fell from four code defects to eight prose and
artifact defects, and the last of them was found by reading the paper rather
than by any check here. The gates establish what was checked and how; they do
not establish completeness, and this sign-off should not be read as claiming it.

---

## Addendum, 27 November 2026: the math prescreen, and what it did not resolve

`math_lint.py` (M1 to M26) was run for the first time on this manuscript. It
started at 8 MAJOR and ends at 3. The five that were resolved were resolved by
changing the paper, not the gate.

| gate | what it wanted | what changed |
|---|---|---|
| M3 | an equation labelled and never referenced | the reward equation, added last round and never cited, is now referenced where its two channels are discussed |
| M7 | complexity for the algorithm box | $O(1)$ per admission and per hand-off, $O(1)$ state per entry, stated in Section 4.4 |
| M8 | confidence intervals | the statistics table reported only $p$ and Holm $p$ for twelve policies. It now leads with the Hodges-Lehmann median paired difference and its distribution-free interval, read off the same exact signed-rank null distribution |
| M9 | recent works compared | the five 2025 and 2026 schemes are cited in the results, at the paragraph that says why the baselines are the canonical scheme per mechanism class |
| M15 | a link layer in the system model | Section 3.1 now states the link abstraction, what a hop costs, and the two consequences that matter, instead of naming the radio and moving on |

### The three left standing, and why

1. **M20, a 30 to 35 reference budget.** That is a Transactions page-limit
   budget. Discover Computing publishes no reference limit, and the supervisor's
   tenth comment asked for a reference to be added. Declined.
2. **M23, a constraint-violation rate for the RL method.** The learned agent
   here is a baseline we compare against, not the proposed method, and it
   optimizes an unconstrained reward. There is no constraint to violate.
   Declined.
3. **M25, Doppler in a satellite ISAC paper.** This is not a satellite paper.
   The gate fires on the words "satellite" and "LEO", which occur in the
   related-work discussion of age-driven caching over a constellation and in
   the expansion of GNSS. Verified by inspection: 8 occurrences of "satellite",
   none in the system model. Declined as a misclassification.

One MINOR is also declined on purpose. **M16** asks for three to five
theorem-like results and the paper has two. The third would have to be
manufactured, and the supervisor's fifth comment went the other way: it asked
for the former Proposition 2 to stop being stated as a proved result. Adding
theorems to clear a lint would undo that. The one result added this round,
Proposition 2 on the adversarial ceiling, was added because it is provable from
the algorithm in four lines and because it states a security guarantee the
paper had only in prose, not to raise a count.

### A literature check that found nothing, which is the useful outcome

Because the supervisor's first comment was that the paper's headline mechanism
was already published, the same question was asked again against 2024 to 2026
work: has anything newer than FDC set a cached copy's expiry on the publisher's
clock? A search plus a reading of a May 2026 survey of NDN caching
(arXiv:2605.13104) turned up nothing that does. The closest remains Amadeo et
al. 2020, which the paper credits.

One paper could not be settled: Zhang, Lung, Lambadaris and St-Hilaire,
"IoT Data Lifetime-Based Cooperative Caching Scheme for ICN-IoT Networks",
IEEE ICC 2018, 10.1109/ICC.2018.8422100. Its title puts data lifetime at the
center of a caching scheme. Its abstract is elided by the publisher, Semantic
Scholar records it as closed access with no open PDF, and IEEE Xplore did not
serve the text, so **its mechanism is undetermined**. It was therefore **not
cited**: a citation placed in a paragraph that makes a negative claim about a
group of schemes, written from a title, is the exact failure that let the FDC
overlap stand. What protects the paper here is that it no longer claims
priority for the expiry rule at all, so an unread prior work cannot falsify a
claim the paper does not make.
