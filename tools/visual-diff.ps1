#requires -Version 5.1
<#
.SYNOPSIS
  Build the Cab Rot Standalone, launch it, capture a PNG of the editor window.

.DESCRIPTION
  Phase 0 deliverable: produces a screenshot of the running editor for visual
  reference. Phase 1+ will extend this with image diffing against the Stitch
  reference (ImageMagick `compare`).

  Implementation notes:
    - JUCE 8 on Windows renders with Direct2D. PrintWindow with flag 0 returns
      black; flag 2 (PW_RENDERFULLCONTENT) is required.
    - JUCE + AudioDeviceManager init takes 8 to 10 seconds before
      MainWindowHandle is populated. Don't shorten -WaitSeconds without testing.
    - $p.Refresh() is mandatory: the cached MainWindowHandle predates window
      creation and reads zero without it.

.PARAMETER Configuration
  Release (default) or Debug. Determines which build artefact we capture.

.PARAMETER SkipBuild
  Skip the cmake --build step. Use when iterating on capture itself.

.PARAMETER OutPath
  Output PNG path. Defaults to design/screenshots/phase-<N>-<timestamp>.png.

.PARAMETER ReferencePath
  Optional reference image to diff against. If supplied AND ImageMagick is on
  PATH, runs `compare -metric AE` and prints the per-pixel difference count.

.PARAMETER WaitSeconds
  Seconds to wait after launch before capture. Default 10.

.EXAMPLE
  .\tools\visual-diff.ps1
  .\tools\visual-diff.ps1 -Configuration Debug -OutPath design\screenshots\debug.png
  .\tools\visual-diff.ps1 -ReferencePath design\screenshots\phase-0-baseline.png
#>

param(
  [ValidateSet('Release','Debug','RelWithDebInfo')]
  [string] $Configuration = 'Release',
  [switch] $SkipBuild,
  [string] $OutPath,
  [string] $ReferencePath,
  [int]    $WaitSeconds = 10
)

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
$repoRoot = Split-Path -Parent $PSScriptRoot
$buildDir = Join-Path $repoRoot 'build'
$exePath  = Join-Path $buildDir "CabRot_artefacts/$Configuration/Standalone/Cab Rot.exe"
$screenshotsDir = Join-Path $repoRoot 'design/screenshots'

if (-not (Test-Path $screenshotsDir)) {
    New-Item -ItemType Directory -Path $screenshotsDir | Out-Null
}

if (-not $OutPath) {
    $stamp   = Get-Date -Format 'yyyyMMdd-HHmmss'
    $OutPath = Join-Path $screenshotsDir "snapshot-$stamp.png"
}

# ---------------------------------------------------------------------------
# Build (unless skipped)
# ---------------------------------------------------------------------------
if (-not $SkipBuild) {
    $cmake = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if (-not $cmake) {
        $vsCMake = "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
        if (Test-Path $vsCMake) { $cmake = Get-Item $vsCMake } else { throw "cmake.exe not found on PATH or under VS Build Tools." }
    }

    if (-not (Test-Path (Join-Path $buildDir 'CMakeCache.txt'))) {
        Write-Host ">> cmake configure"
        & $cmake.Source -S $repoRoot -B $buildDir -G 'Visual Studio 18 2026' -A x64
        if ($LASTEXITCODE -ne 0) { throw "cmake configure failed ($LASTEXITCODE)" }
    }

    Write-Host ">> cmake --build ($Configuration)"
    & $cmake.Source --build $buildDir --config $Configuration --target CabRot_Standalone -- /m /nologo /verbosity:minimal
    if ($LASTEXITCODE -ne 0) { throw "cmake build failed ($LASTEXITCODE)" }
}

if (-not (Test-Path $exePath)) {
    throw "Standalone exe missing: $exePath. Did the build succeed?"
}

# ---------------------------------------------------------------------------
# Capture
# ---------------------------------------------------------------------------
$exeName = [IO.Path]::GetFileNameWithoutExtension($exePath)
Get-Process $exeName -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 1

Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public class CabRotSnap {
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint f);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
}
"@

Write-Host ">> launch + capture"
$p = Start-Process $exePath -PassThru
try {
    Start-Sleep -Seconds $WaitSeconds
    $p.Refresh()
    $hwnd = $p.MainWindowHandle
    if ($hwnd -eq [IntPtr]::Zero) {
        throw "MainWindowHandle still zero after $WaitSeconds seconds. App may have crashed; check Event Viewer."
    }

    $r = New-Object CabRotSnap+RECT
    [CabRotSnap]::GetWindowRect($hwnd, [ref]$r) | Out-Null
    $w = $r.R - $r.L
    $h = $r.B - $r.T
    if ($w -lt 100 -or $h -lt 100) {
        throw "Window too small ($w x $h). Probably captured a launcher window. Try a longer -WaitSeconds."
    }

    $bmp = New-Object System.Drawing.Bitmap($w, $h)
    $g   = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc()
    $ok  = [CabRotSnap]::PrintWindow($hwnd, $hdc, 2)  # flag 2 = PW_RENDERFULLCONTENT, required for Direct2D
    $g.ReleaseHdc($hdc)
    $g.Dispose()

    if (-not $ok) { $bmp.Dispose(); throw "PrintWindow returned false." }

    $bmp.Save($OutPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    Write-Host "Saved $OutPath ($w x $h)"
}
finally {
    $p | Stop-Process -Force -ErrorAction SilentlyContinue
}

# ---------------------------------------------------------------------------
# Optional diff (Phase 1+: compare against Stitch reference)
# ---------------------------------------------------------------------------
if ($ReferencePath) {
    if (-not (Test-Path $ReferencePath)) { throw "Reference not found: $ReferencePath" }
    $magick = Get-Command magick -ErrorAction SilentlyContinue
    if (-not $magick) {
        Write-Host "ImageMagick not on PATH; skipping diff. Install via 'winget install ImageMagick.ImageMagick'."
    } else {
        $diffPath = [IO.Path]::ChangeExtension($OutPath, '.diff.png')
        $stats = & $magick.Source compare -metric AE -fuzz 1% $OutPath $ReferencePath $diffPath 2>&1
        Write-Host "AE pixel difference: $stats"
        Write-Host "Diff image: $diffPath"
    }
}
