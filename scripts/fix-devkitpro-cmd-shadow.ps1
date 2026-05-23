# Fix devkitPro MSYS shadowing Windows cmd.exe
#
# Problem: c:\devkitPro\msys2\usr\bin\cmd (a bash script, no extension) is listed
# before C:\Windows\System32\cmd.exe on PATH. PowerShell and tools that run
# "cmd /c ..." then hit the MSYS shim and may open the Windows app picker.
#
# Fix: rename the MSYS shim so "cmd" resolves to System32\cmd.exe.
# Safe: the shim only forwards to %COMSPEC% when invoked from MSYS bash.
#
# Run once (may need an elevated PowerShell if rename is denied):
#   powershell -ExecutionPolicy Bypass -File scripts/fix-devkitpro-cmd-shadow.ps1
#
# devkitPro/MSYS updates may restore usr\bin\cmd — re-run this script if that happens.

#Requires -Version 5.1
$ErrorActionPreference = 'Stop'

$MsysCmd = 'C:\devkitPro\msys2\usr\bin\cmd'
$MsysBackup = 'C:\devkitPro\msys2\usr\bin\cmd.msys-shim'

if (-not (Test-Path 'C:\devkitPro\msys2\usr\bin')) {
    Write-Host 'devkitPro MSYS not found — nothing to do.'
    exit 0
}

if (Test-Path $MsysBackup) {
    Write-Host "Already fixed: $MsysBackup exists."
} elseif (Test-Path $MsysCmd) {
    Rename-Item -LiteralPath $MsysCmd -NewName 'cmd.msys-shim'
    Write-Host "Renamed $MsysCmd -> $MsysBackup"
} else {
    Write-Host 'MSYS cmd shim not found — already removed or devkitPro layout changed.'
}

$winCmd = (Get-Command cmd.exe -ErrorAction SilentlyContinue).Source
Write-Host "cmd.exe resolves to: $winCmd"

$probe = cmd /c "echo CMD_PROBE_OK" 2>&1
Write-Host "cmd /c probe: $probe"

if ($probe -ne 'CMD_PROBE_OK') {
    Write-Error 'cmd /c probe failed — check PATH manually.'
    exit 1
}

Write-Host 'Done. Open a new terminal for PATH/command cache to refresh everywhere.'
