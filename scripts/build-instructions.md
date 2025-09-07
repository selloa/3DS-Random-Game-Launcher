# Build Instructions for 3DS Random Title Picker

## Option 1: GitHub Actions (Recommended - No Local Setup)

1. Push your code to GitHub
2. The GitHub Actions workflow will automatically build your project
3. Download the built `.3dsx` file from the Actions tab
4. No local devkitPro installation required!

## Option 2: Docker (Local Development)

If you have Docker installed:

```bash
# Build the Docker image
docker build -t 3ds-builder .

# Run the build
docker run --rm -v $(pwd):/workspace 3ds-builder

# Or use docker-compose
docker-compose up
```

## Option 3: Online Development Environments

### Replit
- Create a new Repl
- Install devkitPro in the shell
- Upload your source code
- Build using `make`

### Gitpod
- Use the Gitpod button in your GitHub repo
- Install devkitPro in the cloud environment
- Build your project

## Option 4: WSL (Windows Subsystem for Linux)

If you're on Windows:

```bash
# Install WSL2
wsl --install

# Install devkitPro in WSL
wget https://github.com/devkitPro/pacman/releases/download/devkitpro-pacman-1.0.2/devkitpro-pacman.amd64.deb
sudo dpkg -i devkitpro-pacman.amd64.deb
sudo dkp-pacman -S --noconfirm 3ds-dev

# Build your project
make
```

## Option 5: Virtual Machine

1. Create a Linux VM (Ubuntu recommended)
2. Install devkitPro in the VM
3. Share your project folder with the VM
4. Build inside the VM

## Testing Your Build

Once you have a `.3dsx` file:

1. Copy it to your 3DS SD card in `/3ds/` folder
2. Launch via Homebrew Launcher
3. Test the game name lookup functionality

## Troubleshooting

- **Build fails**: Check the GitHub Actions logs for specific errors
- **Docker issues**: Ensure Docker is properly installed and running
- **WSL problems**: Make sure WSL2 is enabled and updated

## Next Steps

After building successfully:
1. Test on real 3DS hardware
2. Expand the title database using `scripts/expand_database.py`
3. Add more features to the application
