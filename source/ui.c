#include "ui.h"

#include <stdio.h>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

static const char *page_title(u32 page)
{
	switch (page) {
	case 0:
		return "Game";
	case 1:
		return "Details";
	default:
		return "Technical";
	}
}

static const char *toggle_text(bool enabled)
{
	return enabled ? "ON" : "OFF";
}

static const char *media_inclusion_status(bool included)
{
	return included ? "on" : "off";
}

static void print_label_value(const char *label, const char *value)
{
	printf("\x1b[90m%s:\x1b[0m ", label);
	if (value != NULL && value[0] != '\0')
		printf("\x1b[1;37m%s\x1b[0m\n", value);
	else
		printf("\x1b[90m(unavailable)\x1b[0m\n");
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

static void print_user_page(const ui_view_t *view)
{
	const title_pick_t *pick = view->pick;

	printf("\x1b[1;37m%s\x1b[0m\n\n", pick->display_name);
	print_label_value("Type", title_meta_category_name(pick->meta.content_category));

	if (pick->is_homebrew)
		printf("\x1b[90mHomebrew:\x1b[0m \x1b[1;37mYes\x1b[0m\n");
}

static void print_dev_details_page(const ui_view_t *view)
{
	const title_pick_t *pick = view->pick;
	char ratings[TITLE_SMDH_LINE_UTF8_MAX];
	char region[TITLE_SMDH_LINE_UTF8_MAX];
	char flags[TITLE_SMDH_LINE_UTF8_MAX];
	char eula[32];

	printf("\x1b[90mName source:\x1b[0m %s\n", name_source_label(pick->name_source));
	if (pick->name_source == TITLE_NAME_SOURCE_CATALOG && pick->catalog_name != NULL)
		print_label_value("Catalog fallback", pick->catalog_name);

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

static void print_dev_technical_page(const ui_view_t *view)
{
	const title_pick_t *pick = view->pick;
	const title_filter_options_t *filters = view->filters;
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

	printf("\n\x1b[90mLibrary\x1b[0m\n");
	printf("\x1b[90mSD:\x1b[0m %lu (%s)  \x1b[90mNAND:\x1b[0m %lu (%s)\n",
		view->sd_title_count, media_inclusion_status(view->include_sd),
		view->nand_title_count, media_inclusion_status(view->include_nand));
	printf("\x1b[90mScanned:\x1b[0m %lu  \x1b[90mEligible:\x1b[0m %lu\n",
		view->active_title_count, view->eligible_title_count);

	if (filters != NULL) {
		printf("\n");
		printf("\x1b[90mFilters:\x1b[0m Patches %s | DLC %s | System %s\n",
			toggle_text(filters->include_patches),
			toggle_text(filters->include_dlc),
			toggle_text(filters->include_system));
		printf("\x1b[90m         \x1b[0m Demos %s | DSiWare %s | Content %s\n",
			toggle_text(filters->include_demos),
			toggle_text(filters->include_dsiware),
			toggle_text(filters->include_content_packs));
		printf("\x1b[90m         \x1b[0m Native %s | VC %s\n",
			toggle_text(filters->include_native_apps),
			toggle_text(filters->include_virtual_console));
		printf("\x1b[90mSources:\x1b[0m SD %s | NAND %s | Homebrew %s\n",
			toggle_text(view->include_sd),
			toggle_text(view->include_nand),
			toggle_text(view->include_homebrew));
	}
}

static void print_user_controls(const ui_view_t *view)
{
	printf("\n\x1b[90mPool:\x1b[0m \x1b[37m%lu pickable\x1b[0m\n", view->eligible_title_count);
	printf("\n\x1b[37mA\x1b[0m Launch   \x1b[37mY\x1b[0m Reroll   \x1b[90mSELECT\x1b[0m Options\n");
	printf("\x1b[90mL/R\x1b[0m Details   \x1b[90mX\x1b[0m Homebrew   \x1b[90mSTART\x1b[0m Exit\n");
}

void ui_draw_header(void)
{
	consoleClear();
	printf("\x1b[37mRANDOM GAME LAUNCHER\x1b[0m  \x1b[90mv%s\x1b[0m\n", APP_VERSION);
}

void ui_draw_main_screen(const ui_view_t *view)
{
	if (view == NULL || view->pick == NULL)
		return;

	ui_draw_header();
	printf("\x1b[90m%s · %lu/%d\x1b[0m\n\n", page_title(view->page), view->page + 1, UI_PAGE_COUNT);

	switch (view->page) {
	case 0:
		print_user_page(view);
		break;
	case 1:
		print_dev_details_page(view);
		break;
	case 2:
		print_dev_technical_page(view);
		break;
	default:
		break;
	}

	if (view->page == 0)
		print_user_controls(view);
	else
		printf("\n\x1b[90mL/R\x1b[0m Change page   \x1b[90mSTART\x1b[0m Exit\n");
}

u32 ui_count_enabled_filters(u32 row_count, ui_filter_row_enabled_fn row_enabled)
{
	u32 row;
	u32 count = 0;

	if (row_enabled == NULL)
		return 0;

	for (row = 0; row < row_count; row++) {
		if (row_enabled(row))
			count++;
	}

	return count;
}

void ui_draw_filter_menu(u32 cursor, u32 row_count, u32 filters_on,
	ui_filter_row_enabled_fn row_enabled, ui_filter_row_label_fn row_label)
{
	u32 row;

	consoleClear();
	printf("\n\x1b[37mOptions\x1b[0m  \x1b[90m(%lu on)\x1b[0m\n\n", filters_on);

	for (row = 0; row < row_count; row++) {
		if (row == 0)
			printf("\x1b[90mContent types\x1b[0m\n");
		else if (row == 8)
			printf("\n\x1b[90mSources\x1b[0m\n");
		else if (row == 10)
			printf("\n\x1b[90mOther\x1b[0m\n");

		if (row == cursor)
			printf("\x1b[1;37m> ");
		else
			printf("  ");

		printf("%-16s %s\x1b[0m\n", row_label(row),
			row_enabled(row) ? "\x1b[37mON" : "\x1b[90mOFF");
	}

	printf("\n\x1b[90mUp/Down  A toggle  B/SELECT close\x1b[0m\n");
}

void ui_draw_empty_pool_message(u32 sd_count, u32 nand_count, bool include_sd, bool include_nand,
	bool suggest_sources)
{
	ui_draw_header();
	printf("\nNo titles match these filters.\n\n");
	printf("\x1b[90mSD:\x1b[0m %lu (%s)  \x1b[90mNAND:\x1b[0m %lu (%s)\n\n",
		sd_count, media_inclusion_status(include_sd),
		nand_count, media_inclusion_status(include_nand));
	printf("Try \x1b[90mSELECT\x1b[0m Options");
	if (suggest_sources)
		printf(" and enable SD or NAND sources");
	printf(".\n\n");
	printf("\x1b[90mSTART\x1b[0m Exit\n");
}
