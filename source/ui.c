// SPDX-License-Identifier: MIT
// Copyright (c) 2025 selloa

#include "ui.h"

#include <stdio.h>
#include <string.h>

#ifndef APP_VERSION
#define APP_VERSION "0.0.0"
#endif

#define UI_CONSOLE_ROWS 30
#define UI_CONSOLE_COLS 50
#define UI_INVERTED_BAR "\x1b[47m\x1b[30m"
#define UI_BAR_MAIN "\x1b[30m"
#define UI_BAR_DIM "\x1b[90m"
#define UI_CONTROLS_ROW_FILTER_STATUS 25
#define UI_CONTROLS_ROW_FILTER_ACTIONS 26
#define UI_CONTROLS_ROW_FOOTER_GAP 27
#define UI_CONTENT_ROW_TITLE 6
#define UI_CONTENT_ROW_PUBLISHER 7
#define UI_CONTENT_ROW_GAP 9
#define UI_CONTENT_ROW_ACTIONS 10
#define UI_DETAILS_CONTENT_ROW 10
#define UI_FIELD_LABEL_COLS 14
#define UI_TECH_LABEL_COLS 18
#define UI_FILTER_LABEL_COLS 16
#define UI_FILTER_STATE_COLS 3

static void print_page_tab(u32 page, u32 active_page)
{
	if (page == active_page) {
		switch (page) {
		case 0:
			printf("\x1b[37mGAME\x1b[0m");
			break;
		case 1:
			printf("\x1b[37mDETAILS\x1b[0m");
			break;
		default:
			printf("\x1b[37mTECHNICAL\x1b[0m");
			break;
		}
	} else {
		printf("\x1b[90m");
		switch (page) {
		case 0:
			printf("game");
			break;
		case 1:
			printf("details");
			break;
		default:
			printf("technical");
			break;
		}
		printf("\x1b[0m");
	}
}

static void print_page_tabs(u32 active_page)
{
	u32 page;

	printf("\x1b[90m< \x1b[0m");
	for (page = 0; page < UI_PAGE_COUNT; page++) {
		if (page > 0)
			printf("\x1b[90m | \x1b[0m");
		print_page_tab(page, active_page);
	}
	printf("\x1b[90m >\x1b[0m\n\n");
}

static const char *media_inclusion_status(bool included)
{
	return included ? "on" : "off";
}

static void print_section_header(const char *title, bool first)
{
	if (!first)
		printf("\n");
	printf("\x1b[90m%s\x1b[0m\n", title);
}

static void print_field_inline_cols(u32 labelCols, const char *label, const char *value, bool dim_value)
{
	printf("\x1b[90m%-*s\x1b[0m", (int)labelCols, label);
	if (value == NULL || value[0] == '\0')
		printf("\x1b[90m(unavailable)\x1b[0m\n");
	else if (dim_value)
		printf("\x1b[90m%s\x1b[0m\n", value);
	else
		printf("\x1b[37m%s\x1b[0m\n", value);
}

static void print_field_inline_ex(const char *label, const char *value, bool dim_value)
{
	print_field_inline_cols(UI_FIELD_LABEL_COLS, label, value, dim_value);
}

static void print_field_inline(const char *label, const char *value)
{
	print_field_inline_ex(label, value, false);
}

static void print_tech_field_inline(const char *label, const char *value)
{
	print_field_inline_cols(UI_TECH_LABEL_COLS, label, value, false);
}

static void print_tech_field_inline_dim(const char *label, const char *value)
{
	print_field_inline_cols(UI_TECH_LABEL_COLS, label, value, true);
}

static void print_toggle_state(bool enabled)
{
	if (enabled)
		printf("\x1b[37m%-*s\x1b[0m", UI_FILTER_STATE_COLS, "ON");
	else
		printf("\x1b[90m%-*s\x1b[0m", UI_FILTER_STATE_COLS, "OFF");
}

static void print_filter_row2(const char *label1, bool state1, const char *label2, bool state2)
{
	printf("\x1b[90m%-*s\x1b[0m", UI_FILTER_LABEL_COLS, label1);
	print_toggle_state(state1);
	if (label2 != NULL && label2[0] != '\0') {
		printf("  \x1b[90m%-*s\x1b[0m", UI_FILTER_LABEL_COLS, label2);
		print_toggle_state(state2);
	}
	printf("\n");
}

static u32 wrap_segment_length(const char *text, u32 maxCols, const char **outNext)
{
	const char *lastBreak = NULL;
	u32 len = 0;
	u32 segmentLen;

	if (text == NULL || text[0] == '\0') {
		*outNext = text;
		return 0;
	}

	while (text[len] != '\0' && len < maxCols) {
		if (text[len] == ' ' || text[len] == ';' || text[len] == '\n' || text[len] == '\r')
			lastBreak = text + len;
		len++;
	}

	if (text[len] == '\0') {
		*outNext = text + len;
		return len;
	}

	if (lastBreak != NULL) {
		if (*lastBreak == ';')
			segmentLen = (u32)(lastBreak - text) + 1;
		else
			segmentLen = (u32)(lastBreak - text);
		*outNext = text + segmentLen;
		while (**outNext == ' ' || **outNext == '\n' || **outNext == '\r')
			(*outNext)++;
		return segmentLen;
	}

	segmentLen = maxCols;
	while (segmentLen > 0 && ((const unsigned char)text[segmentLen] & 0xC0) == 0x80)
		segmentLen--;
	if (segmentLen == 0)
		segmentLen = maxCols;

	*outNext = text + segmentLen;
	return segmentLen;
}

static void collapse_title_newlines(const char *display_name, char *out, size_t outSize)
{
	size_t i = 0;
	size_t j = 0;

	if (out == NULL || outSize == 0)
		return;

	out[0] = '\0';
	if (display_name == NULL)
		return;

	while (display_name[i] != '\0' && j + 1 < outSize) {
		if (display_name[i] == '\n' || display_name[i] == '\r')
			out[j++] = ' ';
		else
			out[j++] = display_name[i];
		i++;
	}
	out[j] = '\0';
}

static void print_value_column_indent(u32 cols)
{
	u32 i;

	for (i = 0; i < cols; i++)
		putchar(' ');
}

static void print_field_inline_wrap(const char *label, const char *value)
{
	char flattened[TITLE_SMDH_LONG_NAME_UTF8_MAX];
	const char *cursor;
	u32 valueCols;
	bool first = true;

	if (value == NULL || value[0] == '\0') {
		printf("\x1b[90m%-*s\x1b[0m", (int)UI_FIELD_LABEL_COLS, label);
		printf("\x1b[90m(unavailable)\x1b[0m\n");
		return;
	}

	collapse_title_newlines(value, flattened, sizeof(flattened));
	valueCols = UI_CONSOLE_COLS - UI_FIELD_LABEL_COLS;
	cursor = flattened;
	while (cursor[0] != '\0') {
		u32 take;
		const char *next;

		take = wrap_segment_length(cursor, valueCols, &next);
		if (first) {
			printf("\x1b[90m%-*s\x1b[0m", (int)UI_FIELD_LABEL_COLS, label);
			printf("\x1b[37m%.*s\x1b[0m\n", (int)take, cursor);
			first = false;
		} else {
			print_value_column_indent(UI_FIELD_LABEL_COLS);
			printf("\x1b[37m%.*s\x1b[0m\n", (int)take, cursor);
		}
		cursor = next;
	}
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
	char status[48];

	if (smdh->result == TITLE_SMDH_OK)
		return;

	switch (smdh->result) {
	case TITLE_SMDH_ERR_EMPTY:
		snprintf(status, sizeof(status), "empty");
		break;
	case TITLE_SMDH_ERR_FORMAT:
		snprintf(status, sizeof(status), "bad format");
		break;
	default:
		if (R_FAILED(title_smdh_get_last_result()))
			snprintf(status, sizeof(status), "unavailable %08lX",
				(unsigned long)title_smdh_get_last_result());
		else
			snprintf(status, sizeof(status), "unavailable");
		break;
	}

	print_field_inline("SMDH read", status);
}

static void print_user_page(const title_pick_t *pick)
{
	char title[TITLE_SMDH_LONG_NAME_UTF8_MAX];

	collapse_title_newlines(pick->display_name, title, sizeof(title));

	printf("\x1b[%d;1H\x1b[K", UI_CONTENT_ROW_TITLE);
	printf("\x1b[37m%s\x1b[0m", title);

	printf("\x1b[%d;1H\x1b[K", UI_CONTENT_ROW_PUBLISHER);
	if (pick->smdh.publisher[0] != '\0')
		printf("\x1b[90m%s\x1b[0m", pick->smdh.publisher);

	printf("\x1b[%d;1H\x1b[K", UI_CONTENT_ROW_GAP);

	printf("\x1b[%d;1H\x1b[K", UI_CONTENT_ROW_ACTIONS);
	printf("\x1b[37mA\x1b[0m Launch   \x1b[37mY\x1b[0m Reroll");

	printf("\x1b[11;1H\x1b[K");
	printf("\x1b[12;1H\x1b[K");
}

static void print_dev_details_page(const ui_view_t *view)
{
	const title_pick_t *pick = view->pick;
	char ratings[TITLE_SMDH_LINE_UTF8_MAX];
	char region[TITLE_SMDH_LINE_UTF8_MAX];
	char flags[TITLE_SMDH_LINE_UTF8_MAX];
	char eula[32];
	char streetpass[16];

	title_smdh_format_ratings(&pick->smdh, ratings, sizeof(ratings));
	title_smdh_format_region_lock(pick->smdh.region_lock, region, sizeof(region));
	title_smdh_format_flags(pick->smdh.flags, flags, sizeof(flags));
	snprintf(eula, sizeof(eula), "%u.%u", pick->smdh.eula_major, pick->smdh.eula_minor);
	snprintf(streetpass, sizeof(streetpass), "%08lX", pick->smdh.cec_id);

	printf("\x1b[%d;1H", UI_DETAILS_CONTENT_ROW);
	print_section_header("Names", true);
	print_field_inline("Source", name_source_label(pick->name_source));
	if (pick->name_source == TITLE_NAME_SOURCE_CATALOG && pick->catalog_name != NULL)
		print_field_inline("Catalog", pick->catalog_name);
	print_field_inline("Short", pick->smdh.short_name);
	print_field_inline_wrap("Long", pick->smdh.long_name);
	print_field_inline("Publisher", pick->smdh.publisher);
	print_smdh_status(&pick->smdh);

	print_section_header("Restrictions", false);
	print_field_inline_wrap("Ratings", ratings);
	print_field_inline("Region", region);
	print_field_inline_wrap("Flags", flags);

	print_section_header("Other", false);
	print_field_inline("EULA", eula);
	print_field_inline("StreetPass ID", streetpass);
}

static void print_dev_technical_page(const ui_view_t *view)
{
	const title_pick_t *pick = view->pick;
	const title_filter_options_t *filters = view->filters;
	char titleId[32];
	char platformCode[16];
	char categoryCode[16];
	char uniqueId[16];
	char variation[8];
	char new3ds[8];
	char size[32];
	char version[16];
	char extdata[32];
	char status[48];

	snprintf(titleId, sizeof(titleId), "%016llx", pick->titleId);
	snprintf(platformCode, sizeof(platformCode), "0x%04X", pick->meta.platform);
	snprintf(categoryCode, sizeof(categoryCode), "0x%04X", pick->meta.content_category);
	snprintf(uniqueId, sizeof(uniqueId), "0x%06lX", pick->meta.unique_id);
	snprintf(variation, sizeof(variation), "0x%02X", pick->meta.variation);
	snprintf(new3ds, sizeof(new3ds), "%s", pick->meta.new3ds_only ? "Yes" : "No");

	print_tech_field_inline("Title ID", titleId);
	print_tech_field_inline("Platform", title_meta_platform_name(pick->meta.platform));
	print_tech_field_inline("Platform code", platformCode);
	print_tech_field_inline("Category", title_meta_category_name(pick->meta.content_category));
	print_tech_field_inline("Category code", categoryCode);
	print_tech_field_inline("Unique ID", uniqueId);
	print_tech_field_inline("Variation", variation);
	print_tech_field_inline("New 3DS only", new3ds);

	if (pick->meta.product_code[0] != '\0')
		print_tech_field_inline("Product code", pick->meta.product_code);
	else if (R_FAILED(pick->meta.product_code_result)) {
		snprintf(status, sizeof(status), "unavailable %08lX",
			(unsigned long)pick->meta.product_code_result);
		print_tech_field_inline_dim("Product code", status);
	}

	printf("\n");

	if (R_SUCCEEDED(pick->meta.title_info_result)) {
		title_meta_format_version(pick->meta.version_major, pick->meta.version_minor, version, sizeof(version));
		title_meta_format_size(pick->meta.installed_size, size, sizeof(size));
		print_tech_field_inline("Version", version);
		print_tech_field_inline("Installed size", size);
	} else {
		snprintf(status, sizeof(status), "unavailable %08lX",
			(unsigned long)pick->meta.title_info_result);
		print_tech_field_inline_dim("Title info", status);
	}

	if (pick->meta.has_extdata) {
		snprintf(extdata, sizeof(extdata), "%011llX", pick->meta.extdata_id);
		print_tech_field_inline("Extdata ID", extdata);
	} else if (R_FAILED(pick->meta.extdata_result)) {
		snprintf(status, sizeof(status), "unavailable %08lX",
			(unsigned long)pick->meta.extdata_result);
		print_tech_field_inline_dim("Extdata ID", status);
	} else {
		print_tech_field_inline("Extdata ID", NULL);
	}

	print_tech_field_inline("Media", pick->media == MEDIATYPE_NAND ? "NAND" : "SD");

	printf("\n");
	printf("\x1b[90mSD:\x1b[0m %lu (%s)  \x1b[90mNAND:\x1b[0m %lu (%s)\n",
		view->sd_title_count, media_inclusion_status(view->include_sd),
		view->nand_title_count, media_inclusion_status(view->include_nand));
	printf("\x1b[90mScanned:\x1b[0m %lu  \x1b[90mEligible:\x1b[0m %lu\n",
		view->active_title_count, view->eligible_title_count);

	if (filters != NULL) {
		printf("\n");
		print_filter_row2("Native apps", filters->include_native_apps,
			"Virtual Console", filters->include_virtual_console);
		print_filter_row2("DSiWare", filters->include_dsiware,
			"Demos", filters->include_demos);
		print_filter_row2("Content", filters->include_content_packs,
			"DLC", filters->include_dlc);
		print_filter_row2("Patches", filters->include_patches,
			"System", filters->include_system);
		print_filter_row2("SD titles", view->include_sd,
			"NAND titles", view->include_nand);
		print_filter_row2("Unlisted only", view->include_homebrew, NULL, false);
	}
}

static void ui_draw_nav_footer(const char *nav_label)
{
	u32 middleLen;
	u32 middleCol;
	u32 versionLen;
	u32 versionCol;

	middleLen = 4 + (u32)strlen(nav_label);
	middleCol = (UI_CONSOLE_COLS - middleLen) / 2 + 1;
	versionLen = 1 + (u32)strlen(APP_VERSION);
	versionCol = UI_CONSOLE_COLS - versionLen + 1;

	printf("\x1b[%d;1H" UI_INVERTED_BAR "\x1b[K", UI_CONSOLE_ROWS);
	printf(UI_BAR_MAIN "START" UI_BAR_DIM " Exit");
	printf("\x1b[%d;%dH" UI_INVERTED_BAR "\x1b[K", UI_CONSOLE_ROWS, (int)middleCol);
	printf(UI_BAR_MAIN "L/R" UI_BAR_DIM " %s", nav_label);
	printf("\x1b[%d;%dH" UI_INVERTED_BAR UI_BAR_DIM "v%s\x1b[0m", UI_CONSOLE_ROWS, (int)versionCol,
		APP_VERSION);
}

static void print_user_controls(u32 eligible_title_count, u32 active_title_count, bool homebrew_only)
{
	printf("\x1b[%d;1H\x1b[K", UI_CONTROLS_ROW_FILTER_STATUS);
	printf("\x1b[37m%lu\x1b[0m \x1b[90mof\x1b[0m \x1b[37m%lu\x1b[0m \x1b[90mtitles filtered\x1b[0m",
		eligible_title_count, active_title_count);

	printf("\x1b[%d;1H\x1b[K", UI_CONTROLS_ROW_FILTER_ACTIONS);
	printf("\x1b[37mSELECT\x1b[90m filters   \x1b[37mX\x1b[0m ");
	if (homebrew_only)
		printf("\x1b[37mhomebrew only *\x1b[0m");
	else
		printf("\x1b[90mhomebrew only\x1b[0m");

	printf("\x1b[%d;1H\x1b[K", UI_CONTROLS_ROW_FOOTER_GAP);

	printf("\x1b[28;1H\x1b[K");
	printf("\x1b[29;1H\x1b[K");

	ui_draw_nav_footer("Details");
}

void ui_draw_header(void)
{
	const char *byline = "by selloa";
	u32 bylineCol = UI_CONSOLE_COLS - (u32)strlen(byline) + 1;

	consoleClear();
	printf("\x1b[1;1H" UI_INVERTED_BAR "\x1b[K");
	printf(UI_BAR_MAIN "RANDOM GAME LAUNCHER (2026)");
	printf("\x1b[1;%dH" UI_INVERTED_BAR UI_BAR_DIM "%s\x1b[0m\n", (int)bylineCol, byline);
}

void ui_draw_main_screen(const ui_view_t *view)
{
	if (view == NULL || view->pick == NULL)
		return;

	ui_draw_header();
	print_page_tabs(view->page);

	switch (view->page) {
	case 0:
		print_user_page(view->pick);
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
		print_user_controls(view->eligible_title_count, view->active_title_count,
			view->include_homebrew);
	else
		ui_draw_nav_footer("Change page");
}

void ui_draw_filter_menu(u32 cursor, u32 row_count, u32 eligible_count,
	ui_filter_row_enabled_fn row_enabled, ui_filter_row_label_fn row_label,
	ui_filter_row_is_action_fn row_is_action)
{
	u32 row;

	consoleClear();
	printf("\n\x1b[37mOptions\x1b[0m\n");
	printf("\x1b[90mPool:\x1b[0m \x1b[37m%lu pickable titles\x1b[0m\n\n", eligible_count);

	for (row = 0; row < row_count; row++) {
		if (row == 0)
			printf("\x1b[90mContent types\x1b[0m\n");
		else if (row == 8)
			printf("\n\x1b[90mSources\x1b[0m\n");
		else if (row == 10)
			printf("\n\x1b[90mOther\x1b[0m\n");

		if (row == cursor)
			printf("\x1b[37m> ");
		else
			printf("  ");

		if (row_is_action != NULL && row_is_action(row)) {
			printf("%s\x1b[0m\n", row_label(row));
		} else {
			printf("%-16s %s\x1b[0m\n", row_label(row),
				row_enabled(row) ? "\x1b[37mON" : "\x1b[90mOFF");
		}
	}

	printf("\n\x1b[90mUp/Down  A toggle/apply  B/SELECT close\x1b[0m\n");
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
	printf("\x1b[90mSELECT\x1b[0m Options   \x1b[90mSTART\x1b[0m Exit\n");
}
