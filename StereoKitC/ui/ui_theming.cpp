#include "ui_theming.h"
#include "ui_core.h"
#include "ui_layout.h"

#include "../libraries/array.h"
#include "../utils/sdf.h"
#include "../sk_math.h"
#include "../platforms/platform.h"

#include <float.h>

namespace sk {

///////////////////////////////////////////

struct ui_el_visual_t {
	mesh_t     mesh;
	material_t material;
	vec2       min_size;
	sound_t    snd_activate;
	sound_t    snd_deactivate;
	ui_color_  color_category;
};

struct ui_theme_color_t {
	color128 normal;
	color128 active;
	color128 disabled;
};

///////////////////////////////////////////

font_t                    skui_font;

material_t                skui_mat;
material_t                skui_mat_dbg;
material_t                skui_font_mat;

sprite_t                  skui_toggle_off;
sprite_t                  skui_toggle_on;
sprite_t                  skui_radio_off;
sprite_t                  skui_radio_on;
sprite_t                  skui_arrow_left;
sprite_t                  skui_arrow_right;
sprite_t                  skui_arrow_up;
sprite_t                  skui_arrow_down;
sprite_t                  skui_spr_close;
sprite_t                  skui_spr_backspace;
sprite_t                  skui_spr_shift;
sprite_t                  skui_spr_list;
sprite_t                  skui_spr_grid;

sound_t                   skui_snd_interact;
sound_t                   skui_snd_uninteract;
sound_t                   skui_snd_grab;
sound_t                   skui_snd_ungrab;
sound_t                   skui_snd_tick;

mesh_t                    skui_box_dbg;

id_hash_t                 skui_anim_id[3];
float                     skui_anim_time[3];

color128                  skui_tint;

array_t<ui_el_visual_t>   skui_visuals;
array_t<ui_theme_color_t> skui_palette;
array_t<text_style_t>     skui_font_stack;
array_t<color128>         skui_tint_stack;
array_t<bool32_t>         skui_grab_aura_stack;

sound_t                   skui_active_sound_off        = nullptr;
sound_inst_t              skui_active_sound_inst       = {};
vec3                      skui_active_sound_pos        = vec3_zero;
id_hash_t                 skui_active_sound_element_id = 0;

const float               skui_anim_duration           = 0.2f;
const float               skui_anim_overshoot          = 10;
const float               skui_anim_focus_duration     = 0.1f;

///////////////////////////////////////////

void _ui_gen_quadrant_mesh(mesh_t* mesh, ui_corner_ rounded_corners, float corner_radius, uint32_t corner_resolution, bool32_t delete_flat_sides, const ui_lathe_pt_t* lathe_pts, int32_t lathe_pt_count);
void ui_default_aura_mesh (mesh_t* mesh, float diameter, float rounding, float shrink, int32_t quadrant_slices, int32_t tube_corners);
void ui_theme_visuals_update ();
void ui_theme_visuals_release();
void ui_theme_visuals_assign ();

///////////////////////////////////////////

void ui_theming_init() {
	skui_tint            = { 1,1,1,1 };
	skui_font_stack      = {};
	skui_tint_stack      = {};
	skui_grab_aura_stack = {};
	skui_visuals.add_empties(ui_vis_max);
	skui_palette.add_empties(ui_color_max);
	memset(skui_anim_id,   0, sizeof(skui_anim_id));
	memset(skui_anim_time, 0, sizeof(skui_anim_time));

	ui_set_color(color_hsv(0.07f, 0.5f, 0.75f, 1));

	skui_font_mat   = material_find(default_id_material_font);
	material_set_queue_offset(skui_font_mat, -12);
	skui_font       = font_find(default_id_font);
	text_style_t style = text_make_style_mat(skui_font, 0.01f, skui_font_mat, { 1,1,1,1 });
	// TODO: v0.4, switch these to something more intentional instead of backwards compatible (like lineheight 1.4)
	// This matches the original SK size for compat, for now.
	text_style_set_line_height_pct(style, 1.1f);
	skui_font_stack.add(style);

	// TODO: v0.4, this sets up default values when zeroed out, with a
	// ui_get_settings, this isn't really necessary anymore!
	ui_settings_t settings = {};
	ui_settings(settings);

	// Create default sprites for the toggles

	// #defines because we can't capture anything in these lambdas D:
	#define OUTER_RADIUS 31
	#define RING_WIDTH 8
	#define RADIUS (OUTER_RADIUS - RING_WIDTH)
	#define INNER_RADIUS (OUTER_RADIUS - RING_WIDTH*2)

	skui_toggle_on = sdf_create_sprite(ui_default_id_spr_toggle_on, 64, 64, [](vec2 pt) {
		return
			sdf_union(
				sdf_box(pt, {INNER_RADIUS, INNER_RADIUS}),
				sdf_subtract(
					sdf_box_round(pt, {RADIUS, RADIUS}, 4),
					sdf_box_round(pt, {OUTER_RADIUS,OUTER_RADIUS}, 8)))/32.0f;
	}, 40);
	skui_toggle_off = sdf_create_sprite(ui_default_id_spr_toggle_off, 64, 64, [](vec2 pt) {
		return
			sdf_subtract(
				sdf_box_round(pt, {RADIUS, RADIUS}, 4),
				sdf_box_round(pt, {OUTER_RADIUS,OUTER_RADIUS}, 8))/32.0f;
	}, 40);
	skui_radio_on = sdf_create_sprite(ui_default_id_spr_radio_on, 64, 64, [](vec2 pt) {
		float dist = vec2_magnitude(pt);
		return
			sdf_union(
				dist - INNER_RADIUS,
				sdf_subtract(
					dist - (OUTER_RADIUS - RING_WIDTH),
					dist - OUTER_RADIUS)) / 32.0f;
	}, 40);
	skui_radio_off = sdf_create_sprite(ui_default_id_spr_radio_off, 64, 64, [](vec2 pt) {
		float dist = vec2_magnitude(pt);
		return sdf_subtract(dist - (OUTER_RADIUS - RING_WIDTH), dist - OUTER_RADIUS)/32.0f;
	}, 40);

	// Create some sprites for common UI icons
	skui_spr_backspace = sdf_create_sprite(ui_default_id_spr_backspace, 96, 64, [](vec2 pt) {
		return
			sdf_subtract(
				sdf_rounded_x(pt - vec2{3.5f,0}, 20, 3.5f),
				sdf_union(
					sdf_box    (pt - vec2{10, 0}, {20,20})-7,
					sdf_diamond(pt + vec2{10, 0}, {20,20})-7)) / 40.0f;
	}, 40);
	skui_spr_shift = sdf_create_sprite(ui_default_id_spr_shift, 64, 64, [](vec2 pt) {
		return
			sdf_union(
				sdf_triangle(pt + vec2{0,24}, {24,20}) - 4,
				sdf_box(pt + vec2{0,-6}, {6, 12}) - 7) / 40.0f;
	}, 40);
	skui_spr_close = sdf_create_sprite(ui_default_id_spr_close, 64, 64, [](vec2 pt) {
		return sdf_rounded_x(pt, 36, 7) / 40.0f;
	}, 40);
	skui_arrow_left = sdf_create_sprite(ui_default_id_spr_arrow_left, 64, 64, [](vec2 pt) {
		return (sdf_triangle({ pt.y, pt.x + 24 }, {26,44}) - 4) / 40.0f;
	}, 40);
	skui_arrow_right = sdf_create_sprite(ui_default_id_spr_arrow_right, 64, 64, [](vec2 pt) {
		return (sdf_triangle({ pt.y, -pt.x + 24 }, {26,44}) - 4) / 40.0f;
	}, 40);
	skui_arrow_up = sdf_create_sprite(ui_default_id_spr_arrow_up, 64, 64, [](vec2 pt) {
		return (sdf_triangle({ pt.x, pt.y + 24 }, {26,44}) - 4) / 40.0f;
	}, 40);
	skui_arrow_down = sdf_create_sprite(ui_default_id_spr_arrow_down, 64, 64, [](vec2 pt) {
		return (sdf_triangle({ pt.x, -pt.y + 24 }, {26,44}) - 4) / 40.0f;
	}, 40);

	skui_spr_list = sdf_create_sprite(ui_default_id_spr_list, 64, 64, [](vec2 pt) {
		return
			sdf_union(
			sdf_union(
				sdf_box_round(pt + vec2{ 0, 20 }, vec2{30, 6}, 6 ),
				sdf_box_round(pt + vec2{ 0, 0  }, vec2{30, 6}, 6 )),
				sdf_box_round(pt + vec2{ 0,-20 }, vec2{30, 6}, 6 ))/32.0f;
	}, 40);
	skui_spr_grid = sdf_create_sprite(ui_default_id_spr_grid, 64, 64, [](vec2 pt) {
		return
			sdf_union(
			sdf_union(
				sdf_union(
				sdf_union(
					sdf_box_round(pt + vec2{-20, 20 }, vec2{6, 6}, 3 ),
					sdf_box_round(pt + vec2{-20, 0  }, vec2{6, 6}, 3 )),
					sdf_box_round(pt + vec2{-20,-20 }, vec2{6, 6}, 3 )),
				sdf_union(
				sdf_union(
					sdf_box_round(pt + vec2{  0, 20 }, vec2{6, 6}, 3 ),
					sdf_box_round(pt + vec2{  0, 0  }, vec2{6, 6}, 3 )),
					sdf_box_round(pt + vec2{  0,-20 }, vec2{6, 6}, 3 ))),
				sdf_union(
				sdf_union(
					sdf_box_round(pt + vec2{ 20, 20 }, vec2{6, 6}, 3 ),
					sdf_box_round(pt + vec2{ 20, 0  }, vec2{6, 6}, 3 )),
					sdf_box_round(pt + vec2{ 20,-20 }, vec2{6, 6}, 3 ))) / 32.0f;
	}, 40);

	skui_box_dbg  = mesh_find(default_id_mesh_cube);
	skui_mat_dbg  = material_copy_id(default_id_material);
	material_set_transparency(skui_mat_dbg, transparency_add);
	material_set_color       (skui_mat_dbg, "color", { .3f,.3f,.3f,.3f });
	material_set_depth_write (skui_mat_dbg, false);
	material_set_depth_test  (skui_mat_dbg, depth_test_always);
	material_set_id          (skui_mat_dbg, "sk/ui/debug_mat");

	skui_mat = material_copy_id(default_id_material_ui);
	material_set_id(skui_mat, "sk/ui/default_mat");

	// Create a sound for the HSlider
	skui_snd_tick = sound_generate([](float t){
		float x     = t / 0.03f;
		float band1 = sinf(t*8000) * (x * powf(1 - x, 10)) / 0.03f;
		float band2 = sinf(t*6550) * (x * powf(1 - x, 12)) / 0.03f;

		return (band1*0.6f + band2*0.4f) * 0.05f;
	}, .03f);
	sound_set_id(skui_snd_tick, "sk/ui/tick_snd");

	skui_snd_interact   = sound_find(default_id_sound_click);
	skui_snd_uninteract = sound_find(default_id_sound_unclick);
	skui_snd_grab       = sound_find(default_id_sound_grab);
	skui_snd_ungrab     = sound_find(default_id_sound_ungrab);

	ui_theme_visuals_assign();

	ui_set_element_color(ui_vis_none,                 ui_color_common);
	ui_set_element_color(ui_vis_default,              ui_color_common);
	ui_set_element_color(ui_vis_window_head,          ui_color_primary);
	ui_set_element_color(ui_vis_window_head_only,     ui_color_primary);
	ui_set_element_color(ui_vis_window_body,          ui_color_background);
	ui_set_element_color(ui_vis_window_body_only,     ui_color_background);
	ui_set_element_color(ui_vis_separator,            ui_color_primary);
	ui_set_element_color(ui_vis_slider_line,          ui_color_common);
	ui_set_element_color(ui_vis_slider_line_active,   ui_color_primary);
	ui_set_element_color(ui_vis_slider_line_inactive, ui_color_common);
	ui_set_element_color(ui_vis_slider_pinch,         ui_color_primary);
	ui_set_element_color(ui_vis_slider_push,          ui_color_primary);
	ui_set_element_color(ui_vis_aura,                 ui_color_text);
	ui_set_element_color(ui_vis_panel,                ui_color_complement);
	ui_set_element_color(ui_vis_handle,               ui_color_primary);
	ui_set_element_color(ui_vis_button,               ui_color_common);
	ui_set_element_color(ui_vis_toggle,               ui_color_common);
	ui_set_element_color(ui_vis_button_round,         ui_color_common);
	ui_set_element_color(ui_vis_input,                ui_color_common);
	ui_set_element_color(ui_vis_carat,                ui_color_text);

	ui_set_element_sound(ui_vis_default,              skui_snd_interact, skui_snd_uninteract);
	ui_set_element_sound(ui_vis_handle,               skui_snd_grab,     skui_snd_ungrab);
	ui_set_element_sound(ui_vis_window_body,          skui_snd_grab,     skui_snd_ungrab);
	ui_set_element_sound(ui_vis_window_head,          skui_snd_grab,     skui_snd_ungrab);
	ui_set_element_sound(ui_vis_window_body_only,     skui_snd_grab,     skui_snd_ungrab);
	ui_set_element_sound(ui_vis_window_head_only,     skui_snd_grab,     skui_snd_ungrab);
	ui_set_element_sound(ui_vis_slider_line,          skui_snd_tick,     skui_snd_tick);
}

///////////////////////////////////////////

void ui_theming_shutdown() {
	skui_font_stack     .free();
	skui_tint_stack     .free();
	skui_grab_aura_stack.free();

	for (int32_t i = 0; i < skui_visuals.count; i++) {
		mesh_release    (skui_visuals[i].mesh);
		material_release(skui_visuals[i].material);
		sound_release   (skui_visuals[i].snd_activate);
		sound_release   (skui_visuals[i].snd_deactivate);
		skui_visuals[i] = {};
	}
	skui_visuals.free();
	skui_palette.free();

	ui_theme_visuals_release();

	sound_release   (skui_active_sound_off); skui_active_sound_off = nullptr;
	sound_release   (skui_snd_interact);     skui_snd_interact     = nullptr;
	sound_release   (skui_snd_uninteract);   skui_snd_uninteract   = nullptr;
	sound_release   (skui_snd_grab);         skui_snd_grab         = nullptr;
	sound_release   (skui_snd_ungrab);       skui_snd_ungrab       = nullptr;
	sound_release   (skui_snd_tick);         skui_snd_tick         = nullptr;

	mesh_release    (skui_box_dbg);        skui_box_dbg        = nullptr;

	material_release(skui_mat);            skui_mat            = nullptr;
	material_release(skui_mat_dbg);        skui_mat_dbg        = nullptr;
	material_release(skui_font_mat);       skui_font_mat       = nullptr;

	sprite_release  (skui_toggle_off);     skui_toggle_off     = nullptr;
	sprite_release  (skui_toggle_on);      skui_toggle_on      = nullptr;
	sprite_release  (skui_radio_off);      skui_radio_off      = nullptr;
	sprite_release  (skui_radio_on);       skui_radio_on       = nullptr;
	sprite_release  (skui_arrow_left);     skui_arrow_left     = nullptr;
	sprite_release  (skui_arrow_right);    skui_arrow_right    = nullptr;
	sprite_release  (skui_arrow_up);       skui_arrow_up       = nullptr;
	sprite_release  (skui_arrow_down);     skui_arrow_down     = nullptr;
	sprite_release  (skui_spr_backspace);  skui_spr_backspace  = nullptr;
	sprite_release  (skui_spr_close);      skui_spr_close      = nullptr;
	sprite_release  (skui_spr_shift);      skui_spr_shift      = nullptr;
	sprite_release  (skui_spr_list);       skui_spr_list       = nullptr;
	sprite_release  (skui_spr_grid);       skui_spr_grid       = nullptr;

	font_release    (skui_font);           skui_font           = nullptr;
}

///////////////////////////////////////////

void ui_theming_update() {
	if (skui_active_sound_element_id == 0) return;

	// See if our current sound on/off pair is still from a valid ui element
	if (ui_id_active(skui_active_sound_element_id))
		return;
	// If the "on" sound instance is still playing, we don't want to stomp on
	// it with the off sound
	if (sound_inst_is_playing(skui_active_sound_inst))
		return;

	// Play it, and clean everything up, we're done here!
	if (skui_active_sound_off) {
		sound_play(skui_active_sound_off, skui_active_sound_pos, 1);
	}
	sound_release(skui_active_sound_off);
	skui_active_sound_off        = nullptr;
	skui_active_sound_inst       = {};
	skui_active_sound_element_id = 0;
}

///////////////////////////////////////////
// Element visuals                       //
///////////////////////////////////////////

void ui_set_element_visual(ui_vis_ element_visual, mesh_t mesh, material_t material, vec2 min_size) {
	if (element_visual >= skui_visuals.count) skui_visuals.add_empties(element_visual + 1 - skui_visuals.count);
	ui_el_visual_t *vis = &skui_visuals[element_visual];

	if (mesh          != nullptr) mesh_addref     (mesh);
	if (material      != nullptr) material_addref (material);
	if (vis->mesh     != nullptr) mesh_release    (vis->mesh);
	if (vis->material != nullptr) material_release(vis->material);

	vis->mesh     = mesh;
	vis->material = material;
	vis->min_size = min_size;
}

///////////////////////////////////////////

void ui_set_element_color(ui_vis_ element, ui_color_ color_category) {
	if (element        >= skui_visuals.count) skui_visuals.add_empties(element        + 1 - skui_visuals.count);
	if (color_category >= skui_palette.count) skui_palette.add_empties(color_category + 1 - skui_palette.count);

	skui_visuals[element].color_category = color_category;
}

///////////////////////////////////////////

void ui_set_element_sound(ui_vis_ element, sound_t activate, sound_t deactivate) {
	if (element >= skui_visuals.count) skui_visuals.add_empties(element + 1 - skui_visuals.count);

	if (activate   != nullptr) sound_addref(activate);
	if (deactivate != nullptr) sound_addref(deactivate);
	if (skui_visuals[element].snd_activate   != nullptr) sound_release(skui_visuals[element].snd_activate);
	if (skui_visuals[element].snd_deactivate != nullptr) sound_release(skui_visuals[element].snd_deactivate);

	skui_visuals[element].snd_activate   = activate;
	skui_visuals[element].snd_deactivate = deactivate;
}

///////////////////////////////////////////

mesh_t ui_get_mesh(ui_vis_ element_visual) {
	return element_visual < skui_visuals.count && skui_visuals[element_visual].mesh
		? skui_visuals[element_visual].mesh
		: skui_visuals[ui_vis_default].mesh;
}

///////////////////////////////////////////

vec2 ui_get_mesh_minsize(ui_vis_ element_visual) {
	return element_visual < skui_visuals.count && skui_visuals[element_visual].mesh
		? skui_visuals[element_visual].min_size
		: skui_visuals[ui_vis_default].min_size;
}

///////////////////////////////////////////

material_t ui_get_material(ui_vis_ element_visual) {
	return element_visual < skui_visuals.count && skui_visuals[element_visual].material
		? skui_visuals[element_visual].material
		: skui_visuals[ui_vis_default].material;
}

///////////////////////////////////////////

ui_color_ ui_get_color(ui_vis_ element_visual) {
	return element_visual < skui_visuals.count && skui_visuals[element_visual].color_category != ui_color_none
		? skui_visuals[element_visual].color_category
		: skui_visuals[ui_vis_default].color_category;
}

///////////////////////////////////////////

sound_t ui_get_sound_on(ui_vis_ element_visual) {
	return element_visual < skui_visuals.count && skui_visuals[element_visual].snd_activate
		? skui_visuals[element_visual].snd_activate
		: skui_visuals[ui_vis_default].snd_activate;
}

///////////////////////////////////////////

sound_t ui_get_sound_off(ui_vis_ element_visual) {
	return element_visual < skui_visuals.count && skui_visuals[element_visual].snd_deactivate
		? skui_visuals[element_visual].snd_deactivate
		: skui_visuals[ui_vis_default].snd_deactivate;
}

///////////////////////////////////////////

color128 ui_get_element_color(ui_vis_ element_visual, float focus) {
	ui_color_ color       = ui_get_color(element_visual);
	color128  final_color = ui_is_enabled()
		? color_lerp(skui_palette[color].normal, skui_palette[color].active, focus)
		: skui_palette[color].disabled;
	final_color   = final_color * skui_tint;
	final_color.a = focus;

	return final_color;
}

///////////////////////////////////////////

void ui_draw_element_color(ui_vis_ element_visual, ui_vis_ element_color, vec3 start, vec3 size, float focus) {
	/*if (size.x < skui_box_min.x) size.x = skui_box_min.x;
	if (size.y < skui_box_min.y) size.y = skui_box_min.y;
	if (size.z < skui_box_min.z) size.z = skui_box_min.z;*/

	render_add_mesh(
		ui_get_mesh    (element_visual),
		ui_get_material(element_visual),
		matrix_ts(start - size / 2, size),
		ui_get_element_color(element_color, focus));
}

///////////////////////////////////////////

void ui_draw_element(ui_vis_ element_visual, vec3 start, vec3 size, float focus) {
	render_add_mesh(
		ui_get_mesh    (element_visual),
		ui_get_material(element_visual),
		matrix_ts(start - size / 2, size),
		ui_get_element_color(element_visual, focus));
}

///////////////////////////////////////////

void ui_play_sound_on_off(ui_vis_ element_visual, id_hash_t element_id, vec3 at) {
	sound_t snd_on  = ui_get_sound_on(element_visual);
	sound_t snd_off = ui_get_sound_off(element_visual);

	if (snd_off) sound_addref(snd_off);
	sound_release(skui_active_sound_off);

	skui_active_sound_off        = snd_off;
	skui_active_sound_pos        = at;
	skui_active_sound_element_id = element_id;

	if (snd_on)
		skui_active_sound_inst = sound_play(snd_on, at, 1);
}

///////////////////////////////////////////

void ui_play_sound_on(ui_vis_ element_visual, vec3 at) {
	sound_t snd = ui_get_sound_on(element_visual);
	if (snd) sound_play(snd, at, 1);
}

///////////////////////////////////////////

void ui_play_sound_off(ui_vis_ element_visual, vec3 at) {
	sound_t snd = ui_get_sound_off(element_visual);
	if (snd) sound_play(snd, at, 1);
}

///////////////////////////////////////////

void ui_draw_cube(vec3 start, vec3 size, ui_color_ color, float focus) {
	vec3     center      = start - size / 2;
	color128 final_color = ui_is_enabled()
		? color_lerp(skui_palette[color].normal, skui_palette[color].active, focus)
		: skui_palette[color].disabled;
	final_color = final_color * skui_tint;

	render_add_mesh(skui_box_dbg, skui_mat, matrix_ts(center, size), final_color);
}

///////////////////////////////////////////
// Sizing and Settings                   //
///////////////////////////////////////////

void ui_settings(ui_settings_t settings) {
	if (settings.backplate_border == 0) settings.backplate_border = 0.5f * mm2m;
	if (settings.backplate_depth  == 0) settings.backplate_depth  = 0.4f;
	if (settings.depth            == 0) settings.depth   = 10 * mm2m;
	if (settings.gutter           == 0) settings.gutter  = 10 * mm2m;
	if (settings.padding          == 0) settings.padding = 10 * mm2m;
	if (settings.margin           == 0) settings.margin  = settings.padding;
	if (settings.rounding         == 0) settings.rounding= 7.5f * mm2m;

	bool rebuild_meshes = skui_settings.rounding != settings.rounding;
	skui_settings = settings;

	if (rebuild_meshes) {
		ui_theme_visuals_update();
	}
}

///////////////////////////////////////////
// Colors                                //
///////////////////////////////////////////

void ui_set_color(color128 color) {
	vec3 hsv = color_to_hsv(color);
	
	ui_set_theme_color(ui_color_primary,    color);                                                     // Primary color: Headers, separators, etc.
	ui_set_theme_color(ui_color_background, color_hsv(hsv.x, hsv.y * 0.2f,   hsv.z * 0.45f, color.a) ); // Dark color: body and backgrounds
	ui_set_theme_color(ui_color_common,     color_hsv(hsv.x, hsv.y * 0.075f, hsv.z * 0.65f, color.a) ); // Primary element color: buttons, sliders, etc.
	ui_set_theme_color(ui_color_complement, color_hsv(hsv.x, hsv.y * 0.2f,   hsv.z * 0.35f, color.a) ); // Complement color: unused so far?
	ui_set_theme_color(ui_color_text,       color128{1, 1, 1, 1});                                      // Text color
}

///////////////////////////////////////////

void ui_set_theme_color_state(ui_color_ color_type, ui_color_state_ state, color128 color_gamma) {
	if (color_type >= skui_palette.count) skui_palette.add_empties(color_type + 1 - skui_palette.count);

	color128 linear = color_to_linear(color_gamma);

	switch (state) {
	case ui_color_state_normal:   skui_palette[color_type].normal   = linear; break;
	case ui_color_state_active:   skui_palette[color_type].active   = linear; break;
	case ui_color_state_disabled: skui_palette[color_type].disabled = linear; break;
	}
}

///////////////////////////////////////////

color128 ui_get_theme_color_state(ui_color_ color_type, ui_color_state_ state) {
	if (color_type >= skui_palette.count) color_type = ui_color_none;

	switch (state) {
	case ui_color_state_normal:   return color_to_gamma(skui_palette[color_type].normal);
	case ui_color_state_active:   return color_to_gamma(skui_palette[color_type].active);
	case ui_color_state_disabled: return color_to_gamma(skui_palette[color_type].disabled);
	default: return { 1,1,1,1 };
	}
}

///////////////////////////////////////////

void ui_set_theme_color(ui_color_ color_type, color128 color_gamma) {
	if (color_type >= skui_palette.count) skui_palette.add_empties(color_type + 1 - skui_palette.count);

	color128 linear = color_to_linear(color_gamma);
	skui_palette[color_type].normal   = linear;
	skui_palette[color_type].active   = linear*color128{ 2,2,2,1 };
	skui_palette[color_type].disabled = linear*color128{ 0.5f,0.5f,0.5f,1 };
}

///////////////////////////////////////////

color128 ui_get_theme_color(ui_color_ color_type) {
	if (color_type >= skui_palette.count) color_type = ui_color_none;

	return color_to_gamma(skui_palette[color_type].normal);
}

///////////////////////////////////////////
// Style stack                           //
///////////////////////////////////////////

void ui_push_grab_aura(bool32_t enabled) {
	skui_grab_aura_stack.add(enabled);
}

///////////////////////////////////////////

void ui_pop_grab_aura() {
	if (skui_grab_aura_stack.count == 0) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "grab aura");
		return;
	}
	skui_grab_aura_stack.pop();
}

///////////////////////////////////////////

bool32_t ui_grab_aura_enabled() {
	return skui_grab_aura_stack.count > 0
		? skui_grab_aura_stack.last()
		: true;
}

///////////////////////////////////////////

void ui_push_text_style(text_style_t style) {
	skui_font_stack.add(style);
}

///////////////////////////////////////////

void ui_pop_text_style() {
	if (skui_font_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "text styles");
		return;
	}
	skui_font_stack.pop();
}

///////////////////////////////////////////

text_style_t ui_get_text_style() {
	return skui_font_stack.last();
}

///////////////////////////////////////////

void ui_push_tint(color128 tint_gamma) {
	skui_tint = color_to_linear(tint_gamma);
	skui_tint_stack.add(skui_tint);
}

///////////////////////////////////////////

void ui_pop_tint() {
	if (skui_tint_stack.count <= 1) {
		log_errf("Tried to pop too many %s! Do you have a push/pop mismatch?", "tints");
		return;
	}
	skui_tint_stack.pop();
	skui_tint = skui_tint_stack.last();
}

///////////////////////////////////////////
// Animation                             //
///////////////////////////////////////////

void ui_anim_start(id_hash_t id, int32_t channel) {
	if (skui_anim_id[channel] != id) {
		skui_anim_id[channel] = id;
		skui_anim_time[channel] = time_totalf_unscaled();
	}
}

///////////////////////////////////////////

void ui_anim_cancel(id_hash_t id, int32_t channel) {
	if (skui_anim_id[channel] == id) {
		skui_anim_id[channel] = 0;
		skui_anim_time[channel] = 0;
	}
}

///////////////////////////////////////////

bool ui_anim_has(id_hash_t id, int32_t channel, float duration) {
	if (id == skui_anim_id[channel]) {
		if ((time_totalf_unscaled() - skui_anim_time[channel]) < duration)
			return true;
		skui_anim_id[channel] = 0;
	}
	return false;
}

///////////////////////////////////////////

float ui_anim_elapsed(id_hash_t id, int32_t channel, float duration, float max) {
	return skui_anim_id[channel] == id ? fminf(max, (time_totalf_unscaled() - skui_anim_time[channel]) / duration) : 0;
}

///////////////////////////////////////////

float ui_anim_elapsed_total(id_hash_t id, int32_t channel) {
	return skui_anim_id[channel] == id ? (time_totalf_unscaled() - skui_anim_time[channel]) : 0;
}

///////////////////////////////////////////

float ui_get_anim_focus(id_hash_t id, button_state_ focus_state, button_state_ activation_state) {
	if (activation_state & button_state_just_active)
		ui_anim_start(id, 0);
	float anim = activation_state & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(id, 0, skui_anim_duration)) {
		float t = ui_anim_elapsed(id, 0, skui_anim_duration);
		anim = math_ease_overshoot(0, 1, skui_anim_overshoot, t);
	}
	return fmaxf(anim, (focus_state & button_state_active) * 0.5f); // !! This assumes button_state_active == 1 !!
}

///////////////////////////////////////////

float ui_get_anim_focus_inout(id_hash_t id, button_state_ activation_state) {
	const float delay = 0.2f;
	if (activation_state & button_state_just_active) ui_anim_start(id, 0);
	if (activation_state & button_state_just_inactive) {
		float elapsed = ui_anim_elapsed_total(id, 0);
		if (elapsed > delay || elapsed == 0) ui_anim_start(id, 1);
		ui_anim_cancel(id, 0);
	}

	float anim = activation_state & button_state_active ? 1.0f : 0.0f;
	if (ui_anim_has(id, 0, delay + skui_anim_focus_duration)) {
		anim = math_ease_smooth(0, 1, math_clamp((ui_anim_elapsed_total(id, 0)-delay)/ skui_anim_focus_duration, 0, 1));
	} else if (ui_anim_has(id, 1, skui_anim_focus_duration)) {
		anim = math_ease_smooth(1, 0, ui_anim_elapsed(id, 1, skui_anim_focus_duration));
	}
	return anim;
}

///////////////////////////////////////////
// Asset generation                      //
///////////////////////////////////////////

void ui_quadrant_size_verts(vert_t *verts, int32_t count, float overflow) {
	float left      =  FLT_MAX;
	float left_in   = -FLT_MAX;
	float right     = -FLT_MAX;
	float right_in  =  FLT_MAX;
	float top       = -FLT_MAX;
	float top_in    =  FLT_MAX;
	float bottom    =  FLT_MAX;
	float bottom_in = -FLT_MAX;
	float depth     = 0;
	for (int32_t i = 0; i < count; i++) {
		if (verts[i].pos.x < left                           ) left       = verts[i].pos.x;
		if (verts[i].pos.x < 0 && verts[i].pos.x > left_in  ) left_in    = verts[i].pos.x;
		if (verts[i].pos.x > right                          ) right      = verts[i].pos.x;
		if (verts[i].pos.x > 0 && verts[i].pos.x < right_in ) right_in   = verts[i].pos.x;
		if (verts[i].pos.y < bottom                         ) bottom     = verts[i].pos.y;
		if (verts[i].pos.y < 0 && verts[i].pos.y > bottom_in) bottom_in  = verts[i].pos.y;
		if (verts[i].pos.y > top                            ) top        = verts[i].pos.y;
		if (verts[i].pos.y > 0 && verts[i].pos.y < top_in   ) top_in     = verts[i].pos.y;
		if (verts[i].pos.z < depth                          ) depth      = verts[i].pos.z;
	}

	left   = math_lerp(left,   left_in,   overflow);
	right  = math_lerp(right,  right_in,  overflow);
	bottom = math_lerp(bottom, bottom_in, overflow);
	top    = math_lerp(top,    top_in,    overflow);
	depth  = fabsf(depth * 2);
	if (depth == 0) depth = 1;

	for (int32_t i = 0; i < count; i++) {
		float quadrant_x = verts[i].pos.x / fabsf(verts[i].pos.x);
		float quadrant_y = verts[i].pos.y / fabsf(verts[i].pos.y);
		if (verts[i].pos.x == 0) quadrant_x = 0;
		if (verts[i].pos.y == 0) quadrant_y = 0;

		verts[i].uv = vec2{ quadrant_x, quadrant_y };
		if      (quadrant_x < 0) verts[i].pos.x -= left;
		else if (quadrant_x > 0) verts[i].pos.x -= right;
		if      (quadrant_y < 0) verts[i].pos.y -= bottom;
		else if (quadrant_y > 0) verts[i].pos.y -= top;
		verts[i].pos.z = verts[i].pos.z / depth;
	}
}

///////////////////////////////////////////

void ui_quadrant_size_mesh(mesh_t ref_mesh, float overflow) {
	vert_t *verts      = nullptr;
	int32_t vert_count = 0;
	mesh_get_verts        (ref_mesh, verts, vert_count, memory_reference);
	ui_quadrant_size_verts(verts, vert_count, overflow);
	mesh_set_verts        (ref_mesh, verts, vert_count);
}

///////////////////////////////////////////

void ui_default_aura_mesh(mesh_t *mesh, float tube_diameter, float corner_radius, float shrink, int32_t quadrant_slices, int32_t tube_corners) {
	if (*mesh == nullptr) {
		*mesh = mesh_create();
	}

	int32_t vert_count = quadrant_slices * tube_corners * 4;
	int32_t ind_count  = quadrant_slices * tube_corners * 6 * 4;
	vert_t *verts      = sk_malloc_t(vert_t, vert_count);
	vind_t *inds       = sk_malloc_t(vind_t, ind_count );

	vind_t ind    = 0;
	vind_t steps  = quadrant_slices * 4;
	float  radius = tube_diameter / 2;
	for (vind_t i = 0; i < steps; i++) {
		vind_t off  = i * tube_corners;
		vind_t offn = ((i + 1) % steps) * tube_corners;

		// Manual quadrantification, due to our perfect corners
		int32_t quad = i / quadrant_slices;
		float   quad_pct = (i - quad * quadrant_slices) / (float)(quadrant_slices - 1);

		vec2 uv = {};
		if      (quad == 0) uv = vec2{  1, 1 };
		else if (quad == 1) uv = vec2{ -1, 1 };
		else if (quad == 2) uv = vec2{ -1,-1 };
		else if (quad == 3) uv = vec2{  1,-1 }; 
		vec3 quad_off = vec3{ uv.x, uv.y,0 } * -(corner_radius+shrink);

		float corner_ang  = (quad+quad_pct) * MATH_PI * 0.5f;
		vec3  corner_norm = vec3{ cosf(corner_ang), sinf(corner_ang), 0 };
		vec3  corner_pt   = quad_off + corner_norm * corner_radius;

		for (vind_t c = 0; c < (vind_t)tube_corners; c++) {
			float tube_ang  = ((float)c / tube_corners) * MATH_PI * 2;
			float tube_x    = cosf(tube_ang);
			float tube_y    = sinf(tube_ang);
			vec3  tube_norm = vec3_normalize( vec3{ corner_norm.x * tube_x, corner_norm.y * tube_x, tube_y } );
			vec3  tube_pt   = corner_pt + tube_norm*radius;

			verts[off + c] = { tube_pt, tube_norm, uv, {255,255,255,255} };

			vind_t cn   = (c + 1) % tube_corners;
			inds[ind++] = off  + c;
			inds[ind++] = offn + c;
			inds[ind++] = offn + cn;
			inds[ind++] = off  + c;
			inds[ind++] = offn + cn;
			inds[ind++] = off  + cn;
		}
	}
	mesh_set_data(*mesh, verts, vert_count, inds, ind_count);

	sk_free(verts);
	sk_free(inds);
}

///////////////////////////////////////////

uint32_t _lathe_corner_root_index(uint32_t corner, ui_corner_ rounded_corners, uint32_t corner_resolution, uint32_t lathe_points, uint32_t lathe_step) {
	uint32_t roundedCount = (uint32_t)(
		(corner > 0 && (rounded_corners & ui_corner_top_left    ) != 0 ? 1 : 0) +
		(corner > 1 && (rounded_corners & ui_corner_top_right   ) != 0 ? 1 : 0) +
		(corner > 2 && (rounded_corners & ui_corner_bottom_right) != 0 ? 1 : 0) +
		(corner > 3 && (rounded_corners & ui_corner_bottom_left ) != 0 ? 1 : 0));
	uint32_t sharpCount = corner-roundedCount;
	return (roundedCount * corner_resolution + sharpCount) * lathe_step + corner * (lathe_points - lathe_step);
}

///////////////////////////////////////////

void _ui_gen_quadrant_mesh(mesh_t *mesh, ui_corner_ rounded_corners, float corner_radius, uint32_t corner_resolution, bool32_t delete_flat_sides, bool32_t quadrantified, const ui_lathe_pt_t* lathe_pts, int32_t lathe_pt_count) {
	float     angle_step = 90 / (float)(corner_resolution - 1);
	uint32_t  lathe_step = 0;
	for (size_t i = 0; i < lathe_pt_count; i++)
		if (lathe_pts[i].pt.x != 0) lathe_step += 1;
	uint32_t  lathe_roots = lathe_pt_count - lathe_step;

	array_t<vert_t> verts = {};
	array_t<vind_t> inds  = {};
	vind_t  curr_inds[6];
	int32_t curr_ind_ct = 0;

	for (uint32_t c = 0; c < 4; c++) {
		uint32_t idx_root_curr = _lathe_corner_root_index( c,      rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step);
		uint32_t idx_root_next = _lathe_corner_root_index((c+1)%4, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step);

		bool rounded      = (rounded_corners & (ui_corner_)(1 << (int32_t)c)) != 0;
		bool rounded_next = (rounded_corners & (ui_corner_)(1 << (int32_t)((c + 1) % 4))) != 0;
		bool rounded_prev = (rounded_corners & (ui_corner_)(1 << (int32_t)((c+3)%4))) != 0;
		// 1,1   -1,1   -1,-1   1,-1
		float u = c == 0 || c == 3 ? 1.f : -1.f;
		float v = c == 0 || c == 1 ? 1.f : -1.f;

		vec3     offset       = quadrantified > 0 ? vec3{ -u * corner_radius, -v * corner_radius, 0 } : vec3{ 0,0,0 };
		uint32_t corner_count = corner_resolution;
		float    angle_start  = c * 90.f;
		float    curr_radius  = corner_radius;
		if (delete_flat_sides && rounded == false && (rounded_next == false || rounded_prev == false)) {
			corner_count = 1;
			if (rounded_prev == true) {
				float ang = (angle_start + 90) * deg2rad;
				offset += vec3{cosf(ang), sinf(ang), 0} * corner_radius;
			} else if (rounded_prev == false) {
				float ang = angle_start * deg2rad;
				offset += vec3{ cosf(ang), sinf(ang), 0 } * corner_radius;
				angle_start += 90;
			}
		} else if (rounded == false) {
			curr_radius  *= 1.41421356237f; // sqrt of 2
			corner_count  = 1;
			angle_start  += 45;
		}

		for (uint32_t s = 0; s < corner_count; s++) {
			float ang = (angle_start + s*angle_step) * deg2rad;
			vec2  dir = vec2{ cosf(ang), sinf(ang) };

			int32_t p_ct = -1;
			for (uint32_t p = 0; p < (uint32_t)lathe_pt_count; p++) {
				ui_lathe_pt_t lp = lathe_pts[p];
				vert_t        vert;
				vert.col  = lp.color;
				vert.uv   = { u, v };
				vert.norm = { lp.normal.x * dir.x, lp.normal.x * dir.y, lp.normal.y * -1 };
				vert.pos  = vec3{ lp.pt.x * dir.x * curr_radius, lp.pt.x * dir.y * curr_radius, lp.pt.y } + offset;

				bool is_root = lp.pt.x == 0;
				if (is_root == false || s==0) // If it's a root vert, it only needs added once
					verts.add(vert);

				if (is_root == false) p_ct += 1;

				if (lp.connect_next == false || p+1 == (uint32_t)lathe_pt_count || (s+1==corner_count && delete_flat_sides && rounded == false && rounded_next == false)) continue;

				bool     top_is_root   = lathe_pts[p + 1].pt.x == 0;
				bool     next_corner   = s + 1 >= corner_count;
				uint32_t next_root_idx = next_corner ? idx_root_next : idx_root_curr;
				uint32_t curr_bot = is_root     || s == 0      ? idx_root_curr + p   : idx_root_curr +  s   *lathe_step + lathe_roots + p_ct;
				uint32_t next_bot = is_root     || next_corner ? next_root_idx + p   : next_root_idx + (s+1)*lathe_step + lathe_roots + p_ct;
				uint32_t curr_top = top_is_root || s == 0      ? idx_root_curr + p+1 : idx_root_curr +  s   *lathe_step + lathe_roots + p_ct+1;
				uint32_t next_top = top_is_root || next_corner ? next_root_idx + p+1 : next_root_idx + (s+1)*lathe_step + lathe_roots + p_ct+1;

				if (is_root && !top_is_root && !next_corner) {
					curr_inds[0] = next_top;
					curr_inds[1] = curr_top;
					curr_inds[2] = curr_bot;
					curr_ind_ct = 3;
				} else if (!is_root && top_is_root && !next_corner) {
					curr_inds[2] = next_bot;
					curr_inds[1] = curr_bot;
					curr_inds[0] = curr_top;
					curr_ind_ct = 3;
				} else {
					curr_inds[0] = next_top;
					curr_inds[1] = curr_top;
					curr_inds[2] = curr_bot;

					curr_inds[3] = next_bot;
					curr_inds[4] = next_top;
					curr_inds[5] = curr_bot;
					curr_ind_ct = 6;
				}
				// add the indices, forwards or backwards depending on if this should be flipped
				if (lp.flip_face == false) inds.add_range(curr_inds, curr_ind_ct);
				else for (int32_t i = curr_ind_ct-1; i >= 0; i--) inds.add(curr_inds[i]);
			}
		}
	}

	// Center quad
	{
		uint32_t tr_f = _lathe_corner_root_index(0, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step);
		uint32_t tl_f = _lathe_corner_root_index(1, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step);
		uint32_t bl_f = _lathe_corner_root_index(2, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step);
		uint32_t br_f = _lathe_corner_root_index(3, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step);

		inds.add(tl_f); inds.add(tr_f); inds.add(br_f);
		inds.add(tl_f); inds.add(br_f); inds.add(bl_f);
	}

	if (lathe_pts[lathe_pt_count-1].connect_next) {
		uint32_t tr_f = _lathe_corner_root_index(0, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step) + (uint32_t)(lathe_pt_count-1);
		uint32_t tl_f = _lathe_corner_root_index(1, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step) + (uint32_t)(lathe_pt_count-1);
		uint32_t bl_f = _lathe_corner_root_index(2, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step) + (uint32_t)(lathe_pt_count-1);
		uint32_t br_f = _lathe_corner_root_index(3, rounded_corners, corner_resolution, (uint32_t)lathe_pt_count, lathe_step) + (uint32_t)(lathe_pt_count-1);

		curr_inds[0] = br_f;
		curr_inds[1] = tr_f;
		curr_inds[2] = tl_f;
		curr_inds[3] = bl_f;
		curr_inds[4] = br_f;
		curr_inds[5] = tl_f;
		curr_ind_ct = 6;

		// add the indices, forwards or backwards depending on if this should be flipped
		if (lathe_pts[lathe_pt_count - 1].flip_face == false) inds.add_range(curr_inds, curr_ind_ct);
		else for (int32_t i = curr_ind_ct - 1; i >= 0; i--) inds.add(curr_inds[i]);
	}

	if (*mesh == nullptr)
		*mesh = mesh_create();
	mesh_set_data(*mesh, verts.data, verts.count, inds.data, inds.count);
	verts.free();
	inds .free();
}

///////////////////////////////////////////

mesh_t ui_gen_quadrant_mesh(ui_corner_ rounded_corners, float corner_radius, uint32_t corner_resolution, bool32_t delete_flat_sides, bool32_t quadrantified, const ui_lathe_pt_t* lathe_pts, int32_t lathe_pt_count) {
	mesh_t result = nullptr;
	_ui_gen_quadrant_mesh(&result, rounded_corners, corner_radius, corner_resolution, delete_flat_sides, quadrantified, lathe_pts, lathe_pt_count);
	return result;
}

///////////////////////////////////////////

mesh_t theme_mesh_button       = nullptr;
mesh_t theme_mesh_button_round = nullptr;
mesh_t theme_mesh_input        = nullptr;
mesh_t theme_mesh_plane        = nullptr;
mesh_t theme_mesh_panel        = nullptr;
mesh_t theme_mesh_panel_top    = nullptr;
mesh_t theme_mesh_panel_bot    = nullptr;
mesh_t theme_mesh_slider       = nullptr;
mesh_t theme_mesh_slider_left  = nullptr;
mesh_t theme_mesh_slider_right = nullptr;
mesh_t theme_mesh_slider_pinch = nullptr;
mesh_t theme_mesh_slider_push  = nullptr;
mesh_t theme_mesh_separator    = nullptr;
mesh_t theme_mesh_aura         = nullptr;
mesh_t theme_mesh_carat        = nullptr;

material_t theme_mat_opaque          = nullptr;
material_t theme_mat_opaque_same_z   = nullptr;
material_t theme_mat_transparent     = nullptr;
material_t theme_mat_transparent_noq = nullptr;
material_t theme_mat_aura            = nullptr;
material_t theme_mat_carat           = nullptr;

void ui_theme_visuals_update() {
	color32 white_corner = {255,255,255,255};
	color32 white        = {255,255,255,255};
	color32 black        = {0,0,0,0};
	color32 gray         = {200, 200, 200, 255};
	color32 shadow_center = {0, 0, 0, 200};
	color32 shadow_edge   = {0, 0, 0, 50 };
	const ui_lathe_pt_t lathe_button[] = {
		{ {0,    -0.5f},  {0, 1}, white,         true  },
		{ {1,    -0.5f},  {0, 1}, white,         false },
		{ {1,    -0.5f},  {1, 0}, white,         true  },
		{ {1,    -0.1f},  {1, 0}, white,         false },
		{ {1.2f,  0.49f}, {0, 1}, black,         true, true },
		{ {0.0f,  0.49f}, {0, 1}, shadow_center, true, true } };
	const ui_lathe_pt_t lathe_input[] = {
		{ {0,    -0.1f }, { 0, 1}, gray,          true  },
		{ {0.8f, -0.1f }, { 0, 1}, gray,          false },
		{ {0.8f, -0.1f }, {-1, 0}, gray,          true  },
		{ {0.8f, -0.5f }, {-1, 0}, white,         false },
		{ {0.8f, -0.5f }, { 0, 1}, white,         true  },
		{ {1,    -0.5f }, { 0, 1}, white,         false },
		{ {1,    -0.5f }, { 1, 0}, white,         true  },
		{ {1,    -0.1f }, { 1, 0}, white,         false },
		{ {1.2f,  0.49f}, { 0, 1}, black,         true, true },
		{ {0,     0.49f}, { 0, 1}, shadow_center, true, true }, };
	const ui_lathe_pt_t lathe_plane[] = {
		{ {0, 0}, {0, 1}, white, true  },
		{ {1, 0}, {0, 1}, white, false } };
	const ui_lathe_pt_t lathe_panel[] = {
		{ {0, -0.5f}, {0, 1}, white, true  },
		{ {1, -0.5f}, {0, 1}, white, false },
		{ {1, -0.5f}, {1, 0}, white, true  },
		{ {1,  0.5f}, {1, 0}, white, false },
		{ {1,  0.5f}, {0,-1}, white, true  },
		{ {0,  0.5f}, {0,-1}, white, true  } };
	const ui_lathe_pt_t lathe_slider[] = {
		{ {0,    -0.5f},  {0, 1}, white,       true  },
		{ {1,    -0.5f},  {0, 1}, white,       false },
		{ {1,    -0.5f},  {1, 0}, white,       true  },
		{ {1,     0.5f},  {1, 0}, white_corner,false },
		{ {1,     0.49f}, {0, 1}, shadow_edge, true  },
		{ {2.0f,  0.49f}, {0, 1}, black,       false } };
	const ui_lathe_pt_t lathe_slider_btn[] = {
		{ {0,    -0.5f},  {0, 1}, white,       true  },
		{ {0.8f, -0.5f},  {0, 1}, white,       true  },
		{ {1,    -0.4f},  {1, 0}, white,       true  },
		{ {1,     0.5f},  {1, 0}, white_corner,false },
		{ {1,     0.49f}, {0, 1}, shadow_edge, true  },
		{ {2.0f,  0.49f}, {0, 1}, black,       false } };

	bool needs_id = theme_mesh_button == nullptr;

	float button_rounding = fminf(ui_line_height() * 0.5f, skui_settings.rounding);
	_ui_gen_quadrant_mesh(&theme_mesh_button_round, ui_corner_all, 0.5f,            8, false, false, lathe_button, _countof(lathe_button));
	_ui_gen_quadrant_mesh(&theme_mesh_button,       ui_corner_all, button_rounding, 8, false, true,  lathe_button, _countof(lathe_button));
	_ui_gen_quadrant_mesh(&theme_mesh_input,        ui_corner_all, button_rounding, 8, false, true,  lathe_input,  _countof(lathe_input ));
	_ui_gen_quadrant_mesh(&theme_mesh_plane,        ui_corner_all, skui_settings.rounding - (skui_settings.margin - skui_settings.gutter), 8, false, true, lathe_plane, _countof(lathe_plane));
	
	float panel_rounding = fminf(ui_line_height(), skui_settings.rounding);
	_ui_gen_quadrant_mesh(&theme_mesh_panel,        ui_corner_all,    panel_rounding, 8, false, true, lathe_panel, _countof(lathe_panel));
	_ui_gen_quadrant_mesh(&theme_mesh_panel_top,    ui_corner_top,    panel_rounding, 8, true,  true, lathe_panel, _countof(lathe_panel));
	_ui_gen_quadrant_mesh(&theme_mesh_panel_bot,    ui_corner_bottom, panel_rounding, 8, true,  true, lathe_panel, _countof(lathe_panel));

	float slider_rounding = fminf(fmaxf(skui_settings.padding, ui_line_height() / 6.f) / 2.f, skui_settings.rounding * 0.65f);
	_ui_gen_quadrant_mesh(&theme_mesh_slider,       ui_corner_all,   slider_rounding, 5, false, true, lathe_slider, _countof(lathe_slider));
	_ui_gen_quadrant_mesh(&theme_mesh_slider_left,  ui_corner_left,  slider_rounding, 5, true,  true, lathe_slider, _countof(lathe_slider));
	_ui_gen_quadrant_mesh(&theme_mesh_slider_right, ui_corner_right, slider_rounding, 5, true,  true, lathe_slider, _countof(lathe_slider));

	_ui_gen_quadrant_mesh(&theme_mesh_slider_pinch, ui_corner_all, fminf((ui_line_height() * 0.5f)/2.f, skui_settings.rounding), 5, false, true, lathe_slider_btn, _countof(lathe_slider_btn));
	_ui_gen_quadrant_mesh(&theme_mesh_slider_push,  ui_corner_all, fminf((ui_line_height() * 0.55f)/2.f, skui_settings.rounding), 5, false, true, lathe_slider_btn, _countof(lathe_slider_btn));
	_ui_gen_quadrant_mesh(&theme_mesh_separator,    ui_corner_all, fminf((text_style_get_baseline(ui_get_text_style()) * 0.4f)/2.f, skui_settings.rounding * 0.1f),  3, false, true, lathe_slider, _countof(lathe_slider));
	
	float aura_mesh_radius = skui_aura_radius * 0.75f;
	ui_default_aura_mesh(&theme_mesh_aura, 0, fminf(ui_line_height(), skui_settings.rounding) + aura_mesh_radius, skui_aura_radius - aura_mesh_radius, 7, 5);

	if (theme_mesh_carat == nullptr) theme_mesh_carat = mesh_find(default_id_mesh_cube);

	if (theme_mat_transparent == nullptr) {
		theme_mat_transparent = material_copy_id(default_id_material_ui_quadrant);
		material_set_id          (theme_mat_transparent, "sk/ui/mat_transparent");
		material_set_transparency(theme_mat_transparent, transparency_blend);
		material_set_depth_test  (theme_mat_transparent, depth_test_less_or_eq);
		material_set_queue_offset(theme_mat_transparent, -20);

		theme_mat_transparent_noq = material_copy_id(default_id_material_ui);
		material_set_id          (theme_mat_transparent_noq, "sk/ui/mat_transparent_noq");
		material_set_transparency(theme_mat_transparent_noq, transparency_blend);
		material_set_depth_test  (theme_mat_transparent_noq, depth_test_less_or_eq);
		material_set_queue_offset(theme_mat_transparent_noq, -20);

		theme_mat_opaque = material_copy_id(default_id_material_ui_quadrant);
		material_set_id          (theme_mat_opaque, "sk/ui/mat_opaque");

		theme_mat_opaque_same_z = material_copy_id(default_id_material_ui_quadrant);
		material_set_id          (theme_mat_opaque_same_z, "sk/ui/mat_opaque_same_z");
		material_set_depth_test  (theme_mat_opaque_same_z, depth_test_less_or_eq);
		material_set_queue_offset(theme_mat_opaque_same_z, -10);

		theme_mat_aura  = material_find(default_id_material_ui_aura);
		theme_mat_carat = material_find(default_id_material_unlit);
	}

	if (needs_id) {
		mesh_set_id(theme_mesh_button_round, "sk/ui/mesh_button_round");
		mesh_set_id(theme_mesh_button,       "sk/ui/mesh_button");
		mesh_set_id(theme_mesh_input,        "sk/ui/mesh_input");
		mesh_set_id(theme_mesh_plane,        "sk/ui/mesh_plane");
		mesh_set_id(theme_mesh_panel,        "sk/ui/mesh_panel");
		mesh_set_id(theme_mesh_panel_top,    "sk/ui/mesh_panel_top");
		mesh_set_id(theme_mesh_panel_bot,    "sk/ui/mesh_panel_bot");
		mesh_set_id(theme_mesh_slider,       "sk/ui/mesh_slider");
		mesh_set_id(theme_mesh_slider_left,  "sk/ui/mesh_slider_left");
		mesh_set_id(theme_mesh_slider_right, "sk/ui/mesh_slider_right");
		mesh_set_id(theme_mesh_slider_pinch, "sk/ui/mesh_slider_pinch");
		mesh_set_id(theme_mesh_slider_push,  "sk/ui/mesh_slider_push");
		mesh_set_id(theme_mesh_separator,    "sk/ui/mesh_separator");
	}
}

///////////////////////////////////////////

void ui_theme_visuals_assign() {
	ui_set_element_visual(ui_vis_default,              theme_mesh_panel,        theme_mat_opaque);
	ui_set_element_visual(ui_vis_button,               theme_mesh_button,       theme_mat_transparent);
	ui_set_element_visual(ui_vis_toggle,               theme_mesh_button,       theme_mat_transparent);
	ui_set_element_visual(ui_vis_button_round,         theme_mesh_button_round, theme_mat_transparent_noq);
	ui_set_element_visual(ui_vis_input,                theme_mesh_input,        theme_mat_transparent);
	ui_set_element_visual(ui_vis_panel,                theme_mesh_plane,        theme_mat_opaque_same_z);
	ui_set_element_visual(ui_vis_window_head,          theme_mesh_panel_top,    theme_mat_opaque);
	ui_set_element_visual(ui_vis_window_body,          theme_mesh_panel_bot,    theme_mat_opaque);
	ui_set_element_visual(ui_vis_window_body_only,     theme_mesh_panel,        theme_mat_opaque);
	ui_set_element_visual(ui_vis_window_head_only,     theme_mesh_panel,        theme_mat_opaque);
	ui_set_element_visual(ui_vis_slider_line,          theme_mesh_slider,       theme_mat_transparent, {skui_settings.rounding * 0.35f, skui_settings.rounding * 0.35f});
	ui_set_element_visual(ui_vis_slider_line_active,   theme_mesh_slider_left,  theme_mat_transparent, {skui_settings.rounding * 0.35f, skui_settings.rounding * 0.35f});
	ui_set_element_visual(ui_vis_slider_line_inactive, theme_mesh_slider_right, theme_mat_transparent, {skui_settings.rounding * 0.35f, skui_settings.rounding * 0.35f});
	ui_set_element_visual(ui_vis_slider_pinch,         theme_mesh_slider_pinch, theme_mat_transparent);
	ui_set_element_visual(ui_vis_slider_push,          theme_mesh_slider_push,  theme_mat_transparent);
	ui_set_element_visual(ui_vis_separator,            theme_mesh_separator,    theme_mat_transparent);
	ui_set_element_visual(ui_vis_aura,                 theme_mesh_aura,         theme_mat_aura);
	ui_set_element_visual(ui_vis_carat,                theme_mesh_carat,        theme_mat_carat);
}

///////////////////////////////////////////

void ui_theme_visuals_release() {
	mesh_release(theme_mesh_button_round); theme_mesh_button_round  = nullptr;
	mesh_release(theme_mesh_button      ); theme_mesh_button       = nullptr;
	mesh_release(theme_mesh_input       ); theme_mesh_input        = nullptr;
	mesh_release(theme_mesh_plane       ); theme_mesh_plane        = nullptr;
	mesh_release(theme_mesh_panel       ); theme_mesh_panel        = nullptr;
	mesh_release(theme_mesh_panel_top   ); theme_mesh_panel_top    = nullptr;
	mesh_release(theme_mesh_panel_bot   ); theme_mesh_panel_bot    = nullptr;
	mesh_release(theme_mesh_slider      ); theme_mesh_slider       = nullptr;
	mesh_release(theme_mesh_slider_left ); theme_mesh_slider_left  = nullptr;
	mesh_release(theme_mesh_slider_right); theme_mesh_slider_right = nullptr;
	mesh_release(theme_mesh_slider_pinch); theme_mesh_slider_pinch = nullptr;
	mesh_release(theme_mesh_slider_push ); theme_mesh_slider_push  = nullptr;
	mesh_release(theme_mesh_separator   ); theme_mesh_separator    = nullptr;
	mesh_release(theme_mesh_aura        ); theme_mesh_aura         = nullptr;
	mesh_release(theme_mesh_carat       ); theme_mesh_carat        = nullptr;

	material_release(theme_mat_opaque         ); theme_mat_opaque          = nullptr;
	material_release(theme_mat_opaque_same_z  ); theme_mat_opaque_same_z   = nullptr;
	material_release(theme_mat_transparent    ); theme_mat_transparent     = nullptr;
	material_release(theme_mat_transparent_noq); theme_mat_transparent_noq = nullptr;
	material_release(theme_mat_aura           ); theme_mat_aura            = nullptr;
	material_release(theme_mat_carat          ); theme_mat_carat           = nullptr;
}

}