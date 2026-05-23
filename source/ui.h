#ifndef UI_H
#define UI_H

#include <3ds.h>

#include "title_meta.h"
#include "title_picker.h"

#define UI_PAGE_COUNT 3

typedef struct {
	const title_pick_t *pick;
	u32 page;
	u32 sd_title_count;
	u32 nand_title_count;
	u32 active_title_count;
	u32 eligible_title_count;
	bool include_sd;
	bool include_nand;
	bool include_homebrew;
	const title_filter_options_t *filters;
} ui_view_t;

typedef bool (*ui_filter_row_enabled_fn)(u32 row);
typedef const char *(*ui_filter_row_label_fn)(u32 row);
typedef bool (*ui_filter_row_is_action_fn)(u32 row);

void ui_draw_header(void);
void ui_draw_main_screen(const ui_view_t *view);

void ui_draw_filter_menu(u32 cursor, u32 row_count, u32 filters_on, u32 eligible_count,
	ui_filter_row_enabled_fn row_enabled, ui_filter_row_label_fn row_label,
	ui_filter_row_is_action_fn row_is_action);

u32 ui_count_enabled_filters(u32 row_count, ui_filter_row_enabled_fn row_enabled,
	ui_filter_row_is_action_fn row_is_action);

void ui_draw_empty_pool_message(u32 sd_count, u32 nand_count, bool include_sd, bool include_nand,
	bool suggest_sources);

#endif
