#include "ui_core.h"
#include "ui_layout.h"

#include "../libraries/array.h"
#include "../utils/sdf.h"
#include "../sk_math.h"

#include <float.h>

namespace sk {

///////////////////////////////////////////

struct ui_el_visual_t {
	mesh_t     mesh;
	material_t material;
	vec2       min_size;
};

struct ui_theme_color_t {
	color128 normal;
	color128 active;
	color128 disabled;
};

///////////////////////////////////////////

font_t          skui_font;

ui_el_visual_t  skui_visuals[ui_vis_max];
mesh_t          skui_box_top;
mesh_t          skui_box_bot;
mesh_t          skui_box;
vec2            skui_box_min;
mesh_t          skui_small_left;
mesh_t          skui_small_right;
mesh_t          skui_small;
vec2            skui_small_min;
mesh_t          skui_cylinder;

material_t      skui_mat;
material_t      skui_mat_quad;
material_t      skui_mat_dbg;
material_t      skui_font_mat;

sprite_t        skui_toggle_off;
sprite_t        skui_toggle_on;
sprite_t        skui_radio_off;
sprite_t        skui_radio_on;

sound_t         skui_snd_interact;
sound_t         skui_snd_uninteract;
sound_t         skui_snd_grab;
sound_t         skui_snd_ungrab;
sound_t         skui_snd_tick;

mesh_t          skui_box_dbg;

uint64_t        skui_anim_id[3];
float           skui_anim_time[3];

ui_theme_color_t skui_palette[ui_color_max];
color128         skui_tint;

array_t<text_style_t> skui_font_stack;
array_t<color128>     skui_tint_stack;

sound_t       skui_active_sound_off        = nullptr;
sound_inst_t  skui_active_sound_inst       = {};
vec3          skui_active_sound_pos        = vec3_zero;
uint64_t      skui_active_sound_element_id = 0;

///////////////////////////////////////////

void ui_default_mesh     (mesh_t* mesh, bool quadrantify, float diameter, float rounding, int32_t quadrant_slices);
void ui_default_mesh_half(mesh_t* mesh, bool quadrantify, float diameter, float rounding, int32_t quadrant_slices, float angle_start);
void ui_default_aura_mesh(mesh_t* mesh,                   float diameter, float rounding, int32_t quadrant_slices, int32_t tube_corners);

///////////////////////////////////////////

void ui_theming_init() {
	skui_tint       = { 1,1,1,1 };
	skui_font_stack = {};
	skui_tint_stack = {};
	memset(skui_visuals,   0, sizeof(skui_visuals));
	memset(skui_anim_id,   0, sizeof(skui_anim_id));
	memset(skui_anim_time, 0, sizeof(skui_anim_time));

	ui_set_color(color_hsv(0.07f, 0.5f, 0.75f, 1));

	skui_font_mat   = material_find(default_id_material_font);
	material_set_queue_offset(skui_font_mat, -12);
	skui_font       = font_find(default_id_font);
	skui_font_stack.add( text_make_style_mat(skui_font, 10*mm2m, skui_font_mat, color_to_gamma( skui_palette[ui_color_text].normal )) );

	// TODO: v0.4, this sets up default values when zeroed out, with a
	// ui_get_settings, this isn't really necessary anymore!
	ui_settings_t settings = {};
	ui_settings(settings);

	ui_default_mesh(&skui_cylinder, false, 1, 4 * cm2m, 5);
	mesh_set_id(skui_box_bot, "sk/ui/cylinder_mesh");

	// Create default sprites for the toggles

	// #defines because we can't capture anything in these lambdas D:
	#define OUTER_RADIUS 31
	#define RING_WIDTH 8
	#define INNER_RADIUS (OUTER_RADIUS - RING_WIDTH*2)

	tex_t toggle_tex_on = sdf_create_tex(64, 64, [](vec2 pt) {
		return
			sdf_union(
				sdf_box(pt, INNER_RADIUS),
				sdf_subtract(
					sdf_box_round(pt, OUTER_RADIUS - RING_WIDTH, 4),
					sdf_box_round(pt, OUTER_RADIUS, 8)))/32.0f;
	}, 40);
	tex_set_address(toggle_tex_on, tex_address_clamp);
	tex_set_id     (toggle_tex_on, "sk/ui/toggle_on_tex");
	tex_t toggle_tex_off = sdf_create_tex(64, 64, [](vec2 pt) {
		return
			sdf_subtract(
				sdf_box_round(pt, OUTER_RADIUS - RING_WIDTH, 4),
				sdf_box_round(pt, OUTER_RADIUS, 8))/32.0f;
	}, 40);
	tex_set_address(toggle_tex_off, tex_address_clamp);
	tex_set_id     (toggle_tex_off, "sk/ui/toggle_off_tex");
	tex_t radio_tex_on = sdf_create_tex(64, 64, [](vec2 pt) {
		float dist = vec2_magnitude(pt);
		return
			sdf_union(
				dist - INNER_RADIUS,
				sdf_subtract(
					dist - (OUTER_RADIUS - RING_WIDTH),
					dist - OUTER_RADIUS)) / 32.0f;
	}, 40);
	tex_set_address(radio_tex_on, tex_address_clamp);
	tex_set_id     (radio_tex_on, "sk/ui/radio_on_tex");
	tex_t radio_tex_off = sdf_create_tex(64, 64, [](vec2 pt) {
		float dist = vec2_magnitude(pt);
		return sdf_subtract(dist - (OUTER_RADIUS - RING_WIDTH), dist - OUTER_RADIUS)/32.0f;
	}, 40);
	tex_set_address(radio_tex_off, tex_address_clamp);
	tex_set_id     (radio_tex_off, "sk/ui/radio_off_tex");
	skui_toggle_on  = sprite_create(toggle_tex_on,  sprite_type_single);
	skui_toggle_off = sprite_create(toggle_tex_off, sprite_type_single);
	skui_radio_on   = sprite_create(radio_tex_on,   sprite_type_single);
	skui_radio_off  = sprite_create(radio_tex_off,  sprite_type_single);
	sprite_set_id(skui_toggle_on,  ui_default_id_toggle_on_spr);
	sprite_set_id(skui_toggle_off, ui_default_id_toggle_off_spr);
	sprite_set_id(skui_radio_on,   ui_default_id_radio_on_spr);
	sprite_set_id(skui_radio_off,  ui_default_id_radio_off_spr);
	tex_release(toggle_tex_on);
	tex_release(toggle_tex_off);
	tex_release(radio_tex_on);
	tex_release(radio_tex_off);

	// Create a sound for the HSlider
	skui_snd_tick = sound_generate([](float t){
		float x     = t / 0.03f;
		float band1 = sinf(t*8000) * (x * powf(1 - x, 10)) / 0.03f;
		float band2 = sinf(t*6550) * (x * powf(1 - x, 12)) / 0.03f;

		return (band1*0.6f + band2*0.4f) * 0.05f;
	}, .03f);
	sound_set_id(skui_snd_tick, "sk/ui/tick_snd");

	skui_box_dbg  = mesh_find(default_id_mesh_cube);
	skui_mat_dbg  = material_copy_id(default_id_material_ui);
	material_set_transparency(skui_mat_dbg, transparency_add);
	material_set_color       (skui_mat_dbg, "color", { .5f,.5f,.5f,1 });
	material_set_depth_write (skui_mat_dbg, false);
	material_set_depth_test  (skui_mat_dbg, depth_test_always);
	material_set_id          (skui_mat_dbg, "sk/ui/debug_mat");

	skui_snd_interact   = sound_find(default_id_sound_click);
	skui_snd_uninteract = sound_find(default_id_sound_unclick);
	skui_snd_grab       = sound_find(default_id_sound_grab);
	skui_snd_ungrab     = sound_find(default_id_sound_ungrab);

	skui_mat = material_copy_id(default_id_material_ui);
	material_set_bool(skui_mat, "ui_tint", true);
	material_set_id  (skui_mat, "sk/ui/default_mat");
	skui_mat_quad = material_find(default_id_material_ui_quadrant);

	ui_set_element_visual(ui_vis_default,              skui_box,         skui_mat_quad, skui_box_min);
	ui_set_element_visual(ui_vis_window_head,          skui_box_top,     nullptr);
	ui_set_element_visual(ui_vis_window_body,          skui_box_bot,     nullptr);
	ui_set_element_visual(ui_vis_separator,            skui_box_dbg,     skui_mat);
	ui_set_element_visual(ui_vis_carat,                skui_box_dbg,     skui_mat);
	ui_set_element_visual(ui_vis_button_round,         skui_cylinder,    skui_mat);
	ui_set_element_visual(ui_vis_slider_line,          skui_small,       skui_mat_quad, skui_small_min);
	ui_set_element_visual(ui_vis_slider_line_active,   skui_small_left,  skui_mat_quad, skui_small_min);
	ui_set_element_visual(ui_vis_slider_line_inactive, skui_small_right, skui_mat_quad, skui_small_min);
	ui_set_element_visual(ui_vis_slider_pinch,         skui_small,       skui_mat_quad, skui_small_min);
	ui_set_element_visual(ui_vis_slider_push,          skui_small,       skui_mat_quad, skui_small_min); 

	mesh_t aura = nullptr;
	material_t aura_mat = material_copy_id(default_id_material_ui_aura);
	ui_default_aura_mesh(&aura, 0, skui_settings.rounding * 2, 7, 5);
	ui_set_element_visual(ui_vis_aura, aura, aura_mat);
}

///////////////////////////////////////////

void ui_theming_shutdown() {
	skui_font_stack.free();
	skui_tint_stack.free();

	for (int32_t i = 0; i < ui_vis_max; i++) {
		mesh_release    (skui_visuals[i].mesh);
		material_release(skui_visuals[i].material);
		skui_visuals[i] = {};
	}

	sound_release   (skui_active_sound_off); skui_active_sound_off = nullptr;
	sound_release   (skui_snd_interact);     skui_snd_interact     = nullptr;
	sound_release   (skui_snd_uninteract);   skui_snd_uninteract   = nullptr;
	sound_release   (skui_snd_grab);         skui_snd_grab         = nullptr;
	sound_release   (skui_snd_ungrab);       skui_snd_ungrab       = nullptr;
	sound_release   (skui_snd_tick);         skui_snd_tick         = nullptr;

	mesh_release    (skui_box);            skui_box            = nullptr;
	mesh_release    (skui_box_top);        skui_box_top        = nullptr;
	mesh_release    (skui_box_bot);        skui_box_bot        = nullptr;
	mesh_release    (skui_small);          skui_small          = nullptr;
	mesh_release    (skui_small_left);     skui_small_left     = nullptr;
	mesh_release    (skui_small_right);    skui_small_right    = nullptr;
	mesh_release    (skui_cylinder);       skui_cylinder       = nullptr;
	mesh_release    (skui_box_dbg);        skui_box_dbg        = nullptr;

	material_release(skui_mat);            skui_mat            = nullptr;
	material_release(skui_mat_quad);       skui_mat_quad       = nullptr;
	material_release(skui_mat_dbg);        skui_mat_dbg        = nullptr;
	material_release(skui_font_mat);       skui_font_mat       = nullptr;

	sprite_release  (skui_toggle_off);     skui_toggle_off     = nullptr;
	sprite_release  (skui_toggle_on);      skui_toggle_on      = nullptr;
	sprite_release  (skui_radio_off);      skui_radio_off      = nullptr;
	sprite_release  (skui_radio_on);       skui_radio_on       = nullptr;

	font_release    (skui_font);           skui_font           = nullptr;
}

///////////////////////////////////////////

void ui_theming_update() {
	if (skui_active_sound_element_id == 0) return;

	// See if our current sound on/off pair is still from a valid ui element
	bool found_active = false;
	for (int32_t i = 0; i < handed_max; i++) {
		if (skui_hand[i].active_prev == skui_active_sound_element_id) {
			return;
		}
	}
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
	ui_el_visual_t visual = {};

	if (mesh                                  != nullptr) mesh_addref     (mesh);
	if (material                              != nullptr) material_addref (material);
	if (skui_visuals[element_visual].mesh     != nullptr) mesh_release    (skui_visuals[element_visual].mesh);
	if (skui_visuals[element_visual].material != nullptr) material_release(skui_visuals[element_visual].material);

	visual.mesh     = mesh;
	visual.material = material;
	visual.min_size = min_size;
	skui_visuals[element_visual] = visual;
}

///////////////////////////////////////////

mesh_t ui_get_mesh(ui_vis_ element_visual) {
	return skui_visuals[element_visual].mesh
		? skui_visuals[element_visual].mesh
		: skui_visuals[ui_vis_default].mesh;
}

///////////////////////////////////////////

vec2 ui_get_mesh_minsize(ui_vis_ element_visual) {
	return skui_visuals[element_visual].mesh
		? skui_visuals[element_visual].min_size
		: skui_visuals[ui_vis_default].min_size;
}

///////////////////////////////////////////

material_t ui_get_material(ui_vis_ element_visual) {
	return skui_visuals[element_visual].material
		? skui_visuals[element_visual].material
		: skui_visuals[ui_vis_default].material;
}

///////////////////////////////////////////

void ui_draw_el(ui_vis_ element_visual, vec3 start, vec3 size, ui_color_ color, float focus) {
	/*if (size.x < skui_box_min.x) size.x = skui_box_min.x;
	if (size.y < skui_box_min.y) size.y = skui_box_min.y;
	if (size.z < skui_box_min.z) size.z = skui_box_min.z;*/

	vec3   pos = start - size / 2;
	matrix mx  = matrix_ts(pos, size);

	color128 final_color = ui_is_enabled()
		? color_lerp(skui_palette[color].normal, skui_palette[color].active, focus)
		: skui_palette[color].disabled;
	final_color = final_color * skui_tint;
	final_color.a = focus;

	render_add_mesh(ui_get_mesh(element_visual), ui_get_material(element_visual), mx, final_color);
}

///////////////////////////////////////////

void ui_play_sound_on_off(ui_vis_ element_visual, uint64_t element_id, vec3 at) {
	sound_t snd_on  = nullptr;
	sound_t snd_off = nullptr;

	// This pattern is until we add sounds to theming
	if (element_visual == ui_vis_handle           ||
		element_visual == ui_vis_window_body      ||
		element_visual == ui_vis_window_body_only ||
		element_visual == ui_vis_window_head      ||
		element_visual == ui_vis_window_head_only) {
		snd_on = skui_snd_grab;
		snd_off= skui_snd_ungrab;
	} else {
		snd_on = skui_snd_interact;
		snd_off= skui_snd_uninteract;
	}

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
	sound_t snd = nullptr;
	// This pattern is until we add sounds to theming
	if (element_visual == ui_vis_handle           ||
		element_visual == ui_vis_window_body      ||
		element_visual == ui_vis_window_body_only ||
		element_visual == ui_vis_window_head      ||
		element_visual == ui_vis_window_head_only) {
		snd = skui_snd_grab;
	} else {
		snd = skui_snd_interact;
	}

	if (snd) sound_play(snd, at, 1);
}

///////////////////////////////////////////

void ui_play_sound_off(ui_vis_ element_visual, vec3 at) {
	sound_t snd = nullptr;
	// This pattern is until we add sounds to theming
	if (element_visual == ui_vis_handle           ||
		element_visual == ui_vis_window_body      ||
		element_visual == ui_vis_window_body_only ||
		element_visual == ui_vis_window_head      ||
		element_visual == ui_vis_window_head_only) {
		snd = skui_snd_ungrab;
	} else {
		snd = skui_snd_uninteract;
	}

	if (snd) sound_play(snd, at, 1);
}

///////////////////////////////////////////

void ui_play_sound(ui_vis_ element_visual, vec3 at) {
	sound_t snd = skui_snd_tick;

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
	if (settings.margin           == 0) settings.margin = settings.padding;
	if (settings.rounding         == 0) settings.rounding= 7.5f * mm2m;

	bool rebuild_meshes = skui_settings.rounding != settings.rounding;
	skui_settings = settings;

	if (rebuild_meshes) {
		int32_t slices  = 4;//  settings.rounding > 20 * mm2m ? 4 : 3;
		bool    set_ids = skui_box == nullptr;
		ui_default_mesh     (&skui_box,       true, settings.rounding*2, 1.25f*mm2m, slices);
		ui_default_mesh_half(&skui_box_top,   true, settings.rounding*2, 1.25f*mm2m, slices, 0);
		ui_default_mesh_half(&skui_box_bot,   true, settings.rounding*2, 1.25f*mm2m, slices, 180 * deg2rad);

		float small = fminf(ui_line_height() / 3.0f, settings.rounding);
		ui_default_mesh     (&skui_small,       true, small, 1.25f*mm2m, slices);
		ui_default_mesh_half(&skui_small_left,  true, small, 1.25f*mm2m, slices, 270 * deg2rad);
		ui_default_mesh_half(&skui_small_right, true, small, 1.25f*mm2m, slices, 90  * deg2rad);

		skui_box_min   = vec2{ settings.padding, settings.padding } / 2;
		skui_small_min = vec2{ small, small } / 2;

		if (set_ids) {
			mesh_set_id(skui_box,         "sk/ui/box_mesh");
			mesh_set_id(skui_box_top,     "sk/ui/box_mesh_top");
			mesh_set_id(skui_box_bot,     "sk/ui/box_mesh_bot");
			mesh_set_id(skui_small,       "sk/ui/small_mesh");
			mesh_set_id(skui_small_left,  "sk/ui/small_mesh_left");
			mesh_set_id(skui_small_right, "sk/ui/small_mesh_right");
		}
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
	ui_set_theme_color(ui_color_complement, color_hsv(hsv.x, hsv.y * 0.2f,   hsv.z * 0.42f, color.a) ); // Complement color: unused so far?
	ui_set_theme_color(ui_color_text,       color128{1, 1, 1, 1});                                      // Text color
}

///////////////////////////////////////////

void ui_set_theme_color_state(ui_color_ color_type, ui_color_state_ state, color128 color_gamma) {
	color128 linear = color_to_linear(color_gamma);

	switch (state) {
	case ui_color_state_normal:   skui_palette[color_type].normal   = linear; break;
	case ui_color_state_active:   skui_palette[color_type].active   = linear; break;
	case ui_color_state_disabled: skui_palette[color_type].disabled = linear; break;
	}
}

///////////////////////////////////////////

color128 ui_get_theme_color_state(ui_color_ color_type, ui_color_state_ state) {
	switch (state) {
	case ui_color_state_normal:   return color_to_gamma(skui_palette[color_type].normal);
	case ui_color_state_active:   return color_to_gamma(skui_palette[color_type].active);
	case ui_color_state_disabled: return color_to_gamma(skui_palette[color_type].disabled);
	default: return { 1,1,1,1 };
	}
}

///////////////////////////////////////////

void ui_set_theme_color(ui_color_ color_type, color128 color_gamma) {
	color128 linear = color_to_linear(color_gamma);
	skui_palette[color_type].normal   = linear;
	skui_palette[color_type].active   = linear*color128{ 2,2,2,1 };
	skui_palette[color_type].disabled = linear*color128{ 0.5f,0.5f,0.5f,1 };
}

///////////////////////////////////////////

color128 ui_get_theme_color(ui_color_ color_type) {
	return color_to_gamma(skui_palette[color_type].normal);
}

///////////////////////////////////////////
// Style stack                           //
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

void ui_anim_start(uint64_t id, int32_t channel) {
	if (skui_anim_id[channel] != id) {
		skui_anim_id[channel] = id;
		skui_anim_time[channel] = time_totalf_unscaled();
	}
}

///////////////////////////////////////////

bool ui_anim_has(uint64_t id, int32_t channel, float duration) {
	if (id == skui_anim_id[channel]) {
		if ((time_totalf_unscaled() - skui_anim_time[channel]) < duration)
			return true;
		skui_anim_id[channel] = 0;
	}
	return false;
}

///////////////////////////////////////////

float ui_anim_elapsed(uint64_t id, int32_t channel, float duration, float max) {
	return skui_anim_id[channel] == id ? fminf(max, (time_totalf_unscaled() - skui_anim_time[channel]) / duration) : 0;
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

void ui_default_mesh(mesh_t *mesh, bool quadrantify, float diameter, float rounding, int32_t quadrant_slices) {
	if (*mesh == nullptr) {
		*mesh = mesh_create();
	}
	
	float radius = diameter / 2;

	vind_t  subd       = (vind_t)quadrant_slices*4;
	int     vert_count = subd * 5 + 2;
	int     ind_count  = subd * 18;
	vert_t *verts      = sk_malloc_t(vert_t, vert_count);
	vind_t *inds       = sk_malloc_t(vind_t, ind_count );

	vind_t ind = 0;

	for (vind_t i = 0; i < subd; i++) {
		float u = 0, v = 0;

		float ang = ((float)i / subd + (0.5f/subd)) *(float)MATH_PI * 2;
		float x = cosf(ang);
		float y = sinf(ang);
		vec3 normal  = {x,y,0};
		vec3 top_pos = normal*(radius-rounding) + vec3{0,0,0.5f};
		vec3 ctr_pos = normal*radius;
		vec3 bot_pos = normal*(radius-rounding) - vec3{0,0,0.5f};

		// strip first
		verts[i * 5  ] = { top_pos,  vec3_normalize(normal+vec3{0,0,2}), {u,v}, {255,255,255,0} };
		verts[i * 5+1] = { ctr_pos,  normal,                             {u,v}, {255,255,255,0} };
		verts[i * 5+2] = { bot_pos,  vec3_normalize(normal-vec3{0,0,2}), {u,v}, {255,255,255,0} };
		// now circular faces
		verts[i * 5+3] = { top_pos,  {0,0, 1},    {u,v}, {255,255,255,255} };
		verts[i * 5+4] = { bot_pos,  {0,0,-1},    {u,v}, {255,255,255,255} };

		vind_t in = (i + 1) % subd;
		// Top slice
		inds[ind++] = i  * 5 + 3;
		inds[ind++] = in * 5 + 3;
		inds[ind++] = subd * 5;
		// Bottom slice
		inds[ind++] = subd * 5+1;
		inds[ind++] = in * 5 + 4;
		inds[ind++] = i  * 5 + 4;

		// Now edge strip quad bottom
		inds[ind++] = in * 5+1;
		inds[ind++] = in * 5;
		inds[ind++] = i  * 5;
		inds[ind++] = i  * 5+1;
		inds[ind++] = in * 5+1;
		inds[ind++] = i  * 5;
		// And edge strip quad top
		inds[ind++] = in * 5+2;
		inds[ind++] = in * 5+1;
		inds[ind++] = i  * 5+1;
		inds[ind++] = i  * 5+2;
		inds[ind++] = in * 5+2;
		inds[ind++] = i  * 5+1;
	}

	// center points for the circle
	verts[subd*5]   = { {0,0, .5f}, {0,0, 1}, {0,0}, {255,255,255,255} };
	verts[subd*5+1] = { {0,0,-.5f}, {0,0,-1}, {0,0}, {255,255,255,255} };
	if (quadrantify)
		ui_quadrant_size_verts(verts, vert_count, 0);

	mesh_set_data(*mesh, verts, vert_count, inds, ind_count);

	sk_free(verts);
	sk_free(inds);
}

///////////////////////////////////////////

void ui_default_mesh_half(mesh_t *mesh, bool quadrantify, float diameter, float rounding, int32_t quadrant_slices, float angle_start) {
	if (*mesh == nullptr) {
		*mesh = mesh_create();
	}

	float radius = diameter / 2;

	vind_t  subd       = (vind_t)quadrant_slices*2 + 2;
	int     vert_count = subd * 5 + 2;
	int     ind_count  = subd * 18 - 12;
	vert_t *verts      = sk_malloc_t(vert_t, vert_count);
	vind_t *inds       = sk_malloc_t(vind_t, ind_count );

	for (vind_t i = 0; i < subd-2; i++) {
		float u = 0, v = 0;

		float ang = angle_start + ((float)i / (subd-2) + (0.5f / (subd-2))) * (MATH_PI);
		float x = cosf(ang);
		float y = sinf(ang);
		vec3 normal  = {x,y,0};
		vec3 top_pos = normal*(radius-rounding) + vec3{0, 0, 0.5f};
		vec3 ctr_pos = normal*radius;
		vec3 bot_pos = normal*(radius-rounding) + vec3{0, 0,-0.5f};

		// strip first
		verts[i * 5  ] = { top_pos,  vec3_normalize(normal+vec3{0,0,2}), {u,v}, {255,255,255,0} };
		verts[i * 5+1] = { ctr_pos,  normal,                             {u,v}, {255,255,255,0} };
		verts[i * 5+2] = { bot_pos,  vec3_normalize(normal-vec3{0,0,2}), {u,v}, {255,255,255,0} };
		// now circular faces
		verts[i * 5+3] = { top_pos,  {0,0, 1},    {u,v}, {255,255,255,255} };
		verts[i * 5+4] = { bot_pos,  {0,0,-1},    {u,v}, {255,255,255,255} };
	}
	// Top half
	{
		int32_t i   = subd - 2;
		float   ang = angle_start + MATH_PI;
		float   x   = cosf(ang);
		float   y   = sinf(ang);
		vec3 normal  = {x,y,0};
		vec3 up      = vec3{-y,x,0 } * radius;
		vec3 top_pos = normal*(radius-rounding) + vec3{0,0, 0.5f} + up;
		vec3 ctr_pos = normal*radius                         + up;
		vec3 bot_pos = normal*(radius-rounding) + vec3{0,0,-0.5f} + up;
		// strip first
		verts[i * 5  ] = { top_pos,  vec3_normalize(normal+vec3{0,0,2}), {0,0}, {255,255,255,0} };
		verts[i * 5+1] = { ctr_pos,  normal,                             {0,0}, {255,255,255,0} };
		verts[i * 5+2] = { bot_pos,  vec3_normalize(normal-vec3{0,0,2}), {0,0}, {255,255,255,0} };
		// now circular faces
		verts[i * 5+3] = { top_pos,  {0,0, 1},    {0,0}, {255,255,255,255} };
		verts[i * 5+4] = { bot_pos,  {0,0,-1},    {0,0}, {255,255,255,255} };

		i   = subd - 1;
		ang = angle_start;
		x   = cosf(ang);
		y   = sinf(ang);
		normal  = {x,y,0};
		top_pos = normal*(radius-rounding) + vec3{0,0, 0.5f} + up;
		ctr_pos = normal*radius                         + up;
		bot_pos = normal*(radius-rounding) + vec3{0,0,-0.5f} + up;
		// strip first
		verts[i * 5  ] = { top_pos,  vec3_normalize(normal+vec3{0,0,2}), {0,0}, {255,255,255,0} };
		verts[i * 5+1] = { ctr_pos,  normal,                             {0,0}, {255,255,255,0} };
		verts[i * 5+2] = { bot_pos,  vec3_normalize(normal-vec3{0,0,2}), {0,0}, {255,255,255,0} };
		// now circular faces
		verts[i * 5+3] = { top_pos,  {0,0, 1},    {0,0}, {255,255,255,255} };
		verts[i * 5+4] = { bot_pos,  {0,0,-1},    {0,0}, {255,255,255,255} };
	}

	vind_t ind = 0;
	for (vind_t i = 0; i < subd; i++) {
		vind_t in = (i + 1) % subd;
		// Top slice
		inds[ind++] = i  * 5 + 3;
		inds[ind++] = in * 5 + 3;
		inds[ind++] = subd * 5;
		// Bottom slice
		inds[ind++] = subd * 5+1;
		inds[ind++] = in * 5 + 4;
		inds[ind++] = i  * 5 + 4;

		if (i == subd - 2) continue;

		// Now edge strip quad bottom
		inds[ind++] = in * 5+1;
		inds[ind++] = in * 5;
		inds[ind++] = i  * 5;
		inds[ind++] = i  * 5+1;
		inds[ind++] = in * 5+1;
		inds[ind++] = i  * 5;
		// And edge strip quad top
		inds[ind++] = in * 5+2;
		inds[ind++] = in * 5+1;
		inds[ind++] = i  * 5+1;
		inds[ind++] = i  * 5+2;
		inds[ind++] = in * 5+2;
		inds[ind++] = i  * 5+1;
	}

	// center points for the circle
	verts[subd*5]   = { {0,0, .5f}, {0,0, 1}, {0,0}, {255,255,255,255} };
	verts[subd*5+1] = { {0,0,-.5f}, {0,0,-1}, {0,0}, {255,255,255,255} };
	if (quadrantify)
		ui_quadrant_size_verts(verts, vert_count, 0);

	mesh_set_data(*mesh, verts, vert_count, inds, ind_count);

	sk_free(verts);
	sk_free(inds);
}

///////////////////////////////////////////

void ui_default_aura_mesh(mesh_t *mesh, float tube_diameter, float corner_radius, int32_t quadrant_slices, int32_t tube_corners) {
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
		vec3 quad_off = vec3{ uv.x, uv.y,0 } * corner_radius * -1.5f;

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
			inds[ind++] = offn + cn;
			inds[ind++] = offn + c;
			inds[ind++] = off  + c;
			inds[ind++] = off  + cn;
			inds[ind++] = offn + cn;
			inds[ind++] = off  + c;
		}
	}
	mesh_set_data(*mesh, verts, vert_count, inds, ind_count);

	sk_free(verts);
	sk_free(inds);
}

///////////////////////////////////////////

}