#!/bin/bash
# Recompute every quantity the manuscript reports from a recorded campaign,
# and run the checks that the reported numbers depend on.
#
# This is the data half of the paper's build. The manuscript's own text gates
# (spelling, placeholders, cross-references) are not here, because they operate
# on LaTeX sources that are not part of this repository.
set -u
RES="${1:?usage: reproduce.sh <results_dir>}"
HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$HERE"

echo "== per-run dataset =="
python3 make_dataset.py "$(dirname "$RES")" ../data

echo "== did every ini assignment reach the module it named? =="
# A NED assignment written without default() is final and silently outranks the
# ini, which is how one control once measured the wrong quantity in 60 runs.
python3 check_effective_params.py "$RES" || true

echo "== is any result file a truncated stub? =="
# A run killed partway still writes a header; judging completeness by file size
# lets those through and silently halves a configuration's seed count.
STUBS=0
for F in "$RES"/*.sca; do
    [ -e "$F" ] || continue
    N=$(grep -cE '^(scalar|statistic) ' "$F" 2>/dev/null || echo 0)
    [ "$N" -lt 100 ] && { echo "  STUB $F ($N recorded values)"; STUBS=$((STUBS+1)); }
done
[ "$STUBS" -eq 0 ] && echo "  none" || echo "  FAIL: $STUBS truncated"

echo "== headline, recomputed =="
python3 - "$RES" <<'PY'
import sys, statistics as st
from pathlib import Path
sys.path.insert(0, ".")
import campaign as C
d = Path(sys.argv[1])
for cfg in ("Fixed-LCE-LRU", "AT-LCE-LRU"):
    m = C.config_metrics(d, cfg)
    if not m:
        print(f"  {cfg}: no runs"); continue
    print(f"  {cfg:16s} n={len(m):3d}  "
          f"stale={100*st.mean(v['stale_delivery'] for v in m.values()):6.2f}%  "
          f"valid cache service={100*st.mean(v['valid_cache_service'] for v in m.values()):6.2f}%")
PY
