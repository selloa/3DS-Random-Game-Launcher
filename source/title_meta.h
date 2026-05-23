#ifndef TITLE_META_H
#define TITLE_META_H

#include <3ds.h>

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

#endif
