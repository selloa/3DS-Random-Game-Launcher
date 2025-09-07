#!/usr/bin/env python3
"""
Script to fix display issues in the title database.
This script cleans problematic characters that could cause display issues on 3DS.
"""

import re
import os
import shutil
from datetime import datetime

def clean_game_title(title):
    """Clean a game title by replacing problematic characters"""
    
    # Replace trademark symbols
    title = title.replace('™', '(TM)')
    title = title.replace('®', '(R)')
    
    # Replace HTML line breaks with spaces
    title = title.replace('<br>', ' ')
    title = title.replace('<BR>', ' ')
    
    # Replace ampersands with 'and' for better readability
    title = title.replace(' & ', ' and ')
    
    # Replace smart quotes with regular quotes
    title = title.replace('"', '"')
    title = title.replace('"', '"')
    title = title.replace(''', "'")
    title = title.replace(''', "'")
    
    # Replace em dashes with regular dashes
    title = title.replace('—', '-')
    title = title.replace('–', '-')
    
    # Replace ellipsis with three dots
    title = title.replace('…', '...')
    
    # Clean up multiple spaces
    title = re.sub(r'\s+', ' ', title)
    
    # Trim whitespace
    title = title.strip()
    
    return title

def fix_database_file(input_file, output_file):
    """Fix the database file by cleaning all game titles"""
    
    print(f"Reading database from: {input_file}")
    
    with open(input_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # Find all game title entries and clean them
    # Pattern: {0xXXXXXXXXXXXXXXULL, "Game Title"},
    pattern = r'(\{0x[0-9A-Fa-f]{16}ULL, ")([^"]+)("(?:,|\}))'
    
    def replace_title(match):
        title_id_part = match.group(1)
        original_title = match.group(2)
        closing_part = match.group(3)
        
        cleaned_title = clean_game_title(original_title)
        
        # Escape quotes in the cleaned title
        escaped_title = cleaned_title.replace('"', '\\"')
        
        return f'{title_id_part}{escaped_title}{closing_part}'
    
    # Apply the cleaning
    cleaned_content = re.sub(pattern, replace_title, content)
    
    # Count changes made
    original_matches = re.findall(pattern, content)
    cleaned_matches = re.findall(pattern, cleaned_content)
    
    changes_made = 0
    for i, (original, cleaned) in enumerate(zip(original_matches, cleaned_matches)):
        if original[1] != cleaned[1]:  # Compare the title part
            changes_made += 1
            if changes_made <= 10:  # Show first 10 changes
                print(f"  Fixed: '{original[1]}' -> '{cleaned[1]}'")
    
    if changes_made > 10:
        print(f"  ... and {changes_made - 10} more changes")
    
    print(f"Total changes made: {changes_made}")
    
    # Write the cleaned content
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(cleaned_content)
    
    print(f"Cleaned database written to: {output_file}")
    
    return changes_made

def main():
    """Main function"""
    print("3DS Title Database Display Issues Fixer")
    print("=" * 50)
    
    input_file = 'source/title_database.c'
    output_file = 'source/title_database_clean.c'
    
    if not os.path.exists(input_file):
        print(f"Error: Input file {input_file} not found!")
        return
    
    # Create backup
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_file = f'source/title_database_broken_backup_{timestamp}.c'
    
    print(f"Creating backup: {backup_file}")
    shutil.copy2(input_file, backup_file)
    
    # Fix the database
    changes_made = fix_database_file(input_file, output_file)
    
    if changes_made > 0:
        print(f"\n✅ Successfully fixed {changes_made} problematic titles!")
        print(f"📁 Clean database saved as: {output_file}")
        print(f"💾 Original backed up as: {backup_file}")
        print("\nNext steps:")
        print("1. Review the cleaned database")
        print("2. Replace the original with the cleaned version")
        print("3. Compile in MSYS2 to test")
    else:
        print("\n✅ No issues found - database is already clean!")
        os.remove(output_file)  # Remove empty output file

if __name__ == "__main__":
    main()
