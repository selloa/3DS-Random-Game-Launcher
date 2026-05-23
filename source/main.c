#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>
#include "title_database.h"
#include "title_meta.h"
#include "title_smdh.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#define TITLE_PAGE_COUNT 3

static bool g_include_homebrew = false;

typedef struct {
	u64 titleId;
	const char *dbName;
	title_smdh_info_t smdh;
	title_meta_t meta;
	u32 page;
} picked_title_t;

static void clear_and_redraw_header(void)
{
	consoleClear();
	printf("\n\x1b[37mRANDOM GAME LAUNCHER\x1b[0m\n");
	printf("\x1b[90mv%s\x1b[0m\n\n", APP_VERSION);
}

static void print_label_value(const char *label, const char *value)
{
	printf("\x1b[90m%s:\x1b[0m ", label);
	if (value != NULL && value[0] != '\0')
		printf("\x1b[1;37m%s\x1b[0m\n", value);
	else
		printf("\x1b[90m(unavailable)\x1b[0m\n");
}

static void print_smdh_status(const title_smdh_info_t *smdh)
{
	if (smdh->result == TITLE_SMDH_OK)
		return;

	printf("\x1b[90mSMDH read:\x1b[0m \x1b[90m");
	switch (smdh->result) {
	case TITLE_SMDH_ERR_EMPTY:
		printf("empty");
		break;
	case TITLE_SMDH_ERR_FORMAT:
		printf("bad format");
		break;
	default:
		printf("unavailable");
		if (R_FAILED(title_smdh_get_last_result()))
			printf(" %08lX", (unsigned long)title_smdh_get_last_result());
		break;
	}
	printf("\x1b[0m\n");
}

static void print_page_summary(const picked_title_t *pick)
{
	char version[16];
	char size[32];

	print_label_value("Database", pick->dbName);
	print_label_value("SMDH short", pick->smdh.short_name);
	print_smdh_status(&pick->smdh);
	printf("\x1b[90mTitle ID:\x1b[0m %016llx\n", pick->titleId);
	print_label_value("Category", title_meta_category_name(pick->meta.content_category));
	printf("\x1b[90mCategory code:\x1b[0m 0x%04X\n", pick->meta.content_category);

	if (pick->meta.product_code[0] != '\0')
		print_label_value("Product code", pick->meta.product_code);

	if (R_SUCCEEDED(pick->meta.title_info_result)) {
		title_meta_format_version(pick->meta.version_major, pick->meta.version_minor, version, sizeof(version));
		title_meta_format_size(pick->meta.installed_size, size, sizeof(size));
		print_label_value("Version", version);
		print_label_value("Installed size", size);
	}
}

static void print_page_smdh(const picked_title_t *pick)
{
	char ratings[TITLE_SMDH_LINE_UTF8_MAX];
	char region[TITLE_SMDH_LINE_UTF8_MAX];
	char flags[TITLE_SMDH_LINE_UTF8_MAX];
	char eula[32];

	print_label_value("Short name", pick->smdh.short_name);
	print_label_value("Long name", pick->smdh.long_name);
	print_label_value("Publisher", pick->smdh.publisher);
	print_smdh_status(&pick->smdh);

	title_smdh_format_ratings(&pick->smdh, ratings, sizeof(ratings));
	title_smdh_format_region_lock(pick->smdh.region_lock, region, sizeof(region));
	title_smdh_format_flags(pick->smdh.flags, flags, sizeof(flags));

	print_label_value("Ratings", ratings);
	print_label_value("Region lock", region);
	print_label_value("Flags", flags);

	snprintf(eula, sizeof(eula), "%u.%u", pick->smdh.eula_major, pick->smdh.eula_minor);
	print_label_value("EULA version", eula);
	printf("\x1b[90mStreetPass ID:\x1b[0m %08lX\n", pick->smdh.cec_id);
}

static void print_page_system(const picked_title_t *pick)
{
	char size[32];
	char version[16];
	char extdata[32];

	printf("\x1b[90mTitle ID:\x1b[0m %016llx\n", pick->titleId);
	print_label_value("Platform", title_meta_platform_name(pick->meta.platform));
	printf("\x1b[90mPlatform code:\x1b[0m 0x%04X\n", pick->meta.platform);
	print_label_value("Category", title_meta_category_name(pick->meta.content_category));
	printf("\x1b[90mCategory code:\x1b[0m 0x%04X\n", pick->meta.content_category);
	printf("\x1b[90mUnique ID:\x1b[0m 0x%06lX\n", pick->meta.unique_id);
	printf("\x1b[90mVariation:\x1b[0m 0x%02X\n", pick->meta.variation);
	printf("\x1b[90mNew 3DS only:\x1b[0m %s\n", pick->meta.new3ds_only ? "Yes" : "No");

	if (pick->meta.product_code[0] != '\0')
		print_label_value("Product code", pick->meta.product_code);
	else if (R_FAILED(pick->meta.product_code_result))
		printf("\x1b[90mProduct code:\x1b[0m \x1b[90munavailable %08lX\x1b[0m\n",
			(unsigned long)pick->meta.product_code_result);

	if (R_SUCCEEDED(pick->meta.title_info_result)) {
		title_meta_format_version(pick->meta.version_major, pick->meta.version_minor, version, sizeof(version));
		title_meta_format_size(pick->meta.installed_size, size, sizeof(size));
		print_label_value("Version", version);
		print_label_value("Installed size", size);
	} else {
		printf("\x1b[90mTitle info:\x1b[0m \x1b[90munavailable %08lX\x1b[0m\n",
			(unsigned long)pick->meta.title_info_result);
	}

	if (pick->meta.has_extdata) {
		snprintf(extdata, sizeof(extdata), "%011llX", pick->meta.extdata_id);
		print_label_value("Extdata ID", extdata);
	} else if (R_FAILED(pick->meta.extdata_result)) {
		printf("\x1b[90mExtdata ID:\x1b[0m \x1b[90munavailable %08lX\x1b[0m\n",
			(unsigned long)pick->meta.extdata_result);
	} else {
		print_label_value("Extdata ID", NULL);
	}

	printf("\x1b[90mMedia:\x1b[0m SD\n");
}

static void print_controls(void)
{
	printf("\n\x1b[90mL / R: info pages\x1b[0m\n");
	printf("\x1b[37mPress A to launch\x1b[0m\n");
	printf("\x1b[37mPress Y to throw the dice again\x1b[0m\n\n");
	printf("\x1b[90mPress X to toggle homebrew mode\x1b[0m\n");
	printf("\x1b[90mHomebrew mode: %s\x1b[0m\n\n", g_include_homebrew ? "\x1b[37mON\x1b[0m" : "\x1b[90mOFF\x1b[0m");
	printf("\x1b[90mPress START to exit\x1b[0m\n");
}

static void print_picked_title(const picked_title_t *pick)
{
	clear_and_redraw_header();
	printf("\x1b[90mPage %lu/%d\x1b[0m\n\n", pick->page + 1, TITLE_PAGE_COUNT);

	switch (pick->page) {
	case 0:
		print_page_summary(pick);
		break;
	case 1:
		print_page_smdh(pick);
		break;
	case 2:
		print_page_system(pick);
		break;
	default:
		break;
	}

	print_controls();
}

static void load_picked_title(u64 titleId, const char *dbName, picked_title_t *pick)
{
	memset(pick, 0, sizeof(*pick));
	pick->titleId = titleId;
	pick->dbName = dbName;
	pick->page = 0;
	title_smdh_load(titleId, MEDIATYPE_SD, &pick->smdh);
	title_meta_load(titleId, MEDIATYPE_SD, &pick->meta);
}

int main()
{
	Result res = 0;
	time_t t;
	bool fsReady = false;

	gfxInitDefault();
	consoleInit(GFX_TOP, NULL);

	res = amInit();
	if (R_FAILED(res))
		goto cleanup_error;

	res = fsInit();
	if (R_FAILED(res))
		goto cleanup_error;
	fsReady = true;

	u32 readTitlesAmount;
	u64 readTitlesID[900] = {};

	res = AM_GetTitleList(&readTitlesAmount, MEDIATYPE_SD, 900, readTitlesID);
	if (R_FAILED(res))
		goto cleanup_error;

	clear_and_redraw_header();

	if (readTitlesAmount == 0) {
		printf("No games found on SD card!\n\n");
		printf("Please install some games to your 3DS SD card\n");
		printf("and try again.\n\n");
		printf("Press START to exit\n\n");

		while (aptMainLoop()) {
			gspWaitForVBlank();
			gfxSwapBuffers();
			hidScanInput();

			if (hidKeysDown() & KEY_START)
				break;
		}

		goto cleanup_normal;
	}

	srand((unsigned)time(&t));

randomPicker:
	u64 randomTitle = 0;
	const char *gameName = NULL;
	picked_title_t pick;
	u32 attempts = 0;
	const u32 max_attempts = 100;

	while ((randomTitle == 0 || gameName == NULL) && attempts < max_attempts) {
		attempts++;
		u32 randomTitlePicked = rand() % readTitlesAmount;

		randomTitle = readTitlesID[randomTitlePicked];
		gameName = lookup_game_name(randomTitle);
		if (gameName == NULL) {
			if (g_include_homebrew) {
				gameName = NULL;
				break;
			}
			randomTitle = 0;
			gameName = NULL;
		} else {
			break;
		}
	}

	if (attempts >= max_attempts) {
		printf("Failed to find a valid game after %lu attempts!\n", max_attempts);
		printf("This may indicate an issue with your game library\n");
		printf("or the title database.\n\n");
		printf("Press START to exit\n\n");

		while (aptMainLoop()) {
			gspWaitForVBlank();
			gfxSwapBuffers();
			hidScanInput();

			if (hidKeysDown() & KEY_START)
				break;
		}

		goto cleanup_normal;
	}

	load_picked_title(randomTitle, gameName, &pick);
	print_picked_title(&pick);

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;

		if (kDown & KEY_A) {
			if (gameName != NULL)
				printf("Launching %s...\n", gameName);
			else
				printf("Launching %016llx...\n", randomTitle);

			aptSetChainloader(randomTitle, MEDIATYPE_SD);
			break;
		}

		if (kDown & KEY_Y) {
			goto randomPicker;
		}

		if (kDown & KEY_X) {
			g_include_homebrew = !g_include_homebrew;
			print_picked_title(&pick);
		}

		if (kDown & KEY_L) {
			pick.page = (pick.page + TITLE_PAGE_COUNT - 1) % TITLE_PAGE_COUNT;
			print_picked_title(&pick);
		}

		if (kDown & KEY_R) {
			pick.page = (pick.page + 1) % TITLE_PAGE_COUNT;
			print_picked_title(&pick);
		}
	}

	goto cleanup_normal;

cleanup_error:
	printf("Press START to exit\n\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		if (hidKeysDown() & KEY_START)
			break;
	}

cleanup_normal:
	if (fsReady)
		fsExit();
	amExit();
	gfxExit();
	return 0;
}
