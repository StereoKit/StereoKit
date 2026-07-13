/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "macos.h"
#if defined(SK_OS_MACOS)

#include <sk_app.h>

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
	// Try common macOS system font paths
	array_t<const char*> fonts = {};
	font_t               result = nullptr;

	// Primary font - San Francisco or Helvetica
	if      (ska_file_exists("/System/Library/Fonts/SFNS.ttf"))      fonts.add("/System/Library/Fonts/SFNS.ttf");
	else if (ska_file_exists("/System/Library/Fonts/Helvetica.ttc")) fonts.add("/System/Library/Fonts/Helvetica.ttc");
	else if (ska_file_exists("/Library/Fonts/Arial.ttf"))            fonts.add("/Library/Fonts/Arial.ttf");
	else if (ska_file_exists("/System/Library/Fonts/Supplemental/Arial.ttf")) fonts.add("/System/Library/Fonts/Supplemental/Arial.ttf");

	// Japanese font fallback
	if      (ska_file_exists("/System/Library/Fonts/ヒラギノ角ゴシック W3.ttc"))             fonts.add("/System/Library/Fonts/ヒラギノ角ゴシック W3.ttc");
	else if (ska_file_exists("/System/Library/Fonts/Supplemental/Hiragino Sans GB W3.otf")) fonts.add("/System/Library/Fonts/Supplemental/Hiragino Sans GB W3.otf");

	if (fonts.count > 0) {
		result = font_create_files(fonts.data, (int32_t)fonts.count);
	}
	fonts.free();
	return result;
}

///////////////////////////////////////////

void platform_msgbox_err(const char *text, const char *header) {
	// Not implemented on macOS yet - would need Cocoa/AppKit
	(void)text;
	(void)header;
}

///////////////////////////////////////////

void platform_print_callstack() { }

}

#endif
