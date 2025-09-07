#!/usr/bin/env python3
"""
Script to fetch all 3DS titles from the 3dsdb API.
This script fetches data from the API endpoints for all categories.
"""

import requests
import json
import time
from typing import List, Tuple, Set

# Base URL for the 3dsdb API
API_BASE = "https://api.ghseshop.cc"

# Categories we want to fetch
CATEGORIES = ["base", "virtual-console", "dsiware"]

def fetch_category_titles(category: str) -> List[Tuple[str, str]]:
    """Fetch all titles from a specific category"""
    print(f"Fetching {category} titles...")
    
    try:
        # First get the list of title IDs in this category
        url = f"{API_BASE}/category/{category}"
        response = requests.get(url, timeout=30)
        response.raise_for_status()
        
        category_data = response.json()
        title_ids = category_data.get('titles', [])
        print(f"Found {len(title_ids)} {category} titles")
        
        titles = []
        
        # Fetch details for each title ID
        for i, title_id in enumerate(title_ids):
            try:
                detail_url = f"{API_BASE}/{title_id}"
                detail_response = requests.get(detail_url, timeout=10)
                detail_response.raise_for_status()
                
                title_data = detail_response.json()
                name = title_data.get('name', 'Unknown')
                
                # Clean the title ID
                clean_title_id = str(title_id).strip().upper()
                
                # Validate the title ID format
                if (len(clean_title_id) == 16 and 
                    clean_title_id.startswith('00040000') and
                    len(name) > 0 and len(name) < 200):
                    titles.append((clean_title_id, name))
                
                # Progress indicator
                if (i + 1) % 100 == 0:
                    print(f"  Processed {i + 1}/{len(title_ids)} {category} titles...")
                
                # Be respectful to the API
                time.sleep(0.1)
                
            except Exception as e:
                print(f"  Error fetching details for {title_id}: {e}")
                continue
        
        print(f"Successfully processed {len(titles)} {category} titles")
        return titles
        
    except Exception as e:
        print(f"Error fetching {category} titles: {e}")
        return []

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

// Complete 3DS title database generated from 3dsdb API
// Total entries: {count}
// Includes regular 3DS games, Virtual Console games, and DSiWare from all regions

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
    print("Starting comprehensive 3DS title database update from 3dsdb API...")
    
    # First, get the stats to confirm what we're fetching
    try:
        stats_response = requests.get(f"{API_BASE}/stats", timeout=30)
        stats = stats_response.json()
        print(f"API Stats: {json.dumps(stats, indent=2)}")
    except Exception as e:
        print(f"Could not fetch API stats: {e}")
    
    all_titles = []
    
    # Fetch titles from each category
    for category in CATEGORIES:
        print(f"\n=== Processing {category} category ===")
        titles = fetch_category_titles(category)
        all_titles.extend(titles)
        print(f"Total titles so far: {len(all_titles)}")
    
    print(f"\nTotal titles found before deduplication: {len(all_titles)}")
    
    # Remove duplicates
    unique_titles = deduplicate_titles(all_titles)
    print(f"Unique titles after deduplication: {len(unique_titles)}")
    
    if not unique_titles:
        print("No titles found! Check the API and try again.")
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
