#!/usr/bin/env python3
"""Generate source/title_vc_catalog.c from ghost-land virtual-console.json."""

import argparse
import json
import os
import sys

import requests

from title_db_common import (
    generate_vc_catalog_c_code,
    is_valid_catalog_name,
    normalize_title_id,
)

VC_URL = (
    "https://raw.githubusercontent.com/ghost-land/3dsdb/main/data/initial_data/"
    "virtual-console.json"
)


def fetch_vc_title_ids() -> list[str]:
    print(f"Fetching {VC_URL}...")
    response = requests.get(VC_URL, timeout=60)
    response.raise_for_status()
    entries = response.json()
    if not isinstance(entries, list):
        raise ValueError("Expected JSON array")

    title_ids: list[str] = []
    for entry in entries:
        if not isinstance(entry, dict):
            continue
        raw_id = entry.get("tid") or entry.get("title_id") or entry.get("TitleID")
        name = entry.get("name") or entry.get("Name") or ""
        if not raw_id:
            continue
        title_id = normalize_title_id(raw_id)
        if is_valid_catalog_name(title_id, str(name)):
            title_ids.append(title_id)

    return sorted(set(title_ids))


def main() -> int:
    parser = argparse.ArgumentParser(description="Build source/title_vc_catalog.c")
    parser.add_argument(
        "--output",
        default="source/title_vc_catalog.c",
        help="Output C file (default: source/title_vc_catalog.c)",
    )
    args = parser.parse_args()

    repo_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    output_path = args.output
    if not os.path.isabs(output_path):
        output_path = os.path.join(repo_root, output_path)

    try:
        title_ids = fetch_vc_title_ids()
    except (requests.exceptions.RequestException, ValueError, json.JSONDecodeError) as exc:
        print(f"Error: {exc}")
        return 1

    if not title_ids:
        print("No VC title IDs collected. Aborting.")
        return 1

    c_code = generate_vc_catalog_c_code(title_ids)
    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as handle:
        handle.write(c_code)

    print(f"Wrote {len(title_ids)} VC title IDs to {output_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
