/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2023-2024 Qualcomm Technologies, Inc.
 */

#include "linux.h"
#if defined(SK_OS_LINUX)

#include "../libraries/sk_fontfile.h"

#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

bool platform_impl_init() {
	fontfile_init();
	return true;
}

///////////////////////////////////////////

void platform_impl_shutdown() {
	fontfile_shutdown();
}

///////////////////////////////////////////

void platform_impl_step() {
}

///////////////////////////////////////////

font_t platform_default_font() {
	char* file = fontfile_name_to_path("sans-serif");
	if (file == nullptr) return nullptr;

	font_t result = font_create_files((const char**)&file, 1);
	free(file);
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
