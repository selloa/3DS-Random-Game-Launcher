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
            echo ""
            echo "Output files are in the dist/ directory with incremental naming."
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
            echo ""
            echo "Output files are in the dist/ directory with incremental naming."
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
    "list")
        echo "Listing available builds..."
        if [ -d "dist" ]; then
            echo ""
            echo "Available builds in dist/:"
            ls -la dist/*.3dsx 2>/dev/null || echo "No builds found in dist/ directory."
        else
            echo "No dist/ directory found."
        fi
        ;;
    *)
        echo "Usage: ./build.sh [debug|release|clean|list]"
        echo ""
        echo "debug   - Build with debug features enabled"
        echo "release - Build optimized release version"
        echo "clean   - Clean build files and dist directory"
        echo "list    - List available builds in dist/ directory"
        echo ""
        echo "If no parameter is provided, this help is shown."
        echo ""
        echo "Builds are automatically numbered and saved to dist/ directory."
        ;;
esac
