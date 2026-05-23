"""Shared helpers for title database fetch/merge scripts."""

import os
import re
from typing import Dict, List, Tuple

UPDATE_NAME_RE = re.compile(r"\bupdate\b", re.IGNORECASE)
BASE_TITLE_PREFIX = "00040000"
CTR_TITLE_PREFIX = "0004"


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


def normalize_title_id(raw_id: str) -> str:
    """Return 16-char uppercase hex title ID without 0x prefix."""
    tid = str(raw_id).strip().upper()
    if tid.startswith("0X"):
        tid = tid[2:]
    return tid


def is_valid_ctr_title_id(title_id: str) -> bool:
    """Any 3DS CTR title ID (apps, updates, DLC, etc.)."""
    if len(title_id) != 16:
        return False
    if not re.fullmatch(r"[0-9A-F]{16}", title_id):
        return False
    return title_id.startswith(CTR_TITLE_PREFIX)


def is_valid_hax0kartik_title(title_id: str, name: str) -> bool:
    """eShop base-app filter for hax0kartik JSON (priority-1 seed)."""
    if not is_valid_ctr_title_id(title_id):
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


def is_valid_catalog_name(title_id: str, name: str) -> bool:
    """Name checks for Nlib/XML gap-fill entries."""
    if not is_valid_ctr_title_id(title_id):
        return False
    if not name or len(name) >= 200:
        return False
    if name.startswith("System") and title_id.startswith(BASE_TITLE_PREFIX):
        return False
    return True


def escape_c_string(value: str) -> str:
    """Escape a string for use inside a C string literal."""
    return (
        value.replace("\\", "\\\\")
        .replace('"', '\\"')
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
    )


def generate_c_code(titles: List[Tuple[str, str]], source_comment: str) -> str:
    """Generate title_database.c contents."""
    lines = [
        '#include "title_database.h"',
        "#include <string.h>",
        "",
        "// Complete 3DS title database — merged offline catalog",
        f"// Total entries: {len(titles)}",
        f"// {source_comment}",
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


class TitleCatalog:
    """First-writer-wins title ID → name map."""

    def __init__(self) -> None:
        self._entries: Dict[str, str] = {}

    def __len__(self) -> int:
        return len(self._entries)

    def contains(self, title_id: str) -> bool:
        return title_id in self._entries

    def merge(
        self,
        pairs: List[Tuple[str, str]],
        *,
        validate,
    ) -> Tuple[int, int]:
        """Return (added, skipped_duplicate) counts."""
        added = 0
        skipped = 0
        for title_id, name in pairs:
            if title_id in self._entries:
                skipped += 1
                continue
            if not validate(title_id, name):
                continue
            self._entries[title_id] = name
            added += 1
        return added, skipped

    def sorted_items(self) -> List[Tuple[str, str]]:
        return sorted(self._entries.items(), key=lambda item: item[0])
