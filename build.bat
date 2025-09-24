@echo off
echo 3DS Random Game Launcher Build Script
echo =====================================

if "%1"=="debug" (
    echo Building DEBUG version...
    make DEBUG=1
    if %errorlevel% equ 0 (
        echo.
        echo DEBUG build completed successfully!
        echo Debug features enabled:
        echo - Verbose debug output
        echo - Additional title information
        echo - No optimization for easier debugging
    ) else (
        echo.
        echo DEBUG build failed!
        echo Please check the error messages above.
    )
) else if "%1"=="release" (
    echo Building RELEASE version...
    make
    if %errorlevel% equ 0 (
        echo.
        echo RELEASE build completed successfully!
        echo Optimized for performance and smaller size.
    ) else (
        echo.
        echo RELEASE build failed!
        echo Please check the error messages above.
    )
) else if "%1"=="clean" (
    echo Cleaning build files...
    make clean
    echo Clean completed.
) else (
    echo Usage: build.bat [debug^|release^|clean]
    echo.
    echo debug   - Build with debug features enabled
    echo release - Build optimized release version
    echo clean   - Clean build files
    echo.
    echo If no parameter is provided, this help is shown.
)

pause
