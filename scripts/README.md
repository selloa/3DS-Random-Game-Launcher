# Scripts Directory

Python utilities for maintaining `source/title_database.c`.

## Recommended workflow

Use **`fetch_3dsdb_complete.py`** — it pulls all regions (USA, EUR, JPN) from the [hax0kartik/3dsdb](https://github.com/hax0kartik/3dsdb) JSON files and generates C code.

```bash
python scripts/fetch_3dsdb_complete.py
```

Review the output, merge entries into `source/title_database.c`, then rebuild with `make`.

## All scripts

| Script | Status | Description |
|--------|--------|-------------|
| `fetch_3dsdb_complete.py` | **Primary** | Full regional fetch from 3dsdb GitHub JSONs |
| `fetch_3dsdb_api.py` | Legacy | Fetches from `api.ghseshop.cc` |
| `fetch_3dsdb_batch.py` | Legacy | Batch variant of the API fetch |
| `expand_database.py` | Legacy | Fetches from 3dsdb.com XML export |
| `fix_display_issues.py` | Utility | Fixes TM, apostrophe, and display characters in the database |

The legacy fetch scripts predate `fetch_3dsdb_complete.py`. Keep them for reference or delete once you confirm the complete script covers your needs.

## Requirements

- Python 3.x
- `requests` (`pip install requests`)

## After updating the database

1. Review generated code for duplicates or bad entries
2. Copy merged entries into `source/title_database.c`
3. Run `make` or `build.bat release` to verify the build
4. Test on hardware with titles that were previously missing or misnamed
