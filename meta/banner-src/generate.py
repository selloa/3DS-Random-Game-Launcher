#!/usr/bin/env python3
"""Generate banner PNGs from config.json and layered assets."""

from __future__ import annotations

import argparse
import copy
import json
import sys
from pathlib import Path
from typing import Any

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Pillow is required: pip install pillow", file=sys.stderr)
    sys.exit(1)


ROOT = Path(__file__).resolve().parent


def load_config(path: Path) -> dict[str, Any]:
    with path.open(encoding="utf-8") as handle:
        return json.load(handle)


def parse_color(value: str) -> tuple[int, int, int, int]:
    value = value.strip()
    if value.startswith("#"):
        hex_value = value[1:]
        if len(hex_value) == 6:
            r = int(hex_value[0:2], 16)
            g = int(hex_value[2:4], 16)
            b = int(hex_value[4:6], 16)
            return r, g, b, 255
        if len(hex_value) == 8:
            r = int(hex_value[0:2], 16)
            g = int(hex_value[2:4], 16)
            b = int(hex_value[4:6], 16)
            a = int(hex_value[6:8], 16)
            return r, g, b, a
    raise ValueError(f"Unsupported color format: {value}")


def parse_background_config(background: Any, scale: float = 1.0) -> dict[str, Any]:
    if isinstance(background, str):
        return {
            "color": background,
            "opacity": 1.0,
            "corner_radius": 0,
        }

    return {
        "color": background.get("color", "#000000"),
        "opacity": float(background.get("opacity", 1.0)),
        "corner_radius": scale_value(int(background.get("corner_radius", 0)), scale),
    }


def create_background_canvas(
    width: int,
    height: int,
    bg_cfg: dict[str, Any],
) -> Image.Image:
    red, green, blue, _ = parse_color(bg_cfg["color"])
    alpha = max(0, min(255, int(round(255 * bg_cfg["opacity"]))))
    radius = int(bg_cfg["corner_radius"])

    if radius <= 0:
        return Image.new("RGBA", (width, height), (red, green, blue, alpha))

    fill = Image.new("RGBA", (width, height), (red, green, blue, alpha))
    mask = Image.new("L", (width, height), 0)
    mask_draw = ImageDraw.Draw(mask)
    mask_draw.rounded_rectangle((0, 0, width - 1, height - 1), radius=radius, fill=255)

    canvas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    canvas.paste(fill, mask=mask)
    return canvas


def composite_on_canvas(
    rendered: Image.Image,
    outer_w: int,
    outer_h: int,
    offset_x: int,
    offset_y: int,
    backdrop_rgb: tuple[int, int, int],
) -> Image.Image:
    outer = Image.new("RGBA", (outer_w, outer_h), (*backdrop_rgb, 255))
    outer.alpha_composite(rendered, dest=(offset_x, offset_y))
    return outer


def ensure_rgba(image: Image.Image) -> Image.Image:
    if image.mode == "RGBA":
        return image
    return image.convert("RGBA")


def resolve_path(base: Path, relative: str) -> Path:
    path = Path(relative)
    if path.is_absolute():
        return path
    return base / path


def find_font(font_cfg: dict[str, Any], base: Path) -> ImageFont.FreeTypeFont:
    size = int(font_cfg["size"])
    candidates: list[Path] = []

    if "path" in font_cfg:
        candidates.append(resolve_path(base, font_cfg["path"]))

    for entry in font_cfg.get("paths", []):
        candidates.append(resolve_path(base, entry))

    assets_fonts = base / "assets" / "fonts"
    if assets_fonts.is_dir():
        candidates.extend(sorted(assets_fonts.glob("*.ttf")))
        candidates.extend(sorted(assets_fonts.glob("*.otf")))

    for candidate in candidates:
        if candidate.is_file():
            return ImageFont.truetype(str(candidate), size)

    family = font_cfg.get("family", "default")
    raise FileNotFoundError(
        f"Font not found for {family}. Add a TTF to assets/fonts/ or set font.path in config.json."
    )


def scale_value(value: int | float, scale: float) -> int:
    return max(1, int(round(value * scale)))


def scale_layout(layout: dict[str, Any], scale: float) -> dict[str, Any]:
    scaled = copy.deepcopy(layout)

    icon = scaled["icon"]
    icon["scale_to"] = scale_value(icon["scale_to"], scale)
    align = icon.get("align_white_to")
    if align and "y" in align:
        align["y"] = scale_value(align["y"], scale)
    position = icon.get("position")
    if position:
        position["x"] = scale_value(position["x"], scale)
        position["y"] = scale_value(position["y"], scale)

    if "icon_to_text_gap" in scaled:
        scaled["icon_to_text_gap"] = scale_value(scaled["icon_to_text_gap"], scale)

    title = scaled["title"]
    title["font"]["size"] = scale_value(title["font"]["size"], scale)
    title_position = title.get("position", {})
    if "x" in title_position:
        title.setdefault("position", {})["x"] = scale_value(title_position["x"], scale)
    if "y" in title_position:
        title.setdefault("position", {})["y"] = scale_value(title_position["y"], scale)
    title["line_spacing"] = scale_value(title.get("line_spacing", 0), scale)

    author = scaled["author"]
    author["font"]["size"] = scale_value(author["font"]["size"], scale)
    author["gap"] = scale_value(author.get("gap", 0), scale)

    for layer in scaled.get("extra_layers", []):
        layer["position"]["x"] = scale_value(layer["position"]["x"], scale)
        layer["position"]["y"] = scale_value(layer["position"]["y"], scale)
        if layer.get("type") == "text":
            layer["font"]["size"] = scale_value(layer["font"]["size"], scale)

    scaled["width"] = scale_value(layout["width"], scale)
    scaled["height"] = scale_value(layout["height"], scale)
    return scaled


def scale_version_cfg(version_cfg: dict[str, Any], scale: float) -> dict[str, Any]:
    scaled = copy.deepcopy(version_cfg)
    scaled["font"]["size"] = scale_value(scaled["font"]["size"], scale)
    scaled["margin_bottom"] = scale_value(scaled.get("margin_bottom", 6), scale)
    return scaled


def white_bbox(image: Image.Image) -> tuple[int, int, int, int] | None:
    pixels = image.load()
    width, height = image.size
    xs: list[int] = []
    ys: list[int] = []
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a > 0 and r > 240 and g > 240 and b > 240:
                xs.append(x)
                ys.append(y)
    if not xs:
        return None
    return min(xs), min(ys), max(xs), max(ys)


def opaque_bbox(image: Image.Image) -> tuple[int, int, int, int]:
    pixels = image.load()
    width, height = image.size
    xs: list[int] = []
    ys: list[int] = []
    for y in range(height):
        for x in range(width):
            if pixels[x, y][3] > 0:
                xs.append(x)
                ys.append(y)
    if not xs:
        return 0, 0, width - 1, height - 1
    return min(xs), min(ys), max(xs), max(ys)


def load_scaled_icon(icon_cfg: dict[str, Any], base: Path) -> Image.Image:
    icon_path = resolve_path(base, icon_cfg["path"])
    icon = Image.open(icon_path).convert("RGBA")
    scale_to = int(icon_cfg["scale_to"])
    return icon.resize((scale_to, scale_to), Image.NEAREST)


def paste_icon(canvas: Image.Image, icon_cfg: dict[str, Any], base: Path) -> float:
    icon = load_scaled_icon(icon_cfg, base)
    scale_to = icon.size[0]

    align = icon_cfg.get("align_white_to")
    bbox = white_bbox(icon)
    if align:
        if bbox is None:
            paste_x = int(align["x"])
            paste_y = int(align["y"])
        else:
            paste_x = int(align["x"]) - bbox[0]
            paste_y = int(align["y"]) - bbox[1]
    else:
        position = icon_cfg.get("position", {"x": 0, "y": 0})
        paste_x = int(position["x"])
        paste_y = int(position["y"])

    canvas.paste(icon, (paste_x, paste_y), icon)

    if align and bbox is not None:
        die_height = bbox[3] - bbox[1] + 1
        return align["y"] + die_height / 2
    return paste_y + scale_to / 2


def measure_text_block(
    title_cfg: dict[str, Any],
    author_cfg: dict[str, Any],
    base: Path,
) -> tuple[int, int]:
    probe = ImageDraw.Draw(Image.new("RGB", (1, 1)))
    title_font = find_font(title_cfg["font"], base)
    author_font = find_font(author_cfg["font"], base)
    line_spacing = int(title_cfg.get("line_spacing", 8))
    gap = int(author_cfg.get("gap", 12))

    block_height = 0
    line_widths: list[int] = []
    lines = title_cfg["lines"]

    for index, line in enumerate(lines):
        bbox = probe.textbbox((0, 0), line, font=title_font)
        block_height += bbox[3] - bbox[1]
        line_widths.append(bbox[2] - bbox[0])
        if index < len(lines) - 1:
            block_height += line_spacing

    author_bbox = probe.textbbox((0, 0), author_cfg["text"], font=author_font)
    block_height += gap + (author_bbox[3] - author_bbox[1])

    return block_height, max(line_widths) if line_widths else 0


def measure_text_width(
    title_cfg: dict[str, Any],
    author_cfg: dict[str, Any],
    base: Path,
) -> int:
    probe = ImageDraw.Draw(Image.new("RGB", (1, 1)))
    title_font = find_font(title_cfg["font"], base)
    author_font = find_font(author_cfg["font"], base)

    line_widths = [
        probe.textbbox((0, 0), line, font=title_font)[2]
        - probe.textbbox((0, 0), line, font=title_font)[0]
        for line in title_cfg["lines"]
    ]
    author_bbox = probe.textbbox((0, 0), author_cfg["text"], font=author_font)
    author_width = author_bbox[2] - author_bbox[0]

    return max(max(line_widths) if line_widths else 0, author_width)


def apply_horizontal_layout(layout: dict[str, Any], base: Path) -> None:
    canvas_width = int(layout["width"])
    gap = int(layout.get("icon_to_text_gap", 8))

    icon = load_scaled_icon(layout["icon"], base)
    opaque = opaque_bbox(icon)
    icon_width = opaque[2] - opaque[0] + 1
    text_width = measure_text_width(layout["title"], layout["author"], base)

    group_width = icon_width + gap + text_width
    group_x = (canvas_width - group_width) // 2

    white = white_bbox(icon)
    align = layout["icon"].setdefault("align_white_to", {})
    if white is not None:
        align["x"] = group_x + white[0] - opaque[0]
    else:
        align["x"] = group_x

    if "y" not in align:
        align["y"] = int(round((int(layout["height"]) - icon.size[1]) / 2))

    layout["title"].setdefault("position", {})
    layout["title"]["position"]["x"] = group_x + icon_width + gap


def draw_title(
    draw: ImageDraw.ImageDraw,
    title_cfg: dict[str, Any],
    base: Path,
) -> tuple[int, int, int, int, int]:
    font = find_font(title_cfg["font"], base)
    color = parse_color(title_cfg["color"])
    x = int(title_cfg["position"]["x"])
    y = int(title_cfg["position"]["y"])
    line_spacing = int(title_cfg.get("line_spacing", 8))

    min_x = x
    min_y = y
    max_x = x
    max_y = y
    line_widths: list[int] = []

    for line in title_cfg["lines"]:
        bbox = draw.textbbox((0, 0), line, font=font)
        draw.text((x - bbox[0], y - bbox[1]), line, font=font, fill=color)
        line_width = bbox[2] - bbox[0]
        line_height = bbox[3] - bbox[1]
        line_widths.append(line_width)
        min_x = min(min_x, x)
        min_y = min(min_y, y)
        max_x = max(max_x, x + line_width)
        max_y = max(max_y, y + line_height)
        y += line_height + line_spacing

    return min_x, min_y, max_x, max_y, max(line_widths) if line_widths else 0


def draw_author(
    draw: ImageDraw.ImageDraw,
    author_cfg: dict[str, Any],
    base: Path,
    title_x: int,
    title_bottom_y: int,
    title_width: int,
) -> None:
    font = find_font(author_cfg["font"], base)
    color = parse_color(author_cfg["color"])
    text = author_cfg["text"]
    gap = int(author_cfg.get("gap", 12))

    bbox = draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    y = title_bottom_y + gap

    align = author_cfg.get("align", "left")
    if align == "center_under_title":
        x = title_x + (title_width - text_width) // 2
    else:
        x = title_x

    draw.text((x - bbox[0], y - bbox[1]), text, font=font, fill=color)


def load_version_text(config: dict[str, Any], base: Path) -> str | None:
    version_cfg = config.get("version")
    if not version_cfg or not version_cfg.get("enabled", True):
        return None

    version_path = resolve_path(base, version_cfg.get("file", "../../VERSION"))
    if not version_path.is_file():
        return None

    raw = version_path.read_text(encoding="utf-8").strip()
    if not raw:
        return None

    prefix = version_cfg.get("prefix", "v")
    if prefix and not raw.startswith(prefix):
        return f"{prefix}{raw}"
    return raw


def draw_version_label(
    canvas: Image.Image,
    version_cfg: dict[str, Any],
    version_text: str,
    base: Path,
    center_x: float | None = None,
) -> None:
    canvas_rgba = ensure_rgba(canvas)
    draw = ImageDraw.Draw(canvas_rgba)
    width, height = canvas_rgba.size
    font = find_font(version_cfg["font"], base)
    color = parse_color(version_cfg["color"])
    margin_bottom = int(version_cfg.get("margin_bottom", 6))

    bbox = draw.textbbox((0, 0), version_text, font=font)
    text_width = bbox[2] - bbox[0]
    anchor_x = width / 2 if center_x is None else center_x
    x = int(round(anchor_x - text_width / 2)) - bbox[0]
    y = height - margin_bottom - bbox[3]
    draw.text((x, y), version_text, font=font, fill=color)


def draw_extra_layers(
    canvas: Image.Image,
    draw: ImageDraw.ImageDraw,
    layers: list[dict[str, Any]],
    base: Path,
) -> None:
    for layer in layers:
        layer_type = layer.get("type")
        if layer_type == "text":
            font = find_font(layer["font"], base)
            color = parse_color(layer["color"])
            position = layer["position"]
            bbox = draw.textbbox((0, 0), layer["text"], font=font)
            draw.text(
                (int(position["x"]) - bbox[0], int(position["y"]) - bbox[1]),
                layer["text"],
                font=font,
                fill=color,
            )
        elif layer_type == "image":
            image = Image.open(resolve_path(base, layer["path"])).convert("RGBA")
            position = layer["position"]
            canvas.paste(image, (int(position["x"]), int(position["y"])), image)
        else:
            raise ValueError(f"Unsupported extra layer type: {layer_type}")


def render_layout(
    layout: dict[str, Any],
    bg_cfg: dict[str, Any],
    base: Path,
) -> tuple[Image.Image, float]:
    width = int(layout["width"])
    height = int(layout["height"])

    canvas = create_background_canvas(width, height, bg_cfg)
    draw = ImageDraw.Draw(canvas)

    apply_horizontal_layout(layout, base)

    icon_center_y = paste_icon(canvas, layout["icon"], base)
    block_height, _ = measure_text_block(layout["title"], layout["author"], base)
    layout["title"]["position"]["y"] = int(round(icon_center_y - block_height / 2))

    title_x = int(layout["title"]["position"]["x"])
    _, _, _, title_bottom, title_width = draw_title(draw, layout["title"], base)
    draw_author(
        draw,
        layout["author"],
        base,
        title_x,
        title_bottom,
        title_width,
    )

    draw_extra_layers(canvas, draw, layout.get("extra_layers", []), base)
    text_center_x = title_x + title_width / 2
    return canvas, text_center_x


def render_output(
    design: dict[str, Any],
    output_cfg: dict[str, Any],
    background: Any,
    base: Path,
    version_cfg: dict[str, Any] | None,
    version_text: str | None,
) -> Image.Image:
    scale = float(output_cfg.get("scale", 1))
    layout = scale_layout(design, scale)
    bg_cfg = parse_background_config(background, scale)
    scaled_version = scale_version_cfg(version_cfg, scale) if version_cfg else None

    if "title_lines" in output_cfg:
        layout["title"]["lines"] = output_cfg["title_lines"]

    rendered, text_center_x = render_layout(layout, bg_cfg, base)

    canvas_cfg = output_cfg.get("canvas")
    offset_x = 0
    backdrop_rgb = parse_color(bg_cfg["color"])[:3]
    if not canvas_cfg:
        final = rendered
    else:
        outer_w = int(canvas_cfg["width"])
        outer_h = int(canvas_cfg["height"])
        if rendered.size == (outer_w, outer_h):
            final = rendered
        else:
            offset_x = (outer_w - rendered.width) // 2 if output_cfg.get("center", True) else 0
            offset_y = (outer_h - rendered.height) // 2 if output_cfg.get("center", True) else 0
            final = composite_on_canvas(rendered, outer_w, outer_h, offset_x, offset_y, backdrop_rgb)

    if version_text and scaled_version:
        draw_version_label(
            final,
            scaled_version,
            version_text,
            base,
            center_x=text_center_x + offset_x,
        )

    return final


def write_outputs(
    config: dict[str, Any],
    base: Path,
    only: set[str] | None,
) -> list[Path]:
    written: list[Path] = []
    design = config["design"]
    background = config.get("background", "#000000")
    version_text = load_version_text(config, base)
    version_cfg = config.get("version")

    for name, output_cfg in config["outputs"].items():
        if only is not None and name not in only:
            continue

        image = render_output(design, output_cfg, background, base, version_cfg, version_text)
        output_path = resolve_path(base, output_cfg["path"])
        output_path.parent.mkdir(parents=True, exist_ok=True)
        image.save(output_path)
        written.append(output_path)

    return written


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--config",
        type=Path,
        default=ROOT / "config.json",
        help="Path to banner config JSON",
    )
    parser.add_argument(
        "--output",
        choices=["all", "banner-large", "banner", "cia-banner"],
        default="all",
        help="Which output file(s) to write",
    )
    args = parser.parse_args()

    config_path = args.config.resolve()
    base = config_path.parent
    config = load_config(config_path)

    only = None if args.output == "all" else {args.output}
    written = write_outputs(config, base, only)

    for path in written:
        print(f"Wrote {path.relative_to(base)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
