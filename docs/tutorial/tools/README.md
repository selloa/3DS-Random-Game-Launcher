# Tools

## `render.py` / `render.ps1` / `render.bat`

Send Mermaid (`.mmd`) from `diagrams/` to [Kroki](https://kroki.io); write images under `out/` (same subpaths as source).

```powershell
cd C:\mmm\mmm-system-design\2026-05-15-mermaid-diagrams

# Re-render every .mmd (double-click or CMD)
tools\render.bat

# Default: writes both landscape.svg and landscape.png under out/
python tools/render.py --phase 01-pre-tooling-community
.\tools\render.ps1 -Phase 01-pre-tooling-community

# Single format only
python tools/render.py --phase 01-pre-tooling-community -f pdf
```

### Solid variant (no dotted arrows)

From one `.mmd`, the tool also writes **`-solid`** siblings when the source has dotted edges (`-.->`, etc.):

```text
ecosystem.mmd  →  ecosystem.svg + ecosystem-solid.svg  (+ .png if default)
```

Invisible layout links (`~~~`) are kept. Disable with `--no-solid-variant`.

Future generators (e.g. YAML → `.mmd`) belong in this folder too.

### SVG notes

Kroki Mermaid SVG uses HTML labels inside some nodes. **Open `.svg` in a browser** (Edge, Chrome, Firefox) for a correct view. Windows Photos / some editors may show empty boxes. For slides or wiki embeds, use **PNG** (`-f png`) instead.
