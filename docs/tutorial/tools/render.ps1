# Render Mermaid via Kroki. Requires Python 3 on PATH.
# Run from work folder root or tools/ — both work.
# Examples:
#   .\tools\render.ps1 -Phase 01-pre-tooling-community
#   .\tools\render.ps1 -All

param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]] $MmdFiles,

    [switch] $All,
    [switch] $NoSolidVariant,
    [string] $Phase,
    [ValidateSet("png", "svg", "pdf", "jpeg", "jpg", "base64")]
    [string] $Format = "png",
    [string] $Output,
    [string] $Kroki = "https://kroki.io",
    [string] $Type = "mermaid"
)

$ToolsDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$WorkRoot = Split-Path -Parent $ToolsDir

$Py = Get-Command python -ErrorAction SilentlyContinue
if (-not $Py) {
    $Py = Get-Command py -ErrorAction SilentlyContinue
}
if (-not $Py) {
    Write-Error "Python 3 not found. Install Python or run: python tools/render.py ..."
    exit 1
}

$Args = @("$ToolsDir\render.py", "-f", $Format, "--kroki", $Kroki, "--type", $Type)

if ($Output) {
    $Args += @("-o", $Output)
}
if ($All) {
    $Args += "--all"
}
if ($Phase) {
    $Args += @("--phase", $Phase)
}
if ($NoSolidVariant) {
    $Args += "--no-solid-variant"
}
if ($MmdFiles) {
    $Args += $MmdFiles
}

Push-Location $WorkRoot
try {
    & $Py.Source @Args
    exit $LASTEXITCODE
}
finally {
    Pop-Location
}
