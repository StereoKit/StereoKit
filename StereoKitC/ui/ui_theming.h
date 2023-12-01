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

void ui_theming_init();
void ui_theming_update();
void ui_theming_shutdown();

vec2     ui_get_mesh_minsize (ui_vis_ element_visual);
void     ui_draw_el          (ui_vis_ element_visual, vec3 start, vec3 size, float focus);
void     ui_draw_el_color    (ui_vis_ element_visual, ui_vis_ element_color, vec3 start, vec3 size, float focus);
color128 ui_get_el_color     (ui_vis_ element_visual, float focus);
void     ui_play_sound_on_off(ui_vis_ element_visual, uint64_t element_id, vec3 at);
void     ui_play_sound_on    (ui_vis_ element_visual, vec3 at);
void     ui_play_sound_off   (ui_vis_ element_visual, vec3 at);
void     ui_draw_cube        (vec3 start, vec3 size, ui_color_ color, float focus);

void  ui_anim_start  (uint64_t id, int32_t channel);
bool  ui_anim_has    (uint64_t id, int32_t channel, float duration);
float ui_anim_elapsed(uint64_t id, int32_t channel, float duration = 1, float max = 1);

}