#include "settings.h"

#include <3ds.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#define SETTINGS_MAGIC 0x314C4752u /* 'RGL1' little-endian */
#define SETTINGS_VERSION 1u
#define SETTINGS_DIR "sdmc:/3ds/3DS-Random-Game-Launcher"
#define SETTINGS_PATH SETTINGS_DIR "/settings.bin"

typedef struct {
	u32 magic;
	u16 version;
	u16 size;
	u8 include_unlisted;
	u8 include_sd;
	u8 include_nand;
	u8 prefer_long_name;
	u8 include_patches;
	u8 include_dlc;
	u8 include_system;
	u8 include_demos;
	u8 include_dsiware;
	u8 include_content_packs;
	u8 include_native_apps;
	u8 include_virtual_console;
	u8 reserved;
	u32 checksum;
} __attribute__((packed)) settings_blob_t;

static u32 settings_blob_checksum(const settings_blob_t *blob)
{
	const u8 *bytes = (const u8 *)blob;
	u32 sum = 0;
	size_t i;

	for (i = 0; i < offsetof(settings_blob_t, checksum); i++)
		sum = (sum * 131u) + bytes[i];

	return sum;
}

static void settings_blob_from_launcher(const launcher_settings_t *settings, settings_blob_t *blob)
{
	memset(blob, 0, sizeof(*blob));
	blob->magic = SETTINGS_MAGIC;
	blob->version = SETTINGS_VERSION;
	blob->size = (u16)sizeof(settings_blob_t);
	blob->include_unlisted = settings->include_unlisted ? 1 : 0;
	blob->include_sd = settings->include_sd ? 1 : 0;
	blob->include_nand = settings->include_nand ? 1 : 0;
	blob->prefer_long_name = settings->prefer_long_name ? 1 : 0;
	blob->include_patches = settings->filters.include_patches ? 1 : 0;
	blob->include_dlc = settings->filters.include_dlc ? 1 : 0;
	blob->include_system = settings->filters.include_system ? 1 : 0;
	blob->include_demos = settings->filters.include_demos ? 1 : 0;
	blob->include_dsiware = settings->filters.include_dsiware ? 1 : 0;
	blob->include_content_packs = settings->filters.include_content_packs ? 1 : 0;
	blob->include_native_apps = settings->filters.include_native_apps ? 1 : 0;
	blob->include_virtual_console = settings->filters.include_virtual_console ? 1 : 0;
	blob->checksum = settings_blob_checksum(blob);
}

static bool settings_blob_to_launcher(const settings_blob_t *blob, launcher_settings_t *settings)
{
	if (blob->magic != SETTINGS_MAGIC)
		return false;
	if (blob->version != SETTINGS_VERSION)
		return false;
	if (blob->size != sizeof(settings_blob_t))
		return false;
	if (blob->checksum != settings_blob_checksum(blob))
		return false;

	settings->include_unlisted = blob->include_unlisted != 0;
	settings->include_sd = blob->include_sd != 0;
	settings->include_nand = blob->include_nand != 0;
	settings->prefer_long_name = blob->prefer_long_name != 0;
	settings->filters.include_patches = blob->include_patches != 0;
	settings->filters.include_dlc = blob->include_dlc != 0;
	settings->filters.include_system = blob->include_system != 0;
	settings->filters.include_demos = blob->include_demos != 0;
	settings->filters.include_dsiware = blob->include_dsiware != 0;
	settings->filters.include_content_packs = blob->include_content_packs != 0;
	settings->filters.include_native_apps = blob->include_native_apps != 0;
	settings->filters.include_virtual_console = blob->include_virtual_console != 0;
	return true;
}

static void settings_ensure_dir(void)
{
	mkdir("sdmc:/3ds", 0777);
	mkdir(SETTINGS_DIR, 0777);
}

void launcher_settings_apply_defaults(launcher_settings_t *settings)
{
	if (settings == NULL)
		return;

	settings->include_unlisted = false;
	settings->include_sd = true;
	settings->include_nand = false;
	settings->prefer_long_name = true;
	settings->filters.include_patches = false;
	settings->filters.include_dlc = false;
	settings->filters.include_system = false;
	settings->filters.include_demos = true;
	settings->filters.include_dsiware = false;
	settings->filters.include_content_packs = false;
	settings->filters.include_native_apps = true;
	settings->filters.include_virtual_console = true;
}

bool launcher_settings_match_defaults(const launcher_settings_t *settings)
{
	launcher_settings_t defaults;

	if (settings == NULL)
		return false;

	launcher_settings_apply_defaults(&defaults);
	return memcmp(settings, &defaults, sizeof(launcher_settings_t)) == 0;
}

bool launcher_settings_load(launcher_settings_t *settings)
{
	settings_blob_t blob;
	FILE *file;

	if (settings == NULL)
		return false;

	file = fopen(SETTINGS_PATH, "rb");
	if (file == NULL)
		return false;

	if (fread(&blob, sizeof(blob), 1, file) != 1) {
		fclose(file);
		return false;
	}
	fclose(file);

	return settings_blob_to_launcher(&blob, settings);
}

bool launcher_settings_save(const launcher_settings_t *settings)
{
	settings_blob_t blob;
	FILE *file;

	if (settings == NULL)
		return false;

	settings_ensure_dir();
	settings_blob_from_launcher(settings, &blob);

	file = fopen(SETTINGS_PATH, "wb");
	if (file == NULL)
		return false;

	if (fwrite(&blob, sizeof(blob), 1, file) != 1) {
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}

bool launcher_settings_delete(void)
{
	FILE *file = fopen(SETTINGS_PATH, "rb");

	if (file == NULL)
		return true;

	fclose(file);
	return remove(SETTINGS_PATH) == 0;
}
