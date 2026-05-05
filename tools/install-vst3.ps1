#requires -Version 5.1
<#
.SYNOPSIS
  Copy the freshly-built VST3 into the user-scope VST3 folder Reaper scans.

.DESCRIPTION
  Replaces the post-build COPY step that JUCE used to do automatically. The
  auto-copy was disabled because it fights Reaper: when Reaper has the
  plugin loaded, Windows locks the .vst3 file and the build fails on the
  copy step. This script lets you control when the install happens.

  Workflow:
    1. Edit code, run cmake --build
    2. Close any Reaper session that has Cab Rot loaded (or unload it)
    3. .\tools\install-vst3.ps1
    4. Rescan plugins in Reaper

.PARAMETER Configuration
  Release (default) or Debug.

.PARAMETER Destination
  Override the install path. Defaults to user-scope VST3 folder.

.PARAMETER KillBlockers
  Stop any running process whose path is inside the destination plugin
  bundle. Use sparingly; will close Reaper if it has the plugin loaded.
#>

param(
    [ValidateSet('Release','Debug','RelWithDebInfo')]
    [string] $Configuration = 'Release',
    [string] $Destination   = (Join-Path $env:LOCALAPPDATA 'Programs/Common/VST3'),
    [switch] $KillBlockers
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$src      = Join-Path $repoRoot "build/CabRot_artefacts/$Configuration/VST3/Cab Rot.vst3"
$dst      = Join-Path $Destination "Cab Rot.vst3"

if (-not (Test-Path $src)) {
    throw "VST3 not built at $src. Run cmake --build first."
}

if (-not (Test-Path $Destination)) {
    New-Item -ItemType Directory -Path $Destination | Out-Null
}

if (Test-Path $dst) {
    # Probe whether the existing bundle is writable.
    $locked = $false
    Get-ChildItem $dst -Recurse -File -ErrorAction SilentlyContinue | ForEach-Object {
        try { ([IO.File]::Open($_.FullName, 'Open', 'Write')).Close() }
        catch { $locked = $true }
    }
    if ($locked) {
        if ($KillBlockers) {
            Get-Process | Where-Object { $_.Modules.FileName -match 'Cab Rot.vst3' } |
                ForEach-Object { Write-Host "Stopping $($_.Name) (PID $($_.Id))"; $_ | Stop-Process -Force }
            Start-Sleep -Seconds 1
        } else {
            throw "$dst is locked. Close Reaper (or whichever DAW has it loaded), or pass -KillBlockers."
        }
    }
    Remove-Item -Recurse -Force $dst
}

Copy-Item -Recurse $src $dst
Write-Host "Installed: $dst"
