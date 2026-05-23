# Scripts Directory

Python utilities for maintaining `source/title_database.c`.

## Recommended workflow

Use **`fetch_3dsdb_complete.py`** — it pulls all regions from [hax0kartik/3dsdb](https://github.com/hax0kartik/3dsdb) (`list_US.json`, `list_GB.json`, `list_JP.json`, etc.) and regenerates `source/title_database.c`.

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
| `fetch_3dsdb_complete.py` | **Primary** | Full regional fetch from 3dsdb GitHub JSONs |
| `fetch_3dsdb_api.py` | Legacy | Fetches from [Nlib API](https://github.com/ghost-land/nlib-api) (`api.nlib.cc/ctr`) |
| `fetch_3dsdb_batch.py` | Legacy | Batch variant of the Nlib API fetch |
| `expand_database.py` | Legacy | Fetches from 3dsdb.com XML export |
| `fix_display_issues.py` | Utility | Cleans TM, HTML tags, and display characters in an existing `.c` file (also applied automatically during fetch) |

The legacy fetch scripts predate `fetch_3dsdb_complete.py` and hit the Nlib `/ctr` endpoints (successor to the archived `api.ghseshop.cc` 3DSDB API). They are slow (~3k+ per-title requests) — use `fetch_3dsdb_complete.py` unless you specifically need Nlib’s category breakdown or media metadata.

## Requirements

- Python 3.x
- `requests` (`pip install requests`)

## After updating the database

1. Review generated code for duplicates or bad entries
2. Copy merged entries into `source/title_database.c`
3. Run `make` or `build.bat release` to verify the build
4. Test on hardware with titles that were previously missing or misnamed
