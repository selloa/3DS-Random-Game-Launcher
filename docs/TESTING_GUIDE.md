# 3DS Random Game Launcher - Testing Guide

## Overview

This guide helps testers validate the launcher on real 3DS hardware — especially with the **rebuilt 8,714-entry offline database**. Report issues across different models, firmware versions, and library compositions.

**Current picker behavior:** no content-category filtering. Any installed SD title listed in the database can be randomly selected (including updates, DLC, Virtual Console, DSiWare). Category and user filtering will be added after this testing pass — do not treat unexpected title types as bugs unless launch or naming fails.

## Controls (current build)

| Button | Action |
|--------|--------|
| **A** | Launch the selected title |
| **Y** | Reroll — pick another random title |
| **X** | Toggle **homebrew mode** (include titles not in the database; shows hex title ID) |
| **START** | Exit |

Homebrew mode is basic today (hex ID fallback only). Richer homebrew handling is planned later.

## Common Error Scenarios to Test

### 1. No Games Installed
- **Test**: Run the launcher on a system with no games installed
- **Expected**: Clear error message explaining no games found
- **Report**: If the error message is unclear or the app crashes

### 2. SD Card Issues
- **Test**: Run the launcher with:
  - No SD card inserted
  - Corrupted SD card
  - SD card with insufficient space
- **Expected**: Appropriate error messages for each scenario
- **Report**: If error messages are incorrect or missing

### 3. Database and name resolution

- **Test**: Libraries with base games, Virtual Console, DSiWare, updates, and DLC installed
- **Expected**: Titles in the offline database show a readable name; unknown titles reroll (or show hex ID with homebrew mode ON)
- **Note**: Being picked for an update or DLC title is **expected** today — note whether launch works and whether users would want that filtered out
- **Report**: Wrong names, missing names for titles that should be in the DB, infinite reroll loops, crashes

### 4. Launch Failures
- **Test**: Try to launch games that may be corrupted or incompatible
- **Expected**: Clear error message if launch fails
- **Report**: If the app crashes or provides unclear error messages

### 5. Memory Issues
- **Test**: Run the launcher on systems with limited available memory
- **Expected**: App should handle memory constraints gracefully
- **Report**: If the app crashes due to memory issues

## Testing Checklist

### Basic Functionality
- [ ] App launches successfully
- [ ] Game selection works (reroll with Y)
- [ ] Names display for titles in the offline database
- [ ] Homebrew mode toggle (X) includes unknown titles
- [ ] Game launch works for valid titles (A)
- [ ] App exits cleanly with START

### Title types (no filter yet — document behavior)
- [ ] Base 3DS application picked and launches
- [ ] Virtual Console title picked and launches
- [ ] DSiWare title picked and launches
- [ ] Update title (`0004000E…`) — note if name shows and whether launch is desirable
- [ ] DLC title — note if name shows and whether launch is desirable
- [ ] Title missing from DB — reroll or hex ID with homebrew mode

### Error Handling
- [ ] No games scenario handled properly
- [ ] SD card issues handled properly
- [ ] Launch failures handled properly
- [ ] Memory issues handled properly
- [ ] Invalid input handled properly

### User Interface
- [ ] All text is readable on top screen
- [ ] Button prompts match actual controls (A/Y/X/START)
- [ ] Homebrew mode status updates when toggling X
- [ ] Error messages are helpful when selection fails

## Debug Build Testing

For verbose build output on PC, use:

```bash
make DEBUG=1
```

Debug builds add a `-debug` suffix to output filenames. On-device behavior is the same as release unless additional debug logging is added to `main.c`.

## Reporting Bugs

When reporting bugs, please include:

### Required Information
1. **System Model**: (e.g., New 3DS XL)
2. **Firmware Version**: (e.g., 11.17.0-50)
3. **Region**: (e.g., USA)
4. **Language**: (e.g., English)
5. **App version**: from `VERSION` / build filename (e.g., v0.1.9)
6. **Title ID**: 16-digit hex if relevant (especially for wrong/missing names)
7. **Homebrew mode**: ON or OFF
8. **Steps to Reproduce**: Detailed steps to trigger the issue
9. **Expected Behavior**: What should have happened
10. **Actual Behavior**: What actually happened

For filtering feedback, also note: **Would you want this title type picked?** (base / VC / DSiWare / update / DLC / homebrew)

### Optional Information
- **Memory Information**: Available/used memory when error occurred
- **Game Library**: Number of games installed
- **SD Card**: Size and type of SD card
- **Homebrew Environment**: How the app was launched (Homebrew Launcher, etc.)

## Example Bug Report

```
**System Information:**
- Model: New 3DS XL
- Firmware: 11.17.0-50
- Region: USA
- Language: English

**Error Details:**
- Error Code: 5
- Result Code: 0xD8E0806A
- Error Message: "Failed to retrieve title list from SD card"

**Steps to Reproduce:**
1. Remove SD card from 3DS
2. Launch 3DS Random Game Launcher
3. App crashes immediately

**Expected Behavior:**
App should display error message about missing SD card

**Actual Behavior:**
App crashes with black screen

**Additional Information:**
- Memory Available: 45,000 KB
- Memory Used: 12,000 KB
```

## Testing Different Scenarios

### Firmware Versions
Test on different firmware versions if possible:
- Older firmware (9.x, 10.x)
- Current firmware (11.17.x)
- Different regions of the same firmware

### System Models
Test on different 3DS models if available:
- Original 3DS
- 3DS XL
- New 3DS
- New 3DS XL
- 2DS
- New 2DS XL

### Game Libraries
Test with different library configurations:
- Small library (1–5 games)
- Large library (50+ games)
- Mix of title types: base 3DS, Virtual Console, DSiWare, updates, DLC
- Games from different regions
- Homebrew titles (with homebrew mode ON and OFF)

## Performance Testing

### Memory Usage
- Monitor memory usage during operation
- Test with large game libraries
- Test on systems with limited memory

### Response Time
- Measure time to load game list
- Measure time to select random game
- Measure time to launch game

## Compatibility Testing

### Homebrew Environments
Test the app when launched from:
- Homebrew Launcher
- Different CFW environments
- Different launcher applications

### System States
Test the app when:
- System is low on battery
- System is in sleep mode
- Other applications are running
- System is under heavy load

## Contact Information

For bug reports and feedback, please provide the information through the appropriate channels with all the details listed above.

Thank you for helping test the 3DS Random Game Launcher!
