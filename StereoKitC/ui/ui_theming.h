#include "../stereokit_ui.h"

namespace sk {

extern color128 skui_tint;

extern sprite_t skui_toggle_off;
extern sprite_t skui_toggle_on;
extern sprite_t skui_radio_off;
extern sprite_t skui_radio_on;

extern sound_t  skui_snd_interact;
extern sound_t  skui_snd_uninteract;
extern sound_t  skui_snd_grab;
extern sound_t  skui_snd_ungrab;
extern sound_t  skui_snd_tick;

extern mesh_t     skui_box_dbg;
extern material_t skui_mat_dbg;
extern material_t skui_mat;

void ui_theming_init();
void ui_theming_shutdown();

vec2 ui_get_mesh_minsize(ui_vis_ element_visual);
void ui_draw_el(ui_vis_ element_visual, vec3 start, vec3 size, ui_color_ color, float focus);
void ui_draw_cube(vec3 start, vec3 size, ui_color_ color, float focus);

void  ui_anim_start  (uint64_t id);
bool  ui_anim_has    (uint64_t id, float duration);
float ui_anim_elapsed(uint64_t id, float duration = 1, float max = 1);

}