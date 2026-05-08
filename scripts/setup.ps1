# FRDM-MCXN947_demo : one-time setup script (Windows / PowerShell)
#
# Usage:   pwsh -File scripts/setup.ps1            # auto-detect or prompt
#          pwsh -File scripts/setup.ps1 -SdkDir "C:\Users\me\Documents\mcuxsdk"
#
# Sets the per-user MCUXSDK_DIR environment variable so the committed
# .vscode/settings.json (which uses ${env:MCUXSDK_DIR}) resolves correctly
# on this machine. Re-run any time the SDK location changes.

param(
    [string]$SdkDir = ""
)

$ErrorActionPreference = 'Stop'

function Test-McuxSdkRoot {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    if (-not (Test-Path $Path)) { return $false }
    $required = @('mcuxsdk', 'manifests', '.west')
    foreach ($name in $required) {
        if (-not (Test-Path (Join-Path $Path $name))) { return $false }
    }
    if (-not (Test-Path (Join-Path $Path 'mcuxsdk\cmake\toolchain\armgcc.cmake'))) { return $false }
    return $true
}

# 1. Resolve SDK path : argument > current env > common candidates > prompt
if (-not [string]::IsNullOrWhiteSpace($SdkDir)) {
    $candidate = $SdkDir
} elseif ($env:MCUXSDK_DIR -and (Test-McuxSdkRoot $env:MCUXSDK_DIR)) {
    Write-Host "MCUXSDK_DIR is already set to: $env:MCUXSDK_DIR"
    $candidate = $env:MCUXSDK_DIR
} else {
    $guesses = @(
        (Join-Path $HOME 'Documents\mcuxsdk'),
        'D:\GitHub\mcuxsdk',
        'C:\GitHub\mcuxsdk',
        'D:\workspace\github\mcuxsdk'
    )
    $candidate = $null
    foreach ($g in $guesses) {
        if (Test-McuxSdkRoot $g) { $candidate = $g; Write-Host "Auto-detected SDK at: $g"; break }
    }
    if (-not $candidate) {
        Write-Host ""
        Write-Host "MCUXpresso SDK (west) root not found automatically."
        Write-Host "Expected layout under <root>: .west/  manifests/  mcuxsdk/"
        $candidate = Read-Host "Enter full path to MCUXpresso SDK west root"
    }
}

# 2. Validate
if (-not (Test-McuxSdkRoot $candidate)) {
    Write-Error "Path does not look like a valid MCUXpresso SDK west root: $candidate"
    Write-Error "  Required: .west/  manifests/  mcuxsdk/  mcuxsdk/cmake/toolchain/armgcc.cmake"
    exit 1
}

# Normalize : forward slashes work cross-platform-ish in CMake
$normalized = $candidate.Replace('\', '/')

# 3. Persist as user-scoped env var (survives shell restart, no admin needed)
[Environment]::SetEnvironmentVariable('MCUXSDK_DIR', $normalized, 'User')
$env:MCUXSDK_DIR = $normalized

Write-Host ""
Write-Host "MCUXSDK_DIR set (User scope):"
Write-Host "  $normalized"
Write-Host ""
Write-Host "Next steps:"
Write-Host "  1. Restart VS Code so it picks up the new environment variable."
Write-Host "  2. Open this folder in VS Code."
Write-Host "  3. CMake Tools should resolve -DSdkRootDirPath=$normalized/mcuxsdk on configure."
