# 3DS Random Game Launcher - Testing Guide

## Overview
This guide is designed to help testers identify and report issues with the 3DS Random Game Launcher across different 3DS systems and firmware versions.

## System Information Collection
The launcher now automatically collects and displays system information for debugging purposes. This information is crucial for identifying compatibility issues.

### How to Access System Information
1. Launch the 3DS Random Game Launcher
2. When a game is selected, press **X** to view system information
3. Note down all the displayed information for bug reports

### System Information Displayed
- **Model**: 2DS, 3DS, 3DS XL, New 3DS, New 3DS XL, New 2DS XL
- **Firmware**: Version number (e.g., 11.17.0-50)
- **Region**: Japan, USA, Europe, Australia, China, Korea
- **Language**: System language setting
- **Memory**: Available and used memory information

## Error Reporting
The launcher now provides detailed error messages to help identify issues. When an error occurs:

1. **Note the Error Code**: A numeric error code will be displayed
2. **Note the Result Code**: A hexadecimal result code (e.g., 0xD8E0806A)
3. **Read the Error Description**: Detailed explanation of what went wrong
4. **Follow Suggested Actions**: The error message may include troubleshooting steps

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

### 3. Database Issues
- **Test**: Run the launcher with games that may not be in the database
- **Expected**: App should handle unknown games gracefully
- **Report**: If the app crashes or gets stuck in an infinite loop

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
- [ ] System information is displayed correctly
- [ ] Game selection works properly
- [ ] Game launch works for valid games
- [ ] App exits cleanly with START button

### Error Handling
- [ ] No games scenario handled properly
- [ ] SD card issues handled properly
- [ ] Launch failures handled properly
- [ ] Memory issues handled properly
- [ ] Invalid input handled properly

### User Interface
- [ ] All text is readable
- [ ] Button prompts are clear
- [ ] Error messages are helpful
- [ ] System information is accurate

## Debug Build Testing
For more detailed debugging, you can build a debug version:

```bash
make DEBUG=1
```

This will:
- Enable additional debug output
- Show all found titles in the console
- Provide more verbose error information

## Reporting Bugs

When reporting bugs, please include:

### Required Information
1. **System Model**: (e.g., New 3DS XL)
2. **Firmware Version**: (e.g., 11.17.0-50)
3. **Region**: (e.g., USA)
4. **Language**: (e.g., English)
5. **Error Code**: (if applicable)
6. **Result Code**: (if applicable, in hexadecimal)
7. **Steps to Reproduce**: Detailed steps to trigger the issue
8. **Expected Behavior**: What should have happened
9. **Actual Behavior**: What actually happened

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
Test with different game library configurations:
- Small library (1-5 games)
- Large library (50+ games)
- Mix of different game types (3DS, Virtual Console, DSiWare)
- Games from different regions

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
