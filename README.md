# Age-Transparent Freshness in NDN-Based IoT Networks

Simulator, scenarios, analysis pipeline and dataset for the manuscript
*Age-Transparent Freshness: Bounding the Age of Cached Sensor Readings in
NDN-Based IoT Networks*, submitted to *Discover Computing* (Springer Nature).

Every number, table and figure in that manuscript is recomputed from recorded
simulation output by the pipeline in `analysis/`; none is typed by hand.

## The problem, in one paragraph

Named Data Networking measures the freshness of a cached object from the moment
that object **arrives at the node holding it**, which the packet format
specification states explicitly. Every Content Store that answers a request
therefore restarts the countdown, and the lifetime already spent upstream is
forgotten. For a sensing deployment this separates the lifetime the network
enforces from the validity the application depends on, and the error compounds
with the number of caches that handle a copy. This repository contains the
measurement of that effect and a rule that removes it: a node expires a cached
copy at the publisher's deadline rather than one lifetime after its own
arrival.

That rule is not new. Freshness-Driven Caching (Amadeo et al., *Sensors* 2020,
20(7), 1985) reaches the same expiry from a producer timestamp and clocks
synchronized to a satellite time source, and FDC is implemented here as a
baseline so that its two halves, a probabilistic admission rule and the expiry
rule, can be run apart and attributed. What this repository adds is a
realization of the expiry rule that a constrained sensing node can run: the
residual lifetime travels hop by hop as an ignorable link-local annotation, so
no two clocks are compared and nothing inside the signed portion of the Data
packet is modified.

## Layout

| path | contents |
|---|---|
| `src/` | the OMNeT++ model: NDN forwarder (PIT, FIB, Content Store), applications, policies |
| `simulations/` | scenario topologies and every configuration reported in the paper |
| `python/` | campaign orchestration, and the bridge for the learned comparators |
| `analysis/` | the analysis pipeline that turns recorded output into every number, table and figure |
| `data/` | the per-run dataset the manuscript is reduced from |

The change the paper proposes lives in `src/ndn/tables/CS.{h,cc,ned}`. It is
small on purpose: `ageTransparent` selects it and `ageMode` chooses the
realization. `origin` reads the publisher's deadline from the packet's
generation time, which is FDC's route and needs a common time base.
`decrement` carries the residual lifetime in its own hop-by-hop annotation
(`hasResidual` and `residualFreshness` in `NdnPackets.msg`), modelling an
ignorable NDNLPv2 header field, and charges its eight bytes to the frame
through `residualHeaderBytes`. Eight is the real TLV size, not an estimate: a
type drawn from the ignorable block [800, 959] takes three octets under NDN's
VAR-NUMBER rule, the length takes one, and a millisecond value large enough for
the 300 s lifetime class takes four. The signed `FreshnessPeriod` is never
rewritten under either mode.

Two properties of the annotation are enforced in the code rather than only
argued for in the paper. It is link-local: a node that does not implement it
neither honors it nor relays it, which `NDNProcessor::processData` effects by
stripping the field. And an honored residual is clamped at the lifetime the
publisher signed, so a forged annotation can buy a copy at most one full
declared period at the next hop, which is what standard NDN already gives. The
Content Store records `annotationsAttached` and `annotationsCharged` so the
on-wire cost is counted rather than inferred.

The FDC baseline lives in `src/shade/ShadeNDNProcessor.cc`, as the `FDC`
admission rule, implementing Equations (2) to (4) of that paper with the
authors' threshold weight of 0.125. Select it with
`**.ndn.processor.experts = "FDC-LRU"`.

| configuration | what it is |
|---|---|
| `Fixed-LCE-LRU` | standard NDN, leave-copy-everywhere with LRU |
| `AT-LCE-LRU` | the expiry rule, timestamp realization |
| `ATD-LCE-LRU` | the expiry rule, clock-free realization |
| `Fixed-FDCA-LRU` | FDC's admission rule on the standard per-hop expiry |
| `AT-FDCA-LRU` | FDC as published, both halves |
| `ATD-FDCA-LRU` | FDC's admission rule on the clock-free expiry |
| `Ver30-LCE-LRU`, `Ver90-LCE-LRU` | versioned naming, 30 and 90 s version intervals |
| `Short2/3/4-LCE-LRU` | a publisher under-declaring the lifetime by 2, 3 and 4 |
| `Half-AT-LCE-LRU`, `Half-ATD-LCE-LRU` | the rule at half the field |
| `C5/C20/C40-*` | the Content Store capacity sweep |
| `Field-*` | the second scenario, every node publishing |

## Requirements

OMNeT++ 6.4 and INET 4.6. The learned comparators additionally need PyTorch and
pyzmq; everything else runs without them.

## Reproducing the paper

```bash
# build
make makefiles && make MODE=release -j$(nproc)

# one configuration, one seed
simulations/run -c Fixed-LCE-LRU -r 10 -f simulations/configs/omnetpp.ini

# a campaign
cd python && python3 -m shade.run_matrix \
    --configs Fixed-LCE-LRU AT-LCE-LRU --seeds 10-29 --out ../results/main --jobs 10

# every number, table and figure in the manuscript
analysis/regen.sh <results_dir>
```

`analysis/regen.sh` also runs the checks the manuscript depends on: that no ini
assignment was silently ignored, that no result file is a truncated stub, that
no quantity the pipeline computes is left unreported, and that the rendered PDF
contains no placeholder.

## Dataset

`data/` holds the reduction behind every reported number: one row per
simulation run across all 1480 runs, plus the hand-off and hop-distance
breakdowns the mechanism sections rest on. See `data/README.md` for the schema
and the denominator conventions. The raw OMNeT++ scalar files are 5.4 GB and
are available from the authors.

## License

MIT for the code in this repository. OMNeT++ and INET carry their own licenses.
