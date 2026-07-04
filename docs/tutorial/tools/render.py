#!/usr/bin/env python3
"""
Render Mermaid (.mmd) sources via the Kroki HTTP API.

Run from the work folder root (recommended):

  python tools/render.py --phase 01-pre-tooling-community
  python tools/render.py --all
  python tools/render.py diagrams/01-pre-tooling-community/landscape.mmd
"""

from __future__ import annotations

import argparse
import re
import sys
import urllib.error
import urllib.request
from pathlib import Path

# Mermaid dotted links: -.->  <->  o-.->  etc. (not solid --->)
_DOTTED_EDGE_RE = re.compile(r"-\.-?>|<-\.->|o-\.|\.-o|==>")

DEFAULT_KROKI = "https://kroki.io"
DEFAULT_FORMATS = ("svg", "png")
DEFAULT_DIAGRAM_TYPE = "mermaid"
SUPPORTED_FORMATS = ("png", "svg", "pdf", "jpeg", "jpg", "base64")

MERMAID_HINTS = (
    "flowchart",
    "graph ",
    "sequenceDiagram",
    "classDiagram",
    "stateDiagram",
    "erDiagram",
    "journey",
    "gantt",
    "pie ",
    "quadrantChart",
    "requirementDiagram",
    "gitGraph",
    "C4Context",
    "C4Container",
    "C4Component",
    "C4Dynamic",
    "C4Deployment",
)


def work_root_from_script() -> Path:
    return Path(__file__).resolve().parent.parent


def resolve_input_path(path: Path, work_root: Path) -> Path:
    if path.is_absolute():
        return path.resolve()
    candidate = Path.cwd() / path
    if candidate.is_file():
        return candidate.resolve()
    under_root = work_root / path
    if under_root.is_file():
        return under_root.resolve()
    return candidate.resolve()


def kroki_headers(diagram_type: str, output_format: str) -> dict[str, str]:
    headers = {
        "Content-Type": "text/plain; charset=utf-8",
        "User-Agent": "mmm-diagram-render/1.0 (Mermaid via Kroki)",
    }
    # HTML labels inside foreignObject break many SVG viewers (Explorer, Inkscape, etc.)
    if diagram_type == "mermaid" and output_format == "svg":
        headers["Kroki-Diagram-Options-Flowchart-Html-Labels"] = "false"
    return headers


def polish_svg(data: bytes) -> bytes:
    text = data.decode("utf-8")
    if not text.lstrip().startswith("<?xml"):
        text = '<?xml version="1.0" encoding="UTF-8"?>\n' + text
    return text.encode("utf-8")


def render(
    source: str,
    *,
    diagram_type: str = DEFAULT_DIAGRAM_TYPE,
    output_format: str,
    kroki_base: str = DEFAULT_KROKI,
) -> bytes:
    base = kroki_base.rstrip("/")
    url = f"{base}/{diagram_type}/{output_format}"
    body = source.encode("utf-8")
    request = urllib.request.Request(
        url,
        data=body,
        method="POST",
        headers=kroki_headers(diagram_type, output_format),
    )
    try:
        with urllib.request.urlopen(request, timeout=120) as response:
            return response.read()
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace").strip()
        raise RuntimeError(
            f"Kroki returned HTTP {exc.code} for {url}\n{detail}"
        ) from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(f"Could not reach Kroki at {url}: {exc.reason}") from exc


def default_output_path(
    input_path: Path,
    output_format: str,
    out_root: Path,
    diagrams_root: Path,
) -> Path:
    ext = "jpg" if output_format == "jpeg" else output_format
    if output_format == "base64":
        ext = "txt"
    try:
        rel = input_path.resolve().relative_to(diagrams_root.resolve())
    except ValueError:
        rel = Path(input_path.name)
    return out_root / rel.with_suffix(f".{ext}")


def looks_like_mermaid(source: str) -> bool:
    return any(hint in source for hint in MERMAID_HINTS)


def line_has_dotted_edge(line: str) -> bool:
    stripped = line.strip()
    if not stripped or stripped.startswith("%%"):
        return False
    return _DOTTED_EDGE_RE.search(stripped) is not None


def source_has_dotted_edges(source: str) -> bool:
    return any(line_has_dotted_edge(line) for line in source.splitlines())


def omit_dotted_edges(source: str) -> str:
    """Drop lines that declare dotted Mermaid edges (-.->, etc.). Keeps ~~~ layout links.

    A preceding comment line ``%% solid`` keeps the next dotted edge in the -solid variant.
    """
    kept: list[str] = []
    keep_next_dotted = False
    for line in source.splitlines():
        stripped = line.strip()
        if stripped == "%% solid":
            keep_next_dotted = True
            continue
        if line_has_dotted_edge(line):
            if keep_next_dotted:
                kept.append(line)
                keep_next_dotted = False
            continue
        keep_next_dotted = False
        kept.append(line)
    return "\n".join(kept) + ("\n" if source.endswith("\n") else "")


def solid_variant_path(path: Path) -> Path:
    return path.with_name(f"{path.stem}-solid{path.suffix}")


def render_file(
    input_path: Path,
    output_path: Path | None,
    output_format: str,
    kroki_base: str,
    diagram_type: str,
    out_root: Path,
    diagrams_root: Path,
    *,
    source: str | None = None,
) -> Path:
    if not input_path.is_file():
        raise FileNotFoundError(f"Input not found: {input_path}")

    if source is None:
        source = input_path.read_text(encoding="utf-8")
    if diagram_type == "mermaid" and not looks_like_mermaid(source):
        print(
            f"warning: {input_path} may not contain Mermaid diagram syntax",
            file=sys.stderr,
        )

    out = output_path or default_output_path(
        input_path, output_format, out_root, diagrams_root
    )
    out.parent.mkdir(parents=True, exist_ok=True)

    data = render(
        source,
        diagram_type=diagram_type,
        output_format=output_format,
        kroki_base=kroki_base,
    )
    if output_format == "svg" and diagram_type == "mermaid":
        data = polish_svg(data)
    out.write_bytes(data)
    return out


def collect_inputs(
    paths: list[Path],
    render_all: bool,
    phase: str | None,
    diagrams_root: Path,
    work_root: Path,
) -> list[Path]:
    if phase:
        phase_dir = diagrams_root / phase
        if not phase_dir.is_dir():
            raise SystemExit(f"Unknown phase folder: {phase_dir}")
        found = sorted(phase_dir.rglob("*.mmd"))
        if not found:
            raise SystemExit(f"No .mmd files under {phase_dir}")
        return found
    if render_all:
        return sorted(diagrams_root.rglob("*.mmd"))
    if paths:
        return [resolve_input_path(p, work_root) for p in paths]
    raise SystemExit(
        "No input files. Pass .mmd path(s), --all, or --phase <folder-name>."
    )


def main(argv: list[str] | None = None) -> int:
    work_root = work_root_from_script()
    diagrams_root = work_root / "diagrams"
    out_root = work_root / "out"

    parser = argparse.ArgumentParser(
        description="Render Mermaid diagrams through Kroki (remote, no local Mermaid CLI)."
    )
    parser.add_argument(
        "inputs",
        nargs="*",
        type=Path,
        help="One or more .mmd files (under diagrams/)",
    )
    parser.add_argument(
        "--all",
        action="store_true",
        help="Render every diagrams/**/*.mmd file",
    )
    parser.add_argument(
        "--phase",
        metavar="FOLDER",
        help="Render all .mmd under diagrams/<FOLDER>/ (e.g. 01-pre-tooling-community)",
    )
    parser.add_argument(
        "-f",
        "--format",
        choices=SUPPORTED_FORMATS,
        default=None,
        help=f"Single output format (default: {' + '.join(DEFAULT_FORMATS)})",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        help="Output file (only valid with a single input)",
    )
    parser.add_argument(
        "--kroki",
        default=DEFAULT_KROKI,
        help=f"Kroki base URL (default: {DEFAULT_KROKI})",
    )
    parser.add_argument(
        "--type",
        default=DEFAULT_DIAGRAM_TYPE,
        dest="diagram_type",
        help=f"Kroki diagram type (default: {DEFAULT_DIAGRAM_TYPE})",
    )
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=None,
        help=f"Output root (default: {out_root}; mirrors diagrams/ layout)",
    )
    parser.add_argument(
        "--no-solid-variant",
        action="store_true",
        help="Do not emit a second -solid output with dotted Mermaid edges removed",
    )
    args = parser.parse_args(argv)

    if args.all and args.phase:
        parser.error("use either --all or --phase, not both")

    if args.out_dir is None:
        out_dir = out_root
    elif args.out_dir.is_absolute():
        out_dir = args.out_dir
    else:
        out_dir = work_root / args.out_dir

    diagrams_root.mkdir(parents=True, exist_ok=True)
    out_dir.mkdir(parents=True, exist_ok=True)

    inputs = collect_inputs(
        args.inputs, args.all, args.phase, diagrams_root, work_root
    )
    if not inputs:
        print(f"No .mmd files found under {diagrams_root}", file=sys.stderr)
        return 1

    formats = (args.format,) if args.format else DEFAULT_FORMATS

    if args.output is not None:
        if len(inputs) != 1:
            parser.error("-o/--output requires exactly one input file")
        if len(formats) != 1:
            parser.error("-o/--output requires a single -f/--format (not default svg+png)")

    output_path = None
    if args.output is not None:
        output_path = args.output if args.output.is_absolute() else work_root / args.output

    failures = 0
    for path in inputs:
        resolved = path.resolve()
        try:
            rel_in = resolved.relative_to(diagrams_root.resolve())
        except ValueError:
            rel_in = Path(resolved.name)

        full_source = resolved.read_text(encoding="utf-8")
        emit_solid = (
            not args.no_solid_variant
            and args.diagram_type == "mermaid"
            and source_has_dotted_edges(full_source)
        )
        solid_source = omit_dotted_edges(full_source) if emit_solid else None

        for output_format in formats:
            try:
                out = render_file(
                    resolved,
                    output_path.resolve() if output_path else None,
                    output_format,
                    args.kroki,
                    args.diagram_type,
                    out_dir.resolve(),
                    diagrams_root.resolve(),
                    source=full_source,
                )
                print(
                    f"OK  {rel_in} -> {out.relative_to(out_dir.resolve())}"
                )
                if emit_solid and solid_source is not None:
                    solid_out = (
                        solid_variant_path(out)
                        if output_path is None
                        else solid_variant_path(out.resolve())
                    )
                    solid_written = render_file(
                        resolved,
                        solid_out,
                        output_format,
                        args.kroki,
                        args.diagram_type,
                        out_dir.resolve(),
                        diagrams_root.resolve(),
                        source=solid_source,
                    )
                    print(
                        f"OK  {rel_in} -> "
                        f"{solid_written.relative_to(out_dir.resolve())} (no dotted)"
                    )
            except (OSError, RuntimeError) as exc:
                failures += 1
                print(
                    f"FAIL  {rel_in} ({output_format}): {exc}",
                    file=sys.stderr,
                )

    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
