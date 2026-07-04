# 3DS Random Game Launcher - Main.c Functionality Documentation

## Overview
The `main.c` file is the core of a Nintendo 3DS homebrew application that randomly selects and launches games from the user's installed game library. It provides an interactive interface for users to discover and play games they might not have tried otherwise.

**Current picker policy (May 2026):** display names come from **SMDH first** (catalog DB fallback, then title ID hex). The random pool uses **category filters** (patches, DLC, system, demos, DSiWare, content packs — off by default; native apps, Virtual Console — on by default) and a **catalog allowlist** when homebrew mode is off. Open **SELECT** for the filter menu; **L/R** pages show metadata for testing.

## Core Functionality

### 1. System Initialization
The application starts by initializing essential 3DS system components:

- **Graphics System**: `gfxInitDefault()` - Sets up the 3DS graphics subsystem
- **Console**: `consoleInit(GFX_TOP, NULL)` - Initializes text output on the top screen
- **Application Manager**: `amInit()` — Enables the app to interact with other installed titles
- **Filesystem**: `fsInit()` — Required for on-device SMDH reads

### 2. Game Discovery
The app queries installed titles via the Application Manager:

```c
AM_GetTitleList(..., MEDIATYPE_SD, ...);
AM_GetTitleList(..., MEDIATYPE_NAND, ...);
```

- **SD** titles are always scanned (up to 900)
- **NAND** titles are scanned at startup; included in the picker only when **Include NAND titles** is ON in the SELECT menu (default OFF)
- Duplicate title IDs on SD and NAND are deduplicated (SD wins)
- Launch uses the correct media type (`MEDIATYPE_SD` or `MEDIATYPE_NAND`) for chainloading and SMDH reads

### 3. Random Game Selection Algorithm

#### Eligible pool (`title_picker.c`)

Before each pick, the app builds a pool of scanned titles that pass:

1. **Category filters** (default: retail/VC applications `0x0000`; native apps and Virtual Console included; patches `0x000E`, DLC `0x008C`, system `0x0005`–`0x0009`, demos, DSiWare, content packs, DLP child, certificate store excluded)
2. **Optional toggles** (SELECT menu): include patches, DLC, system, demos, DSiWare, content packs, native apps, or Virtual Console
3. **Homebrew mode** (default OFF): only title IDs in `title_database.c` may be picked
4. **Homebrew mode ON**: unknown IDs (not in catalog) may be picked; DB classifies them as homebrew

Category is decoded from the title ID: `(u16)((titleId >> 32) & 0xFFFF)`.

#### Display names (SMDH-primary)

For each picked title, names resolve in order:

1. SMDH short name (`title_smdh_load`)
2. Catalog name from `lookup_game_name()` if SMDH fails
3. 16-digit hex title ID

The static database is **not** the primary name source when SMDH succeeds.

#### Homebrew mode

- **OFF (default):** catalog allowlist — only known catalog title IDs are eligible
- **ON:** includes titles not in the catalog; summary page shows `Homebrew: Yes` when applicable
- Toggle via **X** or the SELECT filter menu

### 4. User Interface

#### Display Information
For each selected game, the app shows (L/R pages):
- **Name** from SMDH (with source line: SMDH / Catalog fallback / Title ID)
- **Homebrew: Yes** when homebrew mode is on and the title is not in the catalog
- SMDH metadata (long name, publisher, ratings, flags)
- Title ID / AM metadata (category, product code, version, size)
- Current filter and homebrew mode status

#### Control Scheme
- **A Button**: Launch the selected game
- **Y Button**: Pick a new random title from the eligible pool
- **X Button**: Toggle homebrew mode (rebuilds pool)
- **SELECT**: Open filter menu (patches / DLC / system / demos / DSiWare / content / native / VC / NAND / homebrew)
- **L / R**: Previous / next metadata page
- **START Button**: Exit the application

#### Filter menu (SELECT)
- **Up/Down**: Move cursor
- **A**: Toggle selected row
- **B** or **SELECT**: Close menu and rebuild pool

### 5. Game Launching
When the user presses A:
```c
aptSetChainloader(randomTitle, MEDIATYPE_SD);
```
- Uses the 3DS Application Manager to launch the selected title
- The current app exits and the selected game starts
- No return value handling (success/failure is implicit)

### 6. Error Handling

#### No Games Found
- Displays helpful message to install games
- Waits for START button to exit

#### Selection Failures
- Implements a retry mechanism (max 100 attempts)
- Prevents infinite loops if no valid games are found
- Shows error message if selection consistently fails

#### System Initialization Errors
- Graceful error handling with cleanup
- User-friendly error messages
- Proper resource cleanup on exit

## Key Data Structures

### Global Variables
- `g_include_homebrew`: Boolean flag controlling homebrew inclusion
- `readTitlesID[900]`: Array storing up to 900 title IDs from SD card
- `readTitlesAmount`: Count of actually found titles

### Title Database Structure
```c
typedef struct {
    u64 title_id;        // 64-bit Nintendo title identifier
    const char* game_name; // Human-readable game name
} title_entry_t;
```

## Technical Implementation Details

### Random Number Generation
```c
srand((unsigned) time(&t));
```
- Seeds random number generator with current time
- Ensures different random sequences on each app launch

### Content Category Extraction
```c
unsigned char contentCategory = ((unsigned char*)(&readTitlesID[randomTitlePicked]))[4];
```
- Extracts the 5th byte from the title ID (Nintendo content category)
- **Not used for filtering today** — reserved for future picker rules

### Memory Management
- Uses static arrays for title storage (no dynamic allocation)
- Proper cleanup of system resources on exit
- No memory leaks or dangling pointers

## User Experience Features

### Visual Feedback
- Clear header with app name and author
- Real-time display of homebrew mode status
- Immediate visual feedback for all button presses

### Accessibility
- Simple, intuitive control scheme
- Clear on-screen instructions
- Consistent button mapping throughout the app

### Robustness
- Handles edge cases (no games, invalid titles)
- Prevents infinite loops with attempt limits
- Graceful degradation when database lookups fail

## Integration with 3DS System

### Application Manager Integration
- Proper initialization and cleanup of AM services
- Uses official Nintendo APIs for title launching
- Respects 3DS security model and title permissions

### Graphics System
- Uses 3DS graphics subsystem for display
- Proper frame synchronization with `gspWaitForVBlank()`
- Double buffering for smooth display updates

### Input Handling
- Real-time input scanning with `hidScanInput()`
- Immediate response to button presses
- Proper key state management

## Database Integration

The app relies on `title_database.c`, regenerated via `scripts/build_title_database.py`:

- **8,714 entries** — hax0kartik names + ghost-land/3dsdb bulk JSON + 3dsdb.com XML gap fill
- **Multi-region support** (Japanese, English, Korean, etc. variants)
- **Linear search** via `lookup_game_name()`
- **Broad coverage** — base apps, Virtual Console, DSiWare, updates, DLC, videos

The catalog is intentionally maximal. What the random picker should *launch* (vs. only *name*) will be decided after hardware testing.

## Build and Deployment

The application compiles to multiple formats:
- `.3dsx` - Homebrew launcher format
- `.cia` - Installable CIA format
- `.elf` - Executable format for development

## Summary

The 3DS Random Game Launcher is a well-designed homebrew application that provides a fun way to discover games in a user's library. It combines robust system integration, comprehensive game database support, and an intuitive user interface to create a polished gaming experience. The code demonstrates good practices in 3DS homebrew development, including proper resource management, error handling, and user experience design.
