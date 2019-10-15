#pragma once

#include "stereokit.h"

namespace sk {

SK_API void     ui_init        ();

SK_API void     ui_nextline    ();
SK_API void     ui_sameline    ();
SK_API void     ui_reserve_box (vec2 size);
SK_API void     ui_space       (float space);

SK_API void     ui_label       (const char *text);
SK_API void     ui_image       (sprite_t image, vec2 size);
SK_API bool32_t ui_button      (const char *text);
SK_API bool32_t ui_input       (const char *id, char *buffer, int32_t buffer_size);
SK_API bool32_t ui_affordance  (const char *text, pose_t &movement, vec3 at, vec3 size, bool32_t draw);
SK_API bool32_t ui_hslider     (const char *id, float &value, float min, float max, float step, float width = 0);
SK_API void     ui_window_begin(const char *text, pose_t &pose, vec2 size = vec2{ 0,0 });
SK_API void     ui_window_end  ();

} // namespace sk