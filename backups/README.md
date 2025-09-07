# Backup Files

This folder contains backup files created during the development and improvement of the 3DS Random Title Picker.

## Files

### Apostrophe Fix Backups
- `title_database_apostrophe_backup_20250907_171455.c` - First backup before apostrophe fixes
- `title_database_apostrophe_backup_20250907_171640.c` - Second backup during apostrophe fixes  
- `title_database_apostrophe_backup_20250907_171713.c` - Final backup after comprehensive apostrophe fixes
- `title_database_apostrophe_fixed.c` - Intermediate fixed version (redundant)

## What Was Fixed

These backups were created during the process of standardizing apostrophes in the title database:

- **133 instances** of different Unicode apostrophe characters were found
- **Multiple apostrophe types** including:
  - Left single quotation mark (U+2018)
  - Right single quotation mark (U+2019)
  - Acute accent (U+00B4)
  - And other apostrophe-like characters
- **All apostrophes standardized** to regular ASCII apostrophes (`'`) for better 3DS compatibility

## Current Status

The main database (`source/title_database.c`) now contains:
- ✅ **4,135 titles** (3,515 base games + 623 Virtual Console games)
- ✅ **Standardized apostrophes** for consistent display
- ✅ **Fixed display characters** (TM, R, HTML tags, etc.)
- ✅ **Clean syntax** ready for compilation

These backup files are kept for reference but are no longer needed for the project.
