// SPDX-License-Identifier: MIT
// Copyright (c) 2025 selloa

#include "title_smdh.h"

#include <3ds/util/utf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ExeFS icon is always 0x36C0; FSPXI rejects other read sizes (0xE0C046F9). */
#define TITLE_ICON_SIZE 0x36C0
#define SMDH_METADATA_SIZE 0x2038

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

/* Match scripts/title_db_common.py — symbols that render poorly on the 3DS console. */
static const char *console_codepoint_replacement(u32 cp)
{
	static const char *roman[] = {
		"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X",
		"XI", "XII", "XIII", "XIV", "XV", "XVI"
	};

	switch (cp) {
	case 0x2122:
		return "(TM)";
	case 0x00AE:
		return "(R)";
	case 0x00A9:
		return "(C)";
	case 0x2013:
	case 0x2014:
	case 0x00B7:
	case 0x30FB:
	case 0xFF65:
		return "-";
	case 0x2018:
	case 0x2019:
	case 0x2032:
		return "'";
	case 0x201C:
	case 0x201D:
	case 0x2033:
		return "\"";
	case 0x2026:
		return "...";
	case 0x2022:
	case 0x2605:
	case 0x2606:
	case 0x25CF:
	case 0xFF0A:
		return "*";
	case 0x00D7:
		return "x";
	case 0x00F7:
		return "/";
	case 0x25CB:
		return "O";
	case 0x266A:
	case 0x266B:
		return "*";
	case 0x2665:
	case 0x2661:
		return "*";
	case 0x00A0:
	case 0x2002:
	case 0x2003:
	case 0x2004:
	case 0x2005:
	case 0x2006:
	case 0x2007:
	case 0x2008:
	case 0x2009:
	case 0x200A:
		return " ";
	default:
		break;
	}

	if (cp >= 0x2160 && cp <= 0x216F)
		return roman[cp - 0x2160];
	if (cp >= 0x2170 && cp <= 0x217F)
		return roman[cp - 0x2170];

	return NULL;
}

static bool console_codepoint_should_drop(u32 cp)
{
	if (cp == '\n' || cp == '\r' || cp == '\t')
		return false;
	if (cp < 0x20 || cp == 0x7F)
		return true;
	if (cp == 0xFEFF || cp == 0x200B || cp == 0x200C || cp == 0x200D)
		return true;
	if (cp > 0xFFFF)
		return true;
	if (cp >= 0xE000 && cp <= 0xF8FF)
		return true;
	if (cp >= 0x2190 && cp <= 0x21FF)
		return true;
	if (cp >= 0x2200 && cp <= 0x22FF)
		return true;
	if (cp >= 0x2300 && cp <= 0x23FF)
		return true;
	if (cp >= 0x2600 && cp <= 0x26FF)
		return true;
	if (cp >= 0x2700 && cp <= 0x27BF)
		return true;
	return false;
}

static void append_utf8_text(char *out, size_t *pos, size_t max, const char *text)
{
	while (text != NULL && *text && *pos + 1 < max)
		out[(*pos)++] = *text++;
}

static void append_utf8_codepoint(char *out, size_t *pos, size_t max, u32 cp)
{
	uint8_t encoded[4];
	ssize_t bytes;
	size_t i;

	bytes = encode_utf8(encoded, cp);
	if (bytes <= 0)
		return;

	for (i = 0; i < (size_t)bytes && *pos + 1 < max; i++)
		out[(*pos)++] = (char)encoded[i];
}

static void sanitize_utf16_for_console(const u16 *in, u16 *out, size_t outMax, size_t inMaxChars)
{
	size_t pos = 0;
	size_t i;

	for (i = 0; i < inMaxChars && in[i] != 0; i++) {
		u16 c = in[i];
		const char *replacement = console_codepoint_replacement(c);

		if (replacement != NULL) {
			append_utf16_ascii(out, &pos, outMax, replacement);
			continue;
		}

		if (console_codepoint_should_drop(c))
			continue;

		if (pos + 1 < outMax)
			out[pos++] = c;
	}

	out[pos] = 0;
}

void title_text_sanitize_utf8_for_console(char *text, size_t textSize)
{
	char sanitized[TITLE_SMDH_LONG_NAME_UTF8_MAX];
	const uint8_t *in;
	size_t outPos = 0;
	size_t outMax;

	if (text == NULL || textSize == 0)
		return;

	outMax = textSize < sizeof(sanitized) ? textSize : sizeof(sanitized);
	sanitized[0] = '\0';
	in = (const uint8_t *)text;

	while (*in && outPos + 1 < outMax) {
		const char *replacement;
		uint32_t cp;
		ssize_t consumed;

		if (in[0] == '<' && (in[1] == 'b' || in[1] == 'B') && (in[2] == 'r' || in[2] == 'R') &&
		    in[3] == '>') {
			append_utf8_text(sanitized, &outPos, outMax, " ");
			in += 4;
			continue;
		}

		consumed = decode_utf8(&cp, in);
		if (consumed <= 0) {
			in++;
			continue;
		}

		replacement = console_codepoint_replacement(cp);
		if (replacement != NULL) {
			append_utf8_text(sanitized, &outPos, outMax, replacement);
		} else if (!console_codepoint_should_drop(cp)) {
			append_utf8_codepoint(sanitized, &outPos, outMax, cp);
		}

		in += (size_t)consumed;
	}

	sanitized[outPos] = '\0';
	strncpy(text, sanitized, textSize - 1);
	text[textSize - 1] = '\0';
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

static bool utf16_field_to_utf8(const u16 *in, size_t inMaxChars, char *out, size_t outSize)
{
	u16 sanitized[0x200];
	ssize_t bytes;
	size_t outMax;

	if (out == NULL || outSize == 0)
		return false;

	out[0] = '\0';
	if (in == NULL || in[0] == 0)
		return false;

	outMax = outSize - 1;
	if (outMax == 0)
		return false;

	sanitize_utf16_for_console(in, sanitized, sizeof(sanitized) / sizeof(sanitized[0]), inMaxChars);
	bytes = utf16_to_utf8((uint8_t *)out, sanitized, outMax);
	if (bytes <= 0)
		return false;

	if ((size_t)bytes > outMax)
		strip_truncated_utf8(out, outMax, outSize);
	else
		out[bytes] = '\0';

	return true;
}

static Result read_icon_into_buffer(Handle handle, u8 *iconData)
{
	u32 bytesRead = 0;
	Result res;

	res = FSFILE_Read(handle, &bytesRead, 0, iconData, TITLE_ICON_SIZE);
	if (R_FAILED(res))
		return res;

	if (bytesRead < SMDH_METADATA_SIZE)
		return -1;

	return 0;
}

static Result read_icon_file(u64 titleId, FS_MediaType media, u8 *iconData)
{
	Handle handle = 0;
	u32 low = (u32)titleId;
	u32 high = (u32)(titleId >> 32);
	u32 archPath[] = { low, high, (u32)media, 0 };
	u32 filePath[] = { 0, 0, 2, 0x6E6F6369, 0 };
	FS_Archive archive = 0;
	Result res;

	res = FSUSER_OpenFileDirectly(
		&handle,
		ARCHIVE_SAVEDATA_AND_CONTENT,
		(FS_Path){ PATH_BINARY, sizeof(archPath), archPath },
		(FS_Path){ PATH_BINARY, sizeof(filePath), filePath },
		FS_OPEN_READ,
		0);
	if (R_FAILED(res)) {
		res = FSUSER_OpenArchive(
			&archive,
			ARCHIVE_SAVEDATA_AND_CONTENT,
			(FS_Path){ PATH_BINARY, sizeof(archPath), archPath });
		if (R_FAILED(res))
			return res;

		res = FSUSER_OpenFile(
			&handle,
			archive,
			(FS_Path){ PATH_BINARY, sizeof(filePath), filePath },
			FS_OPEN_READ,
			0);
		if (R_FAILED(res)) {
			FSUSER_CloseArchive(archive);
			return res;
		}
	}

	res = read_icon_into_buffer(handle, iconData);
	FSFILE_Close(handle);
	if (archive != 0)
		FSUSER_CloseArchive(archive);

	return res;
}

typedef enum {
	SMDH_FIELD_SHORT,
	SMDH_FIELD_LONG,
	SMDH_FIELD_PUBLISHER,
} smdh_field_t;

static const u16 *pick_utf16_field(const SMDH_Header *smdh, u8 lang, smdh_field_t field)
{
	const SMDH_ApplicationTitle *slot = &smdh->titles[lang];
	const SMDH_ApplicationTitle *english = &smdh->titles[1];
	const u16 *primary;
	const u16 *fallback;

	switch (field) {
	case SMDH_FIELD_SHORT:
		primary = slot->short_desc;
		fallback = english->short_desc;
		break;
	case SMDH_FIELD_LONG:
		primary = slot->long_desc;
		fallback = english->long_desc;
		break;
	case SMDH_FIELD_PUBLISHER:
		primary = slot->publisher;
		fallback = english->publisher;
		break;
	default:
		return NULL;
	}

	if (primary[0] != 0)
		return primary;
	if (fallback[0] != 0)
		return fallback;
	return NULL;
}

static void append_text(char *buf, size_t bufSize, const char *text)
{
	size_t used;
	size_t room;

	if (buf == NULL || bufSize == 0 || text == NULL || text[0] == '\0')
		return;

	used = strlen(buf);
	if (used >= bufSize - 1)
		return;

	room = bufSize - used;
	if (used > 0 && buf[used - 1] != '\n' && room > 2) {
		strncat(buf, "; ", room - 1);
		used = strlen(buf);
		room = bufSize - used;
	}

	strncat(buf, text, room - 1);
}

static void append_rating(char *buf, size_t bufSize, const char *board, u8 value)
{
	char line[48];

	if (value == 0)
		return;

	if (value == 0x40) {
		snprintf(line, sizeof(line), "%s: Pending", board);
	} else if (value == 0x20) {
		snprintf(line, sizeof(line), "%s: No restriction", board);
	} else if (value & 0x80) {
		snprintf(line, sizeof(line), "%s: %u+", board, (unsigned)(value & 0x1F));
	} else {
		return;
	}

	append_text(buf, bufSize, line);
}

void title_smdh_format_ratings(const title_smdh_info_t *info, char *buf, size_t bufSize)
{
	static const char *boards[16] = {
		"CERO", "ESRB", NULL, "USK", "PEGI", NULL, "PEGI-PT", "BBFC",
		"COB", "GRB", "CGSRR", NULL, NULL, NULL, NULL, NULL
	};
	size_t i;

	if (buf == NULL || bufSize == 0)
		return;

	buf[0] = '\0';
	if (info == NULL)
		return;

	for (i = 0; i < 16; i++) {
		if (boards[i] != NULL)
			append_rating(buf, bufSize, boards[i], info->ratings[i]);
	}

	if (buf[0] == '\0')
		snprintf(buf, bufSize, "(none active)");
}

void title_smdh_format_region_lock(u32 regionLock, char *buf, size_t bufSize)
{
	if (buf == NULL || bufSize == 0)
		return;

	buf[0] = '\0';

	if (regionLock == 0x7FFFFFFF) {
		snprintf(buf, bufSize, "Region free");
		return;
	}

	if (regionLock & 0x01)
		append_text(buf, bufSize, "Japan");
	if (regionLock & 0x02)
		append_text(buf, bufSize, "North America");
	if (regionLock & 0x04)
		append_text(buf, bufSize, "Europe");
	if (regionLock & 0x08)
		append_text(buf, bufSize, "Australia");
	if (regionLock & 0x10)
		append_text(buf, bufSize, "China");
	if (regionLock & 0x20)
		append_text(buf, bufSize, "Korea");
	if (regionLock & 0x40)
		append_text(buf, bufSize, "Taiwan");

	if (buf[0] == '\0')
		snprintf(buf, bufSize, "0x%08lX", (unsigned long)regionLock);
}

void title_smdh_format_flags(u32 flags, char *buf, size_t bufSize)
{
	if (buf == NULL || bufSize == 0)
		return;

	buf[0] = '\0';

	if (flags & 0x0001)
		append_text(buf, bufSize, "Visible on Home Menu");
	if (flags & 0x0002)
		append_text(buf, bufSize, "Auto-boot card title");
	if (flags & 0x0004)
		append_text(buf, bufSize, "Allow 3D");
	if (flags & 0x0008)
		append_text(buf, bufSize, "Require EULA");
	if (flags & 0x0010)
		append_text(buf, bufSize, "Autosave on exit");
	if (flags & 0x0020)
		append_text(buf, bufSize, "Extended banner");
	if (flags & 0x0040)
		append_text(buf, bufSize, "Rating required");
	if (flags & 0x0080)
		append_text(buf, bufSize, "Uses save data");
	if (flags & 0x0100)
		append_text(buf, bufSize, "Record usage");
	if (flags & 0x0400)
		append_text(buf, bufSize, "No SD save backup");
	if (flags & 0x1000)
		append_text(buf, bufSize, "New 3DS exclusive");

	if (buf[0] == '\0')
		snprintf(buf, bufSize, "0x%08lX", (unsigned long)flags);
}

title_smdh_result_t title_smdh_load(u64 titleId, FS_MediaType media, title_smdh_info_t *info)
{
	SMDH_Header *smdh;
	u8 *iconData;
	u8 lang;
	const u16 *field;
	Result res;
	bool hasText = false;

	g_lastSmdhResult = 0;

	if (info == NULL)
		return TITLE_SMDH_ERR_FORMAT;

	memset(info, 0, sizeof(*info));
	info->result = TITLE_SMDH_ERR_OPEN;

	iconData = (u8 *)malloc(TITLE_ICON_SIZE);
	if (iconData == NULL)
		return TITLE_SMDH_ERR_READ;

	res = read_icon_file(titleId, media, iconData);
	g_lastSmdhResult = res;
	if (R_FAILED(res)) {
		free(iconData);
		return TITLE_SMDH_ERR_OPEN;
	}

	smdh = (SMDH_Header *)iconData;
	if (memcmp(&smdh->magic, "SMDH", 4) != 0) {
		free(iconData);
		info->result = TITLE_SMDH_ERR_FORMAT;
		return TITLE_SMDH_ERR_FORMAT;
	}

	lang = get_smdh_language_index();

	field = pick_utf16_field(smdh, lang, SMDH_FIELD_SHORT);
	if (field != NULL && utf16_field_to_utf8(field, 0x40, info->short_name, sizeof(info->short_name)))
		hasText = true;

	field = pick_utf16_field(smdh, lang, SMDH_FIELD_LONG);
	if (field != NULL)
		utf16_field_to_utf8(field, 0x80, info->long_name, sizeof(info->long_name));

	field = pick_utf16_field(smdh, lang, SMDH_FIELD_PUBLISHER);
	if (field != NULL)
		utf16_field_to_utf8(field, 0x40, info->publisher, sizeof(info->publisher));

	memcpy(info->ratings, iconData + 0x2008, sizeof(info->ratings));
	memcpy(&info->region_lock, iconData + 0x2018, sizeof(info->region_lock));
	memcpy(&info->flags, iconData + 0x2028, sizeof(info->flags));
	info->eula_minor = iconData[0x202C];
	info->eula_major = iconData[0x202D];
	memcpy(&info->cec_id, iconData + 0x2034, sizeof(info->cec_id));

	free(iconData);

	if (!hasText) {
		info->result = TITLE_SMDH_ERR_EMPTY;
		return TITLE_SMDH_ERR_EMPTY;
	}

	title_text_sanitize_utf8_for_console(info->short_name, sizeof(info->short_name));
	title_text_sanitize_utf8_for_console(info->long_name, sizeof(info->long_name));
	title_text_sanitize_utf8_for_console(info->publisher, sizeof(info->publisher));

	info->result = TITLE_SMDH_OK;
	return TITLE_SMDH_OK;
}

title_smdh_result_t title_smdh_get_short_name(u64 titleId, FS_MediaType media, char *out, size_t outSize)
{
	title_smdh_info_t info;
	title_smdh_result_t result;

	if (out == NULL || outSize == 0)
		return TITLE_SMDH_ERR_FORMAT;

	out[0] = '\0';
	result = title_smdh_load(titleId, media, &info);
	if (result != TITLE_SMDH_OK)
		return result;

	strncpy(out, info.short_name, outSize - 1);
	out[outSize - 1] = '\0';
	return TITLE_SMDH_OK;
}
