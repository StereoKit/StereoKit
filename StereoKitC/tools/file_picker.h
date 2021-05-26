#pragma once

#include "../stereokit.h"

namespace sk {

void file_picker_update  ();
void file_picker_shutdown();
bool file_picker_cache   (const char *filename, void **out_data, size_t *out_size);

}