# 3DS Random Game Launcher

Can't decide what to play? Let your 3DS pick for you.

## What it does

Scans your SD card, filters out system junk, and launches a random game. Simple as that.

**Controls:**
- `A` - Launch the selected title
- `Y` - Reroll for something else
- `START` - Exit

## Download (For Users)

**Don't want to build? Just download and play!**

1. **Click the "Releases" link** on the right side of this page (or go to https://github.com/selloa/3DS-Random-Game-Launcher/releases)
2. **Download the latest release** - look for the `.3dsx` file (that's what you need)
3. **Copy the `.3dsx` file** to your 3DS SD card in the `/3ds/` folder
4. **Launch from Homebrew Launcher** on your 3DS

**That's it!** No building required. The `.3dsx` file is ready to run.

*Note: If you see CIA files in releases, those are for advanced users who want to install the app directly to their 3DS home menu.*

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

Stick the `.3dsx` in `/3ds/` on your SD card and launch from Homebrew Launcher.

### CIA Building Tools

The `3dsxlauncher_loader-1.1/` directory contains tools from [Rinnegatamante's 3dsxlauncher_loader](https://github.com/Rinnegatamante/3dsxlauncher_loader) for building CIA forwarders. **Note: While the tools themselves are functional, the current CIA file generation process in the Makefile is still buggy and needs work.** The existing CIA files in `releases/cia/` were built manually, can be installed, but ultimately don't run yet.

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
- **CIA compilation** - Provide CIA Files with each release
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

Open source. Check the original repo for details.

---

*Built with libctru, powered by caffeine, and assisted by artificial intelligence.*