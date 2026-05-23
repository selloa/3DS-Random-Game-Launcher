@echo off
setlocal EnableDelayedExpansion

cd /d "%~dp0"

echo 3DS Random Game Launcher - Banner Build
echo ========================================
echo.

echo Step 1: Sync icon from repo root...
copy /Y "..\..\icon.png" "assets\icon.png" >nul
if %errorlevel% neq 0 (
    echo Failed to copy icon.png to assets/
    exit /b 1
)

echo Step 2: Generate banners (version from VERSION file)...
python generate.py
if %errorlevel% neq 0 (
    echo.
    echo generate.py failed!
    echo Install dependency: pip install pillow
    exit /b 1
)

echo Step 3: Copy outputs to meta/...
copy /Y "output\banner-large.png" "..\banner-large.png" >nul
if %errorlevel% neq 0 exit /b 1
copy /Y "output\banner.png" "..\banner.png" >nul
if %errorlevel% neq 0 exit /b 1
copy /Y "output\cia-banner.png" "..\cia-banner.png" >nul
if %errorlevel% neq 0 exit /b 1

echo.
echo Banner build completed successfully!
echo Updated: meta\banner-large.png, meta\banner.png, meta\cia-banner.png
echo Run build_cia.bat to embed cia-banner.png in the CIA.
