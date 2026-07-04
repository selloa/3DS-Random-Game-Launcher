// SPDX-License-Identifier: MIT
// Copyright (c) 2025 selloa

#include "title_picker.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "title_database.h"
#include "title_smdh.h"

bool title_picker_is_eligible(u64 titleId, const title_filter_options_t *filters, bool include_homebrew)
{
	u16 category;
	bool inCatalog;

	if (filters == NULL)
		return false;

	category = title_meta_decode_category(titleId);
	if (!title_meta_passes_filters(category, filters))
		return false;

	if (category == 0x0000) {
		bool isVc = title_database_is_virtual_console(titleId);

		if (isVc && !filters->include_virtual_console)
			return false;
		if (!isVc && !filters->include_native_apps)
			return false;
	}

	inCatalog = title_database_contains(titleId);
	if (include_homebrew) {
		if (inCatalog)
			return false;
	} else if (!inCatalog) {
		return false;
	}

	return true;
}

void title_picker_rebuild_pool(title_picker_pool_t *pool, const title_source_t *titles, u32 titleCount,
	const title_filter_options_t *filters, bool include_homebrew)
{
	u32 i;

	if (pool == NULL)
		return;

	pool->count = 0;

	if (titles == NULL || titleCount == 0 || filters == NULL)
		return;

	for (i = 0; i < titleCount && pool->count < TITLE_PICKER_POOL_MAX; i++) {
		if (title_picker_is_eligible(titles[i].titleId, filters, include_homebrew))
			pool->indices[pool->count++] = i;
	}
}

bool title_picker_pick_random(const title_picker_pool_t *pool, const title_source_t *titles, u32 titleCount,
	u64 *outTitleId, FS_MediaType *outMedia, u32 *outPoolIndex)
{
	u32 slot;
	u32 idx;

	if (pool == NULL || titles == NULL || pool->count == 0)
		return false;

	slot = (u32)(rand() % pool->count);
	idx = pool->indices[slot];
	if (idx >= titleCount)
		return false;

	if (outTitleId != NULL)
		*outTitleId = titles[idx].titleId;
	if (outMedia != NULL)
		*outMedia = titles[idx].media;
	if (outPoolIndex != NULL)
		*outPoolIndex = slot;

	return true;
}

void title_picker_resolve_display_name(u64 titleId, const title_smdh_info_t *smdh, const char *catalogName,
	bool prefer_long_name, char *out, size_t outSize, title_name_source_t *outSource)
{
	const char *primary;
	const char *fallback;

	if (out == NULL || outSize == 0)
		return;

	out[0] = '\0';
	if (outSource != NULL)
		*outSource = TITLE_NAME_SOURCE_TITLE_ID;

	if (smdh != NULL && smdh->result == TITLE_SMDH_OK) {
		if (prefer_long_name) {
			primary = smdh->long_name;
			fallback = smdh->short_name;
		} else {
			primary = smdh->short_name;
			fallback = smdh->long_name;
		}

		if (primary[0] != '\0') {
			strncpy(out, primary, outSize - 1);
			out[outSize - 1] = '\0';
			if (outSource != NULL)
				*outSource = TITLE_NAME_SOURCE_SMDH;
			title_text_sanitize_utf8_for_console(out, outSize);
			return;
		}

		if (fallback[0] != '\0') {
			strncpy(out, fallback, outSize - 1);
			out[outSize - 1] = '\0';
			if (outSource != NULL)
				*outSource = TITLE_NAME_SOURCE_SMDH;
			title_text_sanitize_utf8_for_console(out, outSize);
			return;
		}
	}

	if (catalogName != NULL && catalogName[0] != '\0') {
		strncpy(out, catalogName, outSize - 1);
		out[outSize - 1] = '\0';
		if (outSource != NULL)
			*outSource = TITLE_NAME_SOURCE_CATALOG;
		title_text_sanitize_utf8_for_console(out, outSize);
		return;
	}

	snprintf(out, outSize, "%016llx", titleId);
	if (outSource != NULL)
		*outSource = TITLE_NAME_SOURCE_TITLE_ID;

	title_text_sanitize_utf8_for_console(out, outSize);
}

void title_picker_load_pick(u64 titleId, FS_MediaType media, bool include_homebrew, bool prefer_long_name,
	title_pick_t *pick)
{
	if (pick == NULL)
		return;

	memset(pick, 0, sizeof(*pick));
	pick->titleId = titleId;
	pick->media = media;
	pick->catalog_name = lookup_game_name(titleId);
	pick->is_homebrew = !title_database_contains(titleId);

	title_smdh_load(titleId, media, &pick->smdh);
	title_meta_load(titleId, media, &pick->meta);

	title_picker_resolve_display_name(titleId, &pick->smdh, pick->catalog_name, prefer_long_name,
		pick->display_name, sizeof(pick->display_name), &pick->name_source);
}

bool title_picker_unlisted_needs_reroll(bool unlisted_only, const title_pick_t *pick)
{
	if (!unlisted_only || pick == NULL)
		return false;

	return pick->name_source != TITLE_NAME_SOURCE_SMDH;
}
