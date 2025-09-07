# 3DS Random Game Launcher - Releases

This directory contains the compiled releases of the 3DS Random Game Launcher.

## Files

### CIA Installation
- **`cia/3DS-Random-Game-Launcher.cia`** - Standard CIA file (requires firmware 4.5.0+)
- **`cia/3DS-Random-Game-Launcher-Compatible.cia`** - Compatible CIA file (works with firmware 1.0.0+)

### Payload Files
- **`payload/pl/randomla`** - Required payload file for standard CIA
- **`payload/pl-compatible/00040000`** - Required payload file for compatible CIA

## Installation Instructions

### CIA Installation (Recommended)

#### For Older Firmware (1.0.0 - 4.4.x)
1. **Install the compatible CIA**: Use FBI to install `cia/3DS-Random-Game-Launcher-Compatible.cia`
2. **Place the payload**: Copy the `payload/pl-compatible` folder to the **root of your SD card** (so you have `SD:/pl-compatible/00040000`)

#### For Newer Firmware (4.5.0+)
1. **Install the standard CIA**: Use FBI to install `cia/3DS-Random-Game-Launcher.cia`
2. **Place the payload**: Copy the `payload/pl` folder to the **root of your SD card** (so you have `SD:/pl/randomla`)

### 3DSX Installation (Alternative)
- Place `3DS-Random-Game-Launcher.3dsx` in `/3ds/` on your SD card and launch from Homebrew Launcher

## How It Works

The CIA creates a forwarder on your home menu. When launched:
- It runs the `randomla` payload from your SD card
- The payload launches the actual `3DS-Random-Game-Launcher.3dsx` file
- The app scans your SD card for games and picks a random one to launch

## Controls
- **A** - Launch the selected title
- **Y** - Reroll for something else  
- **START** - Exit

## Requirements
- Custom firmware (CFW) on your 3DS
- FBI or similar CIA installer
- SD card with games installed
