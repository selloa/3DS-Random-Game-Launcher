# 3DS Random Game Launcher

Can't decide what to play? Let your 3DS pick for you.

## What it does

Scans your SD card, filters out system junk, and launches a random game. Simple as that.

**Controls:**
- `A` - Launch the selected title
- `Y` - Reroll for something else
- `START` - Exit

## Download (For Users)

**Easy Setup on 3DS**

1. **Open Universal Updater** on your 3DS
2. **Search for "3DS Random Game Launcher"** and install the CIA file
3. **Exit Universal Updater** - wait a moment for the new app to appear on Homescreen
4. **Launch 3DS Random Game Launcher** from the 3DS Homescreen

**Don't want to build? Just download and play!**

1. **Click the "Releases" link** on the right side of this page (or go to https://github.com/selloa/3DS-Random-Game-Launcher/releases)
2. **Download the latest release** - look for the `.3dsx` file (that's what you need)
3. **Copy the `.3dsx` file** to your 3DS SD card in the `/3ds/` folder
4. **Launch from Homebrew Launcher** on your 3DS

**That's it!** No building required. The `.3dsx` file is ready to run.

## Building (For Developers)

**Dependencies:**
- [devkitPro](https://devkitpro.org/) with devkitARM
- 3ds-dev libraries

You know the drill:
```bash
git clone https://github.com/selloa/3DS-Random-Game-Launcher.git
cd 3DS-Random-Game-Launcher
make
```

The build system automatically creates incremental versions in the `dist/` directory with numbered filenames (e.g., `3DS-Random-Game-Launcher-v1.3dsx`).

### Build Scripts

For easier building, use the provided scripts:
```bash
# Windows
./build.bat release    # Release build
./build.bat debug      # Debug build
./build.bat list       # List available builds
./build.bat clean      # Clean build files

# Linux/Mac
./build.sh release     # Release build
./build.sh debug       # Debug build
./build.sh list        # List available builds
./build.sh clean       # Clean build files
```

### Debug Build

For development and testing, you can build a debug version with additional logging:
```bash
make DEBUG=1
```

Debug builds include the `-debug` suffix and show verbose output during execution.

## Game Database Sources

The title database (`source/title_database.c`) contains **4,135+ 3DS game entries** with title IDs and names. Data was sourced from the comprehensive [3dsdb community database](https://3dsdb.com/) via multiple methods:

- **3dsdb API**: `https://api.ghseshop.cc` (base games, virtual console, DSiWare)
- **3dsdb GitHub**: `https://raw.githubusercontent.com/hax0kartik/3dsdb/master/jsons` (regional data)
- **3dsdb XML**: `https://3dsdb.com/xml.php` (complete database export)

The database includes games from all regions (USA, EUR, JPN) with both English and original Japanese/Chinese titles. Scripts in the `scripts/` directory can be used to update the database with the latest entries.

**Note for developers**: The `title_database.c` file might be interesting for other 3DS homebrew projects that need a complete list of all title names with their corresponding title IDs.

## The story

Started as a basic proof-of-concept by [einso](https://github.com/einso) - solid foundation but needed some love. Most of the heavy lifting here was done by AI (Claude/GPT) because let's be real, parsing 3DS title databases and handling all the edge cases is tedious as hell.

The original was functional but rough around the edges. This version adds proper error handling, better title filtering, and some polish. Still plenty of room for improvement though.

## What's next

This thing has potential. Some ideas for whoever wants to take it further:

- **GUI overhaul** - The text interface works but a proper UI would be sick
- **Better packaging** - Improved distribution and installation
- **Game carousel** - Show actual titles with covers in a carousel instead of just text
- **Icons** - Display game artwork
- **Favorites/blacklist** - Let users customize what gets picked
- **Categories** - Filter by genre
- **Stats** - Track what you actually play
- **Config file** - Save preferences

The codebase is clean and well-commented. Perfect for someone to jump in and add features.

## Credits

- **einso** - Original concept and implementation
- **DevKitPro team** - For the 3ds-examples that made this possible (app_launch template)
- **AI assistants** - Did most of the boring parsing and error handling work
- **3DS homebrew community** - For keeping the scene alive

**Note**: This project uses code from the [devkitPro 3ds-examples app_launch template](https://github.com/devkitPro/3ds-examples/blob/master/app_launch/source/main.c) under MIT license.

## License

MIT License

Copyright (c) 2025 selloa

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
---

*Built with libctru, powered by caffeine, and assisted by artificial intelligence.*
