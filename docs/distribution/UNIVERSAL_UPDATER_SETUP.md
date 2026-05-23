# Universal Updater QR Code Setup Guide

This guide explains how to set up a QR code for Universal Updater so users can easily install your 3DS Random Game Launcher homebrew app.

## How Universal Updater QR Codes Work

Universal Updater uses **UniStore** JSON files to list available apps. When a user scans a QR code:
1. The QR code contains a URL pointing to a UniStore JSON file
2. Universal Updater downloads and parses the JSON file
3. The app appears in Universal Updater's app list
4. Users can then download and install it directly

## What the QR Code Links To

The QR code should link to the **URL of your UniStore JSON file** (not directly to the `.3dsx` file).

For example:
```
https://raw.githubusercontent.com/selloa/3DS-Random-Game-Launcher/main/unistore.json
```

Or if hosting elsewhere:
```
https://yourdomain.com/3DS-Random-Game-Launcher/unistore.json
```

## Setup Steps

### 1. Host Your Files

You need to host:
- **UniStore JSON file** (`unistore.json`) - Contains app metadata
- **App file** (`.3dsx`) - The actual homebrew executable
- **Icon** (`icon.png`) - App icon (optional but recommended)
- **Banner** (`meta/banner.png`) - App banner (optional but recommended)
- **Screenshot** (`meta/screenshot.png`) - App screenshot (optional but recommended)

**Option A: GitHub (Recommended)**
- Upload `unistore.json` to your repository root
- Upload `.3dsx` files to GitHub Releases
- Use GitHub's raw file URLs (e.g., `https://raw.githubusercontent.com/...`)

**Option B: Your Own Server**
- Host all files on a web server with HTTPS
- Ensure files are publicly accessible

### 2. Update the UniStore JSON

Edit `unistore.json` and update:
- `downloadUrl`: Direct URL to your latest `.3dsx` file
- `iconUrl`: URL to your icon (if hosted)
- `bannerUrl`: URL to your banner (if hosted)
- `screenshots`: Array of screenshot URLs (if hosted)
- `version`: Must match [`VERSION`](../VERSION) in the repo root (see [VERSIONING.md](../VERSIONING.md))

**Important URLs:**
- For GitHub Releases: `https://github.com/USERNAME/REPO/releases/latest/download/FILENAME.3dsx`
- For GitHub raw files: `https://raw.githubusercontent.com/USERNAME/REPO/BRANCH/PATH`

### 3. Generate the QR Code

Use any QR code generator to create a QR code containing the URL to your `unistore.json` file.

**Online QR Code Generators:**
- https://www.qr-code-generator.com/
- https://qr-code-generator.com/
- https://www.the-qrcode-generator.com/

**QR Code Content Example:**
```
https://raw.githubusercontent.com/selloa/3DS-Random-Game-Launcher/main/unistore.json
```

### 4. Test the Setup

1. Generate the QR code
2. Open Universal Updater on your 3DS
3. Go to Settings → Select UniStore → Tap "+" → Scan QR Code
4. Scan your QR code
5. Verify your app appears in the list
6. Test downloading and installing

## Example Workflow

1. **Release a new version:**
   - Bump [`VERSION`](../VERSION) per [VERSIONING.md](../VERSIONING.md)
   - Build your `.3dsx` file (`make` → `dist/3DS-Random-Game-Launcher-vX.Y.Z.3dsx`)
   - Create a GitHub Release (tag `vX.Y.Z`) and upload the `.3dsx` file
   - Update `unistore.json` with the new version string and download URL
   - Commit and push `unistore.json` to your repository

2. **Users install:**
   - Users scan your QR code (which points to `unistore.json`)
   - Universal Updater adds your UniStore
   - Users see "3DS Random Game Launcher" in the app list
   - Users tap to download and install

## Tips

- **Keep URLs updated**: When you release new versions, update the `downloadUrl` in `unistore.json`
- **Use GitHub Releases**: GitHub Releases provide stable download URLs that don't change
- **HTTPS required**: Universal Updater requires HTTPS URLs for security
- **Version numbers**: Keep `unistore.json` in sync with [`VERSION`](../VERSION)
- **Test first**: Always test the QR code yourself before sharing it publicly

## Alternative: Direct File QR Code (FBI)

If you want users to install directly via FBI instead of Universal Updater:
- Generate a QR code pointing directly to your `.3dsx` or `.cia` file
- Users scan with FBI → Remote Install → Scan QR Code
- This bypasses Universal Updater entirely

However, Universal Updater is preferred because:
- Users can see app info, screenshots, and descriptions
- Easier to update (just update the JSON)
- Better user experience

## Troubleshooting

**QR code doesn't work:**
- Ensure the URL is accessible (test in a browser)
- Check that the URL uses HTTPS (not HTTP)
- Verify the JSON file is valid JSON

**App doesn't appear:**
- Check that `unistore.json` is valid JSON
- Verify all required fields are present
- Check the console for error messages

**Download fails:**
- Ensure the `downloadUrl` is publicly accessible
- Check that the file exists at that URL
- Verify the URL uses HTTPS
