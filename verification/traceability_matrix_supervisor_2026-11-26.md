# Traceability matrix, supervisor comments of 26 November 2026

One row per comment. "Where" cites the shipped Springer build; "Evidence" cites
the artifact a reader can check, not a claim. Nothing is answered with a
Limitations sentence: the four comments that needed code changed code, and the
two that needed experiments ran them.

| # | Comment | Action class | Where | Evidence |
|---|---|---|---|---|
| 1 | Differentiate from FDC | rewrite plus new experiments | §2.3 (new), Table 1 (new row and column), §4 opening, §6.8 (new) | Amadeo et al. read in full from Europe PMC, not from its abstract; the expiry rule is credited to them in four places; three differentiators each tied to a measurement |
| 2 | Remove first-study claims | deletion | abstract, §1 bullets, §8 | grep for `first\|novel\|to our knowledge\|no prior work` returns no priority claim; the two offending sentences are quoted in the response letter as removed and the letter checker verifies their absence |
| 3 | Digital signature | **code change** plus new subsection | §4.3 (new), §5.2, Algorithm 1 | the residual moved out of the signed `MetaInfo` into an ignorable NDNLPv2 field; `grep setFreshnessPeriod src/` shows no forwarding-path writer; the field is stripped at non-implementing nodes and clamped at the signed lifetime; 8 B charged to the frame and counted |
| 4 | Separate the two implementations | restructure | §4.2.1, §4.2.2, Table 3 (12 rows), §6.9, §6.10, §7.2 | each realization has its own subsection, its own results, its own failure analysis; the comparison table carries requirements, limitations and measured results |
| 5 | Proposition 2 and Equation (4) | demote and measure | §3.3 Hypothesis 1, Equations (5)–(8), §6.6 | the coupling is written out and explicitly not claimed; both premises and the conclusion measured, the conclusion on mean delivered age; the prediction's falsification condition stated |
| 6 | Unsupported generalizations | retract and scope | §4.4, §6.3, §2.4, §1, §8 | the hit-ratio claim is retracted in the text, not softened; the RL claim is scoped in four places and names two untested alternatives; two further generalizations found later in the round were also corrected |
| 7 | FDC as a direct baseline | **new implementation and campaign** | §6.8, Table 10, plus rows in Tables 8, 9, 11 and the capacity grid | FDC's Equations (2)–(4) implemented and recorded; 2x2 attribution at 20 paired seeds; the comparison recomputed independently by a reviewer agent and matching on all 15 values |
| 8 | Clarify the NDN implementation | new subsection and table | §5.2, Table 5 (15 rows) | every function marked implemented, restricted or absent, with what each restriction costs the claims; each entry checked against the source |
| 9 | Content generation and versioning | new subsection plus **new experiments** | §5.4 (new), §6.5 (new), Table 7 | on-demand generation and the naming convention stated; the alternative convention measured at two version intervals rather than described, with the per-regime split that shows where it stops binding |
| 10 | Verify references, add APJITM | mechanical check plus one correction | refs, §1, `verification/references.md` | 53 entries against three registries on title, year, first author, author count and page range; one wrong page range found and fixed; APJITM added and cited where it belongs |

## Items the comments did not ask for, found while answering them

| Found by | What | Outcome |
|---|---|---|
| hostile methods review | annotation honored and relayed by nodes that do not implement it | code changed, arm rerun |
| hostile methods review | security ceiling not enforced | clamp added, proved inert |
| hostile methods review | TLV sized by estimate, not by the encoding rules | 8 B, three arms rerun |
| hostile methods review | nine claims the data did not support | each corrected against the data |
| verification pass | Springer abstract still the pre-revision one | converter now grabs instead of restating; gated |
| verification pass | a second wrong justification for excluding the compromised sensors | corrected against the consumer app's own junk filter |
| read-through | §2.3 promised the reverse of what §6.8 measures | corrected |
| read-through | wrong pCASTING mechanism, in two sections | corrected with the measured reason |
| read-through | inverted cross-reference range | fixed and gated |
| this audit (T5) | the learned comparator was never specified | §5.3 added from the shipped checkpoint's configuration |

## Open items

None of the ten comments is open.

Two things remain outside this matrix and belong to the author: whether to trim
the 55-page Springer build, and submission. One residue is recorded in
`reviewer_proof_audit_2026-11-26.md`: the signature-verification routine is a
stub, which is disclosed in §5.2 with the direction it cuts and is outside the
paper's claims.
