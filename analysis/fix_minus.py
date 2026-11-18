"""Typeset negative numbers in the generated macros and tables with a true minus sign.

The generators format signed values with Python's "-", which LaTeX prints as a
hyphen. \\ensuremath{-} gives the math minus in text and in math mode alike.
"""
import pathlib, re

MS = pathlib.Path(__file__).resolve().parent.parent
PAT = re.compile(r"(?<![\w\\{}=$-])-(?=\d)")
for f in sorted(MS.glob("numbers*.tex")) + sorted(MS.glob("tab_*.tex")):
    s = f.read_text()
    out = []
    for line in s.splitlines(keepends=True):
        if line.lstrip().startswith("%"):
            out.append(line); continue
        if f.name.startswith("numbers"):
            m = re.match(r"(\\newcommand\{\\\w+\}\{)(.*)(\}\s*)$", line, re.S)
            if m:
                line = m.group(1) + PAT.sub(r"\\ensuremath{-}", m.group(2)) + m.group(3)
        else:
            line = PAT.sub(r"\\ensuremath{-}", line)
        out.append(line)
    t = "".join(out)
    if t != s:
        f.write_text(t)
        print("minus fixed in", f.name)
