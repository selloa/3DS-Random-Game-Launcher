# 3DS Random Title Picker

A Nintendo 3DS homebrew application that randomly selects and launches titles from your SD card. Perfect for those moments when you can't decide what to play!

## 📋 Table of Contents

- [Features](#features)
- [Screenshots](#screenshots)
- [Installation](#installation)
- [Usage](#usage)
- [Building from Source](#building-from-source)
- [Technical Details](#technical-details)
- [Planned Features](#planned-features)
- [Contributing](#contributing)
- [Acknowledgments](#acknowledgments)
- [License](#license)

## ✨ Features

### Current Features
- **Random Title Selection**: Scans your SD card and randomly picks from installed titles
- **Smart Filtering**: Automatically filters out system titles and non-game content (categories 0x00 and 0x02 only)
- **Interactive Controls**:
  - `A` - Launch the selected title
  - `Y` - Reroll for a different random title
  - `START` - Exit the application
- **Real-time Display**: Shows the selected title ID and provides feedback during rerolls
- **Error Handling**: Displays error codes if title scanning fails

### How It Works
The application uses the 3DS Application Manager (AM) service to:
1. Scan for all titles installed on the SD card
2. Filter titles by content category to exclude system applications
3. Randomly select from the filtered list
4. Use the APT (Application) service to chain-load the selected title

## 📸 Screenshots

*Screenshots coming soon!*

## 🚀 Installation

### Prerequisites
- Nintendo 3DS/2DS with custom firmware (CFW) installed
- [Luma3DS](https://github.com/LumaTeam/Luma3DS) (latest version recommended)
- Homebrew Launcher access

### Quick Install
1. Download the latest release from the [Releases page](https://github.com/einso/3ds-randomtitlepicker/releases)
2. Extract the `.3dsx` file to your SD card in the `/3ds/` directory
3. Launch the Homebrew Launcher and run "Random Title Picker"

### Manual Build
See the [Building from Source](#building-from-source) section below.

## 🎮 Usage

1. **Launch**: Start the application from the Homebrew Launcher
2. **Wait**: The app will scan your SD card for installed titles
3. **Review**: A random title will be displayed with its title ID
4. **Choose**:
   - Press `A` to launch the selected title
   - Press `Y` to get a different random selection
   - Press `START` to exit

### Controls
| Button | Action |
|--------|--------|
| `A` | Launch selected title |
| `Y` | Reroll for new random title |
| `START` | Exit application |

## 🔨 Building from Source

### Prerequisites
- [devkitPro](https://devkitpro.org/) development environment
- devkitARM toolchain
- 3ds-dev libraries

### Build Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/einso/3ds-randomtitlepicker.git
   cd 3ds-randomtitlepicker
   ```

2. Build the application:
   ```bash
   make
   ```

3. The compiled `.3dsx` file will be created in the project root

### Development Setup
For development, you may want to set up:
- A 3DS emulator (Citra) for testing
- A physical 3DS with CFW for final testing
- Git hooks for code formatting (optional)

## 🔧 Technical Details

### Architecture
- **Language**: C
- **Framework**: libctru (3DS homebrew library)
- **Services Used**:
  - `gfx` - Graphics rendering
  - `am` - Application Manager (title scanning)
  - `apt` - Application (title launching)
  - `hid` - Input handling

### Key Components
- **Title Scanning**: Uses `AM_GetTitleList()` to enumerate SD card titles
- **Content Filtering**: Filters by content category to exclude system titles
- **Random Selection**: Uses `rand()` with time-based seeding
- **Chain Loading**: Uses `aptSetChainloader()` to launch selected titles

### Code Structure
```
source/
└── main.c          # Main application logic
```

### Dependencies
- libctru (3DS homebrew library)
- Standard C library functions

## 🚧 Planned Features

This project has significant potential for enhancement. Here are some features that could be added:

### UI/UX Improvements
- [ ] **Graphical Interface**: Replace text-based UI with a proper GUI
- [ ] **Title Names**: Display actual game names instead of just title IDs
- [ ] **Game Icons**: Show game icons/thumbnails in the selection
- [ ] **Categories**: Filter by game categories (Action, RPG, Puzzle, etc.)
- [ ] **Favorites System**: Allow users to mark favorite games
- [ ] **Blacklist**: Exclude specific games from random selection

### Advanced Features
- [ ] **Play History**: Track recently played games
- [ ] **Statistics**: Show play time and frequency statistics
- [ ] **Smart Recommendations**: Suggest games based on play patterns
- [ ] **Multiple Selection**: Pick multiple random games at once
- [ ] **Scheduled Launches**: Set up automatic game launches at specific times
- [ ] **Custom Themes**: Multiple visual themes for the interface

### Technical Enhancements
- [ ] **Configuration File**: Save user preferences and settings
- [ ] **Database Integration**: Store game metadata and statistics
- [ ] **Network Features**: Share selections with friends or get recommendations
- [ ] **Plugin System**: Allow third-party extensions
- [ ] **Performance Optimization**: Faster title scanning and loading

### Quality of Life
- [ ] **Error Recovery**: Better error handling and recovery
- [ ] **Logging**: Detailed logging for debugging
- [ ] **Backup/Restore**: Save and restore configurations
- [ ] **Multi-language Support**: Internationalization
- [ ] **Accessibility**: Support for users with disabilities

## 🤝 Contributing

We welcome contributions! This project has a lot of potential for growth and improvement.

### How to Contribute
1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow the existing code style
- Add comments for complex logic
- Test on both emulator and real hardware when possible
- Update documentation for new features

### Ideas for Contributions
- Implement any of the planned features listed above
- Improve error handling and user feedback
- Add unit tests
- Optimize performance
- Improve documentation
- Create better graphics/UI

## 🙏 Acknowledgments

- **Original Author**: [einso](https://github.com/einso) for creating this useful tool
- **DevKitPro Team**: For providing the excellent [3ds-examples](https://github.com/devkitPro/3ds-examples/tree/master/app_launch) that this project is based on
- **Contributors**: @suchmememanyskill, @bleck9999, and @dorkeline for their assistance in the original development
- **3DS Homebrew Community**: For the tools, libraries, and support that make projects like this possible

## 📄 License

This project is open source. Please check the original repository for licensing information.

## 🔗 Links

- [Original Repository](https://github.com/einso/3ds-randomtitlepicker)
- [DevKitPro 3DS Examples](https://github.com/devkitPro/3ds-examples/tree/master/app_launch)
- [DevKitPro Website](https://devkitpro.org/)
- [3DS Homebrew Guide](https://3ds.hacks.guide/)

---

**Note**: This is a community-driven fork with plans for significant enhancements. The original project was a great starting point, and we're excited to build upon it with new features and improvements!