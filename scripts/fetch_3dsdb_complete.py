#!/usr/bin/env python3
"""
Complete script to fetch all 3DS titles from the 3dsdb repository.
This script fetches data from all regions and categories including Virtual Console games.
"""

import requests
import json
import os
import sys
from typing import List, Tuple, Set
import time

# Base URL for the 3dsdb repository
BASE_URL = "https://raw.githubusercontent.com/hax0kartik/3dsdb/master/jsons"

# Regions to fetch data from
REGIONS = ["USA", "EUR", "JPN"]

def fetch_json_data(region: str) -> dict:
    """Fetch JSON data for a specific region"""
    url = f"{BASE_URL}/{region}.json"
    print(f"Fetching {region} data from {url}...")
    
    try:
        response = requests.get(url, timeout=30)
        response.raise_for_status()
        return response.json()
    except requests.exceptions.RequestException as e:
        print(f"Error fetching {region} data: {e}")
        return {}
    except json.JSONDecodeError as e:
        print(f"Error parsing {region} JSON: {e}")
        return {}

def extract_titles_from_json(data: dict) -> List[Tuple[str, str]]:
    """Extract title ID and name pairs from JSON data"""
    titles = []
    
    if not isinstance(data, dict):
        return titles
    
    # The JSON structure varies, so we need to handle different formats
    for key, value in data.items():
        if isinstance(value, dict):
            # Check if this looks like a title entry
            if 'titleID' in value or 'titleid' in value or 'title_id' in value:
                title_id = value.get('titleID') or value.get('titleid') or value.get('title_id')
                name = value.get('name') or value.get('title') or value.get('game_name')
                
                if title_id and name:
                    # Clean and validate the data
                    title_id = str(title_id).strip().upper()
                    name = str(name).strip()
                    
                    # Filter for valid 3DS titles
                    if (len(title_id) == 16 and 
                        title_id.startswith('00040000') and
                        len(name) > 0 and len(name) < 200 and
                        not name.startswith('System') and
                        not name.startswith('Nintendo 3DS')):
                        titles.append((title_id, name))
            
            # Recursively search nested structures
            titles.extend(extract_titles_from_json(value))
        elif isinstance(value, list):
            # Handle arrays of titles
            for item in value:
                if isinstance(item, dict):
                    title_id = item.get('titleID') or item.get('titleid') or item.get('title_id')
                    name = item.get('name') or item.get('title') or item.get('game_name')
                    
                    if title_id and name:
                        title_id = str(title_id).strip().upper()
                        name = str(name).strip()
                        
                        if (len(title_id) == 16 and 
                            title_id.startswith('00040000') and
                            len(name) > 0 and len(name) < 200 and
                            not name.startswith('System') and
                            not name.startswith('Nintendo 3DS')):
                            titles.append((title_id, name))
    
    return titles

def deduplicate_titles(titles: List[Tuple[str, str]]) -> List[Tuple[str, str]]:
    """Remove duplicate titles, keeping the first occurrence of each title ID"""
    seen_ids: Set[str] = set()
    unique_titles = []
    
    for title_id, name in titles:
        if title_id not in seen_ids:
            seen_ids.add(title_id)
            unique_titles.append((title_id, name))
    
    return unique_titles

def generate_c_code(titles: List[Tuple[str, str]]) -> str:
    """Generate C code for the title database"""
    c_code = '''#include "title_database.h"
#include <string.h>

// Complete 3DS title database generated from 3dsdb repository
// Total entries: {count}
// Includes regular 3DS games and Virtual Console games from all regions

static const title_entry_t title_database[] = {{
'''.format(count=len(titles))
    
    for i, (title_id, name) in enumerate(titles):
        # Escape quotes and backslashes in the name
        escaped_name = name.replace('\\', '\\\\').replace('"', '\\"')
        # Convert title ID to proper format with ULL suffix
        title_id_formatted = f"0x{title_id}ULL"
        
        c_code += f'    {{{title_id_formatted}, "{escaped_name}"}}'
        if i < len(titles) - 1:
            c_code += ','
        c_code += '\n'
    
    c_code += '''};

// Function to lookup game name by title ID
const char* lookup_game_name(u64 title_id) {
    u32 database_size = get_database_size();

    for (u32 i = 0; i < database_size; i++) {
        if (title_database[i].title_id == title_id) {
            return title_database[i].game_name;
        }
    }

    // Return NULL if not found - caller should handle this
    return NULL;
}

// Function to get total number of entries in database
u32 get_database_size(void) {
    return sizeof(title_database) / sizeof(title_entry_t);
}
'''
    
    return c_code

def main():
    """Main function to fetch and process all 3DS titles"""
    print("Starting comprehensive 3DS title database update...")
    print("Fetching data from 3dsdb repository...")
    
    all_titles = []
    
    # Fetch data from all regions
    for region in REGIONS:
        print(f"\nProcessing {region} region...")
        data = fetch_json_data(region)
        
        if data:
            titles = extract_titles_from_json(data)
            print(f"Found {len(titles)} titles in {region}")
            all_titles.extend(titles)
        else:
            print(f"Failed to fetch {region} data")
        
        # Be respectful to the server
        time.sleep(1)
    
    print(f"\nTotal titles found before deduplication: {len(all_titles)}")
    
    # Remove duplicates
    unique_titles = deduplicate_titles(all_titles)
    print(f"Unique titles after deduplication: {len(unique_titles)}")
    
    if not unique_titles:
        print("No titles found! Check the script and try again.")
        return
    
    # Sort by title ID for consistent ordering
    unique_titles.sort(key=lambda x: x[0])
    
    # Generate C code
    print("Generating C code...")
    c_code = generate_c_code(unique_titles)
    
    # Write to file
    output_file = 'source/title_database.c'
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(c_code)
        print(f"Successfully wrote {len(unique_titles)} titles to {output_file}")
    except Exception as e:
        print(f"Error writing to {output_file}: {e}")
        return
    
    # Create backup of old database
    backup_file = f'source/title_database_backup_{int(time.time())}.c'
    try:
        if os.path.exists(output_file):
            with open(output_file, 'r', encoding='utf-8') as src:
                with open(backup_file, 'w', encoding='utf-8') as dst:
                    dst.write(src.read())
            print(f"Created backup: {backup_file}")
    except Exception as e:
        print(f"Warning: Could not create backup: {e}")
    
    print(f"\nDatabase update complete!")
    print(f"Previous database: 2,508 titles")
    print(f"New database: {len(unique_titles)} titles")
    print(f"Added: {len(unique_titles) - 2508} new titles")

if __name__ == "__main__":
    main()
