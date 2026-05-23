#ifndef TITLE_SMDH_H
#define TITLE_SMDH_H

#include <3ds.h>
#include <stddef.h>

/* Short desc is 64 UTF-16 code units; UTF-8 can exceed 128 bytes (JP/CJK titles). */
#define TITLE_SMDH_SHORT_NAME_UTF8_MAX 192
/* Long desc is 128 UTF-16 code units. */
#define TITLE_SMDH_LONG_NAME_UTF8_MAX 384
#define TITLE_SMDH_PUBLISHER_UTF8_MAX 192
#define TITLE_SMDH_LINE_UTF8_MAX 256

typedef enum {
	TITLE_SMDH_OK = 0,
	TITLE_SMDH_ERR_OPEN,
	TITLE_SMDH_ERR_READ,
	TITLE_SMDH_ERR_FORMAT,
	TITLE_SMDH_ERR_EMPTY,
} title_smdh_result_t;

typedef struct {
	title_smdh_result_t result;
	char short_name[TITLE_SMDH_SHORT_NAME_UTF8_MAX];
	char long_name[TITLE_SMDH_LONG_NAME_UTF8_MAX];
	char publisher[TITLE_SMDH_PUBLISHER_UTF8_MAX];
	u8 ratings[16];
	u32 region_lock;
	u32 flags;
	u8 eula_minor;
	u8 eula_major;
	u32 cec_id;
} title_smdh_info_t;

/// Read installed title SMDH text fields and application settings (not icon pixels).
title_smdh_result_t title_smdh_load(u64 titleId, FS_MediaType media, title_smdh_info_t *info);

/// Read only the short name (wraps title_smdh_load).
title_smdh_result_t title_smdh_get_short_name(u64 titleId, FS_MediaType media, char *out, size_t outSize);

/// FS/AM result from the most recent title_smdh_load/get_short_name call (0 on success).
Result title_smdh_get_last_result(void);

/// Format helpers for console display (output may be truncated to fit buf).
void title_smdh_format_ratings(const title_smdh_info_t *info, char *buf, size_t bufSize);
void title_smdh_format_region_lock(u32 regionLock, char *buf, size_t bufSize);
void title_smdh_format_flags(u32 flags, char *buf, size_t bufSize);

#endif
