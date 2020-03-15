#pragma once

namespace sk {

void platform_msgbox_err(const char *text, const char *header);
bool platform_read_file (const char *filename, void *&out_data, size_t &out_size);

}