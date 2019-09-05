#pragma once

#include "../../StereoKitC/stereokit.h"

void sk_ui_init();
void sk_ui_begin_frame();
void sk_ui_push_pose(pose_t pose);
void sk_ui_pop_pose ();
void sk_ui_box      (vec3 start, vec3 size);
void sk_ui_text     (vec3 start, const char *text);

void sk_ui_nextline   ();
void sk_ui_reserve_box(vec2 size);
void sk_ui_space      (float space);

void sk_ui_button(const char *text);