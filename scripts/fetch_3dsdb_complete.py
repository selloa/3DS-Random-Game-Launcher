#!/usr/bin/env python3
"""
Fetch 3DS title ID → name mappings from the hax0kartik/3dsdb GitHub JSON files
and regenerate source/title_database.c.

Data source: https://github.com/hax0kartik/3dsdb/tree/master/jsons
"""

import argparse
import json
import os
import re
import shutil
import sys
import time
from typing import List, Set, Tuple

import requests

BASE_URL = "https://raw.githubusercontent.com/hax0kartik/3dsdb/master/jsons"

# Region list files in the 3dsdb repo (not the old USA.json / EUR.json names)
REGIONS = {
    "US": "list_US.json",
    "GB": "list_GB.json",
    "JP": "list_JP.json",
    "KR": "list_KR.json",
    "TW": "list_TW.json",
}

# Base application titles only (excludes updates at 0004000E…, DLC, system, etc.)
BASE_TITLE_PREFIX = "00040000"

UPDATE_NAME_RE = re.compile(r"\bupdate\b", re.IGNORECASE)


def clean_title_name(name: str) -> str:
    """Normalize characters that display poorly on the 3DS console."""
    name = name.replace("™", "(TM)")
    name = name.replace("®", "(R)")
    name = name.replace("<br>", " ")
    name = name.replace("<BR>", " ")
    name = name.replace(" & ", " and ")
    name = name.replace("\u201c", '"').replace("\u201d", '"')
    name = name.replace("\u2018", "'").replace("\u2019", "'")
    name = name.replace("\u2014", "-").replace("\u2013", "-")
    name = name.replace("\u2026", "...")
    name = re.sub(r"\s+", " ", name)
    return name.strip()


def fetch_region(region: str, filename: str) -> list:
    """Download and parse one regional title list."""
    url = f"{BASE_URL}/{filename}"
    print(f"Fetching {region} from {url}...")

    try:
        response = requests.get(url, timeout=60)
        response.raise_for_status()
        data = response.json()
    except requests.exceptions.RequestException as exc:
        print(f"  Error fetching {region}: {exc}")
        return []
    except json.JSONDecodeError as exc:
        print(f"  Error parsing {region} JSON: {exc}")
        return []

    if not isinstance(data, list):
        print(f"  Unexpected JSON shape for {region} (expected array)")
        return []

    print(f"  Loaded {len(data)} raw entries")
    return data


def normalize_title_id(raw_id: str) -> str:
    """Return 16-char uppercase hex title ID without 0x prefix."""
    tid = str(raw_id).strip().upper()
    if tid.startswith("0X"):
        tid = tid[2:]
    return tid


def is_valid_base_title(title_id: str, name: str) -> bool:
    """Keep installable base titles suitable for the random picker."""
    if len(title_id) != 16:
        return False
    if not title_id.startswith(BASE_TITLE_PREFIX):
        return False
    if not name or len(name) >= 200:
        return False
    if name.startswith("System") or name.startswith("Nintendo 3DS"):
        return False
    if UPDATE_NAME_RE.search(name):
        return False
    return True


def extract_titles(entries: list) -> List[Tuple[str, str]]:
    """Extract (title_id, name) pairs from 3dsdb list JSON entries."""
    titles: List[Tuple[str, str]] = []

    for entry in entries:
        if not isinstance(entry, dict):
            continue

        raw_id = entry.get("TitleID") or entry.get("titleID") or entry.get("titleid")
        name = entry.get("Name") or entry.get("name")
        if not raw_id or not name:
            continue

        title_id = normalize_title_id(raw_id)
        name = clean_title_name(str(name).strip())

        if is_valid_base_title(title_id, name):
            titles.append((title_id, name))

    return titles


def deduplicate_titles(titles: List[Tuple[str, str]]) -> List[Tuple[str, str]]:
    """Keep first name seen for each title ID."""
    seen: Set[str] = set()
    unique: List[Tuple[str, str]] = []

    for title_id, name in titles:
        if title_id not in seen:
            seen.add(title_id)
            unique.append((title_id, name))

    return unique


def escape_c_string(value: str) -> str:
    """Escape a string for use inside a C string literal."""
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    )


def generate_c_code(titles: List[Tuple[str, str]]) -> str:
    """Generate title_database.c contents."""
    lines = [
        '#include "title_database.h"',
        "#include <string.h>",
        "",
        "// Complete 3DS title database generated from hax0kartik/3dsdb",
        f"// Total entries: {len(titles)}",
        "// Source: https://github.com/hax0kartik/3dsdb/tree/master/jsons",
        "",
        "static const title_entry_t title_database[] = {",
    ]

    for i, (title_id, name) in enumerate(titles):
        escaped = escape_c_string(name)
        suffix = "," if i < len(titles) - 1 else ""
        lines.append(f'    {{0x{title_id}ULL, "{escaped}"}}{suffix}')

    lines.extend(
        [
            "};",
            "",
            "const char* lookup_game_name(u64 title_id) {",
            "    u32 database_size = get_database_size();",
            "",
            "    for (u32 i = 0; i < database_size; i++) {",
            "        if (title_database[i].title_id == title_id) {",
            "            return title_database[i].game_name;",
            "        }",
            "    }",
            "",
            "    return NULL;",
            "}",
            "",
            "u32 get_database_size(void) {",
            "    return sizeof(title_database) / sizeof(title_entry_t);",
            "}",
            "",
        ]
    )

    return "\n".join(lines)


def count_existing_entries(path: str) -> int:
    """Count entries in the current title_database.c if present."""
    if not os.path.isfile(path):
        return 0
    try:
        with open(path, encoding="utf-8") as handle:
            return sum(1 for line in handle if "ULL," in line or "ULL}" in line)
    except OSError:
        return 0


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Regenerate source/title_database.c from hax0kartik/3dsdb JSON files."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Fetch and report stats only; write to source/title_database_generated.c",
    )
    parser.add_argument(
        "--output",
        default="source/title_database.c",
        help="Output C file path (default: source/title_database.c)",
    )
    args = parser.parse_args()

    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    output_path = args.output
    if not os.path.isabs(output_path):
        output_path = os.path.join(repo_root, output_path)

    if args.dry_run:
        output_path = os.path.join(repo_root, "source", "title_database_generated.c")

    print("Fetching 3DS title lists from hax0kartik/3dsdb...\n")

    all_titles: List[Tuple[str, str]] = []
    for region, filename in REGIONS.items():
        entries = fetch_region(region, filename)
        region_titles = extract_titles(entries)
        print(f"  {region}: {len(region_titles)} base titles accepted")
        all_titles.extend(region_titles)
        time.sleep(0.5)

    print(f"\nTotal before deduplication: {len(all_titles)}")
    unique_titles = deduplicate_titles(all_titles)
    unique_titles.sort(key=lambda item: item[0])
    print(f"Unique base titles:       {len(unique_titles)}")

    if not unique_titles:
        print("No titles found. Aborting.")
        return 1

    existing_path = os.path.join(repo_root, "source", "title_database.c")
    previous_count = count_existing_entries(existing_path)
    if previous_count:
        print(f"Current database:         {previous_count} entries")
    print(f"New database:             {len(unique_titles)} entries")
    if previous_count:
        delta = len(unique_titles) - previous_count
        print(f"Delta:                    {delta:+d} entries")

    c_code = generate_c_code(unique_titles)

    if args.dry_run:
        with open(output_path, "w", encoding="utf-8") as handle:
            handle.write(c_code)
        print(f"\nDry run complete. Preview written to: {output_path}")
        print("Review the file, then run without --dry-run to replace title_database.c")
        return 0

    if os.path.isfile(output_path):
        backup_path = f"{output_path}.backup_{int(time.time())}"
        shutil.copy2(output_path, backup_path)
        print(f"\nBackup saved: {backup_path}")

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as handle:
        handle.write(c_code)

    print(f"Wrote {len(unique_titles)} entries to {output_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
