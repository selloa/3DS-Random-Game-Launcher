# CIA Build Tooling

Tools and source for building installable `.cia` files. The main app build (`make` at repo root) produces `.3dsx` for Homebrew Launcher only.

## Layout

| Path | Purpose |
|------|---------|
| `tools/bin/bannertool.exe` | Creates `.bnr` banner and `.icn` icon files (v1.2.0) |
| `tools/bin/makerom.exe` | Prebuilt CTR makerom (Windows); see [Updating makerom](#updating-makerom) |
| `tools/bin/ctrtool.exe` | Prebuilt ctrtool for inspecting NCCH/CIA |
| `tools/3DS-Random-Game-Launcher.rsf` | RSF metadata for this title |
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

#### CBuilder3DS (CIA build reference)

CBuilder3DS is a **template project** for packaging homebrew into `.cia` / `.3ds` files. It is **not** the Random Game Launcher app — the real source is `source/main.c` at the repo root.

Initialize and verify:

```bash
git submodule update --init CBuilder3DS
```

Expected: `git submodule status CBuilder3DS` shows a space (not `-` or `+`) before the commit hash, and the working tree is clean.

**Use the parent repo's tools, not the copies bundled inside CBuilder3DS:**

| Task | Use this |
|------|----------|
| App ELF | Build at repo root (`make` → `3DS-Random-Game-Launcher.elf`) |
| bannertool / makerom | `tools/bin/bannertool.exe`, `tools/bin/makerom.exe` |
| RSF | `tools/3DS-Random-Game-Launcher.rsf` |
| Banner / icon assets | Root `icon.png`, `meta/banner.png` |

See `CBuilder3DS/build.bat` and `CBuilder3DS/BUILDING.md` for the **workflow pattern** (make → bannertool → makerom), but substitute paths above when building this project.

CBuilder3DS ships with a Hello World `source/main.c` — ignore it unless you are experimenting inside that template.

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

Adjust paths and add `-exefslogo` as needed. See the [makerom README](https://github.com/3DSGuy/Project_CTR/blob/master/makerom/README.md) for full options.

## CBuilder3DS reference

The [CBuilder3DS](https://github.com/slalomsquid/CBuilder3DS) submodule is a CIA packaging template (fork of CIABUILDER). Use it to understand the build sequence, not as the app source tree.

Workflow pattern from `CBuilder3DS/build.bat`:

1. Compile ELF
2. `bannertool makebanner` / `makesmdh`
3. `makerom -f cia`

When building **this** project, run those steps from the repo root using `tools/bin/` and the main app's ELF — see the CBuilder3DS section under **bannertool setup** above.

## Updating makerom

`tools/bin/makerom.exe` is included in the repo — no setup needed for CIA builds.

To replace it with a newer build, download a release from [3DSGuy/Project_CTR](https://github.com/3DSGuy/Project_CTR/releases) (look for `makerom-v*` tags) and copy `makerom.exe` to `tools/bin/`.

## Notes

- CIA build outputs (`.cia`, `.cxi`) normally go in `dist/` (gitignored)
- Tracked CIA archives live in [releases/archive/](../releases/archive/) for reference builds
- The RSF references a `romfs/` directory; create it if your CIA needs embedded files.
- CIA installs require CFW. Most users should still use the `.3dsx` from Homebrew Launcher.
