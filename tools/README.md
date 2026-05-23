# CIA Build Tooling

Tools and source for building installable `.cia` files. The main app build (`make` at repo root) produces `.3dsx` for Homebrew Launcher only.

## Layout

| Path | Purpose |
|------|---------|
| `tools/bin/makerom.exe` | Prebuilt CTR makerom (Windows) |
| `tools/bin/ctrtool.exe` | Prebuilt ctrtool for inspecting NCCH/CIA |
| `tools/3DS-Random-Game-Launcher.rsf` | RSF metadata for this title |
| `makerom/` | makerom source (rebuild if needed) |
| `bannertool/` | [diasurgical/bannertool](https://github.com/diasurgical/bannertool) submodule — creates `.bnr` / `.icn` |
| `CBuilder3DS/` | [slalomsquid/CBuilder3DS](https://github.com/slalomsquid/CBuilder3DS) submodule — reference CIA build workflow |

## Submodule setup

After cloning, initialize submodules:

```bash
git submodule update --init --recursive
```

## Typical CIA workflow

1. Build the ELF at repo root: `make` (or `build.bat release`)
2. Create banner and icon with bannertool (see `bannertool/README.md`)
3. Package with makerom using the RSF file:

```bash
tools/bin/makerom.exe -f cia -o dist/3DS-Random-Game-Launcher.cia ^
  -rsf tools/3DS-Random-Game-Launcher.rsf -target t ^
  -elf 3DS-Random-Game-Launcher.elf ^
  -icon icon.icn -banner banner.bnr -desc app:4
```

Adjust paths and add `-exefslogo` as needed. See `makerom/README.md` for full options.

## CBuilder3DS reference

The `CBuilder3DS/` submodule contains a working example that runs `make`, then bannertool + makerom in one script (`CBuilder3DS/build.bat`). Use it as a template when wiring CIA builds into this repo's main Makefile.

## Rebuilding makerom from source

See `makerom/BUILDING.md`. On Windows, build inside the `makerom/` directory and copy the resulting binary to `tools/bin/`.

## Notes

- CIA build outputs (`.cia`, `.cxi`) are gitignored — they belong in `dist/` or GitHub Releases.
- The RSF references a `romfs/` directory; create it if your CIA needs embedded files.
- CIA installs require CFW. Most users should still use the `.3dsx` from Homebrew Launcher.
