#include "../stereokit.h"
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

enum interactor_activation_ {
	interactor_activation_state = 0,
	interactor_activation_position = 1,
};

struct interactor_t {
	// What type of interactions does this provide
	interactor_event_      events;
	interactor_activation_ activation_type;
	interactor_type_       shape_type;

	vec3      capsule_end_world;
	vec3      capsule_start_world;
	float     capsule_radius;
	float     min_distance;

	button_state_ tracked;
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
	float     focus_distance;
	id_hash_t active_prev_prev;
	id_hash_t active_prev;
	id_hash_t active;

	vec3 focus_center_world;
};

void             interaction_init           ();
void             interaction_update         ();
void             interaction_shutdown       ();

void             interaction_1h_box         (id_hash_t id, interactor_event_ event_mask, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_* out_focus_candidacy, int32_t* out_interactor);
void             interaction_1h_plate       (id_hash_t id, interactor_event_ event_mask, vec3 plate_start, vec3 plate_size, button_state_* out_focus_candidacy, int32_t* out_interactor, vec3* out_interaction_at_local);
bool32_t         interaction_handle         (id_hash_t id, pose_t* ref_handle_pose, bounds_t handle_bounds, ui_move_ move_type, ui_gesture_ allowed_gestures);

int32_t          interactor_create          (interactor_type_ shape_type, interactor_event_ events, interactor_activation_ activation_type);
void             interactor_update          (int32_t interactor, vec3 capsule_start, vec3 capsule_end, float capsule_radius, vec3 motion_pos, quat motion_orientation, vec3 motion_anchor, button_state_ active, button_state_ tracked);
void             interactor_min_distance_set(int32_t interactor, float min_distance);
interactor_t*    interactor_get             (int32_t interactor);

int32_t          interactor_last_focused    (                                id_hash_t for_el_id);
bool32_t         interactor_is_preoccupied  (const interactor_t* interactor, id_hash_t for_el_id, interactor_event_ event_mask, bool32_t include_focused);
button_state_    interactor_set_focus       (      interactor_t* interactor, id_hash_t for_el_id, bool32_t focused, float priority, float distance, vec3 element_center_local);
button_state_    interactor_set_active      (      interactor_t* interactor, id_hash_t for_el_id, bool32_t active);
bool32_t         interactor_check_box       (const interactor_t* interactor, bounds_t box, vec3* out_at, float* out_priority);

bool32_t         ui_id_focused              (id_hash_t id);
button_state_    ui_id_focus_state          (id_hash_t id);
bool32_t         ui_id_active               (id_hash_t id);
button_state_    ui_id_active_state         (id_hash_t id);
int32_t          ui_id_active_interactor    (id_hash_t id);
bool32_t         ui_keyboard_focus_lost     (id_hash_t focused_id);

}