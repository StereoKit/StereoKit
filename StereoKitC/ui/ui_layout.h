#include "../stereokit_ui.h"

namespace sk {

typedef int32_t ui_window_id;
struct ui_window_t {
	pose_t       pose;
	vec2         prev_size;
	vec2         curr_size;
	vec3         layout_start;
	vec2         layout_size;
	ui_win_      type;
	ui_move_     move;
	uint64_t     hash;
	uint32_t     age;
};

typedef int32_t ui_layout_id;
struct ui_layout_t {
	ui_window_id window;
	ui_layout_id parent;
	vec3         offset_initial;
	vec3         offset;
	vec3         offset_prev;
	vec2         size;
	vec2         furthest;
	vec2         line;
	vec2         line_prev;
	float        line_pad;
	float        margin;
	vec2         child_min;
	vec2         child_max;
};

extern ui_settings_t skui_settings;

void         ui_layout_init               ();
void         ui_layout_shutdown           ();

ui_window_id ui_layout_curr_window        ();
ui_layout_t* ui_layout_curr               ();
void         ui_layout_reserve_sz         (vec2 size, bool32_t add_padding, vec3* out_position, vec2* out_size);
void         ui_layout_reserve_vertical_sz(vec2 size, bool32_t add_padding, vec3* out_position, vec2* out_size);
void         ui_layout_window             (ui_window_id window, vec3 start, vec2 dimensions, bool32_t add_margin);
void         ui_override_recent_layout    (vec3 start, vec2 size);

ui_window_t *ui_window_get                (ui_window_id window);
ui_window_id ui_window_find_or_add        (id_hash_t hash, vec2 size);

}