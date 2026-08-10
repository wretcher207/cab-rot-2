"""Scan the design docs for encoding damage and stale cross-references.

The section symbol is used heavily in CANONICAL-UI.md, and mixed-encoding
edits on Windows can turn it into mojibake that no citation checker notices
because line numbers still resolve. This also verifies that every internal
"section N.M" cross-reference actually points at a heading that exists.
"""
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = ["design/CANONICAL-UI.md", "design/FACELIFT-REPORT.md"]

# CANONICAL-UI owns the section-9 findings. FACELIFT-REPORT legitimately points
# at them from another file, so its cross-references are resolved against
# CANONICAL-UI's headings rather than its own.
HEADING_OWNER = {
    "design/CANONICAL-UI.md": "design/CANONICAL-UI.md",
    "design/FACELIFT-REPORT.md": "design/CANONICAL-UI.md",
}

SECTION = "\u00a7"
REPLACEMENT = "\ufffd"


def headings_in(path: Path) -> set:
    found = set()
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = re.match(r"^#+\s+" + SECTION + r"?\s*(\d+(?:\.\d+[a-z]?)?)", line)
        if m:
            found.add(m.group(1))
    return found


def main() -> int:
    failures = []
    checked = 0

    for name in DOCS:
        path = ROOT / name
        raw = path.read_bytes()

        # Must be valid UTF-8. A cp1252 fallback would mean damage already.
        try:
            text = raw.decode("utf-8")
        except UnicodeDecodeError as e:
            failures.append(f"{name}: not valid UTF-8 ({e})")
            continue
        checked += 1

        lines = text.splitlines()

        # Unicode replacement chars mean information was already lost.
        for i, line in enumerate(lines, 1):
            if REPLACEMENT in line:
                failures.append(f"{name}:{i}: contains U+FFFD replacement char")

        # Classic mojibake: the section sign mis-decoded, or an 'A' glued to a
        # section number where a section sign belongs.
        for i, line in enumerate(lines, 1):
            if re.search(r"\bA\d+\.\d+\b", line):
                failures.append(
                    f"{name}:{i}: looks like a mangled section sign: "
                    f"{line.strip()[:80]!r}")
            if "\u00c2" in line or "\u00a7\u00c2" in line:
                failures.append(f"{name}:{i}: mojibake byte sequence present")

        # Every "section N" / "N.M" reference must point at a real heading.
        headings = headings_in(ROOT / HEADING_OWNER[name])

        for i, line in enumerate(lines, 1):
            for ref in re.findall(SECTION + r"(\d+\.\d+[a-z]?)", line):
                checked += 1
                if ref not in headings:
                    failures.append(
                        f"{name}:{i}: cross-reference to section {ref} "
                        f"has no matching heading in {HEADING_OWNER[name]}")

    if failures:
        print(f"FAIL: {len(failures)} problem(s) across {checked} checks")
        for f in failures:
            print("  " + f)
        return 1

    print(f"OK: {checked} encoding and cross-reference checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
