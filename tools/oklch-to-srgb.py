#!/usr/bin/env python3
"""
Cab Rot palette generator.

Reads the Stitch tailwind config block from design/stitch-reference.html,
converts every OKLCH triplet to sRGB hex (the OKLab -> linear sRGB ->
gamma-encoded sRGB pipeline), and emits Source/Theme/Palette.h with one
constexpr juce::Colour per token.

Reproducibility: running this script twice with the same input must
produce a byte-identical Palette.h. That's verified by Phase 1 gate
item 2.

Usage:
    python tools/oklch-to-srgb.py
    python tools/oklch-to-srgb.py --check   # exits non-zero if Palette.h is stale
"""

from __future__ import annotations

import argparse
import math
import re
import sys
from pathlib import Path
from typing import Iterable

REPO_ROOT     = Path(__file__).resolve().parent.parent
STITCH_PATH   = REPO_ROOT / "design" / "stitch-reference.html"
HEADER_PATH   = REPO_ROOT / "Source" / "Theme" / "Palette.h"

# ----------------------------------------------------------------------------
# Color math: OKLCH -> sRGB hex
# ----------------------------------------------------------------------------

# Bjorn Ottosson's OKLab matrix. See https://bottosson.github.io/posts/oklab/
def oklab_to_linear_srgb(L: float, a: float, b: float) -> tuple[float, float, float]:
    l_ = L + 0.3963377774 * a + 0.2158037573 * b
    m_ = L - 0.1055613458 * a - 0.0638541728 * b
    s_ = L - 0.0894841775 * a - 1.2914855480 * b

    l = l_ * l_ * l_
    m = m_ * m_ * m_
    s = s_ * s_ * s_

    r =  4.0767416621 * l - 3.3077115913 * m + 0.2309699292 * s
    g = -1.2684380046 * l + 2.6097574011 * m - 0.3413193965 * s
    bl = -0.0041960863 * l - 0.7034186147 * m + 1.7076147010 * s
    return r, g, bl


def linear_to_srgb(c: float) -> float:
    if c <= 0.0031308:
        return 12.92 * c
    return 1.055 * (c ** (1.0 / 2.4)) - 0.055


def clamp01(x: float) -> float:
    if x < 0.0:
        return 0.0
    if x > 1.0:
        return 1.0
    return x


def oklch_to_srgb_hex(L: float, C: float, h_deg: float, alpha: float = 1.0) -> str:
    """Return 8-char ARGB hex (uppercase) suitable for juce::Colour::fromString."""
    h_rad = math.radians(h_deg)
    a = C * math.cos(h_rad)
    b = C * math.sin(h_rad)
    r_lin, g_lin, b_lin = oklab_to_linear_srgb(L, a, b)
    r = clamp01(linear_to_srgb(r_lin))
    g = clamp01(linear_to_srgb(g_lin))
    bv = clamp01(linear_to_srgb(b_lin))
    A = int(round(clamp01(alpha) * 255))
    R = int(round(r  * 255))
    G = int(round(g  * 255))
    B = int(round(bv * 255))
    return f"{A:02X}{R:02X}{G:02X}{B:02X}"


def hex3_or_6_to_argb(hexstr: str) -> str:
    s = hexstr.lstrip("#").upper()
    if len(s) == 3:
        s = "".join(c * 2 for c in s)
    if len(s) != 6:
        raise ValueError(f"Bad hex literal: {hexstr!r}")
    return f"FF{s}"


# ----------------------------------------------------------------------------
# Tailwind config parsing
# ----------------------------------------------------------------------------

# matches: "token-name": "value"   — value is either oklch(...) or #hex
TOKEN_LINE = re.compile(
    r'"(?P<name>[a-z][a-z0-9\-]*)"\s*:\s*"(?P<value>[^"]+)"\s*,?'
)
OKLCH_RE = re.compile(
    r"""oklch\(
        \s*(?P<L>[0-9.]+)\s+
        (?P<C>[0-9.]+)\s+
        (?P<h>[0-9.]+)
        (?:\s*/\s*(?P<a>[0-9.]+)(?P<aunit>%)?)?
        \s*\)""",
    re.VERBOSE,
)


def parse_oklch(value: str) -> tuple[float, float, float, float] | None:
    m = OKLCH_RE.fullmatch(value.strip())
    if not m:
        return None
    L = float(m["L"])
    C = float(m["C"])
    h = float(m["h"])
    a = m["a"]
    if a is None:
        alpha = 1.0
    else:
        alpha = float(a) / (100.0 if m["aunit"] else 1.0)
    return L, C, h, alpha


def parse_hex(value: str) -> str | None:
    s = value.strip()
    if s.startswith("#"):
        return s
    return None


def extract_color_block(html: str) -> str:
    """Pull out the `colors: { ... }` block from the inline Tailwind config."""
    # The block opens with `colors: {` and is closed by a `},` at top level.
    start = html.find("colors:")
    if start < 0:
        raise SystemExit("colors: block not found in stitch-reference.html")
    brace = html.find("{", start)
    depth = 0
    end = brace
    for i, ch in enumerate(html[brace:], start=brace):
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                end = i
                break
    return html[brace:end + 1]


def parse_tokens(block: str) -> list[tuple[str, str, str]]:
    """Return [(name, source_form, argb_hex)]."""
    out: list[tuple[str, str, str]] = []
    for m in TOKEN_LINE.finditer(block):
        name  = m["name"]
        value = m["value"].strip()

        oklch = parse_oklch(value)
        if oklch is not None:
            argb = oklch_to_srgb_hex(*oklch)
            out.append((name, value, argb))
            continue

        hexv = parse_hex(value)
        if hexv is not None:
            argb = hex3_or_6_to_argb(hexv)
            out.append((name, value, argb))
            continue

        # Unknown form, e.g. CSS rgb() or var(). Skip with a noisy comment.
        out.append((name, value, ""))
    return out


# ----------------------------------------------------------------------------
# Header emission
# ----------------------------------------------------------------------------

HEADER_PROLOG = """\
// Generated by tools/oklch-to-srgb.py from design/stitch-reference.html.
// DO NOT EDIT. Re-run the script if the Stitch palette changes.
//
// All OKLCH tokens were converted via the OKLab -> linear sRGB -> sRGB
// gamma pipeline (Bjorn Ottosson's OKLab matrix). Hex tokens were
// promoted to AARRGGBB with full alpha.
//
// Conversion matrix and formula are stable and reproducible: running
// the generator twice on unchanged input produces a byte-identical
// header (verified in Phase 1 gate).

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace cabrot::theme
{
"""

HEADER_EPILOG = """\

} // namespace cabrot::theme
"""


def cpp_identifier(token: str) -> str:
    """`surface-container-highest` -> `surfaceContainerHighest`."""
    parts = token.split("-")
    return parts[0] + "".join(p.capitalize() for p in parts[1:])


def emit_header(tokens: Iterable[tuple[str, str, str]]) -> str:
    rendered = list(tokens)
    rendered.sort(key=lambda t: t[0])

    lines: list[str] = []
    lines.append(HEADER_PROLOG)

    # Group by inferred semantic family for readability. Order doesn't matter
    # to the compiler but a stable, alpha-by-name grouping makes the diff
    # readable when colors change.
    for name, source, argb in rendered:
        ident = cpp_identifier(name)
        if not argb:
            lines.append(f"// SKIPPED {name}: unparsed value {source!r}")
            continue
        # juce::Colour's ctor is not constexpr in JUCE 8, so we use
        # `inline const` rather than `inline constexpr`. Same single-source-
        # of-truth guarantee, slight static-init cost which is negligible.
        lines.append(
            f"inline const juce::Colour {ident} {{ juce::uint32 (0x{argb}) }};"
            f"  // {name} = {source}"
        )

    lines.append(HEADER_EPILOG)
    return "\n".join(lines)


# ----------------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------------

def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--check", action="store_true",
                    help="Exit non-zero if Palette.h is out of sync with the source")
    args = ap.parse_args(argv)

    html = STITCH_PATH.read_text(encoding="utf-8")
    block = extract_color_block(html)
    tokens = parse_tokens(block)
    if not tokens:
        print("ERROR: no tokens parsed from colors block", file=sys.stderr)
        return 2

    out = emit_header(tokens)

    if args.check:
        if not HEADER_PATH.exists():
            print(f"MISSING: {HEADER_PATH}", file=sys.stderr)
            return 1
        if HEADER_PATH.read_text(encoding="utf-8") != out:
            print(f"STALE: {HEADER_PATH} - re-run tools/oklch-to-srgb.py",
                  file=sys.stderr)
            return 1
        print(f"OK: {HEADER_PATH} ({len(tokens)} tokens)")
        return 0

    HEADER_PATH.parent.mkdir(parents=True, exist_ok=True)
    HEADER_PATH.write_text(out, encoding="utf-8", newline="\n")
    print(f"Wrote {HEADER_PATH} ({len(tokens)} tokens)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
