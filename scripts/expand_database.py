#!/usr/bin/env python3
"""
Script to help expand the title database by parsing 3DSDB data.
This script can be used to generate C code for the title_database.c file.
"""

import requests
import json
import re

def fetch_3dsdb_data():
    """Fetch the latest 3DSDB data"""
    try:
        # Try to fetch the XML data from 3DSDB
        response = requests.get('https://3dsdb.com/xml.php')
        if response.status_code == 200:
            return response.text
        else:
            print(f"Failed to fetch 3DSDB data: {response.status_code}")
            return None
    except Exception as e:
        print(f"Error fetching 3DSDB data: {e}")
        return None

def parse_xml_data(xml_data):
    """Parse XML data and extract title ID to name mappings"""
    # This is a simplified parser - in a real implementation,
    # you'd want to use a proper XML parser like xml.etree.ElementTree
    
    # Extract title entries using regex (simplified approach)
    pattern = r'<titleid>([0-9A-Fa-f]{16})</titleid>.*?<name><!\[CDATA\[(.*?)\]\]></name>'
    matches = re.findall(pattern, xml_data, re.DOTALL)
    
    title_mappings = []
    for title_id, name in matches:
        # Clean up the name
        clean_name = name.strip()
        if clean_name and len(clean_name) < 100:  # Reasonable name length
            title_mappings.append((title_id, clean_name))
    
    return title_mappings

def generate_c_code(title_mappings):
    """Generate C code for the title database"""
    c_code = '''// Auto-generated title database
// Generated from 3DSDB data

static const title_entry_t title_database[] = {
'''
    
    for title_id, name in title_mappings:
        # Escape quotes in the name
        escaped_name = name.replace('"', '\\"')
        c_code += f'    {{0x{title_id}, "{escaped_name}"}},\n'
    
    c_code += '''};
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
    with open('source/title_database_generated.c', 'w') as f:
        f.write(c_code)
    
    print("Generated title_database_generated.c")
    print("You can review and integrate this into your title_database.c file")

if __name__ == "__main__":
    main()
