#ifndef TITLE_DATABASE_H
#define TITLE_DATABASE_H

#include <3ds.h>

// Structure to hold title ID and game name mapping
typedef struct {
    u64 title_id;
    const char* game_name;
} title_entry_t;

// Function to lookup game name by title ID
const char* lookup_game_name(u64 title_id);

// Function to get total number of entries in database
u32 get_database_size(void);

#endif // TITLE_DATABASE_H
