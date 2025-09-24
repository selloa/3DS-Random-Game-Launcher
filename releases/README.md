# 3DS Random Game Launcher - Releases

This directory contains the compiled releases of the 3DS Random Game Launcher.

## Files

### 3DSX Installation (Recommended)
- **`3DS-Random-Game-Launcher.3dsx`** - Main executable file for Homebrew Launcher

## Installation Instructions

### 3DSX Installation

1. **Download the 3DSX file**: Get `3DS-Random-Game-Launcher.3dsx` from the releases
2. **Copy to SD card**: Place the `.3dsx` file in `/3ds/` on your SD card
3. **Launch**: Use Homebrew Launcher to run the application

**File structure on SD card:**
```
SD Card Root/
├── 3ds/
│   └── 3DS-Random-Game-Launcher.3dsx
├── Nintendo 3DS/
└── (other folders)
```

## How It Works

The 3DSX file is a standard homebrew executable that:
- Scans your SD card for installed games
- Filters out system titles and invalid entries
- Randomly selects a valid game from your library
- Launches the selected game using 3DS chainloading

## Controls
- **A** - Launch the selected title
- **Y** - Reroll for something else  
- **X** - Show system information (debug features)
- **START** - Exit

## Requirements
- Custom firmware (CFW) on your 3DS
- Homebrew Launcher
- SD card with games installed
- Compatible with all 3DS models and firmware versions
