#pragma once

#include "stereokit.h"

namespace sk {

///////////////////////////////////////////

void                  sh_add                (spherical_harmonics_t &to, vec3 light_dir, vec3 light_color);
void                  sh_windowing          (spherical_harmonics_t &harmonics, float window_width);
void                  sh_window_fit         (spherical_harmonics_t &harmonics);
void                  sh_window_fit_radiance(spherical_harmonics_t &harmonics);
void                  sh_to_fast            (const spherical_harmonics_t &lookup, vec4 *fast_7);
color128              sh_lookup_radiance    (const spherical_harmonics_t &harmonics, vec3 dir);
void                  sh_irradiance_to_radiance(spherical_harmonics_t &harmonics);
float                 sh_delta              (const spherical_harmonics_t &to, const spherical_harmonics_t &from);

}
