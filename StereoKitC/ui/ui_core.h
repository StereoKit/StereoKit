#include "../stereokit_ui.h"

namespace sk {

void             ui_core_init               ();
void             ui_core_update             ();
void             ui_core_shutdown           ();

inline bounds_t  ui_size_box  (vec3 top_left, vec3 dimensions) { return { top_left - dimensions / 2, dimensions }; }
inline id_hash_t ui_stack_hash(const char16_t* string)         { return ui_stack_hash_16(string); }
inline id_hash_t ui_push_id   (const char16_t* id)             { return ui_push_id_16(id); }

bool32_t _ui_handle_begin(id_hash_t id, pose_t* handle_pose, bounds_t handle_bounds, bool32_t draw, ui_move_ move_type, ui_gesture_ allowed_gestures);

}