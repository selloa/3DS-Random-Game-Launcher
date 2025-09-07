#!/usr/bin/env python3
"""
Batch script to fetch 3DS titles from the 3dsdb API in smaller chunks.
This version is more resilient to interruptions and processes data in batches.
"""

import requests
import json
import time
import os
from typing import List, Tuple, Set

# Base URL for the 3dsdb API
API_BASE = "https://api.ghseshop.cc"

# Categories we want to fetch
CATEGORIES = ["base", "virtual-console", "dsiware"]

# Batch size for processing
BATCH_SIZE = 50

def fetch_title_details_batch(title_ids: List[str]) -> List[Tuple[str, str]]:
    """Fetch details for a batch of title IDs"""
    titles = []
    
    for title_id in title_ids:
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
            
            # Small delay to be respectful to the API
            time.sleep(0.05)
            
        except Exception as e:
            print(f"    Error fetching {title_id}: {e}")
            continue
    
    return titles

def fetch_category_titles_batch(category: str) -> List[Tuple[str, str]]:
    """Fetch all titles from a specific category in batches"""
    print(f"Fetching {category} titles...")
    
    try:
        # First get the list of title IDs in this category
        url = f"{API_BASE}/category/{category}"
        response = requests.get(url, timeout=30)
        response.raise_for_status()
        
        category_data = response.json()
        title_ids = category_data.get('titles', [])
        print(f"Found {len(title_ids)} {category} titles")
        
        all_titles = []
        
        # Process in batches
        for i in range(0, len(title_ids), BATCH_SIZE):
            batch = title_ids[i:i + BATCH_SIZE]
            batch_num = (i // BATCH_SIZE) + 1
            total_batches = (len(title_ids) + BATCH_SIZE - 1) // BATCH_SIZE
            
            print(f"  Processing batch {batch_num}/{total_batches} ({len(batch)} titles)...")
            
            batch_titles = fetch_title_details_batch(batch)
            all_titles.extend(batch_titles)
            
            print(f"    Batch {batch_num} complete: {len(batch_titles)} titles")
            
            # Save progress after each batch
            if batch_titles:
                save_progress(category, all_titles, batch_num, total_batches)
            
            # Longer delay between batches
            time.sleep(1)
        
        print(f"Successfully processed {len(all_titles)} {category} titles")
        return all_titles
        
    except Exception as e:
        print(f"Error fetching {category} titles: {e}")
        return []

def save_progress(category: str, titles: List[Tuple[str, str]], batch_num: int, total_batches: int):
    """Save progress to a temporary file"""
    try:
        progress_file = f"temp_{category}_progress.json"
        progress_data = {
            "category": category,
            "titles": titles,
            "batch_num": batch_num,
            "total_batches": total_batches,
            "timestamp": time.time()
        }
        
        with open(progress_file, 'w') as f:
            json.dump(progress_data, f, indent=2)
        
        print(f"    Progress saved: {len(titles)} titles so far")
    except Exception as e:
        print(f"    Warning: Could not save progress: {e}")

def load_progress(category: str) -> List[Tuple[str, str]]:
    """Load progress from a temporary file"""
    progress_file = f"temp_{category}_progress.json"
    
    if os.path.exists(progress_file):
        try:
            with open(progress_file, 'r') as f:
                progress_data = json.load(f)
            
            titles = [(t[0], t[1]) for t in progress_data.get('titles', [])]
            batch_num = progress_data.get('batch_num', 0)
            total_batches = progress_data.get('total_batches', 0)
            
            print(f"Loaded progress for {category}: {len(titles)} titles (batch {batch_num}/{total_batches})")
            return titles
        except Exception as e:
            print(f"Could not load progress for {category}: {e}")
    
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
    print("This version processes data in batches for better reliability.")
    
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
        
        # Try to load existing progress
        existing_titles = load_progress(category)
        if existing_titles:
            print(f"Resuming from previous progress: {len(existing_titles)} titles already fetched")
            all_titles.extend(existing_titles)
        else:
            # Fetch new titles
            titles = fetch_category_titles_batch(category)
            all_titles.extend(titles)
        
        print(f"Total titles so far: {len(all_titles)}")
        
        # Clean up progress file after successful completion
        progress_file = f"temp_{category}_progress.json"
        if os.path.exists(progress_file):
            try:
                os.remove(progress_file)
                print(f"Cleaned up progress file for {category}")
            except:
                pass
    
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
