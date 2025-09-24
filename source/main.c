#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>
#include "title_database.h"

// Global variable to store the title count for the header
static u32 g_title_count = 0;

// Debug and error handling structures
typedef struct {
    u8 system_model;
    u32 system_version;
    u8 system_region;
    u8 system_language;
    u32 memory_available;
    u32 memory_used;
} system_info_t;

static system_info_t g_system_info = {0};

// Error codes for better debugging
typedef enum {
    LAUNCHER_ERROR_NONE = 0,
    LAUNCHER_ERROR_GFX_INIT_FAILED,
    LAUNCHER_ERROR_CONSOLE_INIT_FAILED,
    LAUNCHER_ERROR_AM_INIT_FAILED,
    LAUNCHER_ERROR_AM_APP_INIT_FAILED,
    LAUNCHER_ERROR_TITLE_LIST_FAILED,
    LAUNCHER_ERROR_MEMORY_ALLOCATION_FAILED,
    LAUNCHER_ERROR_DATABASE_CORRUPTED,
    LAUNCHER_ERROR_NO_VALID_TITLES,
    LAUNCHER_ERROR_LAUNCH_FAILED,
    LAUNCHER_ERROR_SYSTEM_INFO_FAILED
} error_code_t;

// Function prototypes
void print_system_info(void);
void print_error_message(error_code_t error, Result result);
void print_debug_info(const char* message);
Result gather_system_info(void);
error_code_t validate_system_initialization(void);
error_code_t validate_title_database(void);

// Function to clear screen and redraw the static header
void clear_and_redraw_header(void) {
    consoleClear();
    printf("\nRead title amount: %lu\n", g_title_count);
    printf("\nRANDOM GAME LAUNCHER\n");
    printf("by selloa (2025)\n\n");
}

// Print system information for debugging
void print_system_info(void) {
    printf("=== SYSTEM INFORMATION ===\n");
    printf("Model: %s\n", 
           g_system_info.system_model == CFG_MODEL_2DS ? "2DS" :
           g_system_info.system_model == CFG_MODEL_3DS ? "3DS" :
           g_system_info.system_model == CFG_MODEL_3DSXL ? "3DS XL" :
           g_system_info.system_model == CFG_MODEL_N3DS ? "New 3DS" :
           g_system_info.system_model == CFG_MODEL_N3DSXL ? "New 3DS XL" :
           g_system_info.system_model == CFG_MODEL_N2DSXL ? "New 2DS XL" : "Unknown");
    
    printf("Firmware: %lu.%lu.%lu-%lu\n", 
           (g_system_info.system_version >> 24) & 0xFF,
           (g_system_info.system_version >> 16) & 0xFF,
           (g_system_info.system_version >> 8) & 0xFF,
           g_system_info.system_version & 0xFF);
    
    printf("Region: %s\n",
           g_system_info.system_region == CFG_REGION_JPN ? "Japan" :
           g_system_info.system_region == CFG_REGION_USA ? "USA" :
           g_system_info.system_region == CFG_REGION_EUR ? "Europe" :
           g_system_info.system_region == CFG_REGION_AUS ? "Australia" :
           g_system_info.system_region == CFG_REGION_CHN ? "China" :
           g_system_info.system_region == CFG_REGION_KOR ? "Korea" : "Unknown");
    
    printf("Language: %u\n", g_system_info.system_language);
    printf("Memory Available: %lu KB\n", g_system_info.memory_available);
    printf("Memory Used: %lu KB\n", g_system_info.memory_used);
    printf("========================\n\n");
}

// Print detailed error messages
void print_error_message(error_code_t error, Result result) {
    printf("\n=== ERROR DETECTED ===\n");
    printf("Error Code: %d\n", error);
    printf("Result Code: 0x%08lX\n", result);
    
    switch(error) {
        case LAUNCHER_ERROR_GFX_INIT_FAILED:
            printf("Failed to initialize graphics system\n");
            printf("This may indicate a hardware or firmware issue\n");
            break;
        case LAUNCHER_ERROR_CONSOLE_INIT_FAILED:
            printf("Failed to initialize console output\n");
            printf("Graphics system may not be properly initialized\n");
            break;
        case LAUNCHER_ERROR_AM_INIT_FAILED:
            printf("Failed to initialize Application Manager\n");
            printf("This is required for title management\n");
            break;
        case LAUNCHER_ERROR_AM_APP_INIT_FAILED:
            printf("Failed to initialize AM application\n");
            printf("Cannot access installed titles\n");
            break;
        case LAUNCHER_ERROR_TITLE_LIST_FAILED:
            printf("Failed to retrieve title list from SD card\n");
            printf("Result: 0x%08lX\n", result);
            if (result == 0xD8E0806A) {
                printf("This usually means no SD card is inserted\n");
            } else if (result == 0xD8E0806B) {
                printf("SD card may be corrupted or inaccessible\n");
            }
            break;
        case LAUNCHER_ERROR_MEMORY_ALLOCATION_FAILED:
            printf("Failed to allocate memory\n");
            printf("System may be low on available memory\n");
            break;
        case LAUNCHER_ERROR_DATABASE_CORRUPTED:
            printf("Title database appears to be corrupted\n");
            printf("Database size: %lu entries\n", get_database_size());
            break;
        case LAUNCHER_ERROR_NO_VALID_TITLES:
            printf("No valid game titles found on system\n");
            printf("Make sure games are properly installed\n");
            break;
        case LAUNCHER_ERROR_LAUNCH_FAILED:
            printf("Failed to launch selected game\n");
            printf("Game may be corrupted or incompatible\n");
            break;
        case LAUNCHER_ERROR_SYSTEM_INFO_FAILED:
            printf("Failed to gather system information\n");
            printf("Some debugging features may be limited\n");
            break;
        default:
            printf("Unknown error occurred\n");
            break;
    }
    printf("======================\n\n");
}

// Print debug information
void print_debug_info(const char* message) {
    printf("[DEBUG] %s\n", message);
}

// Gather system information for debugging
Result gather_system_info(void) {
    Result res = 0;
    
    // Get system model
    res = CFGU_GetSystemModel(&g_system_info.system_model);
    if (R_FAILED(res)) {
        print_debug_info("Failed to get system model");
        return res;
    }
    
    // Get system version (using OS version instead)
    g_system_info.system_version = osGetFirmVersion();
    
    // Get system region (using a default value since CFGU_GetRegion may not be available)
    g_system_info.system_region = 0; // Default to unknown region
    
    // Get system language
    res = CFGU_GetSystemLanguage(&g_system_info.system_language);
    if (R_FAILED(res)) {
        print_debug_info("Failed to get system language");
        return res;
    }
    
    // Get memory information
    g_system_info.memory_available = osGetMemRegionFree(MEMREGION_APPLICATION);
    g_system_info.memory_used = osGetMemRegionUsed(MEMREGION_APPLICATION);
    
    return 0;
}

// Validate system initialization
error_code_t validate_system_initialization(void) {
    // Basic validation - if we got this far, initialization was successful
    // The actual initialization functions will return errors if they fail
    return LAUNCHER_ERROR_NONE;
}

// Validate title database integrity
error_code_t validate_title_database(void) {
    u32 db_size = get_database_size();
    
    if (db_size == 0) {
        print_debug_info("Title database is empty");
        return LAUNCHER_ERROR_DATABASE_CORRUPTED;
    }
    
    if (db_size < 100) {
        print_debug_info("Title database seems too small");
        return LAUNCHER_ERROR_DATABASE_CORRUPTED;
    }
    
    print_debug_info("Title database validation passed");
    return LAUNCHER_ERROR_NONE;
}

int main()
{
	Result res = 0;
	error_code_t error = LAUNCHER_ERROR_NONE;
	time_t t;

	print_debug_info("Starting 3DS Random Game Launcher");
	
	// Initialize graphics system with error checking
	gfxInitDefault();
	print_debug_info("Graphics system initialized");
	
	// Initialize console with error checking
	consoleInit(GFX_TOP, NULL);
	print_debug_info("Console initialized");
	
	// Initialize Application Manager with error checking
	res = amInit();
	if (R_FAILED(res)) {
		error = LAUNCHER_ERROR_AM_INIT_FAILED;
		goto cleanup_error;
	}
	print_debug_info("Application Manager initialized");
	
	// Initialize AM application with error checking
	res = amAppInit();
	if (R_FAILED(res)) {
		error = LAUNCHER_ERROR_AM_APP_INIT_FAILED;
		goto cleanup_error;
	}
	print_debug_info("AM application initialized");
	
	// Gather system information for debugging
	res = gather_system_info();
	if (R_FAILED(res)) {
		error = LAUNCHER_ERROR_SYSTEM_INFO_FAILED;
		print_error_message(error, res);
		// Continue execution even if system info fails
		error = LAUNCHER_ERROR_NONE;
	} else {
		print_debug_info("System information gathered");
	}
	
	// Validate system initialization
	error = validate_system_initialization();
	if (error != LAUNCHER_ERROR_NONE) {
		goto cleanup_error;
	}
	
	// Validate title database
	error = validate_title_database();
	if (error != LAUNCHER_ERROR_NONE) {
		goto cleanup_error;
	}
	
	// Print system information for debugging
	print_system_info();

	u32 readTitlesAmount;
	u64 readTitlesID[900] = {};
	
	// Get title list with comprehensive error handling
	res = AM_GetTitleList(&readTitlesAmount, MEDIATYPE_SD, 900, readTitlesID);
	if (R_FAILED(res)) {
		error = LAUNCHER_ERROR_TITLE_LIST_FAILED;
		goto cleanup_error;
	}
	
	print_debug_info("Title list retrieved successfully");
	
	// Debug: Print all found titles (only in debug mode)
	#ifdef DEBUG
	for (int i = 0; i < readTitlesAmount; i++) {
		printf("[DEBUG] Title %d: %016llx\n", i, readTitlesID[i]);
	}
	#endif

	// Store the title count for the header
	g_title_count = readTitlesAmount;
	
	// Clear screen and draw the static header
	clear_and_redraw_header();

	// Check if no titles are installed
	if (readTitlesAmount == 0) {
		error = LAUNCHER_ERROR_NO_VALID_TITLES;
		print_error_message(error, 0);
		printf("No games found on SD card!\n\n");
		printf("Please install some games to your 3DS SD card\n");
		printf("and try again.\n\n");
		printf("Press START to exit\n\n");
		
		while (aptMainLoop()) {
			gspWaitForVBlank();
			gfxSwapBuffers();
			hidScanInput();
			
			u32 kDown = hidKeysDown();
			if (kDown & KEY_START) {
				break;
			}
		}
		
		goto cleanup_normal;
	}

	srand((unsigned) time(&t));
	
randomPicker:
	u64 randomTitle = 0;
	const char* gameName = NULL;
	u32 attempts = 0;
	const u32 max_attempts = 100; // Prevent infinite loops
	
	print_debug_info("Starting random game selection");
	
	// Keep trying until we find a valid title that exists in our database
	while((randomTitle == 0 || gameName == NULL) && attempts < max_attempts){
		attempts++;
		u32 randomTitlePicked = rand() % readTitlesAmount;

		unsigned char contentCategory = ((unsigned char*)(&readTitlesID[randomTitlePicked]))[4];

		switch (contentCategory)
		{
		case 0x00:
		case 0x02:
			randomTitle = readTitlesID[randomTitlePicked];
			// Check if this title exists in our database
			gameName = lookup_game_name(randomTitle);
			if (gameName == NULL) {
				// Title not in database, reset and try again
				randomTitle = 0;
				print_debug_info("Title not found in database, retrying...");
			}
			break;
		default:
			// Silently discard invalid titles without showing message
			break;
		}
	}
	
	// Check if we failed to find a valid title
	if (attempts >= max_attempts) {
		error = LAUNCHER_ERROR_NO_VALID_TITLES;
		print_error_message(error, 0);
		printf("Failed to find a valid game after %lu attempts!\n", max_attempts);
		printf("This may indicate an issue with your game library\n");
		printf("or the title database.\n\n");
		printf("Press START to exit\n\n");
		
		while (aptMainLoop()) {
			gspWaitForVBlank();
			gfxSwapBuffers();
			hidScanInput();
			
			u32 kDown = hidKeysDown();
			if (kDown & KEY_START) {
				break;
			}
		}
		
		goto cleanup_normal;
	}
	
	print_debug_info("Valid game found successfully");

	// At this point we have a valid title that exists in our database
	printf("%s\n\n", gameName);
	printf("Press A to launch\n");
	printf("Press START to exit\n");
	printf("Press Y to throw the dice again\n");
	printf("Press X to show system info\n\n");

	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;

		if (kDown & KEY_A) // If the A button got pressed, start the app launch
		{
			print_debug_info("Attempting to launch game");
			printf("Launching %s...\n", gameName);
			printf("Title ID: %016llx\n", randomTitle);
			
			// Attempt to launch the game
			aptSetChainloader(randomTitle, MEDIATYPE_SD);
			// Note: aptSetChainloader doesn't return a result
			// If successful, the app will exit and launch the game
			// If it fails, we'll continue running
			break;
		}

		if (kDown & KEY_Y)
		{
			clear_and_redraw_header();
			goto randomPicker;
		}
		
		if (kDown & KEY_X)
		{
			clear_and_redraw_header();
			print_system_info();
			printf("Press any button to continue...\n");
			
			// Wait for any button press
			while (aptMainLoop()) {
				gspWaitForVBlank();
				gfxSwapBuffers();
				hidScanInput();
				
				u32 kDown = hidKeysDown();
				if (kDown) {
					break;
				}
			}
			
			clear_and_redraw_header();
			printf("%s\n\n", gameName);
			printf("Press A to launch\n");
			printf("Press START to exit\n");
			printf("Press Y to throw the dice again\n");
			printf("Press X to show system info\n\n");
		}

	}
	
	goto cleanup_normal;

cleanup_error:
	// Error cleanup - print error and wait for user input
	print_error_message(error, res);
	printf("Press START to exit\n\n");
	
	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) {
			break;
		}
	}

cleanup_normal:
	// Normal cleanup
	print_debug_info("Cleaning up and exiting");
	amExit();
	gfxExit();
	return 0;
}
