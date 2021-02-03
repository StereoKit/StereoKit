#pragma once

#include "stereokit.h"

namespace sk {

///////////////////////////////////////////

spherical_harmonics_t sh_calculate(void **env_map_data, tex_format_ format, int32_t face_size);
void                  sh_to_fast  (const spherical_harmonics_t &lookup, vec4 *fast_9);

}