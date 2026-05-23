# Regenerate banner PNGs and copy into meta/
$ErrorActionPreference = 'Stop'
$Here = $PSScriptRoot
$Root = Resolve-Path "$Here\..\.."

Write-Host '3DS Random Game Launcher - Banner Build'
Write-Host '========================================'
Write-Host ''

Write-Host 'Step 1: Sync icon from repo root...'
Copy-Item "$Root\icon.png" "$Here\assets\icon.png" -Force

Write-Host 'Step 2: Generate banners - version from VERSION file...'
Push-Location $Here
try {
    & python generate.py
    if ($LASTEXITCODE -ne 0) { throw 'generate.py failed. Install: pip install pillow' }
} finally {
    Pop-Location
}

Write-Host 'Step 3: Copy outputs to meta/...'
Copy-Item "$Here\output\banner-large.png" "$Here\..\banner-large.png" -Force
Copy-Item "$Here\output\banner.png" "$Here\..\banner.png" -Force
Copy-Item "$Here\output\cia-banner.png" "$Here\..\cia-banner.png" -Force

Write-Host ''
Write-Host 'Banner build completed successfully!'
Write-Host 'Updated: meta\banner-large.png, meta\banner.png, meta\cia-banner.png'
Write-Host 'Run build_cia.bat to embed cia-banner.png in the CIA.'
