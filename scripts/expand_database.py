#!/usr/bin/env python3
"""
Script to help expand the title database by parsing 3DSDB data.
This script can be used to generate C code for the title_database.c file.
"""

import requests
import json
import re
import xml.etree.ElementTree as ET

def fetch_3dsdb_data():
    """Fetch the latest 3DSDB data"""
    try:
        # Try to fetch the XML data from 3DSDB
        print("Fetching from https://3dsdb.com/xml.php...")
        response = requests.get('https://3dsdb.com/xml.php')
        if response.status_code == 200:
            print(f"Successfully fetched {len(response.text)} characters of data")
            return response.text
        else:
            print(f"Failed to fetch 3DSDB data: {response.status_code}")
            return None
    except Exception as e:
        print(f"Error fetching 3DSDB data: {e}")
        return None

def parse_xml_data(xml_data):
    """Parse XML data and extract title ID to name mappings"""
    try:
        # Parse XML using ElementTree
        root = ET.fromstring(xml_data)
        
        title_mappings = []
        
        # Look for release elements
        for release in root.findall('.//release'):
            title_id_elem = release.find('titleid')
            name_elem = release.find('name')
            
            if title_id_elem is not None and name_elem is not None:
                title_id = title_id_elem.text
                name = name_elem.text
                
                # Clean up the data
                if title_id and name:
                    title_id = title_id.strip().upper()
                    name = name.strip()
                    
                    # Filter for 3DS games only and reasonable names
                    if (len(title_id) == 16 and 
                        title_id.startswith('00040000') and  # Only 3DS games
                        len(name) < 100 and 
                        name and 
                        not name.startswith('System') and
                        not name.startswith('Nintendo 3DS')):
                        title_mappings.append((title_id, name))
        
        return title_mappings
        
    except ET.ParseError as e:
        print(f"XML parsing error: {e}")
        return []
    except Exception as e:
        print(f"Error parsing XML: {e}")
        return []

def generate_c_code(title_mappings):
    """Generate C code for the title database"""
    c_code = '''#include "title_database.h"
#include <string.h>

// Complete 3DS title database generated from 3dsdb.com
// Total entries: {count}

static const title_entry_t title_database[] = {{
'''.format(count=len(title_mappings))
    
    for i, (title_id, name) in enumerate(title_mappings):
        # Escape quotes in the name
        escaped_name = name.replace('"', '\\"')
        # Convert title ID to proper format with ULL suffix
        title_id_formatted = f"0x{title_id}ULL"
        
        c_code += f'    {{{title_id_formatted}, "{escaped_name}"}}'
        if i < len(title_mappings) - 1:
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
    """Main function"""
    print("Fetching 3DSDB data...")
    xml_data = fetch_3dsdb_data()
    
    if xml_data is None:
        print("Failed to fetch data. Using sample data instead.")
        # Sample data for testing
        sample_mappings = [
            ("0004000000030700", "Mario Kart 7"),
            ("0004000000053F00", "Super Mario 3D Land"),
            ("0004000000033600", "The Legend of Zelda: Ocarina of Time 3D"),
        ]
        c_code = generate_c_code(sample_mappings)
    else:
        print("Parsing XML data...")
        title_mappings = parse_xml_data(xml_data)
        print(f"Found {len(title_mappings)} title mappings")
        
        c_code = generate_c_code(title_mappings)
    
    # Write to file
    with open('source/title_database.c', 'w') as f:
        f.write(c_code)
    
    print("Generated complete title_database.c")
    print(f"Database now contains {len(title_mappings)} games!")

if __name__ == "__main__":
    main()
