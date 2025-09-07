#include "title_database.h"
#include <string.h>

// Database of title ID to game name mappings
// This is a subset of popular games - we can expand this later
static const title_entry_t title_database[] = {
    // Popular Nintendo Games
    {0x0004000000030700, "Mario Kart 7"},
    {0x0004000000053F00, "Super Mario 3D Land"},
    {0x0004000000033600, "The Legend of Zelda: Ocarina of Time 3D"},
    {0x0004000000049100, "Star Fox 64 3D"},
    {0x0004000000031D00, "PilotWings Resort"},
    {0x00040000001D1900, "Luigi's Mansion"},
    {0x00040000001D1E00, "Kirby's Extra Epic Yarn"},
    {0x00040000001D1F00, "Kirby's Extra Epic Yarn"},
    {0x00040000001D1200, "Keito no Kirby Plus"},
    
    // Popular Third-Party Games
    {0x00040000001D7100, "Persona Q2: New Cinema Labyrinth"},
    {0x00040000001D7600, "Persona Q2: New Cinema Labyrinth"},
    {0x00040000001CBE00, "Persona Q2: New Cinema Labyrinth"},
    {0x00040000001D6800, "Yo-Kai Watch 3"},
    {0x00040000001D6700, "Yo-Kai Watch 3"},
    {0x00040000001D4E00, "Etrian Odyssey Nexus"},
    {0x00040000001D5200, "Etrian Odyssey Nexus"},
    {0x00040000001CBC00, "Jake Hunter Detective Story: Ghost of the Dusk"},
    {0x00040000001D9300, "Atooi Collection"},
    
    // Classic Games
    {0x0004000000033B00, "Ridge Racer 3D"},
    {0x0004000000033A00, "Tom Clancy's Splinter Cell 3D"},
    {0x0004000000038A00, "Dead or Alive: Dimensions"},
    {0x0004000000033C00, "Super Street Fighter IV: 3D Edition"},
    {0x0004000000033900, "Rayman 3D"},
    {0x0004000000033800, "Puzzle Bobble Universe"},
    {0x0004000000038B00, "Resident Evil: The Mercenaries 3D"},
    {0x0004000000038C00, "LEGO Star Wars III: The Clone Wars"},
    {0x0004000000038900, "Super Monkey Ball 3D"},
    {0x0004000000037500, "Tom Clancy's Ghost Recon: Shadow Wars"},
    
    // Add more entries as needed...
    // We can expand this database by parsing the 3DSDB XML data
};

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
