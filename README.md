# 3DS Random Game Launcher

Can't decide what to play? Let your 3DS pick for you.

## What it does

Scans your SD card, filters out system junk, and launches a random game. Simple as that.

**Controls:**
- `A` - Launch the selected title
- `Y` - Reroll for something else
- `START` - Exit

## Building

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

## The story

Started as a basic proof-of-concept by [einso](https://github.com/einso) - solid foundation but needed some love. Most of the heavy lifting here was done by AI (Claude/GPT) because let's be real, parsing 3DS title databases and handling all the edge cases is tedious as hell.

The original was functional but rough around the edges. This version adds proper error handling, better title filtering, and some polish. Still plenty of room for improvement though.

## What's next

This thing has potential. Some ideas for whoever wants to take it further:

- **GUI overhaul** - The text interface works but a proper UI would be sick
- **Game names** - Show actual titles with covers in a carousel instead of just text
- **Icons** - Display game artwork
- **Favorites/blacklist** - Let users customize what gets picked
- **Categories** - Filter by genre
- **Stats** - Track what you actually play
- **Config file** - Save preferences

The codebase is clean and well-commented. Perfect for someone to jump in and add features.

## Credits

- **einso** - Original concept and implementation
- **DevKitPro team** - For the 3ds-examples that made this possible
- **AI assistants** - Did most of the boring parsing and error handling work
- **3DS homebrew community** - For keeping the scene alive

## License

Open source. Check the original repo for details.

---

*Built with libctru, powered by caffeine, and assisted by artificial intelligence.*