#!/usr/bin/env python3
"""Audit every ini assignment in a campaign against the value the model used.

An OMNeT++ .sca records both sides of every parameter:

    config *.sensor[*].app[0].consumer.validityScale 2.0      <- requested
    par ShadeCampus.sensor[0].app[0].consumer validityScale 1 <- effective

They can disagree in silence, because a NED assignment written without
default() is final and outranks the ini without warning. That is how 60 runs of
the under-declaration control came to score staleness against the wrong
lifetime while every other gate stayed green.

Each `config` pattern is turned into a regex over the full `<module>.<param>`
path, so a parameter is only ever compared against the modules the pattern
actually names. Assignments whose right-hand side is an expression (parentIndex,
uniform, xml) are skipped: they are meant to differ per module.

Usage: check_effective_params.py <results_dir> [...]
Exit 1 if a requested literal never reached any module the pattern names.
"""
import collections
import glob
import gzip
import os
import re
import sys

SKIP_OPTION = re.compile(
    r"^(sim-time-limit|network|result-dir|output-|cmdenv|repeat|seed-set|"
    r"num-rngs|record-|description|extends|abstract|\*\*\.vector-recording)")
EXPRESSION = re.compile(r"parentIndex|uniform|exponential|normal|intuniform|"
                        r"xml\(|\$\{|index|sizeof|ancestorIndex")
UNIT = re.compile(r"(-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)\s*[A-Za-z%]*$")


def opener(path):
    return gzip.open(path, "rt", errors="replace") if path.endswith(".gz") \
        else open(path, errors="replace")


def to_regex(pattern):
    """ini module pattern -> regex over '<modulePath>.<paramName>'."""
    out, i = [], 0
    while i < len(pattern):
        c = pattern[i]
        if pattern.startswith("**", i):
            out.append(r".*"); i += 2
        elif c == "*":
            out.append(r"[^.]*"); i += 1
        elif pattern.startswith("[*]", i):
            out.append(r"\[\d+\]"); i += 3
        else:
            out.append(re.escape(c)); i += 1
    # the leading network name is not written in the ini pattern
    return re.compile(r"^(?:[A-Za-z_]\w*\.)?" + "".join(out) + r"$")


def clean(s):
    """.sca stores string parameters quoted and escaped, sometimes twice."""
    s = s.strip()
    for _ in range(4):
        before = s
        s = s.replace('\\"', '"')
        if len(s) > 1 and s[0] == '"' and s[-1] == '"':
            s = s[1:-1]
        s = s.strip()
        if s == before:
            break
    return s


def as_num(s):
    m = UNIT.fullmatch(clean(s))
    return float(m.group(1)) if m else None


def scan(path):
    req, eff = {}, []
    with opener(path) as fh:
        for line in fh:
            if line.startswith("config "):
                p = line.split(None, 2)
                if len(p) == 3:
                    # A pattern can be assigned twice, once in a base section
                    # and once in the section that extends it (the capacity
                    # sweep does exactly that). Keep both: the effective value
                    # need only match one of them.
                    req.setdefault(p[1], []).append(p[2].strip())
            elif line.startswith("par "):
                p = line.split(None, 3)
                if len(p) == 4:
                    eff.append((f"{p[1]}.{p[2]}", p[3].strip()))
    return req, eff


SCALE = {"": 1.0, "s": 1.0, "ms": 1e-3, "us": 1e-6, "ns": 1e-9,
         "W": 1.0, "mW": 1e-3, "uW": 1e-6, "J": 1.0, "mJ": 1e-3,
         "m": 1.0, "km": 1e3, "B": 1.0, "KiB": 1024.0, "MiB": 1048576.0,
         "bps": 1.0, "kbps": 1e3, "Mbps": 1e6}
NUMUNIT = re.compile(r"(-?\d+(?:\.\d+)?(?:[eE][-+]?\d+)?)\s*([A-Za-z]*)$")


def quantity(s):
    """Value in base units, or None when it is not a plain quantity."""
    m = NUMUNIT.fullmatch(clean(s))
    if not m:
        return None
    unit = m.group(2)
    if unit not in SCALE:
        return None                      # dBm and friends: not comparable
    return float(m.group(1)) * SCALE[unit]


def same(want, got):
    """Does an effective value agree with what the ini asked for?"""
    w, g = clean(want), clean(got)
    if w == g:
        return True
    if w.lower() in ("true", "false") or g.lower() in ("true", "false"):
        return w.lower() == g.lower()
    # typename: the ini may give a short name, the model records the full path
    if "." in g and g.rsplit(".", 1)[-1] == w.rsplit(".", 1)[-1]:
        return True
    qw, qg = quantity(w), quantity(g)
    if qw is not None and qg is not None:
        return abs(qw - qg) <= 1e-9 * max(1.0, abs(qw))
    return False


def check(path):
    """For each parameter the model actually used, find the FIRST ini pattern
    that names it, which is the one OMNeT++ applies, and compare."""
    req, eff = scan(path)
    usable = [(pat, vals, to_regex(pat)) for pat, vals in req.items()
              if not SKIP_OPTION.match(pat) and "." in pat]
    bad = {}
    for name, got in eff:
        for pat, wants, rx in usable:
            if not rx.match(name):
                continue
            if any(EXPRESSION.search(w) for w in wants):
                break                    # per-module by design
            if not any(same(w, got) for w in wants):
                shown = " | ".join(clean(w) for w in wants)
                hard = any(quantity(w) is not None
                           or clean(w).lower() in ("true", "false") for w in wants)
                bad.setdefault((pat, shown, hard), set()).add(clean(got))
            break
    return [(p, w, h, sorted(v)[:4]) for (p, w, h), v in bad.items()]


def main(dirs, verbose=False):
    seen = fails = 0
    for d in dirs:
        byconf = {}
        for p in sorted(glob.glob(os.path.join(d, "*.sca")) +
                        glob.glob(os.path.join(d, "*.sca.gz"))):
            cfg = re.sub(r"-\d+\.sca(\.gz)?$", "", os.path.basename(p))
            byconf.setdefault(cfg, p)
        for cfg, p in sorted(byconf.items()):
            seen += 1
            bad = check(p)
            hard = [b for b in bad if b[2]]
            if hard:
                fails += 1
            for pattern, want, is_hard, got in bad:
                tag = "IGNORED" if is_hard else "note   "
                if is_hard or verbose:
                    print(f"{tag} {cfg}: {pattern} = {want}  ->  effective {got}")
    print(f"configurations audited: {seen}   "
          f"with an ignored numeric/boolean assignment: {fails}")
    return 1 if fails else 0


if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if a != "-v"]
    sys.exit(main(args or ["."], "-v" in sys.argv))
