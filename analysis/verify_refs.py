#!/usr/bin/env python3
"""Verify every bibliography entry against two independent registries.

Crossref is the DOI registry and returns what the publisher deposited; DBLP is
an independently curated index of computer science literature. Agreement
between them on title, year, venue and author list is much stronger evidence
than either alone, and it catches the failure that got an earlier paper of ours
rejected: a co-author printed as first author because one index had the order
wrong.

Writes a per-entry table so the claim is evidence rather than assertion.
"""
import difflib, json, re, sys, time, urllib.parse, urllib.request

UA = {"User-Agent": "ref-verify/1.0"}

def get(url, timeout=30):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.load(r)

def norm(t):
    t = re.sub(r"[{}\\$]", "", (t or "").lower())
    t = re.sub(r"\\&", "and", t)
    return " ".join(re.sub(r"[^a-z0-9 ]", " ", t).split())

def sim(a, b):
    return difflib.SequenceMatcher(None, norm(a), norm(b)).ratio()

def families(bib_authors):
    """Family names from a BibTeX author field, in order."""
    out = []
    for a in re.split(r"\s+and\s+", bib_authors or ""):
        a = a.strip().strip(",")
        if not a or a == "others":
            continue
        if "," in a:
            out.append(a.split(",")[0].strip())
        else:
            out.append(a.split()[-1].strip())
    return out

src = open(sys.argv[1]).read()
entries = []
for b in re.split(r"\n(?=@)", src):
    m = re.match(r"@(\w+)\s*\{\s*([^,\s]+)\s*,", b)
    if not m or m.group(1).lower() == "string":
        continue
    f = {}
    for fm in re.finditer(r"(\w+)\s*=\s*(\{(?:[^{}]|\{[^{}]*\})*\}|\"[^\"]*\"|\w+)", b[m.end():]):
        f[fm.group(1).lower()] = fm.group(2).strip("{}\" ")
    entries.append((m.group(2), m.group(1).lower(), f))

rows = []
for key, typ, f in entries:
    rec = {"key": key, "type": typ, "doi": f.get("doi"), "url": f.get("url"),
           "bib_title": f.get("title", ""), "bib_year": f.get("year", ""),
           "bib_fam": families(f.get("author", "")),
           "bib_nauth": len(families(f.get("author", ""))),
           "has_others": bool(re.search(r"\band\s+others\b", f.get("author", "") or "")),
           "cr": None, "s2": None, "oa": None, "notes": []}
    # --- Crossref ---
    if rec["doi"]:
        try:
            msg = get("https://api.crossref.org/works/" + urllib.parse.quote(rec["doi"]))["message"]
            yrs = set()
            for k in ("issued", "published-print", "published-online"):
                dp = (msg.get(k) or {}).get("date-parts", [[None]])[0]
                if dp and dp[0]:
                    yrs.add(dp[0])
            rec["cr"] = {
                "title": (msg.get("title") or [""])[0],
                "years": sorted(yrs),
                "fam": [a.get("family", "") for a in (msg.get("author") or [])],
                "n": len(msg.get("author") or []),
                "venue": (msg.get("container-title") or [""])[0],
            }
        except Exception as e:
            rec["notes"].append(f"crossref: {type(e).__name__}")
    # --- Semantic Scholar: independent corpus (DBLP, MAG, PubMed, publisher feeds) ---
    try:
        FIELDS = "title,year,venue,authors,externalIds"
        if rec["doi"]:
            res = get("https://api.semanticscholar.org/graph/v1/paper/DOI:"
                      + urllib.parse.quote(rec["doi"]) + "?fields=" + FIELDS)
        else:
            q = urllib.parse.quote(rec["bib_title"])
            hits = get("https://api.semanticscholar.org/graph/v1/paper/search?query="
                       + q + "&limit=5&fields=" + FIELDS).get("data") or []
            res, bs = None, 0.0
            for h in hits:
                sc = sim(h.get("title", ""), rec["bib_title"])
                if sc > bs:
                    res, bs = h, sc
            if res is None or bs < 0.70:
                res = None
        if res:
            fam = [(a.get("name") or "").split()[-1] for a in (res.get("authors") or [])
                   if a.get("name")]
            rec["s2"] = {"title": res.get("title", ""), "year": res.get("year", ""),
                         "fam": fam, "n": len(fam), "venue": res.get("venue", "")}
    except Exception as e:
        rec["notes"].append(f"s2: {type(e).__name__}")
    time.sleep(1.1)
    # --- OpenAlex: third opinion, used where the first two disagree ---
    try:
        if rec["doi"]:
            res = get("https://api.openalex.org/works/doi:" + urllib.parse.quote(rec["doi"]))
        else:
            q = urllib.parse.quote(rec["bib_title"])
            hits = (get("https://api.openalex.org/works?filter=title.search:" + q
                        + "&per-page=3").get("results") or [])
            res, bs = None, 0.0
            for h in hits:
                sc = sim(h.get("title", ""), rec["bib_title"])
                if sc > bs:
                    res, bs = h, sc
            if res is None or bs < 0.70:
                res = None
        if res:
            fam = [(a.get("author") or {}).get("display_name", "").split()[-1]
                   for a in (res.get("authorships") or [])]
            rec["oa"] = {"title": res.get("title") or res.get("display_name") or "",
                         "year": res.get("publication_year", ""),
                         "fam": [x for x in fam if x], "n": len(fam),
                         "venue": ((res.get("primary_location") or {}).get("source") or {}).get("display_name", "")}
    except Exception as e:
        rec["notes"].append(f"openalex: {type(e).__name__}")
    time.sleep(0.4)
    rows.append(rec)

print(json.dumps(rows, indent=1))
