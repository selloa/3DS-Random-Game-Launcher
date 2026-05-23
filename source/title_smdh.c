#include "title_smdh.h"

#include <3ds/util/utf.h>
#include <stdlib.h>
#include <string.h>

/* ExeFS icon is always 0x36C0; FSPXI rejects other read sizes (0xE0C046F9). */
#define TITLE_ICON_SIZE 0x36C0

typedef struct {
	u16 short_desc[0x40];
	u16 long_desc[0x80];
	u16 publisher[0x40];
} SMDH_ApplicationTitle;

typedef struct {
	u32 magic;
	u16 version;
	u16 reserved;
	SMDH_ApplicationTitle titles[16];
} SMDH_Header;

static Result g_lastSmdhResult = 0;

Result title_smdh_get_last_result(void)
{
	return g_lastSmdhResult;
}

static u8 get_smdh_language_index(void)
{
	u8 lang = 1;

	if (R_SUCCEEDED(cfguInit())) {
		CFGU_GetSystemLanguage(&lang);
		cfguExit();
	}

	if (lang > 11)
		lang = 1;

	return lang;
}

static void append_utf16_ascii(u16 *out, size_t *pos, size_t max, const char *ascii)
{
	while (*ascii && *pos + 1 < max) {
		out[(*pos)++] = (u16)(unsigned char)*ascii;
		ascii++;
	}
}

/* Match scripts/title_db_common.py — symbols that render badly on the 3DS console. */
static void sanitize_utf16_for_console(const u16 *in, u16 *out, size_t outMax)
{
	size_t pos = 0;
	size_t i;

	for (i = 0; i < 0x40 && in[i] != 0; i++) {
		u16 c = in[i];

		switch (c) {
		case 0x2122: /* ™ */
			append_utf16_ascii(out, &pos, outMax, "(TM)");
			break;
		case 0x00AE: /* ® */
			append_utf16_ascii(out, &pos, outMax, "(R)");
			break;
		case 0x00A9: /* © */
			append_utf16_ascii(out, &pos, outMax, "(C)");
			break;
		case 0x2013:
		case 0x2014:
			if (pos + 1 < outMax)
				out[pos++] = '-';
			break;
		case 0x2018:
		case 0x2019:
			if (pos + 1 < outMax)
				out[pos++] = '\'';
			break;
		case 0x201C:
		case 0x201D:
			if (pos + 1 < outMax)
				out[pos++] = '"';
			break;
		case 0x2026:
			append_utf16_ascii(out, &pos, outMax, "...");
			break;
		default:
			if (pos + 1 < outMax)
				out[pos++] = c;
			break;
		}
	}

	out[pos] = 0;
}

static void strip_truncated_utf8(char *out, size_t written, size_t maxLen)
{
	size_t len = written;

	if (len >= maxLen)
		len = maxLen - 1;

	while (len > 0 && ((unsigned char)out[len - 1] & 0xC0) == 0x80)
		len--;

	if (len > 0) {
		unsigned char b = (unsigned char)out[len - 1];

		if ((b & 0xE0) == 0xE0 && len + 2 > written)
			len--;
		else if ((b & 0xF0) == 0xF0 && len + 3 > written)
			len--;
	}

	out[len] = '\0';
}

static Result read_icon_into_buffer(Handle handle, u8 *iconData)
{
	u32 bytesRead = 0;
	Result res;

	res = FSFILE_Read(handle, &bytesRead, 0, iconData, TITLE_ICON_SIZE);
	if (R_FAILED(res))
		return res;

	if (bytesRead < sizeof(SMDH_Header))
		return -1;

	return 0;
}

static Result read_smdh_via_open_file(FS_Archive archive, SMDH_Header *out)
{
	Handle handle = 0;
	u32 filePath[] = { 0, 0, 2, 0x6E6F6369, 0 };
	u8 *iconData = NULL;
	Result res;

	iconData = (u8 *)malloc(TITLE_ICON_SIZE);
	if (iconData == NULL)
		return -1;

	res = FSUSER_OpenFile(
		&handle,
		archive,
		(FS_Path){ PATH_BINARY, sizeof(filePath), filePath },
		FS_OPEN_READ,
		0);
	if (R_FAILED(res))
		goto cleanup;

	res = read_icon_into_buffer(handle, iconData);
	FSFILE_Close(handle);
	if (R_FAILED(res))
		goto cleanup;

	memcpy(out, iconData, sizeof(SMDH_Header));
	res = 0;

cleanup:
	free(iconData);
	return res;
}

static Result read_smdh_via_open_file_directly(u64 titleId, FS_MediaType media, SMDH_Header *out)
{
	Handle handle = 0;
	u32 low = (u32)titleId;
	u32 high = (u32)(titleId >> 32);
	u32 archPath[] = { low, high, (u32)media, 0 };
	u32 filePath[] = { 0, 0, 2, 0x6E6F6369, 0 };
	u8 *iconData = NULL;
	Result res;

	iconData = (u8 *)malloc(TITLE_ICON_SIZE);
	if (iconData == NULL)
		return -1;

	res = FSUSER_OpenFileDirectly(
		&handle,
		ARCHIVE_SAVEDATA_AND_CONTENT,
		(FS_Path){ PATH_BINARY, sizeof(archPath), archPath },
		(FS_Path){ PATH_BINARY, sizeof(filePath), filePath },
		FS_OPEN_READ,
		0);
	if (R_FAILED(res))
		goto cleanup;

	res = read_icon_into_buffer(handle, iconData);
	FSFILE_Close(handle);
	if (R_FAILED(res))
		goto cleanup;

	memcpy(out, iconData, sizeof(SMDH_Header));
	res = 0;

cleanup:
	free(iconData);
	return res;
}

static Result read_smdh_via_open_archive(u64 titleId, FS_MediaType media, SMDH_Header *out)
{
	FS_Archive archive;
	u32 low = (u32)titleId;
	u32 high = (u32)(titleId >> 32);
	u32 archPath[] = { low, high, (u32)media, 0 };
	Result res;

	res = FSUSER_OpenArchive(
		&archive,
		ARCHIVE_SAVEDATA_AND_CONTENT,
		(FS_Path){ PATH_BINARY, sizeof(archPath), archPath });
	if (R_FAILED(res))
		return res;

	res = read_smdh_via_open_file(archive, out);
	FSUSER_CloseArchive(archive);
	return res;
}

static Result read_smdh_header(u64 titleId, FS_MediaType media, SMDH_Header *out)
{
	Result res;

	res = read_smdh_via_open_file_directly(titleId, media, out);
	if (R_SUCCEEDED(res))
		return 0;

	if (R_SUCCEEDED(read_smdh_via_open_archive(titleId, media, out)))
		return 0;

	return res;
}

title_smdh_result_t title_smdh_get_short_name(u64 titleId, FS_MediaType media, char *out, size_t outSize)
{
	SMDH_Header smdh;
	u8 lang;
	const u16 *name;
	Result res;

	g_lastSmdhResult = 0;

	if (out == NULL || outSize == 0)
		return TITLE_SMDH_ERR_FORMAT;

	out[0] = '\0';

	res = read_smdh_header(titleId, media, &smdh);
	g_lastSmdhResult = res;
	if (R_FAILED(res))
		return TITLE_SMDH_ERR_OPEN;

	if (memcmp(&smdh.magic, "SMDH", 4) != 0)
		return TITLE_SMDH_ERR_FORMAT;

	lang = get_smdh_language_index();
	name = smdh.titles[lang].short_desc;
	if (name[0] == 0)
		name = smdh.titles[1].short_desc;

	if (name[0] == 0)
		return TITLE_SMDH_ERR_EMPTY;

	{
		u16 sanitized[0x80];
		ssize_t bytes;
		size_t outMax = outSize - 1;

		if (outMax == 0)
			return TITLE_SMDH_ERR_FORMAT;

		sanitize_utf16_for_console(name, sanitized, sizeof(sanitized) / sizeof(sanitized[0]));
		bytes = utf16_to_utf8((uint8_t *)out, sanitized, outMax);
		if (bytes <= 0)
			return TITLE_SMDH_ERR_EMPTY;

		if ((size_t)bytes > outMax)
			strip_truncated_utf8(out, outMax, outSize);
		else
			out[bytes] = '\0';
	}

	return TITLE_SMDH_OK;
}
