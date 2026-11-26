# Equation = code, checked against the source that produced the campaign

Binary A md5 `443512d98a0149cd11eb078fc5e4c450`, binary B md5
`46c35bc964211b42f79b959de9d2a66e` (`results/BINARY_PIN.txt`). First checked
18 November 2026 against A; rechecked 19 November against B after the
clock-free realization, the `nominalFreshness` field and `validityScale` were
added. Line numbers below are binary B's. The two builds are proved to produce
identical results on the configurations they share; see
`binary_equivalence.md`.

## Eq. (1): delivered age, `A = t - t_0`

`NdnConsumerApp.cc:244`: `simtime_t age = simTime() - data->getOriginTime();`
`originTime` is set once, by the publisher, in `NdnProducerApp.cc`
(`data->setOriginTime(simTime())`), and is never rewritten in the forwarding
path (grep for `setOriginTime` returns the producer and the test driver only).

The lifetime the age is judged against is read from a field the forwarding path
never touches. `NdnConsumerApp.cc:255`:

```cpp
const double nominalLife = data->getNominalFreshness() > SIMTIME_ZERO
                               ? data->getNominalFreshness().dbl()
                               : data->getFreshnessPeriod().dbl();
const double appLife = nominalLife * validityScale;
...
bool fresh = appLife <= 0 || age.dbl() <= appLife;
```

`nominalFreshness` is written once, by the publisher, beside the declared
period, and nothing else writes it. This separation exists because the
clock-free realization rewrites `freshnessPeriod` in flight to carry the
residual lifetime; reading that field here would shrink the yardstick in step
with the mechanism being measured. `validityScale` is 1 everywhere except the
under-declaration control of Section 6.4, where the publisher declares `1/k` of
nominal and `validityScale = k` restores the nominal value for the test, so
that shortening the declared lifetime cannot move the definition of stale along
with it. With `validityScale = 1` and `nominalFreshness = F` the test reduces to
`age <= F`, which is Eq. (1) unchanged. Matches.

## Eq. (2): `A = sum_m W_m + tau`, with M the hand-off count

`M` is the `restamps` field. It is incremented at exactly one site,
`CS.cc:501`, on the cache-hit path:
`served->setRestamps(entry->data->getRestamps() + 1);`
One site means one increment per Content Store that answers, so `M` counts
hand-offs and nothing else. The identity itself is an accounting statement
about where the copy spent its time and needs no further code support.

Empirical check of the bound in Eq. (3): deliveries with `M = 0` are stale in
0.00% of cases and `M = 1` in 0.05%, against 12.33% at `M = 2`. Proposition 1
says a stale delivery beyond the in-flight term needs `M >= 2`, and the
measurement agrees to two decimal places.

## Eq. (5): standard expiry, `e_std = t_a + F`

`CS.h`, `CSEntry` constructor: `expiryTime = simTime() + d->getFreshnessPeriod();`
`simTime()` at construction is the local arrival instant `t_a`. Matches, and it
matches the specification's own wording (wait `F` after arrival).

## Eq. (6): age-transparent expiry, `e_at = t_a + max(0, F - a)`

`CS.h`: `originExpiryTime = origin + d->getFreshnessPeriod();` with
`origin = d->getOriginTime()`, so `originExpiryTime = t_0 + F`.
`CS.cc:198`, `insertEntry`: `if (ageTransparent && !ageDecrement &&
entry->originExpiryTime > SIMTIME_ZERO) entry->expiryTime =
entry->originExpiryTime;`

Algebra: `a = t_a - t_0`, so `t_a + (F - a) = t_a + F - t_a + t_0 = t_0 + F`.
The code sets exactly that. The `max(0, .)` is realized rather than written: if
`t_0 + F < t_a` the entry is constructed already past its expiry and
`isExpired()` is true on the first lookup, which is what a clamp at zero means.
Matches.

## Eq. (7): the reporting identity

`valid cache service = cache service - stale cache hits`, with all three over
the same denominator (Data packets received). `figures_src/campaign.py`,
`run_metrics`: `cache_service = fromCache/sat`,
`valid_cache_service = freshFromCache/sat`,
`stale_cache_hits = (fromCache - freshFromCache)/sat`. True by construction.

Note: the shared collector `python/shade/collect.py` divides
`valid_cache_service` by Interests **issued** while dividing the other two by
Interests **satisfied**, so its three fields do not satisfy the identity. The
manuscript does not use those fields; `campaign.py` recomputes all of them.

## What `MustBeFresh` would change: nothing

`CS::handleLookupRequest` treats `entry->isExpired()` as a miss unconditionally,
so a store never answers with a copy its own clock has expired. That is the
behavior `MustBeFresh` mandates, applied to every Interest whether or not the
selector is set (`mustBeFresh` is never set in any configuration). Every stale
delivery reported here was therefore fresh at the node that served it, and the
selector cannot remove it.

## Eq. (6b): the clock-free realization

Selected by `cs.ageMode = "decrement"`, which sets `ageDecrement`
(`CS.cc:44`). Insertion then takes no special path: the entry constructor's
`expiryTime = simTime() + d->getFreshnessPeriod()` is already correct, because
the arriving `freshnessPeriod` carries the residual lifetime rather than the
nominal one. The correction is applied on the way out instead,
`CS.cc:509`, on the cache-hit path:

```cpp
if (ageTransparent && ageDecrement) {
    const simtime_t residual = entry->expiryTime - simTime();
    served->setFreshnessPeriod(residual > SIMTIME_ZERO ? residual : SIMTIME_ZERO);
}
```

An expired entry is never served, because the lookup above drops it, so the
residual is strictly positive at this point and no entry can be handed on with
a zero period (which `isExpired()` would read as "never expires").

The realization is exact only up to the time a copy spends in flight, which a
node cannot subtract because it does not know it. The manuscript measures that
residue rather than assuming it away: the excess over the timestamp form is
0.000 percentage points at zero hand-offs, 0.096 at one and 1.220 at four
(Section 6.7), which is the signature of one transit of error per hand-off.

## A latent case the campaign never reaches

`CSEntry`'s constructor only sets an expiry when the arriving
`FreshnessPeriod` is positive, and `isExpired()` reads `expiryTime > 0 &&
simTime() >= expiryTime`. A copy admitted with a zero period therefore never
expires. On the clock-free path the outgoing residual is clamped at zero, so in
principle a copy handed on at the instant of its expiry could reach the next
store with nothing left and become immortal there.

It cannot happen in these runs. `handleLookupRequest` treats an expired entry
as a miss and removes it before any copy is served, so the residual computed at
`CS.cc:509` is strictly positive whenever that line executes. The recorded
campaign is therefore unaffected, and the bound reported in Section 6.7 is not
an average over any immortal copy.

We are leaving the code as it ran rather than adding a defensive clamp, because
changing it would require rebuilding and revalidating every configuration for a
branch no run reaches. It is recorded here so that the next person to touch the
expiry path adds the guard before the invariant is weakened: any change that
lets an expired entry be served, or that admits a copy without consulting the
lookup path first, makes this reachable.

---

# Build C additions, checked 26 November 2026

Binary C md5 `fb9a649c86fbd42c3b68b381572ce9ef`. Line numbers below are
build C's. Build C is proved to reproduce build B bit for bit on every
configuration that does not name its additions (`binary_equivalence.md`).

## Eq. (2): FDC's residual freshness period, `RFP = F + t_0 - t`

Amadeo et al., *Sensors* 2020, Eq. (2), written there as
`RFP = FP + timestamp - currentTime`.

`ShadeNDNProcessor.cc:286`:

```cpp
rfp = (data->getOriginTime() + fp - now).dbl();
if (rfp < 0) rfp = 0;
```

with `fp = data->getFreshnessPeriod()` and `now = simTime()`. Term for term:
`fp` is F, `getOriginTime()` is the publisher's `t_0`, `now` is `t`. The clamp
at zero is ours and is not in their Eq. (2); it only affects a copy already
past its deadline, which the lookup path never serves, so it cannot change an
admission decision on a servable copy.

The admission probability (their Eq. (3)) and the threshold update (their
Eq. (4), `beta = 0.125`):

```cpp
if (x.fdcTh <= 0)
    admit = true;
else
    admit = rfp >= x.fdcTh || uniform(0, 1) < rfp / x.fdcTh;
if (forReal)
    x.fdcTh = x.fdcTh < 0 ? rfp
                          : (1.0 - fdcBeta) * x.fdcTh + fdcBeta * rfp;
```

Checked against their text on four points. Probability one at or above the
threshold: the first disjunct. Probability `RFP/Th` below it: the second.
Updated after every decision regardless of outcome: the update is outside the
branch. Seeded by the first observation rather than by an arbitrary constant:
the `fdcTh < 0` arm. The `forReal` guard is ours and is required rather than
cosmetic: `admitBy` is also called to maintain the shadow caches of the
inactive policies, and without the guard the average would advance several
times per Data packet. `fdcBeta` is read from the NED parameter, default
`0.125`, and is recorded in every `.sca` as a `config`/`par` line, which
`check_effective_params.py` reads back.

## Eq. (7): age-transparent expiry, both realizations

Timestamp realization, `CS.cc:208`:

```cpp
if (ageTransparent && !ageDecrement && entry->originExpiryTime > SIMTIME_ZERO)
    entry->expiryTime = entry->originExpiryTime;
```

`originExpiryTime` is set in `CSEntry`'s constructor (`CS.h:84`) as
`origin + d->getFreshnessPeriod()`, that is `t_0 + F`, which is the publisher's
deadline and therefore `t_a + (F - a)` with `a = t_a - t_0`.

Clock-free realization, `CS.h:77`:

```cpp
expiryTime = simTime() + (d->getHasResidual()
                              ? d->getResidualFreshness()
                              : d->getFreshnessPeriod());
```

so the local deadline is `t_a + r` when an annotation is present and the
standard `t_a + F` when it is not, which is the fallback an unupgraded sender
produces. The annotation is written on the way out, `CS.cc:195`:

```cpp
const simtime_t residual = entry->expiryTime - simTime();
if (!served->getHasResidual())
    served->setByteLength(served->getByteLength() + residualHeaderBytes);
served->setHasResidual(true);
served->setResidualFreshness(residual > SIMTIME_ZERO ? residual : SIMTIME_ZERO);
```

Three properties the manuscript claims, checked here against this code.

1. **Nothing in the signed portion changes.** `setFreshnessPeriod` no longer
   appears in the clock-free path at all. As of build F,
   `grep -rn setFreshnessPeriod src/` returns three call sites outside the
   generated message code, and none is in the forwarding or caching path of a
   reported run:
   - `NdnProducerApp.cc`, the publisher, which is where `F` is meant to be set;
   - `NdnTestDriver.cc`, a test harness not instantiated in any reported
     configuration;
   - `NDNProcessor::createData`, a helper carrying a hard-coded 10 s lifetime
     which is **never called**: `grep -rn createData src/` returns only its own
     definition. It is left in place and recorded here rather than deleted,
     because deleting it would mean another build and another equivalence
     round for no behavioral gain, and a reviewer grepping for the field should
     be able to see that it was checked rather than missed.

   A fourth call site, `CS::reduceFreshness`, existed until build F. It
   belonged to an unrelated penalization mechanism from a vehicular intrusion
   detection lineage, had no callers anywhere in the tree, and was deleted
   along with `CS::bypassVehicleEntries`. The expiry path writes
   `setResidualFreshness` and `setHasResidual` only.
2. **The four bytes are charged once.** The guard is
   `if (!served->getHasResidual())`, so a copy that arrived already annotated
   does not pay again. A copy from the publisher is unannotated, so the first
   cache that answers with it pays, which is where Section 4.3 says the cost
   falls.
3. **`originExpiryTime` is now correct in both realizations**, because
   `getFreshnessPeriod()` is the publisher's F under every mode. Under build B
   the clock-free path overwrote that field with the residual, so
   `originExpiryTime` was computed from a shrinking value. No configuration
   reported from build B read it on that path (the clock-free arms evict by
   LRU, and the policies that read `originExpiry()` are the LFF-origin and TARA
   families, which never ran with `ageMode = decrement`), but both clock-free
   configurations were rerun on build C regardless rather than argued about.

## Eqs. (9) to (11): the conditional identity and the hypothesis

These are analysis-side, not simulator-side. Eq. (9) is an identity and nothing
computes it. The quantity plotted against Eq. (11) comes from
`campaign.py:restamp_table`, which pools, per hand-off count M, the
`M<k>:delivered`, `M<k>:stale` and `M<k>:ageRatioSum` scalars recorded by each
consumer, and divides the age sum by the delivery count. `E[W_1 | M = 1]` is
that mean at M = 1, which `make_numbers.py` writes as `meanWone`. The dashed
line in Figure 3(b) is `M` times that one number, so the comparison uses no
fitted parameter.

## Eq. (8): the coupling condition

No code. It is the sufficient condition for Hypothesis 1 and the manuscript
states explicitly that it is not claimed. What is measured instead is the mean
form of its two premises, from `restamp_table` at each capacity
(`capMeanM*`, `capMeanW*`).

---

# Builds E and F, 26 November 2026

Two adversarial reviews of the revision found three defects in the clock-free
realization itself. Both builds exist to fix them, and both are recorded here
because the fixes changed what the code does, not only what the text says.

- Binary E `d2098d74f452c62bea79047f19803fcf` (the annotation is link-local)
- Binary F `2e97f807bd788d0d3c7b13b33ec70bc9` (clamp, 8-byte TLV, counters)

## Build E: the annotation is link-local, and behaves like it

**What was wrong.** `CSEntry`'s constructor honored an arriving annotation with
no gate on `ageTransparent`, and nothing ever cleared the field, so a node that
does not implement the rule both honored the residual as its own deadline and
relayed it unchanged. Section 4.2.2, Section 4.3, Table 3 and Section 6.11 each
described a different behavior, and only one of the four matched the code. More
importantly, none of them matched the protocol: an NDNLPv2 header field is
link-local, and a receiver that does not understand an ignorable type ignores
it rather than acting on it.

**What it is now.** `CSEntry` always takes the standard per-hop deadline
(`CS.h:72-79`). `CS::insertEntry` overrides it, and only there, where the mode
is known:

```cpp
if (ageTransparent && !ageDecrement && entry->originExpiryTime > SIMTIME_ZERO)
    entry->expiryTime = entry->originExpiryTime;              // timestamp form
else if (ageTransparent && ageDecrement && data && data->getHasResidual()) {
    ...                                                        // clock-free form
}
```

and `NDNProcessor::processData` strips the field at a node that does not
implement it, so it is not relayed:

```cpp
if (data->getHasResidual()) {
    auto *localCs = dynamic_cast<CS *>(getParentModule()->getSubmodule("cs"));
    if (localCs == nullptr || !localCs->carriesResidualAnnotation()) {
        data->setHasResidual(false);
        data->setResidualFreshness(SIMTIME_ZERO);
    }
}
```

**What it changed in the measurements: nothing.** `Half-ATD-LCE-LRU` was rerun
at all 20 seeds and `compare_sca.py` reports IDENTICAL against the build-B runs
on every seed checked, 0 values differing. The code path never fires in this
topology, and the reason is the geometry: Data flows away from the publisher at
`sensor[35]`, the upgraded nodes are `sensor[0..17]` on the far side of the
field, so an annotated copy served by an upgraded store travels only deeper
into the upgraded region. The result is therefore not sensitive to which
reading is implemented, which Section 6.11 now states. The superseded runs are
in `results/pre_linklocal/`.

## Build F, change 1: the honored residual is clamped at the signed lifetime

**What was wrong.** Section 4.3 claimed that the worst a forged annotation can
buy is a copy living one full declared period per hop, which is standard NDN.
Nothing enforced that. `max{0, x.r}` is a lower clamp only, so an annotation of
`10F` would have been honored in full and relayed.

**What it is now**, in `CS::insertEntry`:

```cpp
simtime_t r = data->getResidualFreshness();
if (r < SIMTIME_ZERO) r = SIMTIME_ZERO;
const simtime_t f = data->getFreshnessPeriod();
entry->expiryTime = simTime() + (r < f ? r : f);
```

`f` is the lifetime the publisher signed, which travels unmodified and is
verifiable, so the ceiling is enforced against a quantity an attacker cannot
move. Algorithm 1 line 3 now reads `e <- t_a + min{x.F, max{0, x.r}}`.

The clamp cannot bind in honest operation, because a residual is what remains
of `F`. That is an argument, so it was tested: see the equivalence table below.

## Build F, change 2: the TLV is 8 bytes, not 4

**What was wrong.** The paper sized the annotation at four bytes, "one byte of
type, one of length and a two-byte millisecond value", and
`residualHeaderBytes` was `4B` in every recorded run. Both halves are wrong
against NDN's own encoding. A TLV-TYPE drawn from the ignorable block
[800, 959] exceeds 252 and so takes three octets under VAR-NUMBER, which the
precedent confirms: `CachePolicy` is type 820 and encodes `FD 03 34`. And a
two-byte millisecond value caps at 65.535 s, while two of the three declared
lifetime classes are 90 and 300 s. A non-negative integer of milliseconds
large enough for 300 s needs four bytes.

Three plus one plus four is eight. `residualHeaderBytes` now defaults to `8B`,
and the three configurations that carry an annotation were rerun at all 20
seeds. The superseded runs are in `results/pre_tlv8/`.

## Build F, change 3: the annotations are counted

The paper reported the number of annotated Data packets per run as the byte
difference between the two arms divided by the TLV size. That is not a count:
the arms are independent runs whose traffic differs for other reasons, and the
quotient came out below the number of cache hits, which is a lower bound. The
Content Store now records `annotationsAttached` and `annotationsCharged`
directly, incremented in `CS::annotateResidual`, and the manuscript reports
those.

## Equivalence across builds E and F

Both builds change the clock-free path and nothing else. Tested rather than
asserted, with `compare_sca.py`:

| build | configuration | seeds | result |
|---|---|---|---|
| E | Fixed-LCE-LRU, AT-LCE-LRU, ATD-LCE-LRU, Fixed-FDCA-LRU | 10, 11 | IDENTICAL, 0 differing (8 runs) |
| E | Half-ATD-LCE-LRU vs build B | 10, 11, 15, 29 | IDENTICAL, 0 differing |
| F | Fixed-LCE-LRU, AT-LCE-LRU, AT-FDCA-LRU, Ver30-LCE-LRU | 10, 11 | see `equiv8.log` |

The build-F list deliberately includes an FDC arm and a versioned arm, because
those are the paths builds C and D added and an equivalence test that omitted
them would not have covered them.
