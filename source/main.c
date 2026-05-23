#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>
#include "title_database.h"
#include "title_meta.h"
#include "title_picker.h"
#include "title_smdh.h"
#include "ui.h"
#include "settings.h"

#define FILTER_ROW_NATIVE 0
#define FILTER_ROW_VC 1
#define FILTER_ROW_DSIWARE 2
#define FILTER_ROW_DEMOS 3
#define FILTER_ROW_CONTENT 4
#define FILTER_ROW_DLC 5
#define FILTER_ROW_PATCHES 6
#define FILTER_ROW_SYSTEM 7
#define FILTER_ROW_SD 8
#define FILTER_ROW_NAND 9
#define FILTER_ROW_LONG_NAME 10
#define FILTER_ROW_HOMEBREW 11
#define FILTER_ROW_DEFAULTS 12
#define FILTER_ROW_COUNT 13

static bool g_include_homebrew = false;
static bool g_include_sd = true;
static bool g_include_nand = false;
static bool g_prefer_long_name = true;
static title_filter_options_t g_filters = { false, false, false, true, false, false, true, true };
static u32 g_sd_title_count = 0;
static u32 g_nand_title_count = 0;
static u32 g_eligible_title_count = 0;
static u32 g_active_title_count = 0;
static u64 g_sd_titles[TITLE_SCAN_MAX];
static u64 g_nand_titles[TITLE_SCAN_MAX];
static title_source_t g_active_titles[TITLE_PICKER_POOL_MAX];

typedef struct {
	title_pick_t pick;
	u32 page;
} picked_view_t;

static u32 build_active_title_list(void)
{
	u32 i;
	u32 n = 0;

	if (g_include_sd) {
		for (i = 0; i < g_sd_title_count && n < TITLE_PICKER_POOL_MAX; i++) {
			g_active_titles[n].titleId = g_sd_titles[i];
			g_active_titles[n].media = MEDIATYPE_SD;
			n++;
		}
	}

	if (!g_include_nand)
		return n;

	for (i = 0; i < g_nand_title_count && n < TITLE_PICKER_POOL_MAX; i++) {
		u32 j;
		bool duplicate = false;

		for (j = 0; j < n; j++) {
			if (g_active_titles[j].titleId == g_nand_titles[i]) {
				duplicate = true;
				break;
			}
		}
		if (duplicate)
			continue;

		g_active_titles[n].titleId = g_nand_titles[i];
		g_active_titles[n].media = MEDIATYPE_NAND;
		n++;
	}

	return n;
}

static void rebuild_eligible_pool(title_picker_pool_t *pool)
{
	g_active_title_count = build_active_title_list();
	title_picker_rebuild_pool(pool, g_active_titles, g_active_title_count, &g_filters, g_include_homebrew);
	g_eligible_title_count = pool->count;
}

static ui_view_t make_ui_view(const picked_view_t *view)
{
	ui_view_t ui;

	memset(&ui, 0, sizeof(ui));
	ui.pick = &view->pick;
	ui.page = view->page;
	ui.sd_title_count = g_sd_title_count;
	ui.nand_title_count = g_nand_title_count;
	ui.active_title_count = g_active_title_count;
	ui.eligible_title_count = g_eligible_title_count;
	ui.include_sd = g_include_sd;
	ui.include_nand = g_include_nand;
	ui.include_homebrew = g_include_homebrew;
	ui.filters = &g_filters;
	return ui;
}

static void apply_launcher_settings(const launcher_settings_t *settings)
{
	if (settings == NULL)
		return;

	g_include_homebrew = settings->include_unlisted;
	g_include_sd = settings->include_sd;
	g_include_nand = settings->include_nand;
	g_prefer_long_name = settings->prefer_long_name;
	g_filters = settings->filters;
}

static void read_launcher_settings(launcher_settings_t *settings)
{
	if (settings == NULL)
		return;

	settings->include_unlisted = g_include_homebrew;
	settings->include_sd = g_include_sd;
	settings->include_nand = g_include_nand;
	settings->prefer_long_name = g_prefer_long_name;
	settings->filters = g_filters;
}

static void apply_default_filter_settings(void)
{
	launcher_settings_t settings;

	launcher_settings_apply_defaults(&settings);
	apply_launcher_settings(&settings);
}

static void persist_launcher_settings(void)
{
	launcher_settings_t settings;

	read_launcher_settings(&settings);
	if (launcher_settings_match_defaults(&settings))
		launcher_settings_delete();
	else
		launcher_settings_save(&settings);
}

static bool run_restore_defaults_confirm(void)
{
	consoleClear();
	printf("\n\x1b[37mRestore defaults?\x1b[0m\n\n");
	printf("Resets all options and deletes\n");
	printf("your saved settings file.\n\n");
	printf("\x1b[37mA\x1b[0m Confirm   \x1b[37mB\x1b[0m Cancel\n");

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & KEY_A)
			return true;
		if (kDown & KEY_B)
			return false;
	}

	return false;
}

static void print_picked_view(const picked_view_t *view)
{
	ui_view_t ui = make_ui_view(view);

	ui_draw_main_screen(&ui);
}

static bool filter_row_enabled(u32 row)
{
	switch (row) {
	case FILTER_ROW_NATIVE:
		return g_filters.include_native_apps;
	case FILTER_ROW_VC:
		return g_filters.include_virtual_console;
	case FILTER_ROW_DSIWARE:
		return g_filters.include_dsiware;
	case FILTER_ROW_DEMOS:
		return g_filters.include_demos;
	case FILTER_ROW_CONTENT:
		return g_filters.include_content_packs;
	case FILTER_ROW_DLC:
		return g_filters.include_dlc;
	case FILTER_ROW_PATCHES:
		return g_filters.include_patches;
	case FILTER_ROW_SYSTEM:
		return g_filters.include_system;
	case FILTER_ROW_SD:
		return g_include_sd;
	case FILTER_ROW_NAND:
		return g_include_nand;
	case FILTER_ROW_LONG_NAME:
		return g_prefer_long_name;
	case FILTER_ROW_HOMEBREW:
		return g_include_homebrew;
	default:
		return false;
	}
}

static void toggle_filter_row(u32 row)
{
	switch (row) {
	case FILTER_ROW_NATIVE:
		g_filters.include_native_apps = !g_filters.include_native_apps;
		break;
	case FILTER_ROW_VC:
		g_filters.include_virtual_console = !g_filters.include_virtual_console;
		break;
	case FILTER_ROW_DSIWARE:
		g_filters.include_dsiware = !g_filters.include_dsiware;
		break;
	case FILTER_ROW_DEMOS:
		g_filters.include_demos = !g_filters.include_demos;
		break;
	case FILTER_ROW_CONTENT:
		g_filters.include_content_packs = !g_filters.include_content_packs;
		break;
	case FILTER_ROW_DLC:
		g_filters.include_dlc = !g_filters.include_dlc;
		break;
	case FILTER_ROW_PATCHES:
		g_filters.include_patches = !g_filters.include_patches;
		break;
	case FILTER_ROW_SYSTEM:
		g_filters.include_system = !g_filters.include_system;
		break;
	case FILTER_ROW_SD:
		g_include_sd = !g_include_sd;
		break;
	case FILTER_ROW_NAND:
		g_include_nand = !g_include_nand;
		break;
	case FILTER_ROW_LONG_NAME:
		g_prefer_long_name = !g_prefer_long_name;
		break;
	case FILTER_ROW_HOMEBREW:
		g_include_homebrew = !g_include_homebrew;
		break;
	default:
		break;
	}
}

static const char *filter_row_label(u32 row)
{
	switch (row) {
	case FILTER_ROW_NATIVE:
		return "Native apps";
	case FILTER_ROW_VC:
		return "Virtual Console";
	case FILTER_ROW_DSIWARE:
		return "DSiWare";
	case FILTER_ROW_DEMOS:
		return "Demos";
	case FILTER_ROW_CONTENT:
		return "Content";
	case FILTER_ROW_DLC:
		return "DLC";
	case FILTER_ROW_PATCHES:
		return "Patches";
	case FILTER_ROW_SYSTEM:
		return "System";
	case FILTER_ROW_SD:
		return "SD titles";
	case FILTER_ROW_NAND:
		return "NAND titles";
	case FILTER_ROW_LONG_NAME:
		return "Long game name";
	case FILTER_ROW_HOMEBREW:
		return "Unlisted only";
	case FILTER_ROW_DEFAULTS:
		return "Restore defaults";
	default:
		return "";
	}
}

static bool filter_row_is_action(u32 row)
{
	return row == FILTER_ROW_DEFAULTS;
}

static void draw_filter_menu(u32 cursor, const title_picker_pool_t *pool)
{
	u32 filters_on = ui_count_enabled_filters(FILTER_ROW_COUNT, filter_row_enabled, filter_row_is_action);

	ui_draw_filter_menu(cursor, FILTER_ROW_COUNT, filters_on, pool->count, filter_row_enabled,
		filter_row_label, filter_row_is_action);
}

static bool run_filter_menu(title_picker_pool_t *pool)
{
	u32 cursor = 0;
	bool changed = false;

	draw_filter_menu(cursor, pool);

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & (KEY_B | KEY_SELECT)) {
			if (changed)
				persist_launcher_settings();
			break;
		}

		if (kDown & KEY_UP) {
			cursor = (cursor + FILTER_ROW_COUNT - 1) % FILTER_ROW_COUNT;
			draw_filter_menu(cursor, pool);
		} else if (kDown & KEY_DOWN) {
			cursor = (cursor + 1) % FILTER_ROW_COUNT;
			draw_filter_menu(cursor, pool);
		} else if (kDown & KEY_A) {
			if (filter_row_is_action(cursor)) {
				if (run_restore_defaults_confirm()) {
					apply_default_filter_settings();
					launcher_settings_delete();
					changed = true;
					rebuild_eligible_pool(pool);
				}
			} else {
				toggle_filter_row(cursor);
				changed = true;
				rebuild_eligible_pool(pool);
			}
			draw_filter_menu(cursor, pool);
		}
	}

	return changed;
}

static bool run_empty_pool_screen(title_picker_pool_t *pool, bool suggest_sources)
{
	for (;;) {
		ui_draw_empty_pool_message(g_sd_title_count, g_nand_title_count, g_include_sd, g_include_nand,
			suggest_sources);

		while (aptMainLoop()) {
			gspWaitForVBlank();
			gfxSwapBuffers();
			hidScanInput();

			u32 kDown = hidKeysDown();

			if (kDown & KEY_START)
				return false;

			if (kDown & KEY_SELECT) {
				if (run_filter_menu(pool)) {
					rebuild_eligible_pool(pool);
					if (pool->count > 0)
						return true;
				}
				break;
			}
		}
	}
}

static void wait_for_start_exit(void)
{
	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		if (hidKeysDown() & KEY_START)
			break;
	}
}

static bool pick_random_title(const title_picker_pool_t *pool, u64 *outTitleId, FS_MediaType *outMedia)
{
	return title_picker_pick_random(pool, g_active_titles, g_active_title_count, outTitleId, outMedia, NULL);
}

static bool pick_and_load_random_title(const title_picker_pool_t *pool, picked_view_t *view,
	u64 *outTitleId, FS_MediaType *outMedia)
{
	u32 i;

	if (pool == NULL || pool->count == 0)
		return false;

	for (i = 0; i < pool->count; i++) {
		if (!pick_random_title(pool, outTitleId, outMedia))
			return false;

		title_picker_load_pick(*outTitleId, *outMedia, g_include_homebrew, g_prefer_long_name, &view->pick);
		if (!title_picker_unlisted_needs_reroll(g_include_homebrew, &view->pick))
			return true;
	}

	return false;
}

static bool run_unlisted_unnamed_screen(title_picker_pool_t *pool)
{
	for (;;) {
		ui_draw_header();
		printf("\nNo unlisted titles with readable names.\n\n");
		printf("Turn off \x1b[90mUnlisted\x1b[0m or install titles with icons.\n\n");
		printf("\x1b[90mSELECT\x1b[0m Options   \x1b[90mSTART\x1b[0m Exit\n");

		while (aptMainLoop()) {
			gspWaitForVBlank();
			gfxSwapBuffers();
			hidScanInput();

			u32 kDown = hidKeysDown();

			if (kDown & KEY_START)
				return false;

			if (kDown & KEY_SELECT) {
				if (run_filter_menu(pool)) {
					rebuild_eligible_pool(pool);
					if (!g_include_homebrew || pool->count == 0)
						return true;
				}
				break;
			}
		}
	}
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

	{
		launcher_settings_t settings;

		if (launcher_settings_load(&settings))
			apply_launcher_settings(&settings);
	}

	u32 readTitlesAmount;
	title_picker_pool_t pool;

	res = AM_GetTitleList(&readTitlesAmount, MEDIATYPE_SD, TITLE_SCAN_MAX, g_sd_titles);
	if (R_FAILED(res))
		goto cleanup_error;
	g_sd_title_count = readTitlesAmount;

	res = AM_GetTitleList(&readTitlesAmount, MEDIATYPE_NAND, TITLE_SCAN_MAX, g_nand_titles);
	if (R_FAILED(res))
		g_nand_title_count = 0;
	else
		g_nand_title_count = readTitlesAmount;

	if (g_sd_title_count == 0 && g_nand_title_count == 0) {
		ui_draw_header();
		printf("\nNo titles found on SD or NAND.\n\n");
		printf("Install titles and try again.\n\n");
		printf("\x1b[90mSTART\x1b[0m Exit\n\n");
		wait_for_start_exit();
		goto cleanup_normal;
	}

	srand((unsigned)time(&t));
	rebuild_eligible_pool(&pool);

	if (pool.count == 0) {
		if (!run_empty_pool_screen(&pool, true))
			goto cleanup_normal;
	}

randomPicker:
	if (pool.count == 0) {
		if (!run_empty_pool_screen(&pool, false))
			goto cleanup_normal;
	}

	u64 randomTitle = 0;
	FS_MediaType randomMedia = MEDIATYPE_SD;
	picked_view_t view;
	memset(&view, 0, sizeof(view));
	view.page = 0;

	if (!pick_and_load_random_title(&pool, &view, &randomTitle, &randomMedia)) {
		if (g_include_homebrew && pool.count > 0) {
			if (!run_unlisted_unnamed_screen(&pool))
				goto cleanup_normal;
			goto randomPicker;
		}

		ui_draw_header();
		printf("\nFailed to pick a random title.\n\n");
		printf("\x1b[90mSTART\x1b[0m Exit\n\n");
		wait_for_start_exit();
		goto cleanup_normal;
	}
	print_picked_view(&view);

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & KEY_START)
			break;

		if (kDown & KEY_A) {
			printf("Launching %s...\n", view.pick.display_name);
			aptSetChainloader(randomTitle, view.pick.media);
			break;
		}

		if (kDown & KEY_Y) {
			goto randomPicker;
		}

		if (kDown & KEY_X) {
			g_include_homebrew = !g_include_homebrew;
			rebuild_eligible_pool(&pool);
			if (pool.count == 0 || !title_picker_is_eligible(randomTitle, &g_filters, g_include_homebrew))
				goto randomPicker;
			title_picker_load_pick(randomTitle, view.pick.media, g_include_homebrew, g_prefer_long_name,
				&view.pick);
			if (title_picker_unlisted_needs_reroll(g_include_homebrew, &view.pick))
				goto randomPicker;
			print_picked_view(&view);
		}

		if (kDown & KEY_SELECT) {
			if (run_filter_menu(&pool)) {
				rebuild_eligible_pool(&pool);
				if (pool.count == 0 || !title_picker_is_eligible(randomTitle, &g_filters, g_include_homebrew))
					goto randomPicker;
				title_picker_load_pick(randomTitle, view.pick.media, g_include_homebrew, g_prefer_long_name,
					&view.pick);
				if (title_picker_unlisted_needs_reroll(g_include_homebrew, &view.pick))
					goto randomPicker;
			}
			print_picked_view(&view);
		}

		if (kDown & KEY_L) {
			view.page = (view.page + UI_PAGE_COUNT - 1) % UI_PAGE_COUNT;
			print_picked_view(&view);
		}

		if (kDown & KEY_R) {
			view.page = (view.page + 1) % UI_PAGE_COUNT;
			print_picked_view(&view);
		}
	}

	goto cleanup_normal;

cleanup_error:
	ui_draw_header();
	printf("\nSomething went wrong.\n\n");
	printf("\x1b[90mSTART\x1b[0m Exit\n\n");
	wait_for_start_exit();

cleanup_normal:
	if (fsReady)
		fsExit();
	amExit();
	gfxExit();
	return 0;
}
