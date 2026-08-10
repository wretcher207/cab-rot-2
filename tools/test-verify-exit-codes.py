"""Confirm verify-doc-citations.py returns correct exit codes.

Exit code correctness is what makes the checker usable in CI or a pre-commit
hook. A script that prints FAIL but exits 0 would be silently ignored by every
automation that consumes it, which would reproduce the exact class of defect
the hostile review found in the facelift report.

Restores every mutated file in a finally block.

All file I/O here is binary. An earlier version of this harness used text
mode, which silently rewrote the repo's CRLF line endings to LF on restore:
the content compared equal but git saw the whole file as modified. A test
that damages the tree it is testing is worse than no test.
"""
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
CHECKER = ROOT / "tools" / "verify-doc-citations.py"


def run(*args):
    return subprocess.run([sys.executable, str(CHECKER), *args],
                          capture_output=True, text=True, cwd=ROOT)


def swap(path: pathlib.Path, old: bytes, new: bytes) -> bytes:
    """Byte-exact substitution. Returns the original bytes for restore."""
    original = path.read_bytes()
    assert old in original, f"{path}: {old!r} not found; harness is stale"
    path.write_bytes(original.replace(old, new))
    return original


def main() -> int:
    results = []

    # Baseline: whatever git already thinks is dirty before we touch anything.
    # Comparing against this instead of against "" lets the harness run on a
    # work-in-progress tree without reporting the user's own edits as damage,
    # while still catching any file THIS harness fails to restore.
    watched = ["Source/Theme/Palette.h", "design/CANONICAL-UI.md"]

    def dirt() -> str:
        return subprocess.run(["git", "status", "--porcelain", *watched],
                              capture_output=True, text=True, cwd=ROOT).stdout.strip()

    baseline = dirt()

    r = run()
    results.append(("clean tree exits 0", r.returncode == 0, r.returncode))

    r = run("--self-test")
    results.append(("self-test exits 0", r.returncode == 0, r.returncode))

    # Palette drift must be a non-zero exit, not just a printed complaint.
    pal = ROOT / "Source/Theme/Palette.h"
    original = swap(pal, b"0xFF7FA57A", b"0xFF00FF00")
    try:
        r = run()
        results.append(("palette drift exits non-zero", r.returncode != 0, r.returncode))
        results.append(("palette drift names the token",
                        "stateLive" in r.stdout, r.stdout.strip().splitlines()[-1:]))
    finally:
        pal.write_bytes(original)

    # A dangling citation in a tracked doc must also be a non-zero exit.
    doc = ROOT / "design/CANONICAL-UI.md"
    original_doc = swap(doc, b"`WaspMeter.cpp:145`", b"`WaspMeter.cpp:88888`")
    try:
        r = run()
        results.append(("dangling citation exits non-zero", r.returncode != 0, r.returncode))
    finally:
        doc.write_bytes(original_doc)

    r = run()
    results.append(("restored tree exits 0 again", r.returncode == 0, r.returncode))

    # The harness must leave the tree exactly as it found it, byte for byte.
    after = dirt()
    results.append(("harness restored the tree byte-exactly",
                    after == baseline,
                    "unchanged" if after == baseline
                    else f"baseline={baseline!r} after={after!r}"))

    ok = all(passed for _, passed, _ in results)
    for name, passed, detail in results:
        print(f"  [{'PASS' if passed else 'FAIL'}] {name} (rc/detail: {detail})")
    print("exit-code contract:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
