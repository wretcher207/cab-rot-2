#!/usr/bin/env python3
"""Verify that every `File.ext:N` / `File.ext:N-M` citation in the design docs
points at a line that actually exists in the referenced source file.

This exists because the 2026-08-10 hostile review found that the facelift
report's most confident claims were the least reproducible ones. A spec that
cites line numbers is only trustworthy if the citations are checked; line
numbers drift every time someone edits a source file.

Usage:  python tools/verify-doc-citations.py
Exit 0 = every citation resolves. Exit 1 = at least one is stale/dangling.

Also verifies the palette and the core geometry/typography rules against the
binding brand brief, when that brief can be found. The brief lives OUTSIDE
this repository, so its location is discovered in this order:

  1. $DPD_BRAND_BRIEF          (full path to AI-BRAND-BRIEF.md)
  2. $DPD_BRAND_KIT            (directory containing brand-kit/)
  3. a few conventional sibling locations near the repo

If the brief cannot be found, those checks are reported as SKIPPED rather
than passed. A check that silently passes when its input is missing is
exactly the failure mode this script exists to prevent.

  python tools/verify-doc-citations.py --self-test

runs the script's own negative controls: it proves the checker fails on a
dangling line number and on drifted anchor content, without touching any
tracked file.

Limits, stated honestly: this proves each cited line EXISTS, not that it says
what the doc claims. A `--content` spot-check for a handful of anchor claims
is included below and is the part that actually catches semantic drift.
"""

import os
import re
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = ["design/CANONICAL-UI.md", "design/FACELIFT-REPORT.md"]

# `Source/UI/WaspMeter.cpp:120` or bare `WaspMeter.cpp:99-107`
CITE = re.compile(r"`([A-Za-z0-9_./-]+\.(?:cpp|h|py|txt|ps1)):(\d+)(?:-(\d+))?`")

# Anchor claims: (doc, source file, line, substring that must be on that line).
# These are the citations whose exact content the review's argument rests on.
ANCHORS = [
    ("Source/UI/WaspMeter.cpp", 145, "reference[i] * plot.getHeight()"),
    ("Source/UI/WaspMeter.cpp", 216, "reduceClipRegion"),
    ("Source/UI/WaspMeter.cpp", 120, "getLocalBounds().getRight()"),
    ("Source/PluginEditor.cpp", 49, "setResizeLimits (1000, 650, 1600, 1040)"),
    ("Source/PluginEditor.cpp", 210, "kRightColumnW, scale"),
    ("Source/UI/FizzReadout.h", 24, "66.1f"),
    ("Source/UI/HeaderBar.h", 33, '"4.2%"'),
    ("Source/UI/FooterBar.cpp", 28, "0.70f"),
    ("Source/UI/atoms/LivePill.cpp", 43, "40 and 100"),
    ("Source/UI/atoms/SpectreKnob.cpp", 65, "at 96"),
    ("Source/Theme/SpectreLookAndFeel.h", 53, "spacing = 4"),
    ("Source/UI/KnobRow.cpp", 49, "area.getWidth() / kNumKnobs"),
    (".gitignore", 45, "design/screenshots/"),
]

# Range citations were the blind spot: a range that still fits inside the file
# passes the existence check even after the code it describes has moved. This
# was caught for real when adding setTestReduction() to WaspMeter.h shifted
# the placeholder-data block by 11 lines and the checker stayed green. Anchor
# the first and last line of every range that section 9 leans on.
RANGE_ANCHORS = [
    ("Source/UI/WaspMeter.h", 55, "kNumColumns = 16"),
    ("Source/UI/WaspMeter.h", 72, "};"),
    ("Source/UI/WaspMeter.h", 67, "reductionDb[kNumCurvePoints]"),
    ("Source/UI/FooterBar.cpp", 28, "inMeter.setLevel"),
    ("Source/UI/FooterBar.cpp", 29, "outMeter.setLevel"),
    ("Source/UI/WaspMeter.cpp", 99, "StringArray ticks"),
    ("Source/UI/WaspMeter.cpp", 107, "ticks.size() - 1"),
]
ANCHORS = ANCHORS + RANGE_ANCHORS

# The eleven brand tokens, as they appear in Source/Theme/Palette.h and as the
# brief writes them. Both sides must agree.
PALETTE_TOKENS = [
    ("canvas", "060606"), ("surface1", "0B0B0B"), ("surface2", "101010"),
    ("surface3", "151514"), ("inkPrimary", "F2F2EF"), ("inkBody", "B4B4B0"),
    ("inkMeta", "8A8A85"), ("inkDisabled", "4A4A47"), ("rule", "2E2E2C"),
    ("stateLive", "7FA57A"), ("stateError", "C4574C"),
]

# Rules from the brief that the implementation must not contradict.
BRIEF_RULES = [
    "Border radius is zero",
    "Keep scanlines inside screens or media frames",
]

# Line-exact anchors inside the brief itself. CANONICAL-UI section 9.8 cites
# these by number, and the brief is an external file this repo does not
# control, so its line numbers can drift without any commit here.
BRIEF_ANCHORS = [
    (16, "Border radius is zero"),
    (41, "#060606"),
    (50, "#7FA57A"),
    (51, "#C4574C"),
    (59, "0.32em"),
    (61, "0.18em"),
    (91, "Keep scanlines inside screens"),
    (99, "160ms"),
    (100, "14 to 48 seconds"),
    (103, "Respect reduced motion"),
]


def find_brief() -> Path | None:
    explicit = os.environ.get("DPD_BRAND_BRIEF")
    if explicit and Path(explicit).is_file():
        return Path(explicit)

    kit = os.environ.get("DPD_BRAND_KIT")
    candidates = []
    if kit:
        candidates.append(Path(kit) / "brand-kit" / "AI-BRAND-BRIEF.md")
        candidates.append(Path(kit) / "AI-BRAND-BRIEF.md")

    rel = Path("dead-pixel-design-v4") / "brand-kit" / "AI-BRAND-BRIEF.md"
    for base in (ROOT, ROOT.parent, ROOT.parent.parent, Path.home() / "workspace"):
        candidates.append(base / rel)

    for c in candidates:
        if c.is_file():
            return c
    return None


def check_brand(failures: list[str], notes: list[str]) -> int:
    """Compare Palette.h and the docs against the binding brief. Returns the
    number of checks actually performed (0 if the brief is unavailable)."""
    brief = find_brief()
    if brief is None:
        notes.append(
            "SKIPPED brand-brief checks: AI-BRAND-BRIEF.md not found. "
            "Set DPD_BRAND_BRIEF or DPD_BRAND_KIT to enable them.")
        return 0

    text = brief.read_text(encoding="utf-8", errors="replace")
    upper = text.upper()
    palette = (ROOT / "Source/Theme/Palette.h").read_text(
        encoding="utf-8", errors="replace").upper()

    done = 0
    for name, hexcode in PALETTE_TOKENS:
        done += 1
        in_brief = hexcode.upper() in upper
        in_code = hexcode.upper() in palette
        if not in_brief:
            failures.append(f"brand: {name} #{hexcode} not present in {brief.name}")
        if not in_code:
            failures.append(f"brand: {name} #{hexcode} not present in Palette.h")

    for rule in BRIEF_RULES:
        done += 1
        if rule.lower() not in text.lower():
            failures.append(
                f"brand: rule {rule!r} no longer in the brief; "
                "CANONICAL-UI section 9.8 cites it")

    brief_lines = text.splitlines()
    for lineno, needle in BRIEF_ANCHORS:
        done += 1
        if lineno > len(brief_lines):
            failures.append(
                f"brand anchor {brief.name}:{lineno}: past EOF "
                f"({len(brief_lines)}); CANONICAL-UI section 9.8 cites it")
        elif needle not in brief_lines[lineno - 1]:
            failures.append(
                f"brand anchor {brief.name}:{lineno}: expected {needle!r}, "
                f"found {brief_lines[lineno - 1].strip()!r}; "
                "CANONICAL-UI section 9.8 cites this line")

    notes.append(f"brand brief: {brief}")
    return done


def self_test() -> int:
    """Negative controls, run against copies in a temp dir. Proves this script
    can fail; a checker that only ever prints OK is worthless."""
    import subprocess

    results = []
    with tempfile.TemporaryDirectory() as td:
        tmp = Path(td) / "doc.md"

        # Control 1: a citation past EOF must be reported.
        target = ROOT / "Source/UI/WaspMeter.cpp"
        total = line_count(target)
        tmp.write_text(f"bogus `Source/UI/WaspMeter.cpp:{total + 500}` cite\n",
                       encoding="utf-8")
        bad = [f for f in scan_doc(tmp, label="selftest")]
        results.append(("dangling line number", bool(bad)))

        # Control 2: an in-range citation must NOT be reported.
        tmp.write_text("fine `Source/UI/WaspMeter.cpp:10` cite\n", encoding="utf-8")
        good = [f for f in scan_doc(tmp, label="selftest")]
        results.append(("valid citation accepted", not good))

        # Control 3: an anchor whose content drifted must be reported.
        fails: list[str] = []
        check_anchor(fails, "Source/UI/FizzReadout.h", 24, "NOT-THE-REAL-VALUE")
        results.append(("anchor content drift", bool(fails)))

        # Control 4: a real anchor must pass.
        fails = []
        check_anchor(fails, "Source/UI/FizzReadout.h", 24, "66.1f")
        results.append(("valid anchor accepted", not fails))

    ok = all(passed for _, passed in results)
    for name, passed in results:
        print(f"  [{'PASS' if passed else 'FAIL'}] {name}")
    print("self-test:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


def resolve(name: str) -> Path | None:
    """Docs cite either a repo-relative path or a bare filename."""
    direct = ROOT / name
    if direct.exists():
        return direct
    hits = [p for p in ROOT.rglob(Path(name).name)
            if "build" not in p.parts and "JUCE" not in p.parts]
    return hits[0] if len(hits) == 1 else None


def line_count(path: Path) -> int:
    return len(path.read_text(encoding="utf-8", errors="replace").splitlines())


def scan_doc(doc_path: Path, label: str) -> list[str]:
    """Return a failure string for every citation in one doc that dangles."""
    out: list[str] = []
    for lineno, text in enumerate(doc_path.read_text(
            encoding="utf-8", errors="replace").splitlines(), 1):
        for m in CITE.finditer(text):
            name, start, end = m.group(1), int(m.group(2)), m.group(3)
            target = resolve(name)
            if target is None:
                out.append(f"{label}:{lineno}: cannot resolve '{name}'")
                continue
            total = line_count(target)
            last = int(end) if end else start
            if start < 1:
                out.append(f"{label}:{lineno}: {name}:{start} is not a valid line")
            elif end and int(end) < start:
                out.append(f"{label}:{lineno}: {name}:{start}-{end} is an inverted range")
            elif last > total:
                out.append(
                    f"{label}:{lineno}: {name}:{m.group(2)}"
                    f"{'-' + end if end else ''} exceeds file length ({total})")
    return out


def check_anchor(failures: list[str], name: str, lineno: int, needle: str) -> None:
    target = resolve(name)
    if target is None:
        failures.append(f"anchor: cannot resolve '{name}'")
        return
    lines = target.read_text(encoding="utf-8", errors="replace").splitlines()
    if lineno > len(lines):
        failures.append(f"anchor {name}:{lineno}: past EOF ({len(lines)})")
    elif needle not in lines[lineno - 1]:
        failures.append(
            f"anchor {name}:{lineno}: expected {needle!r}, found "
            f"{lines[lineno - 1].strip()!r}")


def main() -> int:
    if "--self-test" in sys.argv:
        return self_test()

    failures: list[str] = []
    notes: list[str] = []
    checked = 0

    for doc in DOCS:
        doc_path = ROOT / doc
        if not doc_path.exists():
            failures.append(f"{doc}: missing")
            continue
        checked += sum(1 for line in doc_path.read_text(
            encoding="utf-8", errors="replace").splitlines()
            for _ in CITE.finditer(line))
        failures.extend(scan_doc(doc_path, label=doc))

    for name, lineno, needle in ANCHORS:
        checked += 1
        check_anchor(failures, name, lineno, needle)

    checked += check_brand(failures, notes)

    for n in notes:
        print(n)

    if failures:
        print(f"FAIL: {len(failures)} problem(s) across {checked} checks")
        for f in failures:
            print("  " + f)
        return 1

    print(f"OK: {checked} citations and anchors verified")
    return 0


if __name__ == "__main__":
    sys.exit(main())
