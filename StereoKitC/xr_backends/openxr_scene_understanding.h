#pragma once

#include "../stereokit.h"

namespace sk {

bool           oxr_su_init    ();
void           oxr_su_step    ();
void           oxr_su_shutdown();

bool32_t       oxr_su_raycast               (ray_t ray, ray_t* out_intersection);
void           oxr_su_set_occlusion_enabled (bool32_t enabled);
bool32_t       oxr_su_get_occlusion_enabled ();
void           oxr_su_set_raycast_enabled   (bool32_t enabled);
bool32_t       oxr_su_get_raycast_enabled   ();
void           oxr_su_set_refresh_type      (world_refresh_ refresh_type);
world_refresh_ oxr_su_get_refresh_type      ();
void           oxr_su_set_refresh_radius    (float radius_meters);
float          oxr_su_get_refresh_radius    ();
void           oxr_su_set_refresh_interval  (float every_seconds);
float          oxr_su_get_refresh_interval  ();
void           oxr_su_refresh_transforms    ();

}