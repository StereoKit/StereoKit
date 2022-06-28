#pragma once
#include "../stereokit.h"

namespace sk {

bool uwp_init           ();
bool uwp_start_pre_xr   ();
bool uwp_start_post_xr  ();
bool uwp_start_flat     ();
void uwp_step_begin_xr  ();
void uwp_step_begin_flat();
void uwp_step_end_flat  ();
void uwp_stop_flat      ();
void uwp_shutdown       ();

bool  uwp_get_mouse   (vec2 &out_pos);
void  uwp_set_mouse   (vec2 window_pos);
float uwp_get_scroll  ();

void uwp_show_keyboard   (bool show);
bool uwp_keyboard_visible();

}