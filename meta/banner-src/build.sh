#!/bin/bash
set -e

cd "$(dirname "$0")"

echo "3DS Random Game Launcher - Banner Build"
echo "========================================"
echo ""

echo "Step 1: Sync icon from repo root..."
cp "../../icon.png" "assets/icon.png"

echo "Step 2: Generate banners (version from VERSION file)..."
python3 generate.py

echo "Step 3: Copy outputs to meta/..."
cp "output/banner-large.png" "../banner-large.png"
cp "output/banner.png" "../banner.png"
cp "output/cia-banner.png" "../cia-banner.png"

echo ""
echo "Banner build completed successfully!"
echo "Updated: meta/banner-large.png, meta/banner.png, meta/cia-banner.png"
echo "Run build_cia.bat (Windows) to embed cia-banner.png in the CIA."
