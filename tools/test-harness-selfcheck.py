"""Meta-test: prove the exit-code harness detects its OWN failure to restore.

Sabotages the restore step in test-verify-exit-codes.py, confirms the harness
reports BROKEN and exits non-zero, then puts everything back. Without this,
the "harness restored the tree byte-exactly" line is an untested assertion.
"""
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
HARNESS = ROOT / "tools" / "test-verify-exit-codes.py"
PALETTE = ROOT / "Source/Theme/Palette.h"

GOOD = b"        pal.write_bytes(original)"
SABOTAGE = b"        pass  # sabotage: deliberately skip restore"


def main() -> int:
    original_harness = HARNESS.read_bytes()
    original_palette = PALETTE.read_bytes()

    if GOOD not in original_harness:
        print("FAIL: sabotage target not found; this meta-test is stale")
        return 1

    try:
        HARNESS.write_bytes(original_harness.replace(GOOD, SABOTAGE))
        r = subprocess.run([sys.executable, str(HARNESS)],
                           capture_output=True, text=True, cwd=ROOT)
        detected = r.returncode != 0 and "BROKEN" in r.stdout
        line = [l for l in r.stdout.splitlines() if "restored the tree" in l]
        print(f"  sabotaged harness rc={r.returncode}")
        for l in line:
            print("  " + l.strip())
    finally:
        HARNESS.write_bytes(original_harness)
        PALETTE.write_bytes(original_palette)

    # Confirm the real harness is green again after cleanup.
    r2 = subprocess.run([sys.executable, str(HARNESS)],
                        capture_output=True, text=True, cwd=ROOT)
    restored = r2.returncode == 0

    ok = detected and restored
    print(f"  [{'PASS' if detected else 'FAIL'}] harness detects a failed restore")
    print(f"  [{'PASS' if restored else 'FAIL'}] harness green again after cleanup")
    print("meta-test:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
