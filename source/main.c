#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>
#include "title_database.h"

// Global variable to store the title count for the header
static u32 g_title_count = 0;

// Function to clear screen and redraw the static header
void clear_and_redraw_header(void) {
    consoleClear();
    printf("\nRead title amount: %lu\n", g_title_count);
    printf("\nRANDOM GAME LAUNCHER\n");
    printf("by selloa (2025)\n\n");
}

int main()
{
	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);
	amInit();
	amAppInit();
	time_t t;

	u32 readTitlesAmount;
	u64 readTitlesID[900] = {};
	Result WhyBroke;
	WhyBroke = AM_GetTitleList(&readTitlesAmount, MEDIATYPE_SD, 900, readTitlesID);
	for (int i = 0; i < readTitlesAmount; i++) {
  			printf("%016llx\n", readTitlesID[i]);
	}

	if (WhyBroke != 0) {
		printf("%ld\n", WhyBroke);
	}

	// Store the title count for the header
	g_title_count = readTitlesAmount;
	
	// Clear screen and draw the static header
	clear_and_redraw_header();

	srand((unsigned) time(&t));
	
randomPicker:
	u64 randomTitle = 0;
	const char* gameName = NULL;
	
	// Keep trying until we find a valid title that exists in our database
	while(randomTitle == 0 || gameName == NULL){
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
			}
			break;
		default:
			// Silently discard invalid titles without showing message
			break;
		}
	}

	// At this point we have a valid title that exists in our database
	printf("%s\n\n", gameName);
	printf("Press A to launch\n");
	printf("Press START to exit\n");
	printf("Press Y to throw the dice again\n\n");

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
			aptSetChainloader(randomTitle, MEDIATYPE_SD); // NOT the *EUR* camera app title ID anymore
			break;
		}

		if (kDown & KEY_Y)
		{
			clear_and_redraw_header();
			goto randomPicker;
		}

	}
	
	amExit();
	gfxExit();
	return 0;
}
