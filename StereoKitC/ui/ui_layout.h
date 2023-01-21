#include "../stereokit_ui.h"

namespace sk {

struct ui_window_t {
	pose_t       pose;
	vec2         prev_size;
	vec2         curr_size;
	vec3         layout_start;
	vec2         layout_size;
	ui_win_      type;
	uint64_t     hash;
};

struct ui_layout_t {
	ui_window_t* window;
	vec3         offset_initial;
	vec3         offset;
	vec2         size;
	float        line_height;
	float        max_x;
	vec3         prev_offset;
	float        prev_line_height;
};

extern ui_settings_t skui_settings;

ui_window_t* ui_layout_curr_window        ();
ui_layout_t* ui_layout_curr               ();
void         ui_layout_reserve_sz         (vec2 size, bool32_t add_padding, vec3* out_position, vec2* out_size);
void         ui_layout_reserve_vertical_sz(vec2 size, bool32_t add_padding, vec3* out_position, vec2* out_size);
void         ui_layout_push_win           (ui_window_t* window, vec3 start, vec2 dimensions, bool32_t add_margin);

}