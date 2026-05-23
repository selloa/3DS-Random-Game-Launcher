# CIA Build Tooling

Tools and source for building installable `.cia` files. The main app build (`make` at repo root) produces `.3dsx` for Homebrew Launcher only.

## Layout

| Path | Purpose |
|------|---------|
| `tools/bin/bannertool.exe` | Creates `.bnr` banner and `.icn` icon files (v1.2.0) |
| `tools/bin/makerom.exe` | Prebuilt CTR makerom (Windows) |
| `tools/bin/ctrtool.exe` | Prebuilt ctrtool for inspecting NCCH/CIA |
| `tools/3DS-Random-Game-Launcher.rsf` | RSF metadata for this title |
| `makerom/` | makerom source (rebuild if needed) |
| `bannertool/` | [diasurgical/bannertool](https://github.com/diasurgical/bannertool) submodule — source for bannertool |
| `CBuilder3DS/` | [slalomsquid/CBuilder3DS](https://github.com/slalomsquid/CBuilder3DS) submodule — reference CIA build workflow |

## bannertool setup

Initialize the bannertool submodule (includes nested `buildtools`):

```bash
git submodule update --init --recursive bannertool
```

Verify: `git submodule status bannertool` should show a space (not `-`) before the commit hash, and `bannertool/buildtools/make_base` should exist.

### Getting bannertool.exe

**Option A — use the committed binary (recommended)**

`tools/bin/bannertool.exe` is included in the repo. After submodule init, you're ready for CIA asset creation.

**Option B — build from source**

Requires a native Windows C/C++ toolchain (mingw-w64 `gcc`/`g++`). devkitARM alone is not enough — bannertool builds a PC host tool (`TARGET := NATIVE`).

```bash
cd bannertool
make
```

Copy `output/windows-x86_64/bannertool.exe` to `tools/bin/`.

**Option C — download official release**

Download [bannertool v1.2.0](https://github.com/diasurgical/bannertool/releases/tag/1.2.0) (`bannertool.zip`), extract `windows-x86_64/bannertool.exe` to `tools/bin/`.

**Option D — Docker**

See [bannertool/README.md](../bannertool/README.md) for containerized builds.

### Other submodules

CBuilder3DS is separate — initialize when working on CIA packaging:

```bash
git submodule update --init CBuilder3DS
```

## Typical CIA workflow

1. Build the ELF at repo root: `make` (or `build.bat release`)
2. Create banner and icon with bannertool:

```bash
tools/bin/bannertool.exe makebanner -i meta/banner.png -a audio.wav -o banner.bnr
tools/bin/bannertool.exe makesmdh -s "Random Launcher" -l "3DS Random Game Launcher" -p "selloa" -i icon.png -o icon.icn
```

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
