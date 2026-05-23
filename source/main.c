#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <time.h>
#include "title_database.h"
#include "title_meta.h"
#include "title_picker.h"
#include "title_smdh.h"

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#define TITLE_PAGE_COUNT 3

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
static u64 g_sd_titles[TITLE_SCAN_MAX];
static u64 g_nand_titles[TITLE_SCAN_MAX];
static title_source_t g_active_titles[TITLE_PICKER_POOL_MAX];
static u32 g_active_title_count = 0;

typedef struct {
	title_pick_t pick;
	u32 page;
} picked_view_t;

static void clear_and_redraw_header(void)
{
	consoleClear();
	printf("\n\x1b[37mRANDOM GAME LAUNCHER\x1b[0m\n");
	printf("\x1b[90mv%s\x1b[0m\n\n", APP_VERSION);
}

static const char *name_source_label(title_name_source_t source)
{
	switch (source) {
	case TITLE_NAME_SOURCE_SMDH:
		return "SMDH";
	case TITLE_NAME_SOURCE_CATALOG:
		return "Catalog fallback";
	default:
		return "Title ID";
	}
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

static void print_page_summary(const title_pick_t *pick)
{
	char version[16];
	char size[32];

	print_label_value("Name", pick->display_name);
	printf("\x1b[90mName source:\x1b[0m %s\n", name_source_label(pick->name_source));

	if (pick->name_source == TITLE_NAME_SOURCE_CATALOG && pick->catalog_name != NULL)
		print_label_value("Catalog fallback", pick->catalog_name);

	if (pick->is_homebrew)
		printf("\x1b[90mHomebrew:\x1b[0m \x1b[1;37mYes\x1b[0m\n");
	else if (g_include_homebrew)
		printf("\x1b[90mHomebrew:\x1b[0m No\n");

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

static void print_page_smdh(const title_pick_t *pick)
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

static void print_page_system(const title_pick_t *pick)
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

	printf("\x1b[90mMedia:\x1b[0m %s\n", pick->media == MEDIATYPE_NAND ? "NAND" : "SD");
}

static const char *media_inclusion_status(bool included)
{
	return included ? "included" : "off";
}

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

static void print_filter_status_line(void)
{
	printf("\x1b[90mFilters:\x1b[0m Patches %s | DLC %s | System %s\n",
		g_filters.include_patches ? "ON" : "off",
		g_filters.include_dlc ? "ON" : "off",
		g_filters.include_system ? "ON" : "off");
	printf("\x1b[90m         \x1b[0m Demos %s | DSiWare %s | Content %s\n",
		g_filters.include_demos ? "ON" : "off",
		g_filters.include_dsiware ? "ON" : "off",
		g_filters.include_content_packs ? "ON" : "off");
	printf("\x1b[90m         \x1b[0m Native apps %s | Virtual Console %s\n",
		g_filters.include_native_apps ? "ON" : "off",
		g_filters.include_virtual_console ? "ON" : "off");
}

static void print_controls(void)
{
	printf("\n\x1b[90mSD titles found:\x1b[0m %lu (%s)", g_sd_title_count, media_inclusion_status(g_include_sd));
	printf(" \x1b[90m| NAND:\x1b[0m %lu (%s)", g_nand_title_count, media_inclusion_status(g_include_nand));
	if (g_eligible_title_count != g_active_title_count)
		printf(" \x1b[90m(%lu eligible)\x1b[0m", g_eligible_title_count);
	printf("\n");
	printf("\n\x1b[90mL / R: info pages\x1b[0m\n");
	printf("\x1b[90mSELECT: filter menu\x1b[0m\n");
	print_filter_status_line();
	printf("\x1b[37mPress A to launch\x1b[0m\n");
	printf("\x1b[37mPress Y to throw the dice again\x1b[0m\n\n");
	printf("\x1b[90mPress X to toggle homebrew mode\x1b[0m\n");
	printf("\x1b[90mHomebrew mode: %s\x1b[0m\n\n", g_include_homebrew ? "\x1b[37mON\x1b[0m" : "\x1b[90mOFF\x1b[0m");
	printf("\x1b[90mPress START to exit\x1b[0m\n");
}

static void print_picked_view(const picked_view_t *view)
{
	clear_and_redraw_header();
	printf("\x1b[90mPage %lu/%d\x1b[0m\n\n", view->page + 1, TITLE_PAGE_COUNT);

	switch (view->page) {
	case 0:
		print_page_summary(&view->pick);
		break;
	case 1:
		print_page_smdh(&view->pick);
		break;
	case 2:
		print_page_system(&view->pick);
		break;
	default:
		break;
	}

	print_controls();
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
		return "Include patches";
	case FILTER_ROW_DLC:
		return "Include DLC";
	case FILTER_ROW_SYSTEM:
		return "Include system titles";
	case FILTER_ROW_DEMOS:
		return "Include demos";
	case FILTER_ROW_DSIWARE:
		return "Include DSiWare";
	case FILTER_ROW_CONTENT:
		return "Include content packs";
	case FILTER_ROW_NATIVE:
		return "Include native apps";
	case FILTER_ROW_VC:
		return "Include Virtual Console";
	case FILTER_ROW_SD:
		return "Include SD titles";
	case FILTER_ROW_NAND:
		return "Include NAND titles";
	case FILTER_ROW_HOMEBREW:
		return "Homebrew mode";
	default:
		return "";
	}
}

static void draw_filter_menu(u32 cursor)
{
	u32 row;

	consoleClear();
	printf("\n\x1b[37mPicker filters\x1b[0m\n\n");

	for (row = 0; row < FILTER_ROW_COUNT; row++) {
		if (row == cursor)
			printf("\x1b[1;37m> ");
		else
			printf("  ");

		printf("%s: %s\x1b[0m\n", filter_row_label(row),
			filter_row_enabled(row) ? "\x1b[37mON" : "\x1b[90mOFF");
	}

	printf("\n\x1b[90mUp/Down: move  A: toggle\x1b[0m\n");
	printf("\x1b[90mB or SELECT: close\x1b[0m\n");
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

	clear_and_redraw_header();

	if (g_sd_title_count == 0 && g_nand_title_count == 0) {
		printf("No titles found on SD or NAND!\n\n");
		printf("Install titles to SD or system memory\n");
		printf("and try again.\n\n");
		printf("Press START to exit\n\n");
		wait_for_start_exit();
		goto cleanup_normal;
	}

	srand((unsigned)time(&t));
	rebuild_eligible_pool(&pool);

	if (pool.count == 0) {
		clear_and_redraw_header();
		printf("No titles match current filters.\n\n");
		printf("\x1b[90mSD titles found:\x1b[0m %lu (%s)\n", g_sd_title_count, media_inclusion_status(g_include_sd));
		printf("\x1b[90mNAND titles found:\x1b[0m %lu (%s)\n\n", g_nand_title_count, media_inclusion_status(g_include_nand));
		printf("Open SELECT to adjust filters");
		if ((g_sd_title_count > 0 && !g_include_sd) || (g_nand_title_count > 0 && !g_include_nand))
			printf(" or enable SD/NAND scanning");
		printf(",\n");
		printf("or install more titles.\n\n");
		printf("Press START to exit\n\n");
		wait_for_start_exit();
		goto cleanup_normal;
	}

randomPicker:
	if (pool.count == 0) {
		clear_and_redraw_header();
		printf("No titles match current filters.\n\n");
		printf("\x1b[90mSD titles found:\x1b[0m %lu (%s)\n", g_sd_title_count, media_inclusion_status(g_include_sd));
		printf("\x1b[90mNAND titles found:\x1b[0m %lu (%s)\n\n", g_nand_title_count, media_inclusion_status(g_include_nand));
		printf("Open SELECT to adjust filters.\n\n");
		printf("Press START to exit\n\n");
		wait_for_start_exit();
		goto cleanup_normal;
	}

	u64 randomTitle = 0;
	FS_MediaType randomMedia = MEDIATYPE_SD;
	picked_view_t view;
	memset(&view, 0, sizeof(view));
	view.page = 0;

	if (!pick_random_title(&pool, &randomTitle, &randomMedia)) {
		clear_and_redraw_header();
		printf("Failed to pick a random title.\n\n");
		printf("Press START to exit\n\n");
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
			view.page = (view.page + TITLE_PAGE_COUNT - 1) % TITLE_PAGE_COUNT;
			print_picked_view(&view);
		}

		if (kDown & KEY_R) {
			view.page = (view.page + 1) % TITLE_PAGE_COUNT;
			print_picked_view(&view);
		}
	}

	goto cleanup_normal;

cleanup_error:
	printf("Press START to exit\n\n");
	wait_for_start_exit();

cleanup_normal:
	if (fsReady)
		fsExit();
	amExit();
	gfxExit();
	return 0;
}
