#!/bin/bash
# Publish this repository to GitHub.
#
# Everything is committed and swept; only the network action is left, and this
# session is not permitted to take it. Run this and the deployment is done.
#
#   ./publish.sh            public  (what the manuscript's Data Availability
#                                    statement assumes)
#   ./publish.sh --private  private (flip later with
#                                    gh repo edit --visibility public)
set -euo pipefail
cd "$(dirname "$0")"

NAME="age-transparent-freshness-ndn-iot"
VIS="--public"
[ "${1:-}" = "--private" ] && VIS="--private"

echo "== pre-push sweep =="
if git log --format='%an <%ae>%n%B' | grep -qiE 'claude|anthropic|co-authored|🤖'; then
    echo "ABORT: attribution trailer found in the commit history"; exit 1
fi
# --exclude the script: the pattern below is itself credential-shaped, and
# without this the sweep matches its own source and refuses to ever publish.
if grep -rqiE 'gho_|ghp_|BEGIN [A-Z ]*PRIVATE KEY' . --exclude-dir=.git \
        --exclude="$(basename "$0")"; then
    echo "ABORT: a credential-shaped string is present"; exit 1
fi
echo "   commit history clean, no credentials"

echo "== creating $NAME ($VIS) =="
gh repo create "$NAME" $VIS --source=. --remote=origin \
   --description "Simulator, scenarios, analysis and dataset for age-transparent freshness in NDN-based IoT caching (OMNeT++ / INET)" \
   --push

URL=$(gh repo view "$NAME" --json url -q .url)
echo
echo "published: $URL"
echo
echo "The manuscript's Data availability statement names:"
echo "   https://github.com/Muhammaduazir69/$NAME"
echo "If the account or repository name differs from that, update the"
echo "Data availability block in discover_computing/convert.py and rebuild."
