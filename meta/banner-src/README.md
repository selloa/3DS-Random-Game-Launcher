# Banner source

Reproducible, config-driven rebuild of `meta/banner-large.png` and the smaller store/CIA variants.

## Layout

```
meta/banner-src/
  config.json       # text, colors, positions, output sizes
  generate.py       # Pillow renderer
  build.bat         # Windows: sync icon, generate, copy to meta/
  build.sh          # Linux/Mac: sync icon, generate, copy to meta/
  assets/
    icon.png        # dice icon (from repo-root icon.png)
    fonts/          # optional: drop TTF/OTF here for portable builds
  output/           # generated PNGs (gitignored scratch; copies live in meta/)
```

## Quick start

One-shot regen + copy into `meta/` (recommended):

```powershell
.\build.ps1 banners
```

```bat
build.bat banners
```

Windows batch (direct):

```bat
meta\banner-src\build.bat
```

Linux/Mac:

```bash
./build.sh banners
# or
meta/banner-src/build.sh
```

Requires Python 3 and Pillow:

```bat
pip install -r requirements-dev.txt
```

### Windows: `cmd` / app picker issues

**Cause:** devkitPro MSYS installs `c:\devkitPro\msys2\usr\bin\cmd` (a bash script, no `.exe`) *before* `System32\cmd.exe` on the system PATH. Anything that runs `cmd /c ...` may open the app picker instead of executing batch files.

**Permanent fix (all projects):** run once from an elevated PowerShell if needed:

```powershell
powershell -ExecutionPolicy Bypass -File scripts/fix-devkitpro-cmd-shadow.ps1
```

This renames the MSYS shim to `cmd.msys-shim`. devkitPro updates may restore it — re-run the script if `where cmd` no longer shows `System32\cmd.exe`.

**Also applied on this machine:** PowerShell profiles define a `cmd` function as a fallback (`Documents\PowerShell\Microsoft.PowerShell_profile.ps1` and Windows PowerShell equivalent).

Prefer `.\build.ps1` or `.\build.bat` over bare `cmd /c` when possible.

Manual steps (equivalent):

```bash
python meta/banner-src/generate.py
```

Outputs:

| File | Size | Notes |
|------|------|-------|
| `output/banner-large.png` | 1536×1024 | master artwork |
| `output/banner.png` | 256×128 | Universal-DB / README |
| `output/cia-banner.png` | 256×128 | `build_cia.bat` input |

`build.bat banners` copies these into `meta/` automatically. See [docs/VERSIONING.md](../../docs/VERSIONING.md) for the release checklist.

## Customize

All layout lives in **`design`** at the native **256×128** banner size (3DS / Universal-DB format). Smaller outputs render at that size; `banner-large` upscales uniformly (default 6×) and centers on a wider canvas.

Edit `config.json`:

- **`design.title.lines`** — banner headline (one string per line)
- **`outputs.cia-banner.title_lines`** — optional per-output title override
- **`design.author.text`** — credit line
- **`design.title.color` / `design.author.color`** — hex colors
- **`design.icon.scale_to` / `design.icon.align_white_to.y`** — dice size and vertical placement
- **`design.icon_to_text_gap`** — space between icon and title (horizontal centering is automatic)
- **`version`** — reads repo [`VERSION`](../../VERSION), drawn at the bottom band centered under the title block
- **`design.title.font` / `design.author.font`** — size and font paths

### Add extra elements later

Append to **`design.extra_layers`**:

```json
{
  "type": "text",
  "text": "v0.2.1",
  "color": "#878587",
  "font": { "size": 36, "paths": ["C:/Windows/Fonts/arial.ttf"] },
  "position": { "x": 1400, "y": 980 }
}
```

```json
{
  "type": "image",
  "path": "assets/badge.png",
  "position": { "x": 1200, "y": 80 }
}
```

Drop custom fonts in `assets/fonts/` to avoid hard-coded system paths.

## Options

```bash
python meta/banner-src/generate.py --output banner-large
python meta/banner-src/generate.py --config meta/banner-src/config.json
```
