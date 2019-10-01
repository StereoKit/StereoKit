#pragma once

#include "stereokit.h"

SK_API void     sk_ui_init        ();

SK_API void     sk_ui_nextline    ();
SK_API void     sk_ui_reserve_box (vec2 size);
SK_API void     sk_ui_space       (float space);

SK_API void     sk_ui_label       (const char *text);
SK_API void     sk_ui_image       (sprite_t image, vec2 size);
SK_API bool32_t sk_ui_button      (const char *text);
SK_API bool32_t sk_ui_affordance  (const char *text, pose_t &movement, vec3 at, vec3 size);
SK_API bool32_t sk_ui_hslider     (const char *id, float &value, float min, float max, float step, float width = 0);
SK_API void     sk_ui_window_begin(const char *text, pose_t &pose, vec2 size = vec2{ 0,0 });
SK_API void     sk_ui_window_end  ();