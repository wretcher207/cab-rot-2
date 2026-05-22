Title: SUNDER Style Guide v1
Document Type: Style Guide
Author: Claude (design skill)
Created Date: 2026-05-21
Last Modified Date: 2026-05-21

# SUNDER — Style Guide v1

## Color tokens
| Token | Hex | Use |
|-------|-----|-----|
| `--bg` | `#0C0C0E` | Window background, flat matte |
| `--surface` | `#16161A` | Inactive elements, encoder track ring |
| `--gridline` | `#1C1C21` | Canvas grid, 0.5px, near-imperceptible |
| `--text-2` | `#8E8E93` | Labels, scales, secondary lines |
| `--text-1` | `#F5F5F7` | Value readouts, active wordmark |
| `--accent` | `#D4A359` | Active arcs, reduction veil, active states |
| `--accent-soft` | `rgba(212,163,89,0.25)` | Veil ceiling, hover glow |

Single accent rule: amber is the only chromatic color. Everything else is
charcoal → silver. v2 swaps `--accent` to `#5B84B1` (slate blue) for comparison.

## Typography
Self-contained: no web-font downloads. System stacks chosen to approximate Inter
/ SF Pro and JetBrains Mono.

| Role | Stack | Size | Tracking | Weight | Case | Color |
|------|-------|------|----------|--------|------|-------|
| Wordmark | Inter / SF Pro / Segoe UI | 16px | +0.2em | 600 | UPPER | `--text-1` |
| Section / param header | same | 10px | +0.1em | 500 | UPPER | `--text-2` |
| Value indicator | JetBrains Mono / SF Mono / Consolas | 14px | 0 | 500 | — | `--text-1` |
| Fine print / scales | same sans | 8px | +0.05em | 400 | — | `--text-2` |

Value readouts are **monospace** so digits don't jitter horizontally while turning.

## Spacing
- Outer margin: `W * 0.025` (≈24px @ 960). Generous; restraint over density.
- Zone separation by whitespace + 1px hairline at ≤12% opacity.
- Encoder pitch in dock: `(dockWidth) / 6`, centered, equal gutters.

## Vector encoder anatomy
1. Track ring: stroke `--surface`, 1.5px, full circle.
2. Value arc: stroke `--accent`, 1.5px, from min angle (−135°) sweeping clockwise
   to value; round cap.
3. Center: value text, monospace 14px `--text-1`, with unit (`%`).
4. Label: 10px uppercase `--text-2`, +0.1em, centered below the ring.
5. Hover: value text → `--text-1` brighter, arc gains a soft amber glow.
No pointer, no notch, no bevel, no box.

## Spectral canvas
- Grid: 4 horizontal dB lines (0, −12, −24, −36) + 5 vertical freq lines
  (200Hz, 1k, 2k, 5k, 10k), stroke `--gridline`, 0.5px.
- Input spectrum: smoothed cubic path, stroke `--text-2` at ~40% alpha, 1.5px.
- Reduction veil: filled path draping down over the targeted band, gradient
  `--accent-soft` (top) → transparent (bottom). Smooth, anti-aliased, no stair-steps.
- Scales: dB labels left edge, freq labels bottom edge, fine-print style.

## Motion (mockup)
Subtle idle drift on the spectrum + veil to read as "liquid," not flicker.
Production replaces this with the smoothed real FFT (spec Phase 4).
