# Scripts Directory

Python utilities for maintaining `source/title_database.c`.

See [docs/TITLE_RESOLUTION_ROADMAP.md](../docs/TITLE_RESOLUTION_ROADMAP.md) for the full title-resolution plan, source priority, and merge rules.

## Recommended workflow

**`build_title_database.py`** regenerates the offline catalog in one step:

1. **hax0kartik/3dsdb** — regional eShop JSONs (US → GB → JP → KR → TW); primary names
2. **ghost-land/3dsdb** — bulk category JSON from GitHub (`data/initial_data/*.json`); add missing title IDs
3. **3dsdb.com/xml.php** — last-resort gap fill

The catalog includes **all ghost-land/3dsdb categories** (base, Virtual Console, DSiWare, updates, DLC, videos). The random picker does **not** filter by category today — any installed title in the database can be selected. Filtering rules will be added in `main.c` after hardware testing.

```bash
# Preview (writes source/title_database_generated.c)
python scripts/build_title_database.py --dry-run

# Replace title_database.c (creates a timestamped backup first)
python scripts/build_title_database.py
```

Run individual steps with `--steps 1`, `--steps 1,2`, etc.

Then rebuild with `make` or `build.bat release`.

## All scripts

| Script | Status | Description |
|--------|--------|-------------|
| `build_title_database.py` | **Primary** | Unified merge: hax0kartik → ghost-land/3dsdb → XML |
| `title_db_common.py` | Library | Shared cleaners and catalog merge helpers |
| `fetch_3dsdb_complete.py` | Interim | hax0kartik regional JSONs only |
| `fetch_3dsdb_api.py` | Legacy | Nlib API fetch (`api.nlib.cc/ctr`); reference only |
| `fetch_3dsdb_batch.py` | Legacy | Batch variant of the Nlib fetch |
| `expand_database.py` | Legacy | 3dsdb.com XML export only |
| `fix_display_issues.py` | Utility | Cleans TM, HTML tags, and display characters in an existing `.c` file |

### Source priority (summary)

| Priority | Source | Adds |
|----------|--------|------|
| 1 | hax0kartik/3dsdb JSON | eShop names (best quality) |
| 2 | ghost-land/3dsdb bulk JSON | All missing IDs — base, VC, DSiWare, updates, DLC, videos |
| 3 | 3dsdb.com XML | Remaining gaps only |

**Merge rule:** first source to provide a title ID wins for the **name**; later sources only add **new** IDs.

**Do not use for offline builds:** per-title Nlib API calls (slow; data is from ghost-land/3dsdb). **Retired:** `api.ghseshop.cc` ([3DSDBAPI](https://github.com/ghost-land/3DSDBAPI), archived).

## Requirements

- Python 3.x
- `requests` (`pip install requests`)

## After updating the database

1. Review generated code for duplicates or bad entries
2. Run `make` or `build.bat release` to verify the build
3. Test on hardware with titles that were previously missing or misnamed (updates, VC, DSiWare)
