#pragma once

#include "stereokit.h"

namespace sk {

///////////////////////////////////////////

spherical_harmonics_t sh_create   (vec3 *light_directions, color128 *light_colors, int32_t light_count);
void                  sh_add      (spherical_harmonics_t &to, vec3 light_dir, color128 light_color);
spherical_harmonics_t sh_calculate(void **env_map_data, tex_format_ format, int32_t face_size);
color128              sh_lookup   (const spherical_harmonics_t &lookup, vec3 normal);
void                  sh_to_fast  (const spherical_harmonics_t &lookup, vec4 *fast_9);

}