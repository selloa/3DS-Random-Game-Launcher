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
| `fetch_3dsdb_api.py` | Legacy | Fetches from `api.ghseshop.cc` |
| `fetch_3dsdb_batch.py` | Legacy | Batch variant of the API fetch |
| `expand_database.py` | Legacy | Fetches from 3dsdb.com XML export |
| `fix_display_issues.py` | Utility | Cleans TM, HTML tags, and display characters in an existing `.c` file (also applied automatically during fetch) |

The legacy fetch scripts predate `fetch_3dsdb_complete.py`. Keep them for reference or delete once you confirm the complete script covers your needs.

## Requirements

- Python 3.x
- `requests` (`pip install requests`)

## After updating the database

1. Review generated code for duplicates or bad entries
2. Copy merged entries into `source/title_database.c`
3. Run `make` or `build.bat release` to verify the build
4. Test on hardware with titles that were previously missing or misnamed
