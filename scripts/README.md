# Scripts Directory

This directory contains utility scripts for the 3DS Random Title Picker project.

## expand_database.py

A Python script to help expand the title database by fetching data from 3DSDB.

### Usage

```bash
python3 scripts/expand_database.py
```

### What it does

1. Fetches the latest game database from 3DSDB.com
2. Parses the XML data to extract title ID to game name mappings
3. Generates C code that can be integrated into `source/title_database.c`

### Requirements

- Python 3.x
- requests library (`pip install requests`)

### Output

The script generates `source/title_database_generated.c` which contains:
- A C array of title ID to game name mappings
- Properly formatted for integration into the main codebase

### Integration

After running the script:
1. Review the generated code
2. Copy relevant entries to `source/title_database.c`
3. Test the build to ensure everything compiles correctly

### Notes

- The script includes error handling for network issues
- It filters out unreasonably long game names
- The generated code is ready to compile with the 3DS toolchain
