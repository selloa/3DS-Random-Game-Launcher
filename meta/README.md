# Meta assets

Images and audio used for **distribution** (Universal-DB / Universal Updater) and **CIA packaging** (3DS Home Menu). These files are not compiled into the `.3dsx` homebrew binary.

App logic lives in `source/`. Build output goes to `dist/`.

## Two pipelines

| Pipeline | When it matters | Key files |
|----------|-----------------|-----------|
| **Universal-DB** | Users browse/install/update via Universal Updater | `icon.png`, `meta/banner.png`, `meta/screenshot.png` |
| **CIA build** | Installed title on the 3DS Home Menu (top-screen banner, icon, startup sound) | `icon.png`, `meta/cia-banner.png`, `meta/audio.wav` |

UniStore and the Home Menu use **different files**. Updating `meta/banner.png` does not change an already-built CIA, and updating `meta/cia-banner.png` does not change the Universal-DB listing image.

## Files in `meta/`

| File | Size | Used by | Purpose |
|------|------|---------|---------|
| [`cia-banner.png`](cia-banner.png) | 256×128 | `build_cia.bat` → bannertool → `.cia` | Top-screen banner when the title is highlighted on the Home Menu |
| [`audio.wav`](audio.wav) | 44.1 kHz stereo PCM (~1.7 s) | `build_cia.bat` → bannertool → `.cia` | Banner sound on the Home Menu (Game Boy startup jingle) |
| [`banner.png`](banner.png) | 256×128 | [Universal-DB](https://db.universal-team.net/3ds/3ds-random-game-launcher) | App page / UniStore listing banner |
| [`screenshot.png`](screenshot.png) | 400×240 | Universal-DB (optional) | Screenshot on the app page in Universal Updater |
| [`banner-large.png`](banner-large.png) | 1536×1024 | *(none currently)* | High-res marketing banner kept for reference; not referenced by build scripts or Universal-DB |

## Related file (repo root)

| File | Size | Used by | Purpose |
|------|------|---------|---------|
| [`../icon.png`](../icon.png) | 48×48 | `Makefile` / `.3dsx` SMDH, `build_cia.bat` → `.cia`, Universal-DB | Home Menu grid icon, Homebrew Launcher icon, UniStore icon |

## CIA build chain

From repo root:

```bat
build_cia.bat
```

| Input | Tool step | Embedded in CIA as |
|-------|-----------|-------------------|
| `meta/cia-banner.png` + `meta/audio.wav` | `bannertool makebanner` → `build/banner.bnr` | Top-screen banner + sound |
| `icon.png` | `bannertool makesmdh` → `build/icon.icn` | Home Menu icon |

See [../tools/README.md](../tools/README.md) for full CIA tooling.

## Universal-DB references

Listing config: [Universal-Team/db — `3ds-random-game-laucher.json`](https://github.com/Universal-Team/db/blob/master/source/apps/3ds-random-game-laucher.json)

Raw URLs on `main` (must stay stable for the store):

- `icon.png`
- `meta/banner.png`

See [../docs/distribution/UNIVERSAL_UPDATER_SETUP.md](../docs/distribution/UNIVERSAL_UPDATER_SETUP.md) for distribution workflow.

## Editing guidelines

| Asset | Notes |
|-------|--------|
| `cia-banner.png` | Must stay **256×128**. Keep ~8px+ margin on the right; long titles clip easily. |
| `audio.wav` | **16-bit PCM, 44100 Hz, stereo** for bannertool. Convert MP3 with ffmpeg if needed. |
| `banner.png` | Store listing; can differ from `cia-banner.png` (e.g. different title wording). |
| `icon.png` | Must stay **48×48** for 3DS icon format. |

After changing CIA assets, run `build_cia.bat` and reinstall the new `.cia` to verify on hardware.
