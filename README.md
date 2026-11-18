# Age-Transparent Freshness in NDN-Based IoT Networks

Simulator, scenarios, analysis pipeline and dataset for the manuscript
*Age-Transparent Freshness: Bounding the Age of Cached Sensor Readings in
NDN-Based IoT Networks*.

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

## Layout

| path | contents |
|---|---|
| `src/` | the OMNeT++ model: NDN forwarder (PIT, FIB, Content Store), applications, policies |
| `simulations/` | scenario topologies and every configuration reported in the paper |
| `python/` | campaign orchestration, and the bridge for the learned comparators |
| `analysis/` | the analysis pipeline that turns recorded output into every number, table and figure |
| `data/` | the per-run dataset the manuscript is reduced from |

The change the paper proposes lives in `src/ndn/tables/CS.{h,cc,ned}`. It is
small on purpose: `ageTransparent` selects it, and `ageMode` chooses between the
two realizations, one reading the publisher's deadline from the packet and one
carrying the residual lifetime in the existing `FreshnessPeriod` field so that
no clocks need to agree.

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
simulation run across all 1300 runs, plus the hand-off and hop-distance
breakdowns the mechanism sections rest on. See `data/README.md` for the schema
and the denominator conventions. The raw OMNeT++ scalar files are 5.4 GB and
are available from the authors.

## License

MIT for the code in this repository. OMNeT++ and INET carry their own licenses.
