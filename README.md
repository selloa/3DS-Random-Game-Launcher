# 2001: A Space Odyssey - Text Adventure Game

A text-based adventure game for the Nintendo 3DS based on Arthur C. Clarke's novel "2001: A Space Odyssey".

## Description

Experience the journey from ape to star child in this interactive text adventure. Navigate through key scenes from the story, making choices that affect your path through the narrative.

## Gameplay

- **A Button**: Select first option
- **B Button**: Select second option  
- **X Button**: Select third option
- **Y Button**: Select fourth option
- **START**: Exit game

## Scenes

The game features five main scenes:
1. **Prehistoric Landscape**: Witness the dawn of intelligence
2. **Discovery One**: Explore the spacecraft en route to Jupiter
3. **Moon Monolith**: Encounter the mysterious black monolith
4. **HAL 9000's Core**: Face the ship's computer
5. **Star Gate**: Journey through space and time

## Building

### Prerequisites
- devkitARM with 3DS development tools
- make

### Build Instructions
```bash
make clean
make
```

The built files will be available in the `dist/` directory:
- `3DS-space-odyssey-vXX-beta.3dsx` - The main game file
- `3DS-space-odyssey-vXX-beta.elf` - Debug symbols
- `3DS-space-odyssey-vXX-beta.smdh` - Icon and metadata

## Installation

Copy the `.3dsx` file to your 3DS SD card in the `/3ds/` directory and run it using a homebrew launcher.

## Author

Created by selloa (2025)

## License

This project is based on the original 3DS Random Game Launcher codebase, simplified for text adventure gameplay.
