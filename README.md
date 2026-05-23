# 3DS Random Game Launcher

Can't decide what to play? Let your 3DS pick for you.

## Project map

| Path | What it is |
|------|------------|
| `source/` | **App source** — `main.c` and `title_database.c` |
| `Makefile`, `build.bat` | Build `.3dsx` homebrew |
| `dist/` | Versioned build output (gitignored) |
| `scripts/` | Python tools to refresh the title database |
| `meta/`, `unistore.json` | Universal Updater distribution assets |
| `tools/` | CIA build binaries, RSF, and tooling docs |
| `build_cia.bat` | One-step `.cia` packaging (Windows) |
| `docs/` | Developer and tester documentation |

**Roadmap:** [docs/TITLE_RESOLUTION_ROADMAP.md](docs/TITLE_RESOLUTION_ROADMAP.md) — offline catalog (rebuilt), hardware testing, picker filtering, on-device SMDH fallback.

**Quick start for development:** edit `source/`, run `make` or `build.bat release`, grab the `.3dsx` from `dist/`.

## What it does

Scans your SD card and launches a random installed title. Names come from the built-in offline database (8,714 entries). There is **no category filtering** yet — updates, DLC, VC, and base games can all be picked if installed and listed in the database. Filtering options will follow hardware testing.

**Controls:**
- `A` - Launch the selected title
- `Y` - Reroll for something else
- `X` - Toggle homebrew mode (include titles not in the database)
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

The build system reads the semver from [`VERSION`](VERSION) and writes artifacts to `dist/` (e.g. `3DS-Random-Game-Launcher-v0.1.9.3dsx`). See [docs/VERSIONING.md](docs/VERSIONING.md).

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

### CIA builds

Installable `.cia` packaging uses prebuilt tools in `tools/bin/` — no submodules. Run `build_cia.bat` or see [tools/README.md](tools/README.md).

### Documentation

- [docs/README.md](docs/README.md) — full doc index
- [docs/VERSIONING.md](docs/VERSIONING.md) — semver, releases, legacy v18→0.1.8 mapping
- [docs/TITLE_RESOLUTION_ROADMAP.md](docs/TITLE_RESOLUTION_ROADMAP.md) — title lookup roadmap
- [docs/MAIN_C_FUNCTIONALITY.md](docs/MAIN_C_FUNCTIONALITY.md) — how `main.c` works
- [docs/TESTING_GUIDE.md](docs/TESTING_GUIDE.md) — testing checklist

## Game Database Sources

The title database (`source/title_database.c`) maps 3DS title IDs to display names for Layer 1 offline lookup. The shipped database has **8,714 entries** (base games, Virtual Console, DSiWare, updates, DLC, videos). Regenerate with `scripts/build_title_database.py`. Picker-side category filtering is **not implemented yet** — planned after hardware testing.

**Rebuild priority** (see [docs/TITLE_RESOLUTION_ROADMAP.md](docs/TITLE_RESOLUTION_ROADMAP.md)):

1. [hax0kartik/3dsdb](https://github.com/hax0kartik/3dsdb/tree/master/jsons) — regional eShop JSONs (primary names)
2. [ghost-land/3dsdb](https://github.com/ghost-land/3dsdb) — bulk category JSON (`data/initial_data/*.json`)
3. [3dsdb.com](https://3dsdb.com/xml.php) — XML gap fill

The legacy `api.ghseshop.cc` endpoint is retired. Do not use the Nlib API for offline catalog builds — download [ghost-land/3dsdb](https://github.com/ghost-land/3dsdb) JSON directly instead.

**Note for developers**: `title_database.c` may be useful to other 3DS homebrew that needs a broad ID→name table.

## The story

Started as a basic proof-of-concept by [einso](https://github.com/einso) - solid foundation but needed some love. Most of the heavy lifting here was done by AI (Claude/GPT) because let's be real, parsing 3DS title databases and handling all the edge cases is tedious as hell.

The original was functional but rough around the edges. This version adds proper error handling, a rebuilt offline catalog, and some polish. Category filtering and richer homebrew support are next, after hardware testing.

## What's next

Near term: **hardware testing** with the rebuilt 8,714-entry database, then a **filtering suite** in the app (categories, user preferences) based on real-world results. Homebrew mode will be expanded later (SMDH names, clearer UX).

Longer term ideas:

- **GUI overhaul** - The text interface works but a proper UI would be sick
- **Better packaging** - Improved distribution and installation
- **Game carousel** - Show actual titles with covers in a carousel instead of just text
- **Icons** - Display game artwork
- **Favorites/blacklist** - Let users customize what gets picked
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
