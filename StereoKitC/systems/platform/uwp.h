#pragma once

namespace sk {

bool uwp_init      ();
void uwp_init_after_openxr();
void uwp_shutdown  ();
bool uwp_start     ();
void uwp_stop      ();
void uwp_step_begin();
void uwp_step_end  ();
void uwp_vsync     ();

bool  uwp_get_mouse   (vec2 &out_pos);
void  uwp_set_mouse   (vec2 window_pos);
float uwp_get_scroll  ();

void uwp_show_keyboard(bool show);

}