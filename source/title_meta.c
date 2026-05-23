#include "title_meta.h"

#include <stdio.h>
#include <string.h>

void title_meta_load(u64 titleId, FS_MediaType media, title_meta_t *meta)
{
	u32 titleHigh;
	u32 titleLow;
	AM_TitleEntry entry;

	memset(meta, 0, sizeof(*meta));

	titleHigh = (u32)(titleId >> 32);
	titleLow = (u32)titleId;

	meta->platform = (u16)(titleHigh >> 16);
	meta->content_category = (u16)(titleHigh & 0xFFFF);
	meta->variation = (u8)(titleLow & 0xFF);
	meta->unique_id = (titleLow >> 8) & 0xFFFFFF;
	meta->new3ds_only = (titleLow & 0xF0000000) == 0x20000000;

	meta->title_info_result = AM_GetTitleInfo(media, 1, &titleId, &entry);
	if (R_SUCCEEDED(meta->title_info_result)) {
		meta->installed_size = entry.size;
		meta->version_major = (u8)((entry.version >> 8) & 0xFF);
		meta->version_minor = (u8)(entry.version & 0xFF);
	}

	meta->product_code_result = AM_GetTitleProductCode(media, titleId, meta->product_code);
	if (R_FAILED(meta->product_code_result))
		meta->product_code[0] = '\0';

	meta->extdata_result = AM_GetTitleExtDataId(&meta->extdata_id, media, titleId);
	meta->has_extdata = R_SUCCEEDED(meta->extdata_result) && meta->extdata_id != 0;
}

const char *title_meta_category_name(u16 contentCategory)
{
	switch (contentCategory) {
	case 0x0000: return "Application";
	case 0x0001: return "Download Play child";
	case 0x0002: return "Demo";
	case 0x0003: return "Content pack";
	case 0x0004: return "DSiWare app";
	case 0x0005: return "System application";
	case 0x0006: return "System applet";
	case 0x0007: return "Instruction manual";
	case 0x0008: return "System data";
	case 0x0009: return "System tool";
	case 0x000A: return "Download Play parent";
	case 0x000B: return "DSiWare patch";
	case 0x000C: return "DSiWare demo";
	case 0x000D: return "DSiWare content";
	case 0x000E: return "Patch / update";
	case 0x000F: return "Certificate store";
	case 0x008C: return "DLC";
	default: return "Unknown";
	}
}

const char *title_meta_platform_name(u16 platform)
{
	switch (platform) {
	case 0x0001: return "Wii";
	case 0x0002: return "DSi";
	case 0x0003: return "DSiWare";
	case 0x0004: return "3DS";
	case 0x0005: return "Wii U";
	default: return "Unknown";
	}
}

void title_meta_format_size(u64 bytes, char *buf, size_t bufSize)
{
	if (buf == NULL || bufSize == 0)
		return;

	if (bytes >= 1024ULL * 1024ULL)
		snprintf(buf, bufSize, "%.1f MB", (double)bytes / (1024.0 * 1024.0));
	else if (bytes >= 1024ULL)
		snprintf(buf, bufSize, "%.1f KB", (double)bytes / 1024.0);
	else
		snprintf(buf, bufSize, "%llu B", (unsigned long long)bytes);
}

void title_meta_format_version(u8 major, u8 minor, char *buf, size_t bufSize)
{
	if (buf == NULL || bufSize == 0)
		return;

	snprintf(buf, bufSize, "v%u.%u", major, minor);
}
