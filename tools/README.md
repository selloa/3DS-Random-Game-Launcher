# CIA Build Tooling

Tools for building installable `.cia` files. The main app build (`make` or `build.bat release` at repo root) produces `.3dsx` for Homebrew Launcher only.

**No git submodules required.** Prebuilt Windows binaries and assets are included in this repo.

## Quick start (Windows)

From the repo root:

```bat
build.bat banners
build_cia.bat
```

For a release, bump [`VERSION`](../VERSION) first, then run `build.bat banners` so `meta/cia-banner.png` includes the new version label. See [VERSIONING.md](../docs/VERSIONING.md).

`build_cia.bat` runs `make` → bannertool → makerom and writes `dist/3DS-Random-Game-Launcher-v<VERSION>.cia`. It **does not** regenerate banner PNGs — it reads whatever is already in `meta/`.

## What each step uses

| Step | Command | Banner-related inputs |
|------|---------|------------------------|
| Homebrew only | `make` / `build.bat release` | `icon.png` only (`.3dsx` SMDH). No `meta/banner*.png`. |
| Regenerate artwork | `build.bat banners` / `./build.sh banners` | Writes `meta/banner.png`, `meta/cia-banner.png`, `meta/banner-large.png` from [`meta/banner-src/`](../meta/banner-src/) |
| CIA package | `build_cia.bat` | `meta/cia-banner.png`, `meta/audio.wav`, `icon.png` → embedded in `.cia` |

Regenerate banners: `build.bat banners` (Windows) or `./build.sh banners` (Linux/Mac). Requires Python 3 + Pillow (`pip install -r requirements-dev.txt`).

## Layout

| Path | Purpose |
|------|---------|
| `build_cia.bat` | One-step CIA build script (repo root) |
| `tools/bin/bannertool.exe` | Creates `.bnr` banner and `.icn` icon files (v1.2.0) |
| `tools/bin/makerom.exe` | CTR makerom — packages ELF into `.cia` |
| `tools/bin/ctrtool.exe` | Inspect NCCH/CIA (optional) |
| `tools/3DS-Random-Game-Launcher.rsf` | RSF metadata for this title |

Banner, icon, and audio sources are documented in [../meta/README.md](../meta/README.md).

Intermediate files (`build/banner.bnr`, `build/icon.icn`) go in `build/` (gitignored).

## Prerequisites

| Requirement | Notes |
|-------------|-------|
| [devkitPro](https://devkitpro.org/) + devkitARM | For `make` → `.elf` (same as `.3dsx` build) |
| Windows | Prebuilt `tools/bin/*.exe` are Windows binaries |
| CFW on 3DS | CIAs install via FBI, Universal Updater, etc. |

## Manual workflow

If you prefer to run steps yourself:

1. Build the ELF: `make` (or `build.bat release`)
2. Create banner and icon:

```bat
tools\bin\bannertool.exe makebanner -i meta\cia-banner.png -a meta\audio.wav -o build\banner.bnr
tools\bin\bannertool.exe makesmdh -s "3DS Random Game Launcher" -l "3DS Random Game Launcher" -p "selloa" -i icon.png -o build\icon.icn
```

3. Package with makerom:

```bat
tools\bin\makerom.exe -f cia -o dist\3DS-Random-Game-Launcher-v0.1.9.cia ^
  -rsf tools\3DS-Random-Game-Launcher.rsf -target t ^
  -elf 3DS-Random-Game-Launcher.elf ^
  -icon build\icon.icn -banner build\banner.bnr
```

Adjust the output filename to match `VERSION`. See the [makerom README](https://github.com/3DSGuy/Project_CTR/blob/master/makerom/README.md) for full options.

## Updating tools

**bannertool** — download [v1.2.0](https://github.com/diasurgical/bannertool/releases/tag/1.2.0) and copy `windows-x86_64/bannertool.exe` to `tools/bin/`.

**makerom** — download a release from [3DSGuy/Project_CTR](https://github.com/3DSGuy/Project_CTR/releases) (`makerom-v*` tags) and copy `makerom.exe` to `tools/bin/`.

## External references

- [diasurgical/bannertool](https://github.com/diasurgical/bannertool) — banner/icon tool source
- [slalomsquid/CBuilder3DS](https://github.com/slalomsquid/CBuilder3DS) — CIA packaging template (workflow reference)
- [3DSGuy/Project_CTR](https://github.com/3DSGuy/Project_CTR) — makerom source

## Notes

- CIA build outputs (`.cia`, `.cxi`) go in `dist/` (gitignored)
- Tracked CIA archives live in [releases/archive/](../releases/archive/) for reference builds
- The RSF references a `romfs/` directory; create it only if the app needs embedded files
- Most users should use the `.3dsx` from Homebrew Launcher; CIA is for Home Menu installation
