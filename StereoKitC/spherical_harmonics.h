#pragma once

#include "stereokit.h"

namespace sk {

///////////////////////////////////////////

spherical_harmonics_t sh_calculate(void **env_map_data, tex_format_ format, int32_t face_size);
void                  sh_add      (spherical_harmonics_t &to, vec3 light_dir, vec3 light_color);
void                  sh_windowing(spherical_harmonics_t &harmonics, float window_width);
void                  sh_to_fast  (const spherical_harmonics_t &lookup, vec4 *fast_9);

}