/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "linux.h"
#if defined(SK_OS_LINUX)

#include <fontconfig/fontconfig.h>

#include "../libraries/array.h"
#include "../libraries/stref.h"
#include "../sk_memory.h"

namespace sk {

///////////////////////////////////////////

bool platform_impl_init() {
	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
}

///////////////////////////////////////////

void platform_impl_step() {
}

///////////////////////////////////////////

font_t platform_default_font() {
	array_t<char*> fonts = array_t<char*>::make(2);
	font_t         result = nullptr;

	FcConfig* config = FcInitLoadConfigAndFonts();
	FcPattern* pattern = FcNameParse((const FcChar8*)("sans-serif"));
	FcConfigSubstitute(config, pattern, FcMatchPattern);
	FcDefaultSubstitute(pattern);

	FcResult   fc_result;
	FcPattern* font = FcFontMatch(config, pattern, &fc_result);
	if (font) {
		FcChar8* file = nullptr;
		if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch) {
			fonts.add(string_copy((char*)file));
		}
		FcPatternDestroy(font);
	}
	FcPatternDestroy(pattern);
	FcConfigDestroy(config);

	result = font_create_files((const char**)fonts.data, fonts.count);
	for(int32_t i=0; i<fonts.count; i++) sk_free(fonts[i]);
	fonts.free();
	return result;
}

///////////////////////////////////////////

void platform_msgbox_err(const char *text, const char *header) {
	// Not implemented on Linux
}

///////////////////////////////////////////

void platform_print_callstack() { }

}

#endif
