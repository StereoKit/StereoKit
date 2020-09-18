#pragma once

#include "../../stereokit.h"

#ifndef _WIN32
#define _countof(array) (sizeof(array) / sizeof(array[0]))
#endif

namespace sk {

void  platform_msgbox_err(const char *text, const char *header);
bool  platform_read_file (const char *filename, void **out_data, size_t *out_size);
bool  platform_get_cursor(vec2 &out_pos);
void  platform_set_cursor(vec2 window_pos);
float platform_get_scroll();
bool  platform_key_down  (key_ key);
void  platform_debug_output(const char *text);
void  platform_sleep     (int ms);

}