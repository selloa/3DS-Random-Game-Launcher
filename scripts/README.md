# Scripts Directory

Python utilities for maintaining `source/title_database.c`.

See [docs/TITLE_RESOLUTION_ROADMAP.md](../docs/TITLE_RESOLUTION_ROADMAP.md) for the full title-resolution plan, source priority, and merge rules.

## Target workflow (planned)

**`build_title_database.py`** (not yet implemented) will regenerate the offline catalog in one step:

1. **hax0kartik/3dsdb** — regional eShop JSONs (US → GB → JP → KR → TW); primary names
2. **Nlib API** (`api.nlib.cc/ctr`) — all categories; add missing title IDs
3. **3dsdb.com/xml.php** — last-resort gap fill

The catalog includes **everything Nlib tracks** (base, Virtual Console, DSiWare, updates, DLC, themes, videos, extras). What the random picker actually launches is filtered in app code, not at build time.

```bash
# Preview (planned)
python scripts/build_title_database.py --dry-run

# Replace title_database.c with timestamped backup (planned)
python scripts/build_title_database.py
```

Until `build_title_database.py` exists, use the interim scripts below.

## Interim workflow

**`fetch_3dsdb_complete.py`** — hax0kartik JSON only (~4,200 base eShop titles). Does not include Nlib categories yet.

```bash
# Preview fetch (writes source/title_database_generated.c, does not overwrite)
python scripts/fetch_3dsdb_complete.py --dry-run

# Replace title_database.c (creates a timestamped backup first)
python scripts/fetch_3dsdb_complete.py
```

Then rebuild with `make` or `build.bat release`.

## All scripts

| Script | Status | Description |
|--------|--------|-------------|
| `build_title_database.py` | **Planned** | Unified merge: hax0kartik → Nlib (all categories) → XML |
| `fetch_3dsdb_complete.py` | Interim | hax0kartik regional JSONs only |
| `fetch_3dsdb_api.py` | Legacy | Nlib-only fetch (`api.nlib.cc/ctr`); reference for merge script |
| `fetch_3dsdb_batch.py` | Legacy | Batch variant of the Nlib fetch |
| `expand_database.py` | Legacy | 3dsdb.com XML export only |
| `fix_display_issues.py` | Utility | Cleans TM, HTML tags, and display characters in an existing `.c` file |

### Source priority (summary)

| Priority | Source | Adds |
|----------|--------|------|
| 1 | hax0kartik/3dsdb JSON | eShop names (best quality) |
| 2 | Nlib `/ctr/category/*` | All missing IDs — base, VC, DSiWare, updates, DLC, etc. |
| 3 | 3dsdb.com XML | Remaining gaps only |

**Merge rule:** first source to provide a title ID wins for the **name**; later sources only add **new** IDs.

**Retired:** `api.ghseshop.cc` ([3DSDBAPI](https://github.com/ghost-land/3DSDBAPI), archived) — use Nlib instead.

## Requirements

- Python 3.x
- `requests` (`pip install requests`)

## After updating the database

1. Review generated code for duplicates or bad entries
2. Run `make` or `build.bat release` to verify the build
3. Test on hardware with titles that were previously missing or misnamed (updates, VC, DSiWare)
