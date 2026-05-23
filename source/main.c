#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>
#include "title_database.h"
#include "title_smdh.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

// Global variable to track homebrew toggle state
static bool g_include_homebrew = false;


// Function to clear screen and redraw the static header
void clear_and_redraw_header(void) {
    consoleClear();
    printf("\n\x1b[37mRANDOM GAME LAUNCHER\x1b[0m\n");
    printf("\x1b[90mv%s\x1b[0m\n\n", APP_VERSION);
}

static void print_smdh_line(title_smdh_result_t smdhResult, const char *smdhName) {
	printf("\x1b[90mSMDH:\x1b[0m ");
	switch (smdhResult) {
	case TITLE_SMDH_OK:
		printf("\x1b[1;37m%s\x1b[0m\n", smdhName);
		break;
	case TITLE_SMDH_ERR_EMPTY:
		printf("\x1b[90m(empty)\x1b[0m\n");
		break;
	default: {
		Result fsResult = title_smdh_get_last_result();
		printf("\x1b[90m(unavailable");
		if (R_FAILED(fsResult))
			printf(" %08lX", (unsigned long)fsResult);
		printf(")\x1b[0m\n");
		break;
	}
	}
}

static void print_picked_title(u64 titleId, const char *gameName, title_smdh_result_t smdhResult, const char *smdhName) {
	printf("\x1b[90mDatabase:\x1b[0m ");
	if (gameName != NULL) {
		printf("\x1b[1;37m%s\x1b[0m\n", gameName);
	} else {
		printf("\x1b[90m(not found)\x1b[0m\n");
	}

	print_smdh_line(smdhResult, smdhName);
	printf("\x1b[90mTitle ID:\x1b[0m %016llx\n\n", titleId);
}

static void print_controls(void) {
	printf("\x1b[37mPress A to launch\x1b[0m\n");
	printf("\x1b[37mPress Y to throw the dice again\x1b[0m\n\n");
	printf("\x1b[90mPress X to toggle homebrew mode\x1b[0m\n");
	printf("\x1b[90mHomebrew mode: %s\x1b[0m\n\n", g_include_homebrew ? "\x1b[37mON\x1b[0m" : "\x1b[90mOFF\x1b[0m");
	printf("\x1b[90mPress START to exit\x1b[0m\n");
}







int main()
{
	Result res = 0;
	time_t t;
	bool fsReady = false;

	// Initialize graphics system
	gfxInitDefault();
	
	// Initialize console
	consoleInit(GFX_TOP, NULL);
	
	// Initialize Application Manager
	res = amInit();
	if (R_FAILED(res)) {
		goto cleanup_error;
	}

	res = fsInit();
	if (R_FAILED(res)) {
		goto cleanup_error;
	}
	fsReady = true;

	u32 readTitlesAmount;
	u64 readTitlesID[900] = {};
	
	// Get title list
	res = AM_GetTitleList(&readTitlesAmount, MEDIATYPE_SD, 900, readTitlesID);
	if (R_FAILED(res)) {
		goto cleanup_error;
	}
	

	// Clear screen and draw the static header
	clear_and_redraw_header();

	// Check if no titles are installed
	if (readTitlesAmount == 0) {
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
	char smdhName[TITLE_SMDH_SHORT_NAME_UTF8_MAX];
	title_smdh_result_t smdhResult = TITLE_SMDH_ERR_OPEN;
	u32 attempts = 0;
	const u32 max_attempts = 100; // Prevent infinite loops
	
	
	// Keep trying until we find a valid title that exists in our database
	while((randomTitle == 0 || gameName == NULL) && attempts < max_attempts){
		attempts++;
		u32 randomTitlePicked = rand() % readTitlesAmount;

		unsigned char contentCategory = ((unsigned char*)(&readTitlesID[randomTitlePicked]))[4];

		// Debug: Show what we're checking (DISABLED - was showing on screen)
		// if (g_include_homebrew) {
		// 	printf("Checking title %016llx (category: 0x%02x)\n", readTitlesID[randomTitlePicked], contentCategory);
		// }

		// TEMPORARILY DISABLED: Category filtering - now accepts all categories
		// switch (contentCategory)
		// {
		// case 0x00:
		// case 0x02:
			randomTitle = readTitlesID[randomTitlePicked];
			// Check if this title exists in our database
			gameName = lookup_game_name(randomTitle);
			if (gameName == NULL) {
				// Title not in database - check if homebrew mode is enabled
				if (g_include_homebrew) {
					// Use homebrew title - we'll display the titleID as fallback
					gameName = NULL; // Will be handled in display logic
					// printf("Found homebrew title: %016llx\n", randomTitle); // DISABLED - debug message
					break; // Found a homebrew title, exit the switch
				} else {
					// Title not in database and homebrew mode disabled, reset and try again
					randomTitle = 0;
					gameName = NULL;
				}
			} else {
				// Found a title in database, we're good
				break;
			}
		// 	break;
		// default:
		// 	// Silently discard invalid titles without showing message
		// 	if (g_include_homebrew) {
		// 		printf("Skipping title %016llx (category: 0x%02x - not 0x00 or 0x02)\n", readTitlesID[randomTitlePicked], contentCategory);
		// 	}
		// 	break;
		// }
	}
	
	// Check if we failed to find a valid title
	if (attempts >= max_attempts) {
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

	smdhResult = title_smdh_get_short_name(randomTitle, MEDIATYPE_SD, smdhName, sizeof(smdhName));

	// At this point we have a valid title — show DB and SMDH names for comparison
	print_picked_title(randomTitle, gameName, smdhResult, smdhName);
	print_controls();

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
			if (gameName != NULL) {
				printf("Launching %s...\n", gameName);
			} else {
				printf("Launching %016llx...\n", randomTitle);
			}
			
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
			// Toggle homebrew mode
			g_include_homebrew = !g_include_homebrew;
			
			// Clear screen and redraw with updated toggle state
			clear_and_redraw_header();
			print_picked_title(randomTitle, gameName, smdhResult, smdhName);
			print_controls();
		}
		

	}
	
	goto cleanup_normal;

cleanup_error:
	// Error cleanup - wait for user input
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
	if (fsReady)
		fsExit();
	amExit();
	gfxExit();
	return 0;
}
