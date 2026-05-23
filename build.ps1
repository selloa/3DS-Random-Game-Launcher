# PowerShell build wrapper - avoids MSYS cmd shadowing and .bat parenthesis pitfalls.
param(
    [Parameter(Position = 0)]
    [ValidateSet('debug', 'release', 'banners', 'clean', 'list', 'help')]
    [string] $Command = 'help'
)

$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot

function Show-Help {
    Write-Host '3DS Random Game Launcher Build Script'
    Write-Host '====================================='
    Write-Host ''
    Write-Host 'Usage: .\build.ps1 [debug|release|banners|clean|list]'
    Write-Host ''
    Write-Host 'debug   - Build with debug features enabled'
    Write-Host 'release - Build optimized release version'
    Write-Host 'banners - Regenerate meta/banner*.png from meta/banner-src/'
    Write-Host 'clean   - Clean build files and dist directory'
    Write-Host 'list    - List available builds in dist/'
}

switch ($Command) {
    'debug' {
        Write-Host 'Building DEBUG version...'
        & make DEBUG=1
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        Write-Host 'DEBUG build completed. Output in dist/.'
    }
    'release' {
        Write-Host 'Building RELEASE version...'
        & make
        if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
        Write-Host 'RELEASE build completed. Output in dist/.'
    }
    'banners' {
        & "$Root\meta\banner-src\build.ps1"
    }
    'clean' {
        Write-Host 'Cleaning build files...'
        & make clean
        Write-Host 'Clean completed.'
    }
    'list' {
        if (Test-Path "$Root\dist") {
            Get-ChildItem "$Root\dist\*.3dsx" | ForEach-Object { $_.Name }
        } else {
            Write-Host 'No dist/ directory found.'
        }
    }
    default {
        Show-Help
    }
}
