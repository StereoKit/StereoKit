#include "../stereokit_ui.h"

namespace sk {

struct ui_hand_t {
	vec3     finger;
	vec3     finger_prev;
	vec3     finger_world;
	vec3     finger_world_prev;
	vec3     thumb;
	vec3     thumb_world;
	vec3     pinch_pt;
	vec3     pinch_pt_prev;
	vec3     pinch_pt_world;
	vec3     pinch_pt_world_prev;
	bool     tracked;
	bool     ray_enabled;
	bool     ray_discard;
	float    ray_visibility;
	uint64_t focused_prev_prev;
	uint64_t focused_prev;
	uint64_t focused;
	float    focus_priority;
	uint64_t active_prev;
	uint64_t active;
	button_state_ pinch_state;
};

extern ui_hand_t skui_hand[2];
extern float     skui_finger_radius;

void ui_core_init();
void ui_core_update();
void ui_core_shutdown();

inline bounds_t ui_size_box(vec3 top_left, vec3 dimensions) { return { top_left - dimensions / 2, dimensions }; }

bool32_t      ui_in_box(vec3 pt, vec3 pt_prev, float radius, bounds_t box);
void          ui_box_interaction_1h_pinch(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_* out_focus_state, int32_t* out_hand);
void          ui_box_interaction_1h_poke (uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_* out_focus_state, int32_t* out_hand);
bool32_t      _ui_handle_begin(uint64_t id, pose_t& handle_pose, bounds_t handle_bounds, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures);

bool32_t      ui_is_hand_preoccupied(handed_ hand, uint64_t for_el_id, bool32_t include_focused);
int32_t       ui_last_focused_hand(uint64_t for_el_id);
button_state_ ui_focus_set (int32_t hand, uint64_t for_el_id, bool32_t focused, float priority);
button_state_ ui_active_set(int32_t hand, uint64_t for_el_id, bool32_t active);
bool32_t      ui_id_focused(uint64_t id);
bool32_t      ui_keyboard_focus_lost(uint64_t focused_id);

inline uint64_t ui_stack_hash(const char16_t* string) { return ui_stack_hash_16(string); }
inline uint64_t ui_push_id   (const char16_t* id)     { return ui_push_id_16(id); }

}