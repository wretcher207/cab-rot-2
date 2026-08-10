#!/usr/bin/env python3
"""Verify that every `File.ext:N` / `File.ext:N-M` citation in the design docs
points at a line that actually exists in the referenced source file.

This exists because the 2026-08-10 hostile review found that the facelift
report's most confident claims were the least reproducible ones. A spec that
cites line numbers is only trustworthy if the citations are checked; line
numbers drift every time someone edits a source file.

Usage:  python tools/verify-doc-citations.py
Exit 0 = every citation resolves. Exit 1 = at least one is stale/dangling.

Limits, stated honestly: this proves each cited line EXISTS, not that it says
what the doc claims. A `--content` spot-check for a handful of anchor claims
is included below and is the part that actually catches semantic drift.
"""

import re
import sys
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


def main() -> int:
    failures: list[str] = []
    checked = 0

    for doc in DOCS:
        doc_path = ROOT / doc
        if not doc_path.exists():
            failures.append(f"{doc}: missing")
            continue
        for lineno, text in enumerate(doc_path.read_text(encoding="utf-8",
                                                         errors="replace").splitlines(), 1):
            for m in CITE.finditer(text):
                name, start, end = m.group(1), int(m.group(2)), m.group(3)
                target = resolve(name)
                if target is None:
                    failures.append(f"{doc}:{lineno}: cannot resolve '{name}'")
                    continue
                total = line_count(target)
                last = int(end) if end else start
                checked += 1
                if last > total:
                    failures.append(
                        f"{doc}:{lineno}: {name}:{m.group(2)}"
                        f"{'-' + end if end else ''} exceeds file length ({total})")

    for name, lineno, needle in ANCHORS:
        target = resolve(name)
        if target is None:
            failures.append(f"anchor: cannot resolve '{name}'")
            continue
        lines = target.read_text(encoding="utf-8", errors="replace").splitlines()
        checked += 1
        if lineno > len(lines):
            failures.append(f"anchor {name}:{lineno}: past EOF ({len(lines)})")
        elif needle not in lines[lineno - 1]:
            failures.append(
                f"anchor {name}:{lineno}: expected {needle!r}, found "
                f"{lines[lineno - 1].strip()!r}")

    if failures:
        print(f"FAIL: {len(failures)} problem(s) across {checked} checks")
        for f in failures:
            print("  " + f)
        return 1

    print(f"OK: {checked} citations and anchors verified")
    return 0


if __name__ == "__main__":
    sys.exit(main())
