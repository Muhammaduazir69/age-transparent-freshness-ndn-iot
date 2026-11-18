"""Cross-seed assignment control: each node's majority expert over SHADE's
development runs (seeds 0-4 of the selected setting), applied unchanged to
the evaluation seeds. Writes results/assigned_dev_majority.json."""
import collections, json, re, sys
from pathlib import Path

src = Path(sys.argv[1] if len(sys.argv) > 1 else "../results/tune2")
prefix = sys.argv[2] if len(sys.argv) > 2 else "DevB6-lam100"
pat = re.compile(r"scalar ShadeCampus\.sensor\[(\d+)\]\.app\[0\]\.ndn\.processor R\d+:E(\d+):activeEpochs (\S+)")
tot = collections.defaultdict(lambda: collections.Counter())
files = sorted(src.glob(f"{prefix}-[0-4].sca"))
for f in files:
    for m in pat.finditer(f.read_text()):
        tot[int(m.group(1))][int(m.group(2))] += float(m.group(3))
n = 36
assign = [tot[v].most_common(1)[0][0] if tot[v] else 0 for v in range(n)]
out = Path(__file__).resolve().parents[2] / "results" / "assigned_dev_majority.json"
json.dump({"source": [f.name for f in files], "assignment": assign}, open(out, "w"), indent=1)
print(len(files), "dev runs ->", assign)
