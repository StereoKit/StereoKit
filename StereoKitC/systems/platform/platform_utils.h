#pragma once

#include "../../stereokit.h"

namespace sk {

void  platform_msgbox_err(const char *text, const char *header);
bool  platform_read_file (const char *filename, void *&out_data, size_t &out_size);
bool  platform_get_cursor(vec2 &out_pos);
void  platform_set_cursor(vec2 window_pos);
float platform_get_scroll();

}