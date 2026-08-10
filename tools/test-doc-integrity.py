"""Negative controls for verify-doc-integrity.py.

Same standard as the other gates: prove the checker fails on the defects it
claims to catch, rather than trusting a green run. Injects a bad
cross-reference and a U+FFFD replacement char into a copy of the doc, and
restores byte-exactly.
"""
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
CHECKER = ROOT / "tools" / "verify-doc-integrity.py"
DOC = ROOT / "design" / "CANONICAL-UI.md"


def run():
    return subprocess.run([sys.executable, str(CHECKER)],
                          capture_output=True, text=True, cwd=ROOT)


def main() -> int:
    results = []
    original = DOC.read_bytes()

    # Compare against the tree state before this harness runs, so an
    # in-progress edit is not reported as damage while a genuine failure to
    # restore still is. Same fix as tools/test-verify-exit-codes.py.
    def dirt() -> str:
        return subprocess.run(["git", "status", "--porcelain", str(DOC)],
                              capture_output=True, text=True, cwd=ROOT).stdout.strip()

    baseline = dirt()

    r = run()
    results.append(("clean docs pass", r.returncode == 0, r.returncode))

    try:
        # Control 1: a reference to a section that does not exist.
        DOC.write_bytes(original + "\nBogus reference to \u00a79.99 here.\n".encode("utf-8"))
        r = run()
        results.append(("dangling cross-reference caught",
                        r.returncode != 0 and "9.99" in r.stdout, r.returncode))

        # Control 2: encoding damage.
        DOC.write_bytes(original + "\nDamaged \ufffd char.\n".encode("utf-8"))
        r = run()
        results.append(("U+FFFD replacement char caught",
                        r.returncode != 0 and "U+FFFD" in r.stdout, r.returncode))

        # Control 3: invalid UTF-8 entirely.
        DOC.write_bytes(original + b"\n\xff\xfe invalid utf8 \n")
        r = run()
        results.append(("invalid UTF-8 caught",
                        r.returncode != 0 and "UTF-8" in r.stdout, r.returncode))
    finally:
        DOC.write_bytes(original)

    r = run()
    results.append(("restored docs pass again", r.returncode == 0, r.returncode))

    after = dirt()
    results.append(("restored byte-exactly", after == baseline,
                    "unchanged" if after == baseline
                    else f"baseline={baseline!r} after={after!r}"))

    ok = all(p for _, p, _ in results)
    for name, passed, detail in results:
        print(f"  [{'PASS' if passed else 'FAIL'}] {name} ({detail})")
    print("integrity controls:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
