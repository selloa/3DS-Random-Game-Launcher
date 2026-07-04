// SPDX-License-Identifier: MIT
// Copyright (c) 2025 selloa

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdbool.h>

#include "title_meta.h"

typedef struct {
	bool include_unlisted;
	bool include_sd;
	bool include_nand;
	bool prefer_long_name;
	title_filter_options_t filters;
} launcher_settings_t;

void launcher_settings_apply_defaults(launcher_settings_t *settings);

bool launcher_settings_match_defaults(const launcher_settings_t *settings);

/// Load from SD; returns false if missing, invalid, or unsupported version (caller keeps defaults).
bool launcher_settings_load(launcher_settings_t *settings);

/// Write current settings to SD. Returns false on I/O error.
bool launcher_settings_save(const launcher_settings_t *settings);

/// Remove saved settings file from SD. Returns false if delete failed (missing file is success).
bool launcher_settings_delete(void);

#endif
