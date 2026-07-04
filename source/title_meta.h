// SPDX-License-Identifier: MIT
// Copyright (c) 2025 selloa

#ifndef TITLE_META_H
#define TITLE_META_H

#include <3ds.h>

typedef struct {
	bool include_patches;
	bool include_dlc;
	bool include_system;
	bool include_demos;
	bool include_dsiware;
	bool include_content_packs;
	bool include_native_apps;
	bool include_virtual_console;
} title_filter_options_t;

typedef struct {
	u16 platform;
	u16 content_category;
	u8 variation;
	u32 unique_id;
	bool new3ds_only;
	char product_code[17];
	u8 version_major;
	u8 version_minor;
	u64 installed_size;
	u64 extdata_id;
	bool has_extdata;
	Result title_info_result;
	Result product_code_result;
	Result extdata_result;
} title_meta_t;

/// Fill metadata derived from title ID and AM services.
void title_meta_load(u64 titleId, FS_MediaType media, title_meta_t *meta);

const char *title_meta_category_name(u16 contentCategory);
const char *title_meta_platform_name(u16 platform);

void title_meta_format_size(u64 bytes, char *buf, size_t bufSize);
void title_meta_format_version(u8 major, u8 minor, char *buf, size_t bufSize);

u16 title_meta_decode_category(u64 titleId);

bool title_meta_is_patch(u16 category);
bool title_meta_is_dlc(u16 category);
bool title_meta_is_system(u16 category);
bool title_meta_is_demo(u16 category);
bool title_meta_is_dsiware(u16 category);
bool title_meta_is_content_pack(u16 category);

bool title_meta_passes_filters(u16 category, const title_filter_options_t *filters);

#endif
