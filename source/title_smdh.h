#ifndef TITLE_SMDH_H
#define TITLE_SMDH_H

#include <3ds.h>
#include <stddef.h>

#define TITLE_SMDH_SHORT_NAME_UTF8_MAX 129

typedef enum {
	TITLE_SMDH_OK = 0,
	TITLE_SMDH_ERR_OPEN,
	TITLE_SMDH_ERR_READ,
	TITLE_SMDH_ERR_FORMAT,
	TITLE_SMDH_ERR_EMPTY,
} title_smdh_result_t;

/// Read the installed title's SMDH short description (Home Menu name).
title_smdh_result_t title_smdh_get_short_name(u64 titleId, FS_MediaType media, char *out, size_t outSize);

/// FS/AM result from the most recent title_smdh_get_short_name() call (0 on success).
Result title_smdh_get_last_result(void);

#endif
