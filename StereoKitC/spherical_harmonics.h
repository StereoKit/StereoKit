#pragma once

#include "stereokit.h"

namespace sk {

///////////////////////////////////////////

void                  sh_add      (spherical_harmonics_t &to, vec3 light_dir, color128 light_color);
spherical_harmonics_t sh_calculate(void **env_map_data, tex_format_ format, int32_t face_size);
color128              sh_lookup   (const spherical_harmonics_t &lookup, vec3 normal);
tex_t                 sh_to_tex   (const spherical_harmonics_t &lookup, int32_t face_size);
void                  sh_to_fast  (const spherical_harmonics_t &lookup, vec4 *fast_9);

}