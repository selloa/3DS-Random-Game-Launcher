@echo off
setlocal EnableExtensions
REM Re-render all diagrams under diagrams/ (SVG + PNG, plus -solid when applicable).
REM Usage:
REM   render.bat
REM   render.bat --phase 02-software-ecosystem
REM   render.bat --no-solid-variant
REM   render.bat diagrams\02-software-ecosystem\ecosystem.mmd

set "ROOT=%~dp0.."
cd /d "%ROOT%" || exit /b 1

where python >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "PY=python"
    goto :run
)

where py >nul 2>&1
if %ERRORLEVEL% equ 0 (
    set "PY=py -3"
    goto :run
)

echo Python 3 not found. Install Python and ensure it is on PATH.
pause
exit /b 1

:run
if "%~1"=="" (
    %PY% tools\render.py --all
) else (
    %PY% tools\render.py %*
)
set "EC=%ERRORLEVEL%"
if %EC% neq 0 (
    echo.
    echo Render failed with exit code %EC%.
    pause
)
exit /b %EC%
