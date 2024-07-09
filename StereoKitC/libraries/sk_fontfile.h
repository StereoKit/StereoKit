// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2024 Nick Klingensmith
// Copyright (c) 2024 Qualcomm Technologies, Inc.

#pragma once

#include <stdint.h>
#include <stdbool.h>

struct font_fallback_info_t {
	char  filepath[256];
	char  name    [128];
	float scale;
};

void  fontfile_print_font       (const char* name_utf8);
char* fontfile_folder           ();
char* fontfile_name_to_path     (const char* name_utf8);
bool  fontfile_get_fallback_info(const char* name_utf8, font_fallback_info_t **out_info, int32_t *out_info_count);
void  fontfile_from_css         (const char* fontlist_utf8, font_fallback_info_t** out_info, int32_t* out_info_count);