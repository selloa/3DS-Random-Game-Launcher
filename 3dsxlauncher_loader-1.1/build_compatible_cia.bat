@echo off
echo Building compatible CIA for older firmware versions...
@del /Q result\pl 2>NUL
@rmdir result\pl 2>NUL
@del /Q result 2>NUL
@rmdir result 2>NUL

REM Set default values for compatibility
set hb=randomla
set title=3DS Random Game Launcher
set author=selloa
set unique_id=00040000

echo Using settings:
echo - Homebrew folder: %hb%
echo - Title: %title%
echo - Author: %author%
echo - Unique ID: %unique_id%

REM Create temporary directories
@mkdir tmp 2>NUL
@mkdir result 2>NUL
@mkdir result\pl 2>NUL

REM Create banner and icon
@bannertool makesmdh -s "%title%" -l "%title%" -p "%author%" -i files/icon.png -o tmp/icon.bin
@bannertool makebanner -i files/banner.png -a files/audio.wav -o tmp/banner.bin

echo Building compatible CIA file...
REM Use the compatible RSF file
@powershell -Command "(gc cia_compatible.rsf) -replace '(UniqueId\s+:)\s*.*$', '${1} 0x%unique_id%' | Out-File cia_compatible_temp.rsf"

REM Build the CIA with compatible settings
@makerom -f cia -o "result/%title%.cia" -elf loader.elf -rsf cia_compatible_temp.rsf -icon tmp/icon.bin -banner tmp/banner.bin -exefslogo -target t

REM Create the payload
@hex_set "%hb%"
@3dsxtool launcher2.elf files\%hb%
@del /Q launcher2.elf 2>NUL
@move /Y files\%hb% result\pl\%unique_id% 2>NUL

REM Clean up temporary files
@del /Q cia_compatible_temp.rsf 2>NUL

echo.
echo ========================================
echo Compatible CIA build completed!
echo ========================================
echo.
echo Files created:
echo - result/%title%.cia (CIA file)
echo - result/pl/%unique_id% (payload file)
echo.
echo Installation:
echo 1. Install the CIA file using FBI
echo 2. Copy the 'pl' folder to the root of your SD card
echo.
echo This version is compatible with firmware 1.0.0 and above.
echo.
pause
