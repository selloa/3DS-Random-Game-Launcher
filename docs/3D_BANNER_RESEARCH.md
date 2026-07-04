# 3D Banner Research

Research notes on integrating a rotating 3D banner for the HOME Menu.

## How 3D Banners Work on 3DS

What you see on homebrew apps with rotating 3D banners is the **system banner** — the animated graphic shown in the HOME Menu when you highlight a CIA-installed title. The HOME Menu has a built-in renderer that displays a small 3D model with animation.

### The Banner Format

CIA-installed titles include a **banner.bin** file (also called a `.bnr`) inside the title's content. This banner is in the **CBMD** (Common Banner Meta Data) format and contains:

1. **CGFX** — A 3D model container format (CTR Graphics). This is the rotating 3D object you see. It includes geometry, textures, and materials.
2. **BCWAV** — An optional sound effect that plays when the banner animates.
3. **CBMD header** — Metadata tying it together.

The HOME Menu GPU renders the CGFX model automatically — you don't write rendering code yourself. You just supply the model data in the correct format.

### How People Create Them

The typical workflow:

1. **Model the 3D object** in a tool like Blender (low-poly, simple materials/textures).
2. **Export to glTF/DAE/OBJ** depending on which conversion tool you use.
3. **Convert to CGFX** using community tools (see below).
4. **Build the banner.bin** using `bannertool` (already included in `tools/bin/bannertool.exe`).

### The `bannertool` Command

For a 3D banner, the command looks like:

```bash
bannertool makebanner -ci banner.cgfx -ca banner.wav -o banner.bnr
```

For a standard 2D image banner:

```bash
bannertool makebanner -i banner_image.png -a banner_audio.wav -o banner.bnr
```

### Integration Into the Build

To go from the current .3dsx-only build to a CIA with a 3D banner:

1. **Create the 3D model** (e.g., the app icon extruded/rotated as a 3D shape).
2. **Convert to CGFX** format.
3. **Create banner.bin** containing the CGFX and optionally a BCWAV sound.
4. **Build a CIA** using `makerom` (already included at `tools/bin/makerom.exe`) with the banner included via the RSF file (`tools/3DS-Random-Game-Launcher.rsf`).

### Practical Limitations

- **Tooling is scarce**: The official CGFX pipeline uses Nintendo's proprietary SDK. Community tools exist but are fragile and poorly documented.
- **Most homebrew uses 2D banners**: The majority of homebrew sticks with a static or simple animated 2D banner because the tooling for 3D is painful.
- **Model constraints**: The CGFX model must be very simple — limited polygon count, small texture atlas, specific material properties the HOME Menu renderer supports. Max file size is **512KB**.
- **Only visible for CIA installs**: 3D banners only display for CIA-installed titles shown in the HOME Menu. If users run the app via the Homebrew Launcher (.3dsx), they'll never see the banner.

---

## Tooling Research

### pycgfx (Recommended)

**Repository:** https://github.com/skyfloogle/pycgfx

A modern, actively-maintained Python tool that converts **glTF models directly into CGFX** format. This is the cleanest path available.

Supports:
- `.gltf` and `.glb` input files
- Diffuse and normal textures
- Constant roughness factor for specular lighting
- Double-sided materials and alpha blend settings
- Animations and skinning
- Diffuse colour animation using `KHR_animation_pointer`

Key features:
- Warns if output exceeds 512KB limit
- Includes a reference [banner-camera.gltf](https://github.com/skyfloogle/pycgfx/blob/main/banner-camera.gltf) that matches the 3DS HOME Menu camera angle
- Includes an [ImHex pattern](https://github.com/skyfloogle/pycgfx/blob/main/cgfx.hexpat) for viewing/editing CGFX files

Limitations:
- Billboarding (useful for logos) not supported via glTF spec; requires hex editing after conversion
- All animations play simultaneously

### Older Workflow (Every File Explorer + Ohana3DS)

Described in the [GBAtemp guide](https://gbatemp.net/threads/creating-3d-banners-from-3d-models-for-cias.433783/):

1. Export `.obj` from Wings3D/Blender.
2. Import into **Every File Explorer** as a new CGFX (model must be named "COMMON").
3. Verify in **Ohana3DS Rebirth**.
4. Pack into banner.bnr.

This gives a basic rotating model (the HOME Menu rotates it automatically).

### ba-GUI-nnertool

**Info:** https://www.gamebrew.org/wiki/Ba-GUI-nnertool_3DS

A homebrew tool for customizing 2D/3D banners and SMDH icons. Can extract `banner.bnr` from decrypted CIAs, replace BCMDL models, and rebuild. More useful for modifying existing banners than creating from scratch.

### Advanced Banners (CTR-SDK/NW4C)

For anything beyond a basic rotating model (fixed position, microphone interaction, custom animation playback), you'd need Nintendo's proprietary CTR-SDK/NW4C tools which aren't publicly available.

---

## Proposed Workflow for This Project

Using `pycgfx` (fits alongside existing Python-based banner generation tooling):

1. **Model** a simple object in Blender (e.g., app icon on a spinning card/cube, or a die shape matching the icon).
2. **Use the banner camera reference** from `pycgfx` to frame it correctly.
3. **Export as glTF** (`.gltf` or `.glb`).
4. **Convert to CGFX** with `pycgfx`.
5. **Build `banner.bnr`** using `bannertool`:
   ```bash
   bannertool makebanner -ci banner.cgfx -ca banner.wav -o banner.bnr
   ```
6. **Build the CIA** using `makerom` referencing the banner via the RSF.

---

## References

- [CGFX format documentation (3dbrew)](https://www.3dbrew.org/wiki/CGFX)
- [CGFX overview (VG Resource Wiki)](https://wiki.vg-resource.com/CGFX)
- [pycgfx — glTF to CGFX converter](https://github.com/skyfloogle/pycgfx)
- [GBAtemp 3D banner guide](https://gbatemp.net/threads/creating-3d-banners-from-3d-models-for-cias.433783/)
- [ba-GUI-nnertool](https://www.gamebrew.org/wiki/Ba-GUI-nnertool_3DS)
- [bannertool (carstene1ns fork)](https://github.com/carstene1ns/3ds-bannertool)
