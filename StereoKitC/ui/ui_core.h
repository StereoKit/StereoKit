#include "../stereokit_ui.h"

namespace sk {

enum interactor_type_ {
	interactor_type_none,
	interactor_type_point,
	interactor_type_line,
};

enum interactor_event_ {
	interactor_event_poke  = 1 << 1,
	interactor_event_grip  = 1 << 2,
	interactor_event_pinch = 1 << 3
};

struct interactor_t {
	// What type of interactions does this provide
	interactor_event_ events;
	interactor_type_  type;

	vec3      capsule_end;
	vec3      capsule_end_world;
	vec3      capsule_start;
	vec3      capsule_start_world;
	float     capsule_radius;

	bool          tracked;
	button_state_ pinch_state;

	bool      ray_enabled;
	bool      ray_discard;
	float     ray_visibility;

	vec3 position;
	vec3 position_prev;
	quat orientation;
	quat orientation_prev;
	// This is a point that defines where the interactor's motion is rooted,
	// like for a hand ray, this would be the shoulder. This can then be used
	// to amplify or reduce motion based on the distance from this point.
	vec3 motion_anchor;

	vec3 interaction_start_position;
	quat interaction_start_orientation;
	vec3 interaction_start_motion_anchor;

	vec3 interaction_pt_position;
	quat interaction_pt_orientation;
	vec3 interaction_pt_pivot; // the location from where the interaction is happening, relative to the interaction pt

	vec3 interaction_intersection_local;

	id_hash_t focused_prev_prev;
	id_hash_t focused_prev;
	id_hash_t focused;
	float     focus_priority;
	id_hash_t active_prev_prev;
	id_hash_t active_prev;
	id_hash_t active;
};

void             ui_core_init               ();
void             ui_core_update             ();
void             ui_core_shutdown           ();

bool32_t         ui_in_box                  (vec3 pt, vec3 pt_prev, float radius, bounds_t box);
void             ui_box_interaction_1h_pinch(id_hash_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_* out_focus_state, int32_t* out_interactor);
void             ui_box_interaction_1h_poke (id_hash_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_* out_focus_state, int32_t* out_interactor);
bool32_t         _ui_handle_begin           (id_hash_t id, pose_t& handle_pose, bounds_t handle_bounds, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures);
void ui_slider_behavior(bool vertical, id_hash_t id, float* value, float min, float max, float step, vec3 window_relative_pos, vec2 size, vec2 button_size, ui_confirm_ confirm_method, vec2* out_button_center, float* out_finger_offset, button_state_* out_focus_state, button_state_* out_active_state, int32_t* out_interactor);

bool32_t         ui_is_enabled              ();
bool32_t         ui_id_focused              (id_hash_t id);
button_state_    ui_id_focus_state          (id_hash_t id);
bool32_t         ui_id_active               (id_hash_t id);
button_state_    ui_id_active_state         (id_hash_t id);
bool32_t         ui_keyboard_focus_lost     (id_hash_t focused_id);

bool32_t         interactor_is_preoccupied  (int32_t interactor, id_hash_t for_el_id, bool32_t include_focused);
int32_t          interactor_last_focused    (                    id_hash_t for_el_id);
button_state_    interactor_set_focus       (int32_t interactor, id_hash_t for_el_id, bool32_t focused, float priority);
button_state_    interactor_set_active      (int32_t interactor, id_hash_t for_el_id, bool32_t active);
bool32_t         interactor_check_box       (const interactor_t* actor, bounds_t box, vec3* out_at, float* out_priority);
void             interactor_plate_1h        (id_hash_t id, interactor_event_ event_mask, vec3 plate_start, vec2 plate_size, button_state_* out_focus_state, int32_t* out_interactor, vec3* out_interaction_at_local);

inline bounds_t  ui_size_box  (vec3 top_left, vec3 dimensions) { return { top_left - dimensions / 2, dimensions }; }
inline id_hash_t ui_stack_hash(const char16_t* string)         { return ui_stack_hash_16(string); }
inline id_hash_t ui_push_id   (const char16_t* id)             { return ui_push_id_16(id); }

}