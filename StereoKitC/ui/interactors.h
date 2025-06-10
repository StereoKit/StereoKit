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
	interactor_activation_state    = 0,
	interactor_activation_position = 1,
};

struct interactor_t {
	int32_t generation;

	// A description of what the interactor is and how it behaves
	interactor_event_      events;
	interactor_activation_ activation_type;
	interactor_type_       shape_type;
	// A unique ID per-source used to prevent multiple interactors from
	// re-using a source when it's already interacting with an element. A
	// negative id indicates this source is unique, and does not need to check
	// on other interactors.
	int32_t                input_source_id;
	// While generally static, this does change for some interactors! For
	// example, with hand far-rays, the minimum distance reduces as the hand
	// get further from the head.
	float                  min_distance;
	float                  capsule_radius;
	int32_t                secondary_motion_dimensions;

	// User provided per-frame data
	vec3                   capsule_end_world;
	vec3                   capsule_start_world;
	button_state_          tracked;
	button_state_          pinch_state;

	pose_t                 motion;
	pose_t                 motion_prev;
	// This is a point that defines where the interactor's motion is rooted,
	// like for a hand ray, this would be the shoulder. This can then be used
	// to amplify or reduce motion based on the distance from this point.
	vec3                   motion_anchor;
	vec3                   secondary_motion;

	// Internal vars for tracking information needed for interactions.

	// This keeps a record of where our interactor motion data was when the
	// interaction began.
	pose_t                 interaction_start_motion;
	vec3                   interaction_start_motion_anchor;
	// This is the location of the element we're interacting with at the start
	// of the interaction.
	pose_t                 interaction_start_el;
	// the location from where the interaction is happening, relative to the
	// start motion pose.
	vec3                   interaction_start_el_pivot;
	// This is the total amount of secondary motion that has occurred since the
	// interaction began. It needs to be reset when the interaction starts.
	vec3                   interaction_secondary_motion_total;
	// This is local to the motion pose
	vec3                   interaction_intersection_local;

	float                  focus_priority;
	float                  focus_distance;
	id_hash_t              focused_prev_prev;
	id_hash_t              focused_prev;
	id_hash_t              focused;
	id_hash_t              active_prev_prev;
	id_hash_t              active_prev;
	id_hash_t              active;

	pose_t                 focus_pose_world;
	bounds_t               focus_bounds_local;
	// Last known intersection, relative to focus_bounds_local in case the
	// element moves, like a slider's button.
	vec3                   focus_intersection_local;
};

typedef int32_t interactor_id;

void             interaction_init           ();
void             interaction_update         ();
void             interaction_shutdown       ();

void             interaction_1h_box         (id_hash_t id, interactor_event_ event_mask, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_* out_focus_candidacy, interactor_id* out_interactor);
void             interaction_1h_plate       (id_hash_t id, interactor_event_ event_mask, vec3 plate_start, vec3 plate_size, button_state_* out_focus_candidacy, interactor_id* out_interactor, vec3* out_interaction_at_local);
bool32_t         interaction_handle         (id_hash_t id, pose_t* ref_handle_pose, bounds_t handle_bounds, ui_move_ move_type, ui_gesture_ allowed_gestures);

interactor_id    interactor_create          (interactor_type_ shape_type, interactor_event_ events, interactor_activation_ activation_type, int32_t input_source_id, float capsule_radius, int32_t secondary_motion_dimensions);
void             interactor_destroy         (interactor_id interactor);
interactor_t*    interactor_get             (interactor_id interactor);
void             interactor_update          (interactor_id interactor, vec3 capsule_start, vec3 capsule_end, pose_t motion, vec3 motion_anchor, vec3 secondary_motion, button_state_ active, button_state_ tracked);
void             interactor_set_min_distance(interactor_id interactor, float min_distance);
float            interactor_get_min_distance(interactor_id interactor);
void             interactor_set_radius      (interactor_id interactor, float radius);
float            interactor_get_radius      (interactor_id interactor);
button_state_    interactor_get_tracked     (interactor_id interactor);
id_hash_t        interactor_get_focused     (interactor_id interactor);
id_hash_t        interactor_get_active      (interactor_id interactor);
bool32_t         interactor_get_focus_bounds(interactor_id interactor, pose_t *out_pose_world, bounds_t* out_bounds_local);
pose_t           interactor_get_motion      (interactor_id interactor);

bool32_t         interactor_is_preoccupied  (const interactor_t* interactor, id_hash_t for_el_id, interactor_event_ event_mask, bool32_t include_focused);
button_state_    interactor_set_focus       (      interactor_t* interactor, id_hash_t for_el_id, bool32_t physical_focused, bool32_t soft_focused, float priority, float distance, pose_t element_pose_world, bounds_t element_bounds_local, vec3 element_intersection_local);
button_state_    interactor_set_active      (      interactor_t* interactor, id_hash_t for_el_id, bool32_t active);
bool32_t         interactor_check_box       (const interactor_t* interactor, bounds_t box, vec3* out_at, float* out_priority);

bool32_t         ui_id_focused              (id_hash_t id);
button_state_    ui_id_focus_state          (id_hash_t id);
bool32_t         ui_id_active               (id_hash_t id);
button_state_    ui_id_active_state         (id_hash_t id);
interactor_id    ui_id_active_interactor    (id_hash_t id);
id_hash_t        ui_id_last_element         ();
bool32_t         ui_keyboard_focus_lost     (id_hash_t focused_id);

}