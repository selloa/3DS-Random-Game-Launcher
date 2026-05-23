#ifndef TITLE_PICKER_H
#define TITLE_PICKER_H

#include <3ds.h>
#include <stdbool.h>
#include <stddef.h>

#include "title_meta.h"
#include "title_smdh.h"

#define TITLE_PICKER_POOL_MAX 1800
#define TITLE_SCAN_MAX 900

typedef struct {
	u64 titleId;
	FS_MediaType media;
} title_source_t;

typedef enum {
	TITLE_NAME_SOURCE_SMDH = 0,
	TITLE_NAME_SOURCE_CATALOG,
	TITLE_NAME_SOURCE_TITLE_ID,
} title_name_source_t;

typedef struct {
	u32 indices[TITLE_PICKER_POOL_MAX];
	u32 count;
} title_picker_pool_t;

typedef struct {
	u64 titleId;
	FS_MediaType media;
	char display_name[TITLE_SMDH_SHORT_NAME_UTF8_MAX];
	title_name_source_t name_source;
	const char *catalog_name;
	bool is_homebrew;
	title_smdh_info_t smdh;
	title_meta_t meta;
} title_pick_t;

bool title_picker_is_eligible(u64 titleId, const title_filter_options_t *filters, bool include_homebrew);

void title_picker_rebuild_pool(title_picker_pool_t *pool, const title_source_t *titles, u32 titleCount,
	const title_filter_options_t *filters, bool include_homebrew);

bool title_picker_pick_random(const title_picker_pool_t *pool, const title_source_t *titles, u32 titleCount,
	u64 *outTitleId, FS_MediaType *outMedia, u32 *outPoolIndex);

void title_picker_load_pick(u64 titleId, FS_MediaType media, bool include_homebrew, title_pick_t *pick);

void title_picker_resolve_display_name(u64 titleId, const title_smdh_info_t *smdh, const char *catalogName,
	char *out, size_t outSize, title_name_source_t *outSource);

#endif
