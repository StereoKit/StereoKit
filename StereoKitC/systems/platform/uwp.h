#pragma once

namespace sk {
bool uwp_init(const char *app_name);
void uwp_step_begin();
void uwp_step_end();
void uwp_vsync();
void uwp_shutdown();

void uwp_get_mouse(float &out_x, float &out_y, float &out_scroll);
bool uwp_mouse_button(int button);
bool uwp_key_down(int vk);
}