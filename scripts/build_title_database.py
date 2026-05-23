#!/usr/bin/env python3
"""
Merge 3DS title ID → name mappings from multiple sources into title_database.c.

Priority (first writer wins for names):
  1. hax0kartik/3dsdb regional JSONs
  2. ghost-land/3dsdb bulk JSON (all categories)
  3. 3dsdb.com/xml.php — gap fill only

See docs/TITLE_RESOLUTION_ROADMAP.md
"""

import argparse
import json
import os
import shutil
import sys
import time
import xml.etree.ElementTree as ET
from typing import List, Tuple

import requests

from title_db_common import (
    TitleCatalog,
    clean_title_name,
    count_existing_entries,
    generate_c_code,
    generate_vc_catalog_c_code,
    is_valid_catalog_name,
    is_valid_hax0kartik_title,
    normalize_title_id,
)

HAX0KARTIK_BASE = "https://raw.githubusercontent.com/hax0kartik/3dsdb/master/jsons"
HAX0KARTIK_REGIONS = {
    "US": "list_US.json",
    "GB": "list_GB.json",
    "JP": "list_JP.json",
    "KR": "list_KR.json",
    "TW": "list_TW.json",
}

GHOSTLAND_3DSDB_BASE = (
    "https://raw.githubusercontent.com/ghost-land/3dsdb/main/data/initial_data"
)
GHOSTLAND_FILES = {
    "games.json": "base",
    "dsiware.json": "dsiware",
    "virtual-console.json": "virtual-console",
    "updates.json": "updates",
    "dlc.json": "dlc",
    "videos.json": "videos",
}

XML_URL = "https://3dsdb.com/xml.php"

SOURCE_COMMENT = (
    "Sources: hax0kartik/3dsdb (names) → ghost-land/3dsdb (all categories) → 3dsdb.com/xml.php"
)


def step1_hax0kartik(catalog: TitleCatalog) -> None:
    """Priority 1: regional eShop JSON lists."""
    print("=== Step 1: hax0kartik/3dsdb ===")
    pairs: List[Tuple[str, str]] = []

    for region, filename in HAX0KARTIK_REGIONS.items():
        url = f"{HAX0KARTIK_BASE}/{filename}"
        print(f"  Fetching {region}...")
        try:
            response = requests.get(url, timeout=60)
            response.raise_for_status()
            entries = response.json()
        except (requests.exceptions.RequestException, json.JSONDecodeError) as exc:
            print(f"    Error: {exc}")
            continue

        if not isinstance(entries, list):
            print(f"    Unexpected JSON shape (expected array)")
            continue

        region_pairs: List[Tuple[str, str]] = []
        for entry in entries:
            if not isinstance(entry, dict):
                continue
            raw_id = entry.get("TitleID") or entry.get("titleID") or entry.get("titleid")
            name = entry.get("Name") or entry.get("name")
            if not raw_id or not name:
                continue
            title_id = normalize_title_id(raw_id)
            name = clean_title_name(str(name).strip())
            if is_valid_hax0kartik_title(title_id, name):
                region_pairs.append((title_id, name))

        print(f"    {len(region_pairs)} eShop base titles accepted")
        pairs.extend(region_pairs)
        time.sleep(0.3)

    added, skipped = catalog.merge(pairs, validate=is_valid_hax0kartik_title)
    print(f"  Catalog after step 1: {len(catalog)} entries (+{added} new, {skipped} dupes in batch)\n")


def step2_ghostland_3dsdb(catalog: TitleCatalog, vc_ids: set[str]) -> None:
    """Priority 2: ghost-land/3dsdb bulk category JSON files."""
    print("=== Step 2: ghost-land/3dsdb ===")
    total_added = 0
    total_skipped = 0

    for filename, category in GHOSTLAND_FILES.items():
        url = f"{GHOSTLAND_3DSDB_BASE}/{filename}"
        print(f"  Fetching {category} ({filename})...")
        try:
            response = requests.get(url, timeout=60)
            response.raise_for_status()
            entries = response.json()
        except (requests.exceptions.RequestException, json.JSONDecodeError) as exc:
            print(f"    Error: {exc}")
            continue

        if not isinstance(entries, list):
            print(f"    Unexpected JSON shape (expected array)")
            continue

        pairs: List[Tuple[str, str]] = []
        for entry in entries:
            if not isinstance(entry, dict):
                continue
            raw_id = entry.get("tid") or entry.get("title_id") or entry.get("TitleID")
            name = entry.get("name") or entry.get("Name")
            if not raw_id or not name:
                continue
            title_id = normalize_title_id(raw_id)
            name = clean_title_name(str(name).strip())
            if category == "virtual-console" and is_valid_catalog_name(title_id, name):
                vc_ids.add(title_id)
            pairs.append((title_id, name))

        added, skipped = catalog.merge(pairs, validate=is_valid_catalog_name)
        total_added += added
        total_skipped += skipped
        print(f"    {len(entries)} rows, +{added} new, {skipped} dupes in batch")

    print(
        f"  Catalog after step 2: {len(catalog)} entries "
        f"(+{total_added} from ghost-land/3dsdb, {total_skipped} dupes in batch)\n"
    )


def step3_xml(catalog: TitleCatalog) -> None:
    """Priority 3: 3dsdb.com XML gap fill."""
    print("=== Step 3: 3dsdb.com/xml.php ===")
    print(f"  Fetching {XML_URL}...")
    try:
        response = requests.get(XML_URL, timeout=120)
        response.raise_for_status()
        xml_data = response.text
    except requests.exceptions.RequestException as exc:
        print(f"  Error: {exc}\n")
        return

    print(f"  Downloaded {len(xml_data)} bytes")
    pairs: List[Tuple[str, str]] = []

    try:
        root = ET.fromstring(xml_data)
        for release in root.findall(".//release"):
            tid_el = release.find("titleid")
            name_el = release.find("name")
            if tid_el is None or name_el is None or not tid_el.text or not name_el.text:
                continue
            title_id = normalize_title_id(tid_el.text)
            name = clean_title_name(name_el.text.strip())
            pairs.append((title_id, name))
    except ET.ParseError as exc:
        print(f"  XML parse error: {exc}\n")
        return

    added, skipped = catalog.merge(pairs, validate=is_valid_catalog_name)
    print(f"  Catalog after step 3: {len(catalog)} entries (+{added} new, {skipped} dupes in batch)\n")


def parse_steps(raw: str) -> List[int]:
    steps = sorted({int(part.strip()) for part in raw.split(",") if part.strip()})
    for step in steps:
        if step not in (1, 2, 3):
            raise argparse.ArgumentTypeError("steps must be a subset of 1,2,3")
    return steps


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Build source/title_database.c from hax0kartik → ghost-land/3dsdb → XML."
    )
    parser.add_argument(
        "--dry-run",
        action="store_true",
        help="Write source/title_database_generated.c instead of overwriting",
    )
    parser.add_argument(
        "--output",
        default="source/title_database.c",
        help="Output C file (default: source/title_database.c)",
    )
    parser.add_argument(
        "--steps",
        type=parse_steps,
        default=[1, 2, 3],
        help="Comma-separated merge steps to run (default: 1,2,3)",
    )
    args = parser.parse_args()

    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    output_path = args.output
    if not os.path.isabs(output_path):
        output_path = os.path.join(repo_root, output_path)
    if args.dry_run:
        output_path = os.path.join(repo_root, "source", "title_database_generated.c")

    existing_path = os.path.join(repo_root, "source", "title_database.c")
    previous_count = count_existing_entries(existing_path)
    if previous_count:
        print(f"Current database: {previous_count} entries\n")

    catalog = TitleCatalog()
    vc_ids: set[str] = set()
    if 1 in args.steps:
        step1_hax0kartik(catalog)
    if 2 in args.steps:
        step2_ghostland_3dsdb(catalog, vc_ids)
    if 3 in args.steps:
        step3_xml(catalog)

    titles = catalog.sorted_items()
    print(f"=== Final catalog: {len(titles)} entries ===")
    if previous_count:
        print(f"Delta vs current: {len(titles) - previous_count:+d}")

    if not titles:
        print("No titles collected. Aborting.")
        return 1

    c_code = generate_c_code(titles, SOURCE_COMMENT)
    vc_output_path = os.path.join(repo_root, "source", "title_vc_catalog.c")
    vc_code = generate_vc_catalog_c_code(sorted(vc_ids))

    if args.dry_run:
        with open(output_path, "w", encoding="utf-8") as handle:
            handle.write(c_code)
        with open(vc_output_path, "w", encoding="utf-8") as handle:
            handle.write(vc_code)
        print(f"\nDry run written to: {output_path}")
        print(f"VC catalog written to: {vc_output_path} ({len(vc_ids)} entries)")
        return 0

    if os.path.isfile(output_path):
        backup_path = f"{output_path}.backup_{int(time.time())}"
        shutil.copy2(output_path, backup_path)
        print(f"Backup saved: {backup_path}")

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as handle:
        handle.write(c_code)
    with open(vc_output_path, "w", encoding="utf-8") as handle:
        handle.write(vc_code)
    print(f"Wrote {len(titles)} entries to {output_path}")
    print(f"Wrote {len(vc_ids)} VC title IDs to {vc_output_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
