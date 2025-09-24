# 3DS Random Game Launcher - Main.c Functionality Documentation

## Overview
The `main.c` file is the core of a Nintendo 3DS homebrew application that randomly selects and launches games from the user's installed game library. It provides an interactive interface for users to discover and play games they might not have tried otherwise.

## Core Functionality

### 1. System Initialization
The application starts by initializing essential 3DS system components:

- **Graphics System**: `gfxInitDefault()` - Sets up the 3DS graphics subsystem
- **Console**: `consoleInit(GFX_TOP, NULL)` - Initializes text output on the top screen
- **Application Manager**: `amInit()` and `amAppInit()` - Enables the app to interact with other installed titles

### 2. Game Discovery
The app scans the SD card for installed games:

```c
AM_GetTitleList(&readTitlesAmount, MEDIATYPE_SD, 900, readTitlesID);
```

- Retrieves up to 900 installed titles from the SD card
- Stores title IDs in an array for random selection
- Handles the case where no games are found

### 3. Random Game Selection Algorithm

The core random selection process works as follows:

#### Title Category Filtering
The app only considers titles with specific content categories:
- **Category 0x00**: Regular 3DS applications
- **Category 0x02**: System applications
- Other categories are filtered out (DLC, updates, etc.)

#### Database Lookup
For each randomly selected title:
1. Extracts the title ID from the system
2. Looks up the game name in the built-in database (`title_database.c`)
3. The database contains 4,135 entries covering:
   - Regular 3DS games
   - Virtual Console games
   - DSiWare titles
   - Multi-region variants

#### Homebrew Support
The app includes a toggle for homebrew games:
- **Homebrew Mode OFF**: Only shows games found in the database
- **Homebrew Mode ON**: Includes homebrew titles not in the database
- Homebrew games display their Title ID instead of a friendly name

### 4. User Interface

#### Display Information
For each selected game, the app shows:
- **Regular Games**: Game name from database
- **Homebrew Games**: "Homebrew Game" + Title ID
- Current homebrew mode status

#### Control Scheme
- **A Button**: Launch the selected game
- **Y Button**: "Throw the dice again" - select a new random game
- **X Button**: Toggle homebrew mode on/off
- **START Button**: Exit the application

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
- Extracts the 5th byte from the title ID
- This byte contains the content category information

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

The app relies heavily on the `title_database.c` file which contains:
- **4,135 game entries** covering the entire 3DS library
- **Multi-region support** (Japanese, English, Korean variants)
- **Efficient lookup function** using linear search
- **Comprehensive coverage** including Virtual Console and DSiWare

## Build and Deployment

The application compiles to multiple formats:
- `.3dsx` - Homebrew launcher format
- `.cia` - Installable CIA format
- `.elf` - Executable format for development

## Summary

The 3DS Random Game Launcher is a well-designed homebrew application that provides a fun way to discover games in a user's library. It combines robust system integration, comprehensive game database support, and an intuitive user interface to create a polished gaming experience. The code demonstrates good practices in 3DS homebrew development, including proper resource management, error handling, and user experience design.
