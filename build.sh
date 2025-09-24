#!/bin/bash

echo "3DS Random Game Launcher Build Script"
echo "====================================="

case "$1" in
    "debug")
        echo "Building DEBUG version..."
        make DEBUG=1
        if [ $? -eq 0 ]; then
            echo ""
            echo "DEBUG build completed successfully!"
            echo "Debug features enabled:"
            echo "- Verbose debug output"
            echo "- Additional title information"
            echo "- No optimization for easier debugging"
        else
            echo ""
            echo "DEBUG build failed!"
            echo "Please check the error messages above."
        fi
        ;;
    "release")
        echo "Building RELEASE version..."
        make
        if [ $? -eq 0 ]; then
            echo ""
            echo "RELEASE build completed successfully!"
            echo "Optimized for performance and smaller size."
        else
            echo ""
            echo "RELEASE build failed!"
            echo "Please check the error messages above."
        fi
        ;;
    "clean")
        echo "Cleaning build files..."
        make clean
        echo "Clean completed."
        ;;
    *)
        echo "Usage: ./build.sh [debug|release|clean]"
        echo ""
        echo "debug   - Build with debug features enabled"
        echo "release - Build optimized release version"
        echo "clean   - Clean build files"
        echo ""
        echo "If no parameter is provided, this help is shown."
        ;;
esac
