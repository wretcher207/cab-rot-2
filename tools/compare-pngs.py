#!/usr/bin/env python3
"""
Compare two PNGs and report the per-pixel difference percentage.

Used by tools/visual-diff.ps1 as the diff backend when ImageMagick isn't on
PATH. Replicates `compare -metric AE -fuzz <fuzz>%` semantics: counts pixels
whose channels differ by more than the fuzz tolerance.

Output (one line, parseable):
    diff: <num_diff> / <total>   (<percent>%)   fuzz=<f>%   threshold=<t>%   <PASS|FAIL>

Exit code: 0 if percent <= threshold, 1 otherwise.

Usage:
    python tools/compare-pngs.py <a.png> <b.png> [--fuzz 1] [--threshold 5]
"""

from __future__ import annotations

import argparse
import sys

try:
    from PIL import Image
except ImportError:
    print("FATAL: Pillow not installed. Run: python -m pip install pillow", file=sys.stderr)
    sys.exit(2)


def channel_diff(a: tuple, b: tuple) -> int:
    return max(abs(int(a[i]) - int(b[i])) for i in range(min(len(a), len(b))))


def compare(path_a: str, path_b: str, fuzz_pct: float, threshold_pct: float) -> int:
    img_a = Image.open(path_a).convert("RGBA")
    img_b = Image.open(path_b).convert("RGBA")

    if img_a.size != img_b.size:
        # Resize the smaller to the larger so we can still compare.
        target = (max(img_a.size[0], img_b.size[0]), max(img_a.size[1], img_b.size[1]))
        img_a = img_a.resize(target, Image.NEAREST)
        img_b = img_b.resize(target, Image.NEAREST)

    pixels_a = img_a.load()
    pixels_b = img_b.load()
    w, h = img_a.size
    fuzz_abs = int(round(fuzz_pct / 100.0 * 255))
    diff_count = 0

    for y in range(h):
        for x in range(w):
            if channel_diff(pixels_a[x, y], pixels_b[x, y]) > fuzz_abs:
                diff_count += 1

    total = w * h
    pct = 100.0 * diff_count / total if total > 0 else 0.0
    verdict = "PASS" if pct <= threshold_pct else "FAIL"
    print(
        f"diff: {diff_count} / {total}   ({pct:.2f}%)   "
        f"fuzz={fuzz_pct}%   threshold={threshold_pct}%   {verdict}"
    )
    return 0 if verdict == "PASS" else 1


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("a")
    ap.add_argument("b")
    ap.add_argument("--fuzz",      type=float, default=1.0,
                    help="Per-channel tolerance (percent of 255). Default 1.")
    ap.add_argument("--threshold", type=float, default=5.0,
                    help="Maximum allowed mismatched-pixel percentage. Default 5.")
    args = ap.parse_args(argv)
    return compare(args.a, args.b, args.fuzz, args.threshold)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
