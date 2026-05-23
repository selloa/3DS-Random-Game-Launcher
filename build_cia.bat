@echo off
setlocal EnableDelayedExpansion

echo 3DS Random Game Launcher - CIA Build
echo =====================================

set /p VERSION=<VERSION
if "%VERSION%"=="" set VERSION=0.0.0
set VERSION_TAG=v%VERSION%
set TARGET=3DS-Random-Game-Launcher
set OUT_NAME=%TARGET%-%VERSION_TAG%

if not exist build mkdir build
if not exist dist mkdir dist

echo Step 1: Building ELF...
make
if %errorlevel% neq 0 (
    echo.
    echo ELF build failed!
    exit /b 1
)

echo.
echo Step 2: Creating banner and icon...
tools\bin\bannertool.exe makebanner -i meta\cia-banner.png -a meta\audio.wav -o build\banner.bnr
if %errorlevel% neq 0 (
    echo.
    echo bannertool makebanner failed!
    exit /b 1
)

tools\bin\bannertool.exe makesmdh -s "3DS Random Launcher" -l "3DS Random Game Launcher" -p "selloa" -i icon.png -o build\icon.icn
if %errorlevel% neq 0 (
    echo.
    echo bannertool makesmdh failed!
    exit /b 1
)

echo.
echo Step 3: Packaging CIA...
tools\bin\makerom.exe -f cia -o dist\%OUT_NAME%.cia -rsf tools\3DS-Random-Game-Launcher.rsf -target t -elf %TARGET%.elf -icon build\icon.icn -banner build\banner.bnr
if %errorlevel% neq 0 (
    echo.
    echo makerom failed!
    exit /b 1
)

echo.
echo CIA build completed successfully!
echo Output: dist\%OUT_NAME%.cia
