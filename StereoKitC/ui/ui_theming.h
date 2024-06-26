#include "../stereokit_ui.h"

namespace sk {

extern color128 skui_tint;

extern sprite_t skui_toggle_off;
extern sprite_t skui_toggle_on;
extern sprite_t skui_radio_off;
extern sprite_t skui_radio_on;

extern mesh_t     skui_box_dbg;
extern material_t skui_mat_dbg;
extern material_t skui_mat;

extern const float skui_aura_radius;

void     ui_theming_init     ();
void     ui_theming_update   ();
void     ui_theming_shutdown ();

vec2     ui_get_mesh_minsize (ui_vis_ element_visual);
void     ui_play_sound_on_off(ui_vis_ element_visual, id_hash_t element_id, vec3 at);
void     ui_play_sound_on    (ui_vis_ element_visual, vec3 at);
void     ui_play_sound_off   (ui_vis_ element_visual, vec3 at);
void     ui_draw_cube        (vec3 start, vec3 size, ui_color_ color, float focus);

void     ui_anim_start        (id_hash_t id, int32_t channel);
void     ui_anim_cancel       (id_hash_t id, int32_t channel);
bool     ui_anim_has          (id_hash_t id, int32_t channel, float duration);
float    ui_anim_elapsed      (id_hash_t id, int32_t channel, float duration = 1, float max = 1);
float    ui_anim_elapsed_total(id_hash_t id, int32_t channel);

float    ui_get_anim_focus_inout(id_hash_t id, button_state_ activation_state);

}