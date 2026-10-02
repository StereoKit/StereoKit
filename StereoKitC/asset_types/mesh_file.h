#pragma once

#include "../stereokit.h"
#include "mesh_svg.h"

namespace sk {

// Same as the public mesh_create_file and mesh_create_mem, with the SVG
// knobs exposed.
mesh_t mesh_create_file_ex(const char* filename_utf8, int32_t priority, svg_options_t svg_options);
mesh_t mesh_create_mem_ex (const char* filename_utf8, const void* data, size_t data_size, int32_t priority, svg_options_t svg_options);

}
