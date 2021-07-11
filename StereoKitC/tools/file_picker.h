#pragma once

#include "../stereokit.h"

namespace sk {

void file_picker_update    ();
void file_picker_shutdown  ();
bool file_picker_cache_read(const char *filename, void **out_data, size_t *out_size);
bool file_picker_cache_save(const char *filename, void *data, size_t size);

}