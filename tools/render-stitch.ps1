#requires -Version 5.1
<#
.SYNOPSIS
  Render design/stitch-reference.html to a 1200x780 PNG via Chrome headless.

.DESCRIPTION
  Phase 1 visual-diff input. The Stitch export is HTML/Tailwind; we need a
  static raster of it as the reference frame the JUCE ThemeTest is compared
  against. Run once whenever the Stitch reference changes.

.PARAMETER OutPath
  Defaults to design/screenshots/stitch-reference.png.
#>
param(
    [string] $OutPath
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$src      = Join-Path $repoRoot 'design/stitch-reference.html'
if (-not $OutPath) { $OutPath = Join-Path $repoRoot 'design/screenshots/stitch-reference.png' }
$outDir = Split-Path -Parent $OutPath
if ($outDir -and -not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

$chrome = "C:\Program Files\Google\Chrome\Application\chrome.exe"
if (-not (Test-Path $chrome)) {
    $chrome = "C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe"
    if (-not (Test-Path $chrome)) {
        throw "Neither Chrome nor Edge found. Install one or pass -ChromePath."
    }
}

$srcUrl = "file:///$($src -replace '\\','/')"
Write-Host ">> rendering $srcUrl"

$tmpProfile = Join-Path $env:TEMP "cabrot-stitch-render-$(Get-Random)"
try {
    & $chrome `
        --headless=new `
        --disable-gpu `
        --hide-scrollbars `
        --no-sandbox `
        --window-size=1216,820 `
        --user-data-dir="$tmpProfile" `
        "--screenshot=$OutPath" `
        $srcUrl 2>$null | Out-Null

    if (-not (Test-Path $OutPath)) {
        throw "Headless render produced no file at $OutPath"
    }
    $info = Get-Item $OutPath
    Write-Host "Saved $OutPath ($($info.Length) bytes)"
}
finally {
    Remove-Item -Recurse -Force $tmpProfile -ErrorAction SilentlyContinue
}
