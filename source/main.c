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

#define FILTER_ROW_PATCHES 0
#define FILTER_ROW_DLC 1
#define FILTER_ROW_SYSTEM 2
#define FILTER_ROW_DEMOS 3
#define FILTER_ROW_DSIWARE 4
#define FILTER_ROW_CONTENT 5
#define FILTER_ROW_NATIVE 6
#define FILTER_ROW_VC 7
#define FILTER_ROW_SD 8
#define FILTER_ROW_NAND 9
#define FILTER_ROW_HOMEBREW 10
#define FILTER_ROW_COUNT 11

static bool g_include_homebrew = false;
static bool g_include_sd = true;
static bool g_include_nand = false;
static title_filter_options_t g_filters = { false, false, false, true, true, true, true, true };
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

static void print_picked_view(const picked_view_t *view)
{
	ui_view_t ui = make_ui_view(view);

	ui_draw_main_screen(&ui);
}

static bool filter_row_enabled(u32 row)
{
	switch (row) {
	case FILTER_ROW_PATCHES:
		return g_filters.include_patches;
	case FILTER_ROW_DLC:
		return g_filters.include_dlc;
	case FILTER_ROW_SYSTEM:
		return g_filters.include_system;
	case FILTER_ROW_DEMOS:
		return g_filters.include_demos;
	case FILTER_ROW_DSIWARE:
		return g_filters.include_dsiware;
	case FILTER_ROW_CONTENT:
		return g_filters.include_content_packs;
	case FILTER_ROW_NATIVE:
		return g_filters.include_native_apps;
	case FILTER_ROW_VC:
		return g_filters.include_virtual_console;
	case FILTER_ROW_SD:
		return g_include_sd;
	case FILTER_ROW_NAND:
		return g_include_nand;
	case FILTER_ROW_HOMEBREW:
		return g_include_homebrew;
	default:
		return false;
	}
}

static void toggle_filter_row(u32 row)
{
	switch (row) {
	case FILTER_ROW_PATCHES:
		g_filters.include_patches = !g_filters.include_patches;
		break;
	case FILTER_ROW_DLC:
		g_filters.include_dlc = !g_filters.include_dlc;
		break;
	case FILTER_ROW_SYSTEM:
		g_filters.include_system = !g_filters.include_system;
		break;
	case FILTER_ROW_DEMOS:
		g_filters.include_demos = !g_filters.include_demos;
		break;
	case FILTER_ROW_DSIWARE:
		g_filters.include_dsiware = !g_filters.include_dsiware;
		break;
	case FILTER_ROW_CONTENT:
		g_filters.include_content_packs = !g_filters.include_content_packs;
		break;
	case FILTER_ROW_NATIVE:
		g_filters.include_native_apps = !g_filters.include_native_apps;
		break;
	case FILTER_ROW_VC:
		g_filters.include_virtual_console = !g_filters.include_virtual_console;
		break;
	case FILTER_ROW_SD:
		g_include_sd = !g_include_sd;
		break;
	case FILTER_ROW_NAND:
		g_include_nand = !g_include_nand;
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
	case FILTER_ROW_PATCHES:
		return "Patches";
	case FILTER_ROW_DLC:
		return "DLC";
	case FILTER_ROW_SYSTEM:
		return "System";
	case FILTER_ROW_DEMOS:
		return "Demos";
	case FILTER_ROW_DSIWARE:
		return "DSiWare";
	case FILTER_ROW_CONTENT:
		return "Content";
	case FILTER_ROW_NATIVE:
		return "Native apps";
	case FILTER_ROW_VC:
		return "Virtual Console";
	case FILTER_ROW_SD:
		return "SD titles";
	case FILTER_ROW_NAND:
		return "NAND titles";
	case FILTER_ROW_HOMEBREW:
		return "Homebrew";
	default:
		return "";
	}
}

static void draw_filter_menu(u32 cursor)
{
	u32 filters_on = ui_count_enabled_filters(FILTER_ROW_COUNT, filter_row_enabled);

	ui_draw_filter_menu(cursor, FILTER_ROW_COUNT, filters_on, filter_row_enabled, filter_row_label);
}

static bool run_filter_menu(void)
{
	u32 cursor = 0;
	bool changed = false;

	draw_filter_menu(cursor);

	while (aptMainLoop()) {
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		u32 kDown = hidKeysDown();

		if (kDown & (KEY_B | KEY_SELECT))
			break;

		if (kDown & KEY_UP) {
			cursor = (cursor + FILTER_ROW_COUNT - 1) % FILTER_ROW_COUNT;
			draw_filter_menu(cursor);
		} else if (kDown & KEY_DOWN) {
			cursor = (cursor + 1) % FILTER_ROW_COUNT;
			draw_filter_menu(cursor);
		} else if (kDown & KEY_A) {
			toggle_filter_row(cursor);
			changed = true;
			draw_filter_menu(cursor);
		}
	}

	return changed;
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

static void show_empty_pool(bool suggest_sources)
{
	ui_draw_empty_pool_message(g_sd_title_count, g_nand_title_count, g_include_sd, g_include_nand,
		suggest_sources);
	wait_for_start_exit();
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
		show_empty_pool(true);
		goto cleanup_normal;
	}

randomPicker:
	if (pool.count == 0) {
		show_empty_pool(false);
		goto cleanup_normal;
	}

	u64 randomTitle = 0;
	FS_MediaType randomMedia = MEDIATYPE_SD;
	picked_view_t view;
	memset(&view, 0, sizeof(view));
	view.page = 0;

	if (!pick_random_title(&pool, &randomTitle, &randomMedia)) {
		ui_draw_header();
		printf("\nFailed to pick a random title.\n\n");
		printf("\x1b[90mSTART\x1b[0m Exit\n\n");
		wait_for_start_exit();
		goto cleanup_normal;
	}

	title_picker_load_pick(randomTitle, randomMedia, g_include_homebrew, &view.pick);
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
			if (!title_picker_is_eligible(randomTitle, &g_filters, g_include_homebrew))
				goto randomPicker;
			title_picker_load_pick(randomTitle, view.pick.media, g_include_homebrew, &view.pick);
			print_picked_view(&view);
		}

		if (kDown & KEY_SELECT) {
			if (run_filter_menu()) {
				rebuild_eligible_pool(&pool);
				if (!title_picker_is_eligible(randomTitle, &g_filters, g_include_homebrew))
					goto randomPicker;
				title_picker_load_pick(randomTitle, view.pick.media, g_include_homebrew, &view.pick);
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
