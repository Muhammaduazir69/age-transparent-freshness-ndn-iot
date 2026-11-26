# Binary equivalence: may results from the two builds be pooled?

The campaign spans two builds of the simulator. Binary A produced
`results/v2_main` for the configurations that existed before the clock-free
realization was added. Binary B adds `cs.ageMode` (origin vs decrement), the
`nominalFreshness` packet field and the consumer's `validityScale`, and
produced the field scenario, the learned comparators, the under-declaration
controls, the partial-deployment arms and ten seeds of `Fixed-DOOR-LRU`.

Pooling the two is only legitimate if B reproduces A exactly on a configuration
they share. That is a testable claim, so it was tested rather than asserted.
`python/compare_sca.py` compares two `.sca` files by recorded scalar and
statistic values alone, ignoring run metadata such as timestamps and process
ids.

| date | config | seed | recorded values | differing |
|---|---|---|---|---|
| 18 Nov 2026 | `Fixed-LCE-LRU` | 10 | 17597 | 0 |
| 18 Nov 2026 | `Fixed-LCE-LRU` | 10 | 17597 | 0 (after the NdnOverlayApp.ned change) |
| 19 Nov 2026 | `Fixed-DOOR-LRU` | 10 | 17612 | 0 |

The second row matters because `NdnOverlayApp.ned` was edited after binary B was
built, changing `validityScale = parent.validityScale` to
`default(parent.validityScale)`. NED is parsed at run time, so that edit needs
no rebuild but can still change behavior; it does not.

The third row covers the one arm whose seeds span both builds:
`Fixed-DOOR-LRU` seeds 10 to 19 came from A and seeds 20 to 29 from B, after ten
header-only `.sca` stubs were found and rerun.

Binary A md5 `443512d98a0149cd11eb078fc5e4c450`,
binary B md5 `46c35bc964211b42f79b959de9d2a66e`, both kept under
`results/binaries/`.

## Build C, 26 November 2026

Build C adds the FDC admission rule (`experts = "FDC-LRU"`), the hop-by-hop
residual-lifetime annotation that replaces the in-place rewrite of
`FreshnessPeriod`, and the `residualHeaderBytes` parameter that charges the
annotation's four bytes to the frame.

- Binary B `46c35bc964211b42f79b959de9d2a66e`
- Binary C `fb9a649c86fbd42c3b68b381572ce9ef`

Both of the parameters build C adds default to build B's behavior, and the new
admission rule is reachable only from a configuration that names it, so the
expectation is that every previously recorded configuration is untouched.
Tested rather than assumed, with `python/compare_sca.py`, which compares
recorded values only and ignores run metadata:

| configuration | seed | recorded values | differing |
|---|---|---|---|
| Fixed-LCE-LRU | 10 | 17597 | 0 |
| Fixed-LCE-LRU | 11 | 17612 | 0 |
| AT-LCE-LRU | 10 | 17558 | 0 |
| AT-LCE-LRU | 11 | 17516 | 0 |

70283 recorded values, 0 differing, 0 present in only one file. IDENTICAL on
all four runs, so the 700 runs of `results/v2_main` recorded under build B are
combined with build C's additions.

Two configurations were deliberately NOT carried over, because the change does
affect them: `ATD-LCE-LRU` and `Half-ATD-LCE-LRU` use the clock-free path,
which now carries the residual in its own annotation and pays four bytes per
frame for it. Their build-B runs were moved to `results/pre_signature_fix/`
and both configurations were rerun in full on build C.

## Build D, 26 November 2026

Build D changes one thing: the content index that fixes a reading's freshness
class is now parsed from the number following the last `item` component of the
name, rather than from the trailing digits of the whole name. The versioned
arm of Section 6.5 appends a `/v<k>` component, under which the old parse would
have read the version number as the item index and given every item in a time
slot the same lifetime.

- Binary C `fb9a649c86fbd42c3b68b381572ce9ef`
- Binary D `15183423ac680ba7923f435c36375e3f`

For every name reported before the versioned arm the two parses agree by
construction, since those names end in `item<k>`, and the compromised sensors'
junk names contain no `item` component and so take the same trailing-digit
fallback. That is an argument, not a test, so it was tested:

| configuration | seeds | result |
|---|---|---|
| Fixed-LCE-LRU | 10, 11 | IDENTICAL, 0 differing |
| AT-LCE-LRU | 10, 11 | IDENTICAL, 0 differing |
| Fixed-FDCA-LRU | 10, 11 | IDENTICAL, 0 differing |

Six runs, 0 values present in only one file, 0 differing. The third
configuration was added to this round deliberately: build C's additions are
reachable only from it, so an equivalence test that omitted it would not have
covered the FDC path at all.

## Builds E, F and G, 26 November 2026

Recorded here as well as in `equation_code.md`, because this file is the one a
reader looks in for the equivalence protocol.

| build | md5 | what it changed |
|---|---|---|
| E | `d2098d74f452c62bea79047f19803fcf` | the residual annotation is link-local: a node that does not implement it neither honors nor relays it |
| F | `2e97f807bd788d0d3c7b13b33ec70bc9` | the honored residual is clamped at the signed lifetime; the TLV is sized at 8 B; two attach counters per store |
| G | `44bb62d420b300d450d5560dd4ec5d90` | comments and one inert member initializer only |

**Build E.** Four configurations at two seeds each in `results/equiv7`.
`Fixed-LCE-LRU`, `AT-LCE-LRU` and `Fixed-FDCA-LRU` are IDENTICAL against the
build-D runs, 67242 recorded values across six runs, none differing.
`ATD-LCE-LRU` cannot be compared with the strict test, because the copy in
`v2_main` has since been rerun on build F and carries two scalars build E does
not record; the semantics change it embodies was instead tested directly, by
rerunning `Half-ATD-LCE-LRU` and comparing against the pre-change runs in
`results/pre_linklocal`: IDENTICAL on every seed, 0 differing. That is the
stronger test, since `Half-ATD` is the only configuration in which the changed
code path could fire at all.

**Build F.** Four configurations at two seeds each in `results/equiv8`. Build F
adds `annotationsAttached` and `annotationsCharged` per node, so each run holds
72 scalars the earlier build does not, and a strict set comparison reports those
as present on one side only. Among the quantities both builds record, 0 values
differ. The clamp is therefore proved inert on the recorded campaign, which is
what it should be: a residual is what remains of `F` and cannot exceed it.

The three configurations that carry an annotation, `ATD-LCE-LRU`,
`ATD-FDCA-LRU` and `Half-ATD-LCE-LRU`, were rerun at all 20 seeds for the
8-byte size rather than carried over. Their 4-byte runs are in
`results/pre_tlv8/`.

**Build G.** Six configurations at two seeds each in `results/equiv9`, chosen
to cover every code path the revision touched: standard NDN, both realizations
of the rule, the half-upgraded field, an FDC arm and a versioned arm. Build G
changes only comments and one member initializer that the NED parameter
overwrites before it is read, so it must be inert; `equiv9.log` and the
comparison record whether it is.
