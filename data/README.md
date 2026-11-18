# Dataset: age-transparent freshness in NDN-based IoT networks

Every number in the manuscript is reduced from these files.

## per_run_metrics.csv
One row per simulation run. `campaign` is `main` (the single-sink field),
`field` (the distributed field), or `cap5` / `cap20` (the capacity grid).
`config` is the OMNeT++ configuration name; `seed` is the RNG seed, which also
fixes the workload, so a configuration and its age-transparent counterpart at
the same seed see exactly the same Interests.

All delivery-side rates share one denominator, the Data packets a consumer
received. `satisfaction` is the exception and is per Interest issued.
Compromised nodes (0, 5, 30) are excluded from every consumer-side rate.

| column | meaning |
|---|---|
| `cache_service` | share of deliveries answered by a Content Store |
| `valid_cache_service` | share answered by a Content Store and still inside the publisher's lifetime |
| `stale_delivery` | share delivered past the publisher's lifetime |
| `stale_cache_hits` | `cache_service` minus `valid_cache_service` |
| `latency_ms` | mean Interest-to-Data interval, over satisfied Interests |
| `energy_J` | field-wide radio energy drawn, 36 nodes |

## handoff_breakdown.csv
Deliveries grouped by how many Content Stores answered with the copy before it
arrived. This is the paper's independent variable. The counter saturates, so
the highest bucket means "that many hand-offs or more" and is flagged.

## depth_breakdown.csv
Deliveries grouped by the consumer's hop distance from the publisher, measured
from the forwarding state actually installed, and split by operating regime
(0 stationary, 1 drift, 2 pollution, 3 flash).

## Reproducing
`figures_src/regen.sh <results_dir>` recomputes every macro, table and figure
from the raw scalar files and rebuilds the manuscript. These CSVs are the same
reduction, exported for readers who do not want to run the simulator.
