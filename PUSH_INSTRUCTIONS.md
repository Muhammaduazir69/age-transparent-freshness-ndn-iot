# Publishing this repository

Everything is committed and ready. The push itself needs to come from you,
because creating a public repository is an outward-facing action this session
is not permitted to take on your behalf.

## One command

```bash
cd "$(dirname "$0")"
gh repo create age-transparent-freshness-ndn-iot --public --source=. \
   --remote=origin \
   --description "Simulator, scenarios, analysis and dataset for age-transparent freshness in NDN-based IoT caching (OMNeT++ / INET)" \
   --push
```

Swap `--public` for `--private` if you would rather hold it until the paper is
accepted; the Data Availability statement in the manuscript is written so that
either choice reads correctly, and you can flip it later with
`gh repo edit --visibility public`.

## What has already been checked

- No AI vendor name, attribution trailer or co-author line anywhere in the
  tree or in the commit history.
- No credentials, tokens or personal email addresses.
- No absolute paths from this machine (the generated `src/Makefile` was
  removed; `make makefiles` regenerates it).
- The dataset in `data/` reproduces the manuscript's headline numbers exactly
  (6.18% to 0.01% stale, 43.38% to 49.15% valid cache service).

## After pushing

The manuscript's Data Availability statement names the repository URL. If you
change the repository name, update that statement in `sensors_v2/main.tex`.
