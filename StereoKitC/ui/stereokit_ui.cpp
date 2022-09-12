#include "../stereokit_ui.h"
#include "../_stereokit_ui.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../systems/input.h"
#include "../systems/input_keyboard.h"
#include "../platforms/platform_utils.h"
#include "../hands/input_hand.h"
#include "../libraries/ferr_hash.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"

#include <math.h>
#include <float.h>

///////////////////////////////////////////

namespace sk {

struct ui_window_t {
	pose_t   pose;
	vec2     size;
	ui_win_  type;
	uint64_t hash;
};

struct layer_t {
	ui_window_t *window;
	vec3         offset_initial;
	vec3         offset;
	vec2         size;
	float        line_height;
	float        max_x;
	vec3         finger_pos   [handed_max];
	vec3         finger_prev  [handed_max];
	vec3         pinch_pt_pos [handed_max];
	vec3         pinch_pt_prev[handed_max];
};
struct ui_hand_t {
	vec3     finger;
	vec3     finger_prev;
	vec3     finger_world;
	vec3     finger_world_prev;
	vec3     pinch_pt;
	vec3     pinch_pt_prev;
	vec3     pinch_pt_world;
	vec3     pinch_pt_world_prev;
	bool     tracked;
	bool     ray_enabled;
	bool     ray_discard;
	float    ray_visibility;
	uint64_t focused_prev;
	uint64_t focused;
	float    focus_priority;
	uint64_t active_prev;
	uint64_t active;
	button_state_ pinch_state;
};

struct ui_id_t {
	uint64_t id;
};

struct ui_el_visual_t {
	mesh_t     mesh;
	material_t material;
	vec2       min_size;
};

array_t<ui_window_t> skui_sl_windows = {};
array_t<ui_id_t>     skui_id_stack   = {};
array_t<layer_t>     skui_layers     = {};
array_t<text_style_t>skui_font_stack = {};
array_t<color128>    skui_tint_stack = {};
array_t<bool32_t>    skui_enabled_stack = {};
array_t<bool>        skui_preserve_keyboard_stack = {};
array_t<uint64_t>    skui_preserve_keyboard_ids[2] = {};
array_t<uint64_t>   *skui_preserve_keyboard_ids_read;
array_t<uint64_t>   *skui_preserve_keyboard_ids_write;

ui_el_visual_t  skui_visuals[ui_vis_max] = {};
mesh_t          skui_win_top      = nullptr;
mesh_t          skui_win_bot      = nullptr;
mesh_t          skui_box          = nullptr;
vec3            skui_box_min      = {};
mesh_t          skui_box_dbg      = nullptr;
vec3            skui_box_dbg_min  = {};
mesh_t          skui_cylinder;
vec3            skui_cylinder_min = {};
material_t      skui_mat;
material_t      skui_mat_quad;
material_t      skui_mat_dbg;
font_t          skui_font;
material_t      skui_font_mat;
ui_hand_t       skui_hand[2] = {};
float           skui_finger_radius = 0;
bool32_t        skui_show_volumes = false;
bool32_t        skui_enable_far_interact = true;
uint64_t        skui_input_target = 0;
color128        skui_tint = {1,1,1,1};
bool32_t        skui_interact_enabled = true;
uint64_t        skui_last_element = 0xFFFFFFFFFFFFFFFF;

sound_t         skui_snd_interact;
sound_t         skui_snd_uninteract;
sound_t         skui_snd_grab;
sound_t         skui_snd_ungrab;

ui_settings_t skui_settings = {
	10 * mm2m, // padding
	10 * mm2m, // gutter
	10 * mm2m, // depth
	.4f,       // backplate_depth
	1  * mm2m, // backplate_border
};
float skui_fontsize = 10*mm2m;

vec3  skui_prev_offset;
float skui_prev_line_height;
bounds_t skui_recent_layout;

uint64_t skui_anim_id;
float    skui_anim_time;

const color128 skui_color_border = { 1,1,1,1 };
color128 skui_palette[ui_color_max];

///////////////////////////////////////////

// Layout
void ui_layout_reserve_sz(vec2 size, bool32_t add_padding, vec3 *out_position, vec2 *out_final_size);

// Interaction
bool32_t      ui_in_box                  (vec3 pt1, vec3 pt2, float radius, bounds_t box);
void          ui_box_interaction_1h_pinch(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_hand);
void          ui_box_interaction_1h_poke (uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_hand);
bool32_t      ui_is_hand_preoccupied     (handed_ hand, uint64_t for_el_id, bool32_t include_focused);
button_state_ ui_focus_set               (int32_t hand, uint64_t for_el_id, bool32_t focused, float priority);
button_state_ ui_active_set              (int32_t hand, uint64_t for_el_id, bool32_t active);
int32_t       ui_last_active_hand        (uint64_t for_el_id);
int32_t       ui_last_focused_hand       (uint64_t for_el_id);
mesh_t        ui_get_mesh                (ui_vis_ element_visual);
material_t    ui_get_material            (ui_vis_ element_visual);
void          ui_draw_el                 (ui_vis_ element_visual, vec3 start, vec3 size, ui_color_ color, float focus);

// Base render types
void ui_box    (vec3 start, vec3 size, material_t material, color128 color);
void ui_cube   (vec3 start, vec3 size, material_t material, color128 color);
void ui_text_in(vec3 start, vec2 size, const char     *text, text_align_ position, text_align_ align);
void ui_text_in(vec3 start, vec2 size, const char16_t *text, text_align_ position, text_align_ align);

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
	depth = fabsf(depth * 2);
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

void ui_quadrant_mesh(mesh_t *mesh, float padding, int32_t quadrant_slices) {
	if (*mesh == nullptr) {
		*mesh = mesh_create();
	}
	
	float radius = padding / 2;

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
		vec3 top_pos = normal*radius*0.75f + vec3{0,0,0.5f};
		vec3 ctr_pos = normal*radius;
		vec3 bot_pos = normal*radius*0.75f - vec3{0,0,0.5f};

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
	ui_quadrant_size_verts(verts, vert_count, 0);

	mesh_set_data(*mesh, verts, vert_count, inds, ind_count);

	sk_free(verts);
	sk_free(inds);
}

///////////////////////////////////////////

void ui_quadrant_mesh_half(mesh_t *mesh, float padding, int32_t quadrant_slices, float angle_start) {
	if (*mesh == nullptr) {
		*mesh = mesh_create();
	}

	float radius = padding / 2;

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
		vec3 top_pos = normal*radius*0.75f + vec3{0, 0.001f*radius, 0.5f};
		vec3 ctr_pos = normal*radius;
		vec3 bot_pos = normal*radius*0.75f + vec3{0, 0.001f*radius,-0.5f};

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
		vec3 top_pos = normal*radius*0.75f + vec3{0,0, 0.5f} + up;
		vec3 ctr_pos = normal*radius                         + up;
		vec3 bot_pos = normal*radius*0.75f + vec3{0,0,-0.5f} + up;
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
		top_pos = normal*radius*0.75f + vec3{0,0, 0.5f} + up;
		ctr_pos = normal*radius                         + up;
		bot_pos = normal*radius*0.75f + vec3{0,0,-0.5f} + up;
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
	ui_quadrant_size_verts(verts, vert_count, 0);

	mesh_set_data(*mesh, verts, vert_count, inds, ind_count);

	sk_free(verts);
	sk_free(inds);
}

///////////////////////////////////////////

inline bounds_t ui_size_box(vec3 top_left, vec3 dimensions) {
	return { top_left - dimensions/2, dimensions };
}

///////////////////////////////////////////

inline void ui_anim_start(uint64_t id) {
	if (skui_anim_id != id) {
		skui_anim_id = id;
		skui_anim_time = time_getf_unscaled();
	}
}

///////////////////////////////////////////

inline bool ui_anim_has(uint64_t id, float duration) {
	if (id == skui_anim_id) {
		if ((time_getf_unscaled() - skui_anim_time) < duration)
			return true;
		skui_anim_id = 0;
	}
	return false;
}

///////////////////////////////////////////

inline float ui_anim_elapsed(uint64_t id, float duration = 1, float max = 1) {
	return skui_anim_id == id ? fminf(max, (time_getf_unscaled() - skui_anim_time) / duration) : 0;
}

///////////////////////////////////////////

void ui_show_volumes(bool32_t show) {
	skui_show_volumes = show;
}

///////////////////////////////////////////

void ui_enable_far_interact(bool32_t enable) {
	skui_enable_far_interact = enable;
}

///////////////////////////////////////////

bool32_t ui_far_interact_enabled() {
	return skui_enable_far_interact;
}

///////////////////////////////////////////

void ui_settings(ui_settings_t settings) {
	if (settings.backplate_border == 0) settings.backplate_border = 0.5f * mm2m;
	if (settings.backplate_depth  == 0) settings.backplate_depth  = 0.4f;
	if (settings.depth            == 0) settings.depth   = 10 * mm2m;
	if (settings.gutter           == 0) settings.gutter  = 20 * mm2m;
	if (settings.padding          == 0) settings.padding = 10 * mm2m;
	skui_settings = settings; 

	skui_box_min = { settings.padding*0.75f, settings.padding*0.75f, 0 };
	ui_quadrant_mesh     (&skui_box,     settings.padding*0.75f, 3);
	ui_quadrant_mesh_half(&skui_win_top, settings.padding, 3, 0);
	ui_quadrant_mesh_half(&skui_win_bot, settings.padding, 3, 180 * deg2rad);
}

///////////////////////////////////////////

float ui_get_padding() {
	return skui_settings.padding;
}

///////////////////////////////////////////

float ui_get_gutter() {
	return skui_settings.gutter;
}

///////////////////////////////////////////

void ui_set_color(color128 color) {
	vec3 hsv = color_to_hsv(color);
	
	skui_palette[0] = color_to_linear( color );                                                    // Primary color: Headers, separators, etc.
	skui_palette[1] = color_to_linear( color_hsv(hsv.x, hsv.y * 0.2f,   hsv.z * 0.45f, color.a) ); // Dark color: body and backgrounds
	skui_palette[2] = color_to_linear( color_hsv(hsv.x, hsv.y * 0.075f, hsv.z * 0.65f, color.a) ); // Primary element color: buttons, sliders, etc.
	skui_palette[3] = color_to_linear( color_hsv(hsv.x, hsv.y * 0.2f,   hsv.z * 0.42f, color.a) ); // Complement color: unused so far?
	skui_palette[4] = color128{1, 1, 1, 1};                                                        // Text color
}

///////////////////////////////////////////

void ui_set_theme_color(ui_color_ color_type, color128 color_gamma) {
	skui_palette[color_type] = color_to_linear( color_gamma );
}

///////////////////////////////////////////

color128 ui_get_theme_color(ui_color_ color_type) {
	return color_to_gamma(skui_palette[color_type]);
}

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
	matrix mx  = matrix_trs(pos, quat_identity, size);

	color128 final_color = skui_palette[color] * skui_tint;
	if (!skui_enabled_stack.last()) final_color = final_color * color128{.5f, .5f, .5f, 1};
	final_color.a = focus;

	render_add_mesh(ui_get_mesh(element_visual), ui_get_material(element_visual), mx, final_color);
}

///////////////////////////////////////////

void ui_push_text_style(text_style_t style) {
	skui_font_stack.add(style);
	skui_fontsize = text_style_get_char_height(style);
}

///////////////////////////////////////////

void ui_pop_text_style() {
	if (skui_font_stack.count <= 1) {
		log_errf("ui_pop_text_style: tried to pop too many styles! Do you have a push/pop mismatch?");
		return;
	}
	skui_font_stack.pop();
	skui_fontsize = text_style_get_char_height(skui_font_stack.last());
}

///////////////////////////////////////////

void ui_push_tint(color128 tint_gamma) {
	skui_tint = color_to_linear(tint_gamma);
	skui_tint_stack.add(skui_tint);
}

///////////////////////////////////////////

void ui_pop_tint() {
	if (skui_tint_stack.count <= 1) {
		log_errf("ui_pop_tint: tried to pop too many tints! Do you have a push/pop mismatch?");
		return;
	}
	skui_tint_stack.pop();
	skui_tint = skui_tint_stack.last();
}

///////////////////////////////////////////

void ui_push_enabled(bool32_t enabled) {
	skui_interact_enabled = enabled;
	skui_enabled_stack.add(skui_interact_enabled);
}

///////////////////////////////////////////

void ui_pop_enabled() {
	if (skui_enabled_stack.count <= 1) {
		log_errf("ui_pop_enabled: tried to pop too often! Do you have a push/pop mismatch?");
		return;
	}
	skui_enabled_stack.pop();
	skui_interact_enabled = skui_enabled_stack.last();
}

///////////////////////////////////////////

void ui_push_preserve_keyboard(bool32_t preserve_keyboard){
	skui_preserve_keyboard_stack.add(preserve_keyboard);
}

///////////////////////////////////////////

void ui_pop_preserve_keyboard(){
	if (skui_preserve_keyboard_stack.count <= 1) {
		log_errf("ui_pop_preserve_keyboard: tried to pop too many! Do you have a push/pop mismatch?");
		return;
	}
	skui_preserve_keyboard_stack.pop();
}

///////////////////////////////////////////

bool ui_init() {
	ui_set_color(color_hsv(0.07f, 0.5f, 0.75f, 1));

	skui_box_min = { skui_settings.padding*0.75f, skui_settings.padding*0.75f, 0 };
	ui_quadrant_mesh     (&skui_box,     skui_settings.padding*0.75f, 5);
	ui_quadrant_mesh_half(&skui_win_top, skui_settings.padding, 5, 0);
	ui_quadrant_mesh_half(&skui_win_bot, skui_settings.padding, 5, 180 * deg2rad);
	mesh_set_id(skui_box,     "sk/ui/box_mesh");
	mesh_set_id(skui_win_top, "sk/ui/box_mesh_top");
	mesh_set_id(skui_win_bot, "sk/ui/box_mesh_bot");

	skui_box_dbg  = mesh_find(default_id_mesh_cube);
	skui_cylinder = mesh_gen_cylinder(1, 1, {0,0,1}, 24);
	skui_mat_dbg  = material_copy_id(default_id_material_ui);
	material_set_transparency(skui_mat_dbg, transparency_blend);
	material_set_color       (skui_mat_dbg, "color", { 0,1,0,0.25f });

	skui_font_mat   = material_find(default_id_material_font);
	material_set_queue_offset(skui_font_mat, -12);
	skui_font       = font_find(default_id_font);
	skui_font_stack.add( text_make_style_mat(skui_font, skui_fontsize, skui_font_mat, color_to_gamma( skui_palette[4] )) );

	skui_snd_interact   = sound_find(default_id_sound_click);
	skui_snd_uninteract = sound_find(default_id_sound_unclick);
	skui_snd_grab       = sound_find(default_id_sound_grab);
	skui_snd_ungrab     = sound_find(default_id_sound_ungrab);

	skui_mat      = material_find(default_id_material_ui);
	skui_mat_quad = material_find(default_id_material_ui_quadrant);
	ui_set_element_visual(ui_vis_default,     skui_box,     skui_mat_quad, { skui_settings.padding * 0.75f, skui_settings.padding * 0.75f });
	ui_set_element_visual(ui_vis_window_head, skui_win_top, nullptr);
	ui_set_element_visual(ui_vis_window_body, skui_win_bot, nullptr);
	ui_set_element_visual(ui_vis_separator,   skui_box_dbg, skui_mat);

	skui_preserve_keyboard_ids_read  = &skui_preserve_keyboard_ids[0];
	skui_preserve_keyboard_ids_write = &skui_preserve_keyboard_ids[1];

	skui_id_stack.add({ HASH_FNV64_START });

	ui_push_tint             ({ 1,1,1,1 });
	ui_push_enabled          (true);
	ui_push_preserve_keyboard(false);
	return true;
}

///////////////////////////////////////////

void ui_update() {
	skui_finger_radius = 0;
	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < handed_max; i++) {
		const hand_t    *hand    = input_hand((handed_)i);
		const pointer_t *pointer = input_get_pointer(input_hand_pointer_id[i]);

		bool was_ray_enabled = skui_hand[i].ray_enabled && !skui_hand[i].ray_discard;

		skui_finger_radius += hand->fingers[1][4].radius;
		skui_hand[i].finger_world_prev   = skui_hand[i].finger_world;
		skui_hand[i].finger_world        = hand->fingers[1][4].position;
		skui_hand[i].pinch_pt_world_prev = skui_hand[i].pinch_pt;
		skui_hand[i].pinch_pt_world      = hand->pinch_pt;
		skui_hand[i].focused_prev = skui_hand[i].focused;
		skui_hand[i].active_prev  = skui_hand[i].active;

		skui_hand[i].focus_priority = FLT_MAX;
		skui_hand[i].focused = 0;
		skui_hand[i].active  = 0;
		skui_hand[i].finger       = matrix_transform_pt(*to_local, skui_hand[i].finger_world);
		skui_hand[i].finger_prev  = matrix_transform_pt(*to_local, skui_hand[i].finger_world_prev);
		skui_hand[i].pinch_pt     = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt);
		skui_hand[i].pinch_pt_prev= matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_prev);
		skui_hand[i].tracked      = hand->tracked_state & button_state_active;
		skui_hand[i].ray_enabled  = pointer->tracked > 0 && skui_hand[i].tracked && (vec3_dot(hand->palm.orientation * vec3_forward, input_head()->position - hand->palm.position) < 0);

		// Don't let the hand trigger things while popping in and out of
		// tracking
		if (hand->tracked_state & button_state_just_active) {
			skui_hand[i].finger_prev         = skui_hand[i].finger;
			skui_hand[i].finger_world_prev   = skui_hand[i].finger_world;
			skui_hand[i].pinch_pt_prev       = skui_hand[i].pinch_pt;
			skui_hand[i].pinch_pt_world_prev = skui_hand[i].pinch_pt_world;
		}

		// draw hand rays
		skui_hand[i].ray_visibility = math_lerp(skui_hand[i].ray_visibility,
			was_ray_enabled && skui_enable_far_interact && skui_hand[i].ray_enabled && !skui_hand[i].ray_discard ? 1.0f : 0.0f,
			20.0f * time_elapsedf_unscaled());
		if (skui_hand[i].focused_prev != 0) skui_hand[i].ray_visibility = 0;
		if (skui_hand[i].ray_visibility > 0.004f) {
			ray_t       r     = input_get_pointer(input_hand_pointer_id[i])->ray;
			const float scale = 2;
			line_point_t points[5] = {
				line_point_t{r.pos+r.dir*(0.07f              ), 0.001f,  color32{255,255,255,0}},
				line_point_t{r.pos+r.dir*(0.07f + 0.01f*scale), 0.0015f, color32{255,255,255,(uint8_t)(skui_hand[i].ray_visibility * 60 )}},
				line_point_t{r.pos+r.dir*(0.07f + 0.02f*scale), 0.0020f, color32{255,255,255,(uint8_t)(skui_hand[i].ray_visibility * 80)}},
				line_point_t{r.pos+r.dir*(0.07f + 0.07f*scale), 0.0015f, color32{255,255,255,(uint8_t)(skui_hand[i].ray_visibility * 25 )}},
				line_point_t{r.pos+r.dir*(0.07f + 0.11f*scale), 0.001f,  color32{255,255,255,0}} };
			line_add_listv(points, 5);
		}
		skui_hand[i].ray_discard = false;
	}
	skui_finger_radius /= handed_max;

	ui_push_surface(pose_identity);

	// Clear current keyboard ignore elements
	skui_preserve_keyboard_ids_read->clear();
	array_t<uint64_t> *tmp = skui_preserve_keyboard_ids_read;
	skui_preserve_keyboard_ids_read  = skui_preserve_keyboard_ids_write;
	skui_preserve_keyboard_ids_write = tmp;
}

///////////////////////////////////////////

void ui_update_late() {
	ui_pop_surface();
	if (skui_layers                 .count != 0) log_err("ui: Mismatching number of Begin/End calls!");
	if (skui_id_stack               .count != 1) log_err("ui: Mismatching number of id push/pop calls!");
	if (skui_tint_stack             .count != 1) log_err("ui: Mismatching number of tint push/pop calls!");
	if (skui_enabled_stack          .count != 1) log_err("ui: Mismatching number of enabled push/pop calls!");
	if (skui_preserve_keyboard_stack.count != 1) log_err("ui: Mismatching number of preserve keyboard push/pop calls!");
}

///////////////////////////////////////////

void ui_shutdown() {
	for (int32_t i = 0; i < ui_vis_max; i++) {
		mesh_release    (skui_visuals[i].mesh);
		material_release(skui_visuals[i].material);
		skui_visuals[i] = {};
	}

	skui_sl_windows              .free();
	skui_layers                  .free();
	skui_id_stack                .free();
	skui_font_stack              .free();
	skui_tint_stack              .free();
	skui_enabled_stack           .free();
	skui_preserve_keyboard_stack .free();
	skui_preserve_keyboard_ids[0].free();
	skui_preserve_keyboard_ids[1].free();
	sound_release(skui_snd_interact);   skui_snd_interact   = nullptr;
	sound_release(skui_snd_uninteract); skui_snd_uninteract = nullptr;
	sound_release(skui_snd_grab);       skui_snd_grab       = nullptr;
	sound_release(skui_snd_ungrab);     skui_snd_ungrab     = nullptr;
	mesh_release(skui_box);      skui_box      = nullptr;
	mesh_release(skui_cylinder); skui_cylinder = nullptr;
	mesh_release(skui_box_dbg);  skui_box_dbg  = nullptr;
	mesh_release(skui_win_top);  skui_win_top  = nullptr;
	mesh_release(skui_win_bot);  skui_win_bot  = nullptr;
	material_release(skui_mat);      skui_mat      = nullptr;
	material_release(skui_mat_quad); skui_mat_quad = nullptr;
	material_release(skui_mat_dbg);  skui_mat_dbg  = nullptr;
	material_release(skui_font_mat); skui_font_mat = nullptr;
	font_release(skui_font); skui_font = nullptr;
}

///////////////////////////////////////////

uint64_t hash_fnv64_string_16(const char16_t* string, uint64_t start_hash = HASH_FNV64_START) {
	uint64_t hash = start_hash;
	while (*string != '\0') {
		hash = (hash ^ ((*string & 0xFF00) >> 2)) * 1099511628211;
		hash = (hash ^ ( *string & 0x00FF      )) * 1099511628211;
		string++;
	}
	return hash;
}

///////////////////////////////////////////

uint64_t ui_stack_hash(const char *string) {
	return skui_id_stack.count > 0 
		? hash_fnv64_string(string, skui_id_stack.last().id) 
		: hash_fnv64_string(string);
}

///////////////////////////////////////////

uint64_t ui_stack_hash_16(const char16_t *string) {
	return skui_id_stack.count > 0 
		? hash_fnv64_string_16(string, skui_id_stack.last().id) 
		: hash_fnv64_string_16(string);
}
uint64_t ui_stack_hash(const char16_t *string) { return ui_stack_hash_16(string); }

///////////////////////////////////////////

uint64_t ui_stack_hashi(int32_t id) {
	return skui_id_stack.count > 0 
		? hash_fnv64_data(&id, sizeof(int32_t), skui_id_stack.last().id) 
		: hash_fnv64_data(&id, sizeof(int32_t));
}

///////////////////////////////////////////

uint64_t ui_push_id(const char *id) {
	uint64_t result = ui_stack_hash(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

uint64_t ui_push_id_16(const char16_t *id) {
	uint64_t result = ui_stack_hash(id);
	skui_id_stack.add({ result });
	return result;
}
inline uint64_t ui_push_id(const char16_t *id) { return ui_push_id_16(id); }

///////////////////////////////////////////

uint64_t ui_push_idi(int32_t id) {
	uint64_t result = ui_stack_hashi(id);
	skui_id_stack.add({ result });
	return result;
}

///////////////////////////////////////////

void ui_pop_id() {
	skui_id_stack.pop();
}

///////////////////////////////////////////

bool32_t ui_is_interacting(handed_ hand) {
	return skui_hand[hand].active_prev != 0 || skui_hand[hand].focused_prev != 0;
}

///////////////////////////////////////////
//////////////   Layout!   ////////////////
///////////////////////////////////////////

void ui_push_surface(pose_t surface_pose, vec3 layout_start, vec2 layout_dimensions) {
	vec3   right = surface_pose.orientation * vec3_right;
	matrix trs   = matrix_trs(surface_pose.position + right*layout_start, surface_pose.orientation);
	hierarchy_push(trs);

	skui_layers.add(layer_t{
		nullptr,
		layout_start,
		layout_start,
		layout_dimensions, 0, 0
	});

	layer_t      &layer    = skui_layers.last();
	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < handed_max; i++) {
		layer.finger_pos   [i] = skui_hand[i].finger        = matrix_transform_pt(*to_local, skui_hand[i].finger_world);
		layer.finger_prev  [i] = skui_hand[i].finger_prev   = matrix_transform_pt(*to_local, skui_hand[i].finger_world_prev);
		layer.pinch_pt_pos [i] = skui_hand[i].pinch_pt      = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_world);
		layer.pinch_pt_prev[i] = skui_hand[i].pinch_pt_prev = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_world_prev);
	}
}

///////////////////////////////////////////

void ui_pop_surface() {
	hierarchy_pop();
	skui_layers.pop();

	if (skui_layers.count <= 0) {
		for (int32_t i = 0; i < handed_max; i++) {
			skui_hand[i].finger        = skui_hand[i].finger_world;
			skui_hand[i].finger_prev   = skui_hand[i].finger_world_prev;
			skui_hand[i].pinch_pt      = skui_hand[i].pinch_pt_world;
			skui_hand[i].pinch_pt_prev = skui_hand[i].pinch_pt_world_prev;
		}
	} else {
		layer_t &layer = skui_layers.last();
		for (int32_t i = 0; i < handed_max; i++) {
			skui_hand[i].finger        = layer.finger_pos[i];
			skui_hand[i].finger_prev   = layer.finger_prev[i];
			skui_hand[i].pinch_pt      = layer.pinch_pt_pos[i];
			skui_hand[i].pinch_pt_prev = layer.pinch_pt_prev[i];
		}
	}
}

///////////////////////////////////////////

void ui_layout_area(vec3 start, vec2 dimensions) {
	layer_t &layer = skui_layers.last();
	layer.window         = layer.window;
	layer.offset_initial = start;
	layer.offset         = layer.offset_initial - vec3{ skui_settings.padding, skui_settings.padding };
	layer.size           = dimensions;
	layer.max_x          = 0;
	layer.line_height    = 0;

	skui_prev_offset      = layer.offset;
	skui_prev_line_height = layer.line_height;
}

///////////////////////////////////////////

void ui_layout_area(ui_window_t &window, vec3 start, vec2 dimensions) {
	layer_t &layer = skui_layers.last();
	layer.window         = &window;
	ui_layout_area(start, dimensions);
}

///////////////////////////////////////////

vec2 ui_layout_remaining() {
	layer_t &layer = skui_layers.last();
	return vec2{
		fmaxf(-layer.max_x, fmaxf(layer.size.x, layer.window != nullptr ? layer.window->size.x : 0) - (layer.offset_initial.x - layer.offset.x) - skui_settings.padding),
		fmaxf(0,            fmaxf(layer.size.y, layer.window != nullptr ? layer.window->size.y : 0) + (layer.offset.y + layer.offset_initial.y) - skui_settings.padding)
	};
}

///////////////////////////////////////////

vec2 ui_area_remaining() {
	return ui_layout_remaining();
}

///////////////////////////////////////////

vec3 ui_layout_at() {
	return skui_layers.last().offset;
}

///////////////////////////////////////////

bounds_t ui_layout_last() {
	return skui_recent_layout;
}

///////////////////////////////////////////

void ui_layout_reserve_sz(vec2 size, bool32_t add_padding, vec3 *out_position, vec2 *out_size) {
	if (size.x == 0) size.x = ui_layout_remaining().x - (add_padding ? skui_settings.padding*2 : 0);
	if (size.y == 0) size.y = ui_line_height();

	layer_t &layer = skui_layers.last();

	vec3 final_pos  = layer.offset;
	vec2 final_size = add_padding
		? size + vec2{ skui_settings.padding, skui_settings.padding }*2
		: size;

	// If this is not the first element, and it goes outside the active window,
	// then we'll want to start this element on the next line down
	if (final_pos.x  != layer.offset_initial.x-skui_settings.padding &&
		layer.size.x != 0                                            &&
		final_pos.x - final_size.x < layer.offset_initial.x - layer.size.x + skui_settings.padding)
	{
		ui_nextline();
		final_pos = layer.offset;
	}

	// Track the sizes for this line, for ui_layout_remaining, as well as
	// window auto-sizing.
	if (layer.max_x > layer.offset.x - final_size.x)
		layer.max_x = layer.offset.x - final_size.x;
	if (layer.line_height < final_size.y)
		layer.line_height = final_size.y;

	// Advance the UI layout position
	layer.offset -= vec3{ final_size.x + skui_settings.gutter, 0, 0 };

	ui_nextline();

	// Store this as the most recent layout
	skui_recent_layout = {
		final_pos - vec3{final_size.x/2, final_size.y/2, 0}, 
		{final_size.x, final_size.y, 0} 
	};

	*out_position = skui_recent_layout.center + skui_recent_layout.dimensions/2;
	*out_size     = final_size;
}

///////////////////////////////////////////

bounds_t ui_layout_reserve(vec2 size, bool32_t add_padding, float depth) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, add_padding, &final_pos, &final_size);

	bounds_t result = skui_recent_layout;
	result.center.z -= depth/2;
	result.dimensions.z = depth;
	return result;
}

///////////////////////////////////////////

button_state_ ui_last_element_hand_used(handed_ hand) {
	return button_make_state(
		skui_hand[hand].active_prev == skui_last_element || skui_hand[hand].focused_prev == skui_last_element,
		skui_hand[hand].active      == skui_last_element || skui_hand[hand].focused      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_active() {
	return button_make_state(
		skui_hand[handed_left].active_prev == skui_last_element || skui_hand[handed_right].active_prev == skui_last_element,
		skui_hand[handed_left].active      == skui_last_element || skui_hand[handed_right].active      == skui_last_element);
}

///////////////////////////////////////////

button_state_ ui_last_element_focused() {
	return button_make_state(
		skui_hand[handed_left].focused_prev == skui_last_element || skui_hand[handed_right].focused_prev == skui_last_element,
		skui_hand[handed_left].focused      == skui_last_element || skui_hand[handed_right].focused      == skui_last_element);
}

///////////////////////////////////////////

void ui_nextline() {
	layer_t &layer = skui_layers.last();
	skui_prev_offset      = layer.offset;
	skui_prev_line_height = layer.line_height;

	layer.offset.x    = layer.offset_initial.x - skui_settings.padding;
	layer.offset.y   -= layer.line_height + skui_settings.gutter;
	layer.line_height = 0;
}

///////////////////////////////////////////

void ui_sameline() {
	layer_t &layer = skui_layers.last();
	layer.offset      = skui_prev_offset;
	layer.line_height = skui_prev_line_height;
}

///////////////////////////////////////////

float ui_line_height() {
	return skui_settings.padding * 2 + skui_fontsize;
}

///////////////////////////////////////////

void ui_space(float space) {
	layer_t &layer = skui_layers.last();
	if (layer.offset.x == layer.offset_initial.x - skui_settings.padding)
		layer.offset.y -= space;
	else
		layer.offset.x -= space;
}

///////////////////////////////////////////
///////////   Interaction!   //////////////
///////////////////////////////////////////

void ui_box_interaction_1h_pinch(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_hand) {
	*out_hand        = -1;
	*out_focus_state = button_state_inactive;
	
	skui_last_element = id;

	// If the element is disabled, unfocus it and ditch out
	if (!skui_interact_enabled) {
		*out_focus_state = ui_focus_set(-1, id, false, 0);
		return;
	}

	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < handed_max; i++) {
		if (ui_is_hand_preoccupied((handed_)i, id, false))
			continue;
		bool was_focused = skui_hand[i].focused_prev == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (was_focused) {
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		bool          in_box = skui_hand[i].tracked && ui_in_box(skui_hand[i].pinch_pt, skui_hand[i].pinch_pt_prev, skui_finger_radius, ui_size_box(box_start, box_size));
		button_state_ focus  = ui_focus_set(i, id, in_box, 0);
		if (focus != button_state_inactive) {
			*out_hand        = i;
			*out_focus_state = focus;
		}
	}

	if (*out_hand == -1)
		*out_hand = ui_last_focused_hand(id);
}

///////////////////////////////////////////

void ui_box_interaction_1h_poke(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t *out_hand) {
	*out_hand        = -1;
	*out_focus_state = button_state_inactive;

	skui_last_element = id;

	// If the element is disabled, unfocus it and ditch out
	if (!skui_interact_enabled) {
		*out_focus_state = ui_focus_set(-1, id, false, 0);
		return;
	}

	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	for (int32_t i = 0; i < handed_max; i++) {
		if (ui_is_hand_preoccupied((handed_)i, id, false))
			continue;
		bool was_focused = skui_hand[i].focused_prev == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (was_focused) {
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		bool          in_box = skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, skui_finger_radius, ui_size_box(box_start, box_size));
		button_state_ focus  = ui_focus_set(i, id, in_box, 0);
		if (focus != button_state_inactive) {
			*out_hand        = i;
			*out_focus_state = focus;
		}
	}

	if (*out_hand == -1)
		*out_hand = ui_last_focused_hand(id);
}

///////////////////////////////////////////

bool32_t ui_in_box(vec3 pt, vec3 pt_prev, float radius, bounds_t box) {
	if (skui_show_volumes)
		render_add_mesh(skui_box_dbg, skui_mat_dbg, matrix_trs(box.center, quat_identity, box.dimensions));
	return bounds_capsule_contains(box, pt, pt_prev, radius);
}


///////////////////////////////////////////

bool32_t ui_is_hand_preoccupied(handed_ hand, uint64_t for_el_id, bool32_t include_focused) {
	const ui_hand_t &h = skui_hand[hand];
	return (include_focused && h.focused_prev != 0 && h.focused_prev != for_el_id)
		|| (h.active_prev != 0 && h.active_prev != for_el_id);
}

///////////////////////////////////////////

button_state_ ui_focus_set(int32_t hand, uint64_t for_el_id, bool32_t focused, float priority) {
	bool was_focused = skui_hand[hand].focused_prev == for_el_id;
	bool is_focused  = false;

	if (hand == -1) {
		if      (skui_hand[0].active_prev == for_el_id) hand = 0;
		else if (skui_hand[1].active_prev == for_el_id) hand = 1;
	}
	if (focused && hand >= 0 && hand < 2 && priority <= skui_hand[hand].focus_priority) {
		is_focused = focused;
		skui_hand[hand].focused        = for_el_id;
		skui_hand[hand].focus_priority = priority;
	}

	button_state_ result = button_state_inactive;
	if ( is_focused                ) result  = button_state_active;
	if ( is_focused && !was_focused) result |= button_state_just_active;
	if (!is_focused &&  was_focused) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

button_state_ ui_active_set(int32_t hand, uint64_t for_el_id, bool32_t active) {
	if (hand == -1) return button_state_inactive;

	bool was_active = skui_hand[hand].active_prev == for_el_id;
	bool is_active  = false;

	if (active && (was_active || skui_hand[hand].focused_prev == for_el_id || skui_hand[hand].focused == for_el_id)) {
		is_active = active;
		skui_hand[hand].active = for_el_id;
	}

	button_state_ result = button_state_inactive;
	if ( is_active               ) result  = button_state_active;
	if ( is_active && !was_active) result |= button_state_just_active; 
	if (!is_active &&  was_active) result |= button_state_just_inactive;
	return result;
}

///////////////////////////////////////////

int32_t ui_last_active_hand(uint64_t for_el_id) {
	if      (skui_hand[0].active_prev == for_el_id) return 0;
	else if (skui_hand[1].active_prev == for_el_id) return 1;
	return -1;
}

///////////////////////////////////////////

int32_t ui_last_focused_hand(uint64_t for_el_id) {
	if      (skui_hand[0].focused_prev == for_el_id) return 0;
	else if (skui_hand[1].focused_prev == for_el_id) return 1;
	return -1;
}

///////////////////////////////////////////

void ui_button_behavior(vec3 window_relative_pos, vec2 size, uint64_t id, float &finger_offset, button_state_ &button_state, button_state_ &focus_state) {
	button_state = button_state_inactive;
	focus_state  = button_state_inactive;
	int32_t hand = -1;

	// Button interaction focus is detected out in front of the button to 
	// prevent 'reverse' or 'side' presses where the finger comes from the
	// back or side.
	//
	// Once focused is gained, interaction is tracked within a volume that
	// extends from the detection plane, to a good distance through the
	// button's back. This is to help when the user's finger inevitably goes
	// completely through the button. Width and height is added to this 
	// volume to account for vertical or horizontal movement during a press,
	// such as the downward motion often accompanying a 'poke' motion.
	float activation_plane = skui_settings.depth + skui_finger_radius*2;
	vec3  activation_start = window_relative_pos + vec3{ 0, 0, -activation_plane };
	vec3  activation_size  = vec3{ size.x, size.y, 0.0001f };

	vec3  box_size  = vec3{ size.x + 2*skui_finger_radius, size.y + 2*skui_finger_radius, activation_plane + 6*skui_finger_radius  };
	vec3  box_start = window_relative_pos + vec3{ skui_finger_radius, skui_finger_radius, -activation_plane + box_size.z };
	ui_box_interaction_1h_poke(id,
		activation_start, activation_size,
		box_start,        box_size,
		&focus_state, &hand);


	// If a hand is interacting, adjust the button surface accordingly
	finger_offset = skui_settings.depth;
	if (focus_state & button_state_active) {
		finger_offset = -(skui_hand[hand].finger.z+skui_finger_radius) - window_relative_pos.z;
		bool pressed  = finger_offset < skui_settings.depth / 2;
		button_state  = ui_active_set(hand, id, pressed);
		finger_offset = fminf(fmaxf(2*mm2m, finger_offset), skui_settings.depth);
	} else if (focus_state & button_state_just_inactive) {
		button_state = ui_active_set(hand, id, false);
	}
	
	if (button_state & button_state_just_active)
		sound_play(skui_snd_interact, skui_hand[hand].finger_world, 1);
	else if (button_state & button_state_just_inactive)
		sound_play(skui_snd_uninteract, skui_hand[hand].finger_world, 1);
}

///////////////////////////////////////////
////////   Base Visual Elements   /////////
///////////////////////////////////////////

void ui_box(vec3 start, vec3 size, material_t material, color128 color) {
	if (size.x < skui_box_min.x) size.x = skui_box_min.x;
	if (size.y < skui_box_min.y) size.y = skui_box_min.y;
	if (size.z < skui_box_min.z) size.z = skui_box_min.z;

	vec3   pos = start - size / 2;
	matrix mx  = matrix_trs(pos, quat_identity, size);

	render_add_mesh(skui_box, material, mx, color*skui_tint);
}

///////////////////////////////////////////

void ui_cube(vec3 start, vec3 size, material_t material, color128 color) {
	vec3   pos = start - size / 2;
	matrix mx  = matrix_trs(pos, quat_identity, size);

	render_add_mesh(skui_box_dbg, material, mx, color*skui_tint);
}

///////////////////////////////////////////

void ui_cylinder(vec3 start, float radius, float depth, material_t material, color128 color) {
	vec3   pos = start - (vec3{ radius, radius, depth } / 2);
	matrix mx  = matrix_trs(pos, quat_identity, {radius, radius, depth});

	render_add_mesh(skui_cylinder, material, mx, color*skui_tint);
}

///////////////////////////////////////////

void ui_model_at(model_t model, vec3 start, vec3 size, color128 color) {
	matrix mx = matrix_trs(start, quat_identity, size);
	render_add_model(model, mx, color*skui_tint);
}

///////////////////////////////////////////

template<typename C>
button_state_ ui_volumei_at_g(const C *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) {
	uint64_t      id_hash = ui_stack_hash(id);
	button_state_ result  = button_state_inactive;
	button_state_ focus   = button_state_inactive;
	int32_t       hand    = -1;

	vec3 start = bounds.center + bounds.dimensions / 2;
	if (interact_type == ui_confirm_push) {
		ui_box_interaction_1h_poke(id_hash,
			start, bounds.dimensions,
			start, bounds.dimensions,
			&focus, &hand);
	} else {
		ui_box_interaction_1h_pinch(id_hash,
			start, bounds.dimensions,
			start, bounds.dimensions,
			&focus, &hand);
	}

	bool active = focus & button_state_active && !(focus & button_state_just_inactive);
	if (interact_type != ui_confirm_push && hand != -1) {
		active = input_hand((handed_)hand)->pinch_state & button_state_active;
		// Focus can get lost if the user is dragging outside the box, so set
		// it to focused if it's still active.
		focus = ui_focus_set(hand, id_hash, active || focus & button_state_active, 0);
	}
	result = ui_active_set(hand, id_hash, active);

	if (out_opt_hand        != nullptr) *out_opt_hand        = (handed_)hand;
	if (out_opt_focus_state != nullptr) *out_opt_focus_state = focus;
	return result;
}
button_state_ ui_volumei_at(const char *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state) 
{ return ui_volumei_at_g<char>(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }
button_state_ ui_volumei_at_16(const char16_t *id, bounds_t bounds, ui_confirm_ interact_type, handed_ *out_opt_hand, button_state_ *out_opt_focus_state)
{ return ui_volumei_at_g<char16_t>(id, bounds, interact_type, out_opt_hand, out_opt_focus_state); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_volume_at_g(const C *id, bounds_t bounds) {
	uint64_t id_hash = ui_stack_hash(id);
	bool     result  = false;

	skui_last_element = id_hash;

	for (int32_t i = 0; i < handed_max; i++) {
		bool     was_focused = skui_hand[i].focused_prev == id_hash;
		bounds_t size        = bounds;
		if (was_focused) {
			size.dimensions = bounds.dimensions + vec3_one*skui_settings.padding;
		}

		bool          in_box = skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, skui_finger_radius, size);
		button_state_ state  = ui_focus_set(i, id_hash, in_box, 0);
		if (state & button_state_just_active)
			result = true;
	}

	return result;
}
bool32_t ui_volume_at   (const char     *id, bounds_t bounds) { return ui_volume_at_g<char    >(id, bounds); }
bool32_t ui_volume_at_16(const char16_t *id, bounds_t bounds) { return ui_volume_at_g<char16_t>(id, bounds); }

///////////////////////////////////////////

// TODO: Has no Id, might be good to move this over to ui_focus/active_set around v0.4
button_state_ ui_interact_volume_at(bounds_t bounds, handed_ &out_hand) {
	button_state_ result  = button_state_inactive;

	for (int32_t i = 0; i < handed_max; i++) {
		bool     was_active  = skui_hand[i].active_prev  != 0;
		bool     was_focused = skui_hand[i].focused_prev != 0 || was_active;
		if (was_active || was_focused)
			continue;

		if (skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, skui_finger_radius, bounds)) {
			button_state_ state = input_hand((handed_)i)->pinch_state;
			if (state != button_state_inactive) {
				result = state;
				out_hand = (handed_)i;
			}
		}
	}

	return result;
}

///////////////////////////////////////////

template<typename C, float (*text_add_in_t )(const C *text, const matrix &transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear)>
inline void ui_text_in_g(vec3 start, vec2 size, const C *text, text_align_ position, text_align_ align) {
	text_add_in_t(text, matrix_identity, size, text_fit_squeeze, skui_font_stack.last(), position, align, start.x, start.y, start.z, skui_enabled_stack.last() ? color128{1, 1, 1, 1} : color128{.5f, .5f, .5f, 1});
}
void ui_text_in(vec3 start, vec2 size, const char     *text, text_align_ position, text_align_ align) { ui_text_in_g<char,     text_add_in >(start, size, text, position, align); }
void ui_text_in(vec3 start, vec2 size, const char16_t *text, text_align_ position, text_align_ align) { ui_text_in_g<char16_t, text_add_in_16>(start, size, text, position, align); }

///////////////////////////////////////////
///////////   UI Components   /////////////
///////////////////////////////////////////

void ui_hseparator() {
	vec3 pos;
	vec2 size;
	ui_layout_reserve_sz({ 0, skui_fontsize*0.4f }, false, &pos, &size);

	ui_draw_el(ui_vis_separator, pos, vec3{ size.x, size.y, size.y / 2.0f }, ui_color_primary, 1);
}

///////////////////////////////////////////

template<typename C>
void ui_label_sz_g(const C *text, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	ui_text_in(final_pos - vec3{0,0,skui_settings.depth/2}, final_size, text, text_align_top_left, text_align_center_left);
}
void ui_label_sz   (const char     *text, vec2 size) { ui_label_sz_g<char    >(text, size); }
void ui_label_sz_16(const char16_t *text, vec2 size) { ui_label_sz_g<char16_t>(text, size); }

///////////////////////////////////////////

template<typename C, vec2 (*text_size_t)(const C *text, text_style_t style)>
void ui_label_g(const C *text, bool32_t use_padding) {
	vec3 final_pos;
	vec2 final_size;
	vec2 txt_size = text_size_t(text, skui_font_stack.last());
	ui_layout_reserve_sz(txt_size, use_padding, &final_pos, &final_size);

	float pad = use_padding ? skui_settings.gutter : 0;
	ui_text_in(final_pos - vec3{pad,0,skui_settings.depth/2}, final_size, text, text_align_top_left, text_align_center_left);
}
void ui_label   (const char     *text, bool32_t use_padding) { ui_label_g<char,     text_size   >(text, use_padding); }
void ui_label_16(const char16_t *text, bool32_t use_padding) { ui_label_g<char16_t, text_size_16>(text, use_padding); }

///////////////////////////////////////////

template<typename C, float (*text_add_in_t )(const C *text, const matrix &transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear)>
void ui_text_g(const C *text, text_align_ text_align) {
	vec3  offset   = skui_layers.last().offset;
	vec2  size     = { ui_layout_remaining().x, 0 };

	vec3 at = offset - vec3{ 0, 0, skui_settings.depth / 4 };
	size.y = text_add_in_t(text, matrix_identity, size, text_fit_wrap, skui_font_stack.last(), text_align_top_left, text_align, at.x, at.y, at.z, {1,1,1,1});

	ui_layout_reserve(size);
}
void ui_text   (const char     *text, text_align_ text_align) { ui_text_g<char,     text_add_in   >(text, text_align); }
void ui_text_16(const char16_t *text, text_align_ text_align) { ui_text_g<char16_t, text_add_in_16>(text, text_align); }

///////////////////////////////////////////

void ui_image(sprite_t image, vec2 size) {
	float aspect = sprite_get_aspect(image);
	size = vec2{
		size.x==0 ? size.y*aspect : size.x, 
		size.y==0 ? size.x/aspect : size.y };

	float scale = fminf(size.x / aspect, size.y);

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);
	
	sprite_draw_at(image, matrix_ts(final_pos - vec3{size.x/2,size.y/2,2*mm2m }, vec3{ scale, scale, 1 }), text_align_center);
}

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_at_g(const C *text, vec3 window_relative_pos, vec2 size) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = state & button_state_active ? 2.f : 1;
	if (ui_anim_has(id, .2f)) {
		float t     = ui_anim_elapsed    (id, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	float activation = 1 + 1-(finger_offset / skui_settings.depth);
	ui_draw_el(ui_vis_button, window_relative_pos,  vec3{ size.x,   size.y,   finger_offset }, ui_color_common, fmaxf(activation,color_blend));
	ui_text_in(               window_relative_pos - vec3{ size.x/2, size.y/2, finger_offset + 2*mm2m }, vec2{size.x-skui_settings.padding*2, size.y-skui_settings.padding*2}, text, text_align_center, text_align_center);

	return state & button_state_just_active;
}
bool32_t ui_button_at   (const char     *text, vec3 window_relative_pos, vec2 size) { return ui_button_at_g<char    >(text, window_relative_pos, size); }
bool32_t ui_button_at   (const char16_t *text, vec3 window_relative_pos, vec2 size) { return ui_button_at_g<char16_t>(text, window_relative_pos, size); }
bool32_t ui_button_at_16(const char16_t *text, vec3 window_relative_pos, vec2 size) { return ui_button_at_g<char16_t>(text, window_relative_pos, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_img_at_g(const C* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = state & button_state_active ? 2.f : 1;
	if (ui_anim_has(id, .2f)) {
		float t = ui_anim_elapsed(id, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	float activation = 1 + 1 - (finger_offset / skui_settings.depth);
	ui_draw_el(ui_vis_button, window_relative_pos, vec3{ size.x,size.y,finger_offset }, ui_color_common, fmaxf(activation, color_blend));
	
	float pad2       = skui_settings.padding * 2;
	float pad2gutter = pad2 + skui_settings.gutter;
	float depth      = finger_offset + 2 * mm2m;
	vec3  image_at;
	float image_size;
	text_align_ image_align;
	vec3  text_at;
	vec2  text_size;
	text_align_ text_align;
	float aspect = sprite_get_aspect(image);
	switch (image_layout) {
	default:
	case ui_btn_layout_left:
		image_align = text_align_center_left;
		image_size  = fminf(size.y - pad2, ((size.x - pad2gutter)*0.5f) / aspect);
		image_at    = window_relative_pos - vec3{ skui_settings.padding, size.y/2, depth };
			
		text_align = text_align_center_right;
		text_at    = window_relative_pos - vec3{ size.x-skui_settings.padding, size.y/2, depth };
		text_size  = { size.x - (image_size * aspect + pad2gutter), size.y - pad2 };
		break;
	case ui_btn_layout_right:
		image_align = text_align_center_right;
		image_at    = window_relative_pos - vec3{ size.x-skui_settings.padding, size.y / 2, depth };
		image_size  = fminf(size.y - pad2, ((size.x - pad2gutter) * 0.5f) / aspect);
			
		text_align = text_align_center_left;
		text_at    = window_relative_pos - vec3{ skui_settings.padding, size.y / 2, depth };
		text_size  = { size.x - (image_size * aspect + pad2gutter), size.y - pad2 };
		break;
	case ui_btn_layout_center_no_text:
	case ui_btn_layout_center:
		image_align = text_align_center;
		image_size  = fminf(size.y - pad2, (size.x - pad2) / aspect);
		image_at    = window_relative_pos - vec3{ size.x/2, size.y / 2, depth }; 
			
		text_align = text_align_top_center;
		float y = size.y / 2 + image_size / 2;
		text_at    = window_relative_pos - vec3{size.x/2, y, depth};
		text_size  = { size.x-pad2, (size.y-skui_settings.padding*0.25f)-y };
		break;
	}

	if (image_size>0) {
		color128 final_color = skui_tint;
		if (!skui_enabled_stack.last()) final_color = final_color * color128{ .5f, .5f, .5f, 1 };
	
		sprite_draw_at(image, matrix_ts(image_at, { image_size, image_size, image_size }), image_align, color_to_32( final_color ));
		if (image_layout != ui_btn_layout_center_no_text)
			ui_text_in(text_at, text_size, text, text_align, text_align_center);
	}
	return state & button_state_just_active;
}
bool32_t ui_button_img_at   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char>(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at   (const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_sz_g(const C *text, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_button_at(text, final_pos, final_size);
}
bool32_t ui_button_sz   (const char     *text, vec2 size) { return ui_button_sz_g<char    >(text, size); }
bool32_t ui_button_sz_16(const char16_t *text, vec2 size) { return ui_button_sz_g<char16_t>(text, size); }

///////////////////////////////////////////

template<typename C, vec2 (*text_size_t)(const C *text, text_style_t style)>
bool32_t ui_button_g(const C *text) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(text_size_t(text, skui_font_stack.last()), true, &final_pos, &final_size);

	return ui_button_at(text, final_pos, final_size);
}
bool32_t ui_button   (const char     *text) { return ui_button_g<char,     text_size   >(text); }
bool32_t ui_button_16(const char16_t *text) { return ui_button_g<char16_t, text_size_16>(text); }

///////////////////////////////////////////

template<typename C, vec2(*text_size_t)(const C *text, text_style_t style)>
bool32_t ui_button_img_g(const C *text, sprite_t image, ui_btn_layout_ image_layout) {
	vec3 final_pos;
	vec2 final_size;

	vec2 size = {};
	if (image_layout == ui_btn_layout_center || image_layout == ui_btn_layout_center_no_text) {
		size = { skui_fontsize, skui_fontsize };
	} else {
		vec2  txt_size   = text_size_t(text, skui_font_stack.last());
		float aspect     = sprite_get_aspect(image);
		float image_size = skui_fontsize * aspect;
		size = vec2{ txt_size.x + image_size + skui_settings.gutter, skui_fontsize };
	}

	ui_layout_reserve_sz(size, true, &final_pos, &final_size);
	return ui_button_img_at(text, image, image_layout, final_pos, final_size);
}
bool32_t ui_button_img   (const char     *text, sprite_t image, ui_btn_layout_ image_layout) { return ui_button_img_g<char,     text_size   >(text, image, image_layout); }
bool32_t ui_button_img_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout) { return ui_button_img_g<char16_t, text_size_16>(text, image, image_layout); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_img_sz_g(const C *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size) {
	vec3 final_pos;
	vec2 final_size;

	ui_layout_reserve_sz(size, false, &final_pos, &final_size);
	return ui_button_img_at(text, image, image_layout, final_pos, final_size);
}
bool32_t ui_button_img_sz   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size) { return ui_button_img_sz_g<char    >(text, image, image_layout, size); }
bool32_t ui_button_img_sz_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec2 size) { return ui_button_img_sz_g<char16_t>(text, image, image_layout, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_at_g(const C *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = pressed || focus & button_state_active ? 2.f : 1;
	if (ui_anim_has(id, .2f)) {
		float t     = ui_anim_elapsed    (id, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	if (state & button_state_just_active) {
		pressed = !pressed;
	}
	finger_offset = pressed ? fminf(skui_settings.backplate_depth*skui_settings.depth + mm2m, finger_offset) : finger_offset;

	ui_draw_el(ui_vis_toggle, window_relative_pos,  vec3{ size.x,    size.y,   finger_offset }, ui_color_common, color_blend);
	ui_text_in(               window_relative_pos - vec3{ size.x/2,  size.y/2, finger_offset + 2*mm2m }, vec2{size.x-skui_settings.padding*2, size.y-skui_settings.padding*2}, text, text_align_center, text_align_center);

	return state & button_state_just_active;
}
bool32_t ui_toggle_at   (const char     *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_at_g<char    >(text, pressed, window_relative_pos, size); }
bool32_t ui_toggle_at   (const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_at_g<char16_t>(text, pressed, window_relative_pos, size); }
bool32_t ui_toggle_at_16(const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_at_g<char16_t>(text, pressed, window_relative_pos, size); }

///////////////////////////////////////////

template<typename C, vec2 (*text_size_t)(const C *text, text_style_t style)>
bool32_t ui_toggle_g(const C *text, bool32_t &pressed) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(text_size_t(text, skui_font_stack.last()), true, &final_pos, &final_size);

	return ui_toggle_at(text, pressed, final_pos, final_size);
}
bool32_t ui_toggle   (const char     *text, bool32_t &pressed) { return ui_toggle_g<char,     text_size   >(text, pressed); }
bool32_t ui_toggle_16(const char16_t *text, bool32_t &pressed) { return ui_toggle_g<char16_t, text_size_16>(text, pressed); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_sz_g(const C *text, bool32_t &pressed, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_toggle_at(text, pressed, final_pos, final_size);
}
bool32_t ui_toggle_sz   (const char     *text, bool32_t &pressed, vec2 size) { return ui_toggle_sz_g<char    >(text, pressed, size); }
bool32_t ui_toggle_sz_16(const char16_t *text, bool32_t &pressed, vec2 size) { return ui_toggle_sz_g<char16_t>(text, pressed, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_round_at_g(const C *text, sprite_t image, vec3 window_relative_pos, float diameter) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, { diameter,diameter }, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = state & button_state_active ? 2.f : 1;
	float back_size   = skui_settings.backplate_border;
	if (ui_anim_has(id, .2f)) {
		float t     = ui_anim_elapsed    (id, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	ui_cylinder(window_relative_pos, diameter, finger_offset, skui_mat, skui_palette[2] * color_blend);
	ui_cylinder(window_relative_pos + vec3{back_size, back_size, mm2m}, diameter+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m, skui_mat, skui_color_border * color_blend);

	float sprite_scale = fmaxf(1, sprite_get_aspect(image));
	float sprite_size  = (diameter * 0.8f) / sprite_scale;
	sprite_draw_at(image, matrix_ts(window_relative_pos + vec3{ -diameter/2, -diameter/2, -(finger_offset + 2*mm2m) }, vec3{ sprite_size, sprite_size, 1 }), text_align_center);

	return state & button_state_just_active;
}
bool32_t ui_button_round_at   (const char     *text, sprite_t image, vec3 window_relative_pos, float diameter) { return ui_button_round_at_g<char    >(text, image, window_relative_pos, diameter); }
bool32_t ui_button_round_at   (const char16_t *text, sprite_t image, vec3 window_relative_pos, float diameter) { return ui_button_round_at_g<char16_t>(text, image, window_relative_pos, diameter); }
bool32_t ui_button_round_at_16(const char16_t *text, sprite_t image, vec3 window_relative_pos, float diameter) { return ui_button_round_at_g<char16_t>(text, image, window_relative_pos, diameter); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_round_g(const C *id, sprite_t image, float diameter) {
	if (diameter == 0)
		diameter = ui_line_height();
	vec2 size = vec2{diameter, diameter};
	size = vec2_one * fmaxf(size.x, size.y);

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_button_round_at(id, image, final_pos, final_size.x);
}
bool32_t ui_button_round   (const char     *id, sprite_t image, float diameter) { return ui_button_round_g<char>(id, image, diameter); }
bool32_t ui_button_round_16(const char16_t *id, sprite_t image, float diameter) { return ui_button_round_g<char16_t>(id, image, diameter); }

///////////////////////////////////////////

void ui_model(model_t model, vec2 ui_size, float model_scale) {
	if (ui_size.x == 0) ui_size.x = ui_layout_remaining().x - (skui_settings.padding*2);
	if (ui_size.y == 0) {
		bounds_t bounds = model_get_bounds(model);
		ui_size.y = ui_size.x * (bounds.dimensions.x / bounds.dimensions.y);
	}
	if (model_scale == 0) {
		bounds_t bounds = model_get_bounds(model);
		model_scale = fminf(ui_size.x / bounds.dimensions.x, ui_size.y / bounds.dimensions.y);
	}
	vec2 size = ui_size + vec2{ skui_settings.padding, skui_settings.padding }*2;

	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	final_size = final_size / 2;
	ui_model_at(model, { final_pos.x - final_size.x, final_pos.y - final_size.y, final_pos.z }, vec3_one * model_scale, { 1,1,1,1 });
}

///////////////////////////////////////////

inline bool    utf_is_start     (char     ch) { return utf8_is_start(ch); }
inline bool    utf_is_start     (char16_t ch) { return utf16_is_start(ch); }
inline int32_t utf_encode_append(char     *buffer, size_t size, char32_t ch) { return utf8_encode_append (buffer, size, ch); }
inline int32_t utf_encode_append(char16_t *buffer, size_t size, char32_t ch) { return utf16_encode_append(buffer, size, ch); }

inline vec2 text_char_at_o(const char*     text, text_style_t style, int32_t char_index, vec2* opt_size, text_fit_ fit, text_align_ position, text_align_ align) { return text_char_at   (text, style, char_index, opt_size, fit, position, align); }
inline vec2 text_char_at_o(const char16_t* text, text_style_t style, int32_t char_index, vec2* opt_size, text_fit_ fit, text_align_ position, text_align_ align) { return text_char_at_16(text, style, char_index, opt_size, fit, position, align); }

int32_t skui_input_carat     = 0;
int32_t skui_input_carat_end = 0;
float   skui_input_blink     = 0;

template<typename C, vec2 (*text_size_t)(const C *text, text_style_t style)>
bool32_t ui_input_g(const C *id, C *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	uint64_t id_hash  = ui_stack_hash(id);
	bool     result   = false;
	vec3     box_size = vec3{ final_size.x, final_size.y, skui_settings.depth/2 };

	// Find out if the user is trying to focus this UI element
	button_state_ focus;
	int32_t       hand;
	ui_box_interaction_1h_poke(id_hash, final_pos, box_size, final_pos, box_size, &focus, &hand);
	button_state_ state = ui_active_set(hand, id_hash, focus & button_state_active);

	if (state & button_state_just_active) {
		platform_keyboard_show(true,type);
		skui_input_blink  = time_getf();
		skui_input_target = id_hash;
		skui_input_carat  = skui_input_carat_end = utf_charlen(buffer);
		sound_play(skui_snd_interact, skui_hand[hand].finger_world, 1);
	}

	if (state & button_state_just_active)
		ui_anim_start(id_hash);
	float color_blend = skui_input_target == id_hash ? 2.f : 1;
	if (ui_anim_has(id_hash, .2f)) {
		float t     = ui_anim_elapsed    (id_hash, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	// Unfocus this if the user starts interacting with something else
	if (skui_input_target == id_hash) {
		for (int32_t i = 0; i < handed_max; i++) {
			if (ui_is_hand_preoccupied((handed_)i, id_hash, false)) {
				const ui_hand_t& h = skui_hand[i];
				if (h.focused_prev && skui_preserve_keyboard_ids_read->index_of(h.focused_prev) < 0) { 
					skui_input_target = 0;
					platform_keyboard_show(false, type);
				}
			}
		}
	}

	// If focused, acquire any input in the keyboard's queue
	if (skui_input_target == id_hash) {
		uint32_t curr = input_text_consume();
		while (curr != 0) {
			uint32_t add = '\0';
			log_infof("%d", curr);

			if (curr == key_backspace) {
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(buffer + start, count);
					skui_input_carat_end = skui_input_carat = start;
				} else if (skui_input_carat > 0) {
					skui_input_carat_end = skui_input_carat = skui_input_carat - 1;
					utf_remove_chars(buffer + skui_input_carat, 1);
				}
			} else if (curr == 0x7f) {
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(buffer + start, count);
					skui_input_carat_end = skui_input_carat = start;
				} else if (skui_input_carat >= 0) {
					utf_remove_chars(buffer + skui_input_carat, 1);
				}
			} else if (curr == 0x0D) { // Enter, carriage return
				skui_input_target = 0;
				platform_keyboard_show(false, type);
				result = true;
			} else if (curr == 0x0A) { // Shift+Enter, linefeed
				add = '\n';
			} else if (curr == 0x1B) { // Escape
				skui_input_target = 0;
				platform_keyboard_show(false, type);
			} else {
				add = curr;
			}

			if (add != '\0') {
				// Remove any selected
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(buffer + start, count);
					skui_input_carat_end = skui_input_carat = start;
				}
				utf_insert_char(buffer, buffer_size, buffer + skui_input_carat, add);
				skui_input_carat += 1;
				skui_input_carat_end = skui_input_carat;
				result = true;
			}

			curr = input_text_consume();
		}
		if      (input_key(key_shift) & button_state_active && input_key(key_left ) & button_state_just_active) { skui_input_blink = time_getf(); skui_input_carat = maxi(0, skui_input_carat - 1); }
		else if (input_key(key_left ) & button_state_just_active)                                               { skui_input_blink = time_getf(); if (skui_input_carat_end == skui_input_carat) skui_input_carat = maxi(0, skui_input_carat - 1); skui_input_carat_end = skui_input_carat; }
		if      (input_key(key_shift) & button_state_active && input_key(key_right) & button_state_just_active) { skui_input_blink = time_getf(); skui_input_carat = mini((int32_t)utf_charlen(buffer), skui_input_carat + 1); }
		else if (input_key(key_right) & button_state_just_active)                                               { skui_input_blink = time_getf(); if (skui_input_carat_end == skui_input_carat) skui_input_carat = mini((int32_t)utf_charlen(buffer), skui_input_carat + 1); skui_input_carat_end = skui_input_carat; }
	}

	// Render the input UI
	vec2 text_bounds = { final_size.x - skui_settings.padding * 2,final_size.y - skui_settings.padding * 2 };
	ui_draw_el(ui_vis_input, final_pos, vec3{ final_size.x, final_size.y, skui_settings.depth/2 }, ui_color_common, color_blend);
	ui_text_in(              final_pos - vec3{ skui_settings.padding, skui_settings.padding, skui_settings.depth/2 + 2*mm2m }, text_bounds, buffer, text_align_top_left, text_align_center_left);
	
	float line      = ui_line_height() * 0.5f;
	vec2  carat_pos = text_char_at_o(buffer, skui_font_stack.last(), skui_input_carat, &text_bounds, text_fit_squeeze, text_align_top_left, text_align_center_left);
	if (skui_input_carat != skui_input_carat_end) {
		vec2  carat_end = text_char_at_o(buffer, skui_font_stack.last(), skui_input_carat_end, &text_bounds, text_fit_squeeze, text_align_top_left, text_align_center_left);
		float left      = fmaxf(carat_pos.x, carat_end.x);
		float right     = fminf(carat_pos.x, carat_end.x);
		ui_cube(final_pos - vec3{ skui_settings.padding - left, skui_settings.padding - carat_pos.y, skui_settings.depth/2 + 1*mm2m }, vec3{ -(right-left), line, line * 0.01f }, skui_mat, skui_palette[3]);
	}
	// Show a blinking text carat
	if (skui_input_target == id_hash && (int)((time_getf()-skui_input_blink)*2)%2==0) {
		
		ui_cube(final_pos - vec3{ skui_settings.padding - carat_pos.x, skui_settings.padding - carat_pos.y, skui_settings.depth/2 }, vec3{ line * 0.1f, line, line * 0.1f }, skui_mat, skui_palette[4]);
	}

	return result;
}

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	return ui_input_g<char, text_size>(id, buffer, buffer_size, size, type);
}
bool32_t ui_input_16(const char16_t *id, char16_t *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	return ui_input_g<char16_t, text_size_16>(id, buffer, buffer_size, size, type);
}

///////////////////////////////////////////

void ui_progress_bar_at_ex(float percent, vec3 start_pos, vec2 size, float focus) {
	// Find sizes of bar elements
	float bar_height = fmaxf(skui_settings.padding, size.y / 6.f);
	float bar_depth  = bar_height * skui_settings.backplate_depth - mm2m;
	float bar_y      = start_pos.y - size.y / 2.f + bar_height / 2.f;

	// If the left or right side of the bar is too small, then we'll just draw
	// a single solid bar.
	float bar_length = math_saturate(percent) * size.x;
	vec2  min_size   = ui_get_mesh_minsize(ui_vis_slider_line);
	if (bar_length <= min_size.x) {
		ui_draw_el(ui_vis_slider_line,
			vec3{ start_pos.x, bar_y,      start_pos.z },
			vec3{ size.x,      bar_height, bar_depth },
			ui_color_common, focus);
		return;
	} else if (bar_length >= size.x-min_size.x) {
		ui_draw_el(ui_vis_slider_line,
			vec3{ start_pos.x, bar_y,      start_pos.z },
			vec3{ size.x,      bar_height, bar_depth },
			ui_color_primary, focus);
		return;
	}

	// Slide line
	ui_draw_el(ui_vis_slider_line,
		vec3{ start_pos.x, bar_y,      start_pos.z },
		vec3{ bar_length,  bar_height, bar_depth },
		ui_color_primary, focus);
	ui_draw_el(ui_vis_slider_line,
		vec3{ start_pos.x - bar_length, bar_y,      start_pos.z },
		vec3{ size.x      - bar_length, bar_height, bar_depth },
		ui_color_common, focus);
}

///////////////////////////////////////////

void ui_progress_bar_at(float percent, vec3 start_pos, vec2 size) {
	ui_progress_bar_at_ex(percent, start_pos, size, 1);
}

///////////////////////////////////////////

void ui_progress_bar(float percent, float width) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz({ width, 0 }, false, &final_pos, &final_size);

	return ui_progress_bar_at(percent, final_pos, final_size);
}

///////////////////////////////////////////

template<typename C, typename N>
bool32_t ui_hslider_at_g(const C *id_text, N &value, N min, N max, N step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) {
	uint64_t id     = ui_stack_hash(id_text);
	bool     result = false;

	const float snap_scale = 1;
	const float snap_dist  = 7*cm2m;

	// Find sizes of slider elements
	float percent      = (float)((value - min) / (max - min));
	float button_depth = confirm_method == ui_confirm_push ? skui_settings.depth : skui_settings.depth * 1.5f;
	float rule_size    = fmaxf(skui_settings.padding, size.y / 6.f);
	vec2  button_size  = confirm_method == ui_confirm_push
		? vec2{ size.y / 2, size.y / 2 }
		: vec2{ size.y / 4, size.y };

	// Activation bounds sizing
	float activation_plane = button_depth + skui_finger_radius;

	// Set up for getting the state of the sliders.
	button_state_ focus_state   = button_state_inactive;
	button_state_ button_state  = button_state_inactive;
	float         finger_offset = button_depth;
	float         finger_x      = 0;
	int32_t       hand          = -1;
	if (confirm_method == ui_confirm_push) {
		vec3  activation_start = window_relative_pos + vec3{ percent * -(size.x-button_size.x) + button_size.x/2.0f, -(size.y/2 - button_size.y/2) + button_size.y/2.0f, -activation_plane };
		vec3  activation_size  = vec3{ button_size.x*2, button_size.y*2, 0.0001f };
		vec3  sustain_size     = vec3{ size.x + 2*skui_finger_radius, size.y + 2*skui_finger_radius, activation_plane + 6*skui_finger_radius  };
		vec3  sustain_start    = window_relative_pos + vec3{ skui_finger_radius, skui_finger_radius, -activation_plane + sustain_size.z };

		ui_box_interaction_1h_poke(id,
			activation_start, activation_size,
			sustain_start,    sustain_size,
			&focus_state, &hand);

		// Push confirm is like having a regular button on the slider, and it
		// only slides when that button is pressed.
		if (focus_state & button_state_active) {
			finger_offset = -skui_hand[hand].finger.z - window_relative_pos.z;
			bool pressed  = finger_offset < button_depth / 2;
			button_state  = ui_active_set(hand, id, pressed);
			finger_offset = fminf(fmaxf(2*mm2m, finger_offset), button_depth);
		} else if (focus_state & button_state_just_inactive) {
			button_state = ui_active_set(hand, id, false);
		}
		if (hand != -1)
			finger_x = skui_hand[hand].finger.x;
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		vec3 activation_start = window_relative_pos + vec3{ percent * -(size.x-button_size.x) + button_size.x, -(size.y/2 - button_size.y/2), button_depth };
		vec3 activation_size  = vec3{ button_size.x*3, button_size.y, button_depth*2 };

		ui_box_interaction_1h_pinch(id,
			activation_start, activation_size,
			activation_start, activation_size,
			&focus_state, &hand);

		// Pinch confirm uses a handle that the user must pinch, in order to
		// drag it around the slider.
		if (hand != -1) {
			const hand_t *h     = input_hand((handed_)hand);
			button_state_ pinch = h->pinch_state;
			button_state = ui_active_set(hand, id, pinch & button_state_active);
			// Focus can get lost if the user is dragging outside the box, so set
			// it to focused if it's still active.
			focus_state = ui_focus_set(hand, id, button_state & button_state_active || focus_state & button_state_active, 0);
			vec3    pinch_local = hierarchy_to_local_point(h->pinch_pt);
			int32_t scale_step  = (int32_t)((-pinch_local.z-activation_plane) / snap_dist);
			finger_x = pinch_local.x;

			if (confirm_method == ui_confirm_variable_pinch && button_state & button_state_active && scale_step > 0) {
				finger_x = finger_x / (1 + scale_step * snap_scale);
			}
		}
	}

	if (button_state & button_state_active) {
		float pos_in_slider = (float)fmin(1, fmax(0, ((window_relative_pos.x-button_size.x/2)-finger_x) / (size.x-button_size.x)));
		N new_val = (N)min + (N)pos_in_slider*(N)(max-min);
		if (step != 0) {
			new_val = min + ((int)(((new_val - min) / step) + (N)0.5)) * step;
		}
		result = value != new_val;
		value = new_val;
		percent = (float)((value - min) / (max - min));
	}

	if (button_state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = focus_state & button_state_active ? 2.f : 1;
	if (ui_anim_has(id, .2f)) {
		float t     = ui_anim_elapsed    (id, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	// Draw the UI
	float x           = window_relative_pos.x;
	float line_y      = window_relative_pos.y - size.y/2.f + rule_size / 2.f;
	float slide_x_rel = (float)(percent * (size.x-button_size.x));
	float slide_y     = window_relative_pos.y - (size.y-button_size.y)/2;

	ui_progress_bar_at_ex(percent, window_relative_pos, size, color_blend);

	if (confirm_method == ui_confirm_push) {
		ui_draw_el(ui_vis_slider_push,
			vec3{ x - slide_x_rel, slide_y, window_relative_pos.z}, 
			vec3{button_size.x, button_size.y, fmaxf(finger_offset,rule_size*skui_settings.backplate_depth+mm2m)},
			ui_color_primary, color_blend);
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		ui_draw_el(ui_vis_slider_pinch,
			vec3{ x - slide_x_rel, slide_y, window_relative_pos.z},
			vec3{ button_size.x, button_size.y, button_depth}, 
			ui_color_primary, color_blend);

		vec3 pinch_local = hand < 0
			? vec3_zero
			: hierarchy_to_local_point(input_hand((handed_)hand)->pinch_pt);
		int32_t scale_step  = (int32_t)((-pinch_local.z-activation_plane) / snap_dist);
		if (confirm_method == ui_confirm_variable_pinch && button_state & button_state_active && scale_step > 0) {
			float scale    = 1 + scale_step * snap_scale;
			float z        = -activation_plane - (scale_step * snap_dist) + button_depth/2;
			float scaled_x = x+size.x*(scale-1)*0.5f;
			
			float connector_y = line_y - rule_size * 0.5f;
			line_add({ x,        connector_y, window_relative_pos.z}, { scaled_x,              connector_y, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
			line_add({ x-size.x, connector_y, window_relative_pos.z}, { scaled_x-size.x*scale, connector_y, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);

			ui_draw_el(ui_vis_slider_line,
				vec3{ scaled_x, line_y, window_relative_pos.z + z },
				vec3{ slide_x_rel*scale + button_size.x/2, rule_size, rule_size * skui_settings.backplate_depth - mm2m },
				ui_color_primary, color_blend);
			ui_draw_el(ui_vis_slider_line,
				vec3{ scaled_x - slide_x_rel*scale - button_size.x/2, line_y, window_relative_pos.z + z },
				vec3{ (size.x-slide_x_rel)*scale, rule_size, rule_size * skui_settings.backplate_depth - mm2m },
				ui_color_common, color_blend);

			ui_draw_el(ui_vis_slider_pinch,
				vec3{ scaled_x - slide_x_rel*scale, slide_y, window_relative_pos.z+z},
				vec3{ button_size.x, button_size.y, button_depth}, 
				ui_color_primary, color_blend);
		}
	}
	
	if (hand >= 0 && hand < 2) {
		if (button_state & button_state_just_active)
			sound_play(skui_snd_interact, skui_hand[hand].finger_world, 1);
		else if (button_state & button_state_just_inactive)
			sound_play(skui_snd_uninteract, skui_hand[hand].finger_world, 1);
	}

	return result;
}
bool32_t ui_hslider_at   (const char     *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) { return ui_hslider_at_g<char    , float>(id_text, value, min, max, step, window_relative_pos, size, confirm_method); }
bool32_t ui_hslider_at   (const char16_t *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) { return ui_hslider_at_g<char16_t, float>(id_text, value, min, max, step, window_relative_pos, size, confirm_method); }
bool32_t ui_hslider_at_16(const char16_t *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) { return ui_hslider_at_g<char16_t, float>(id_text, value, min, max, step, window_relative_pos, size, confirm_method); }

bool32_t ui_hslider_at_f64   (const char     *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) { return ui_hslider_at_g<char    , double>(id_text, value, min, max, step, window_relative_pos, size, confirm_method); }
bool32_t ui_hslider_at_f64   (const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) { return ui_hslider_at_g<char16_t, double>(id_text, value, min, max, step, window_relative_pos, size, confirm_method); }
bool32_t ui_hslider_at_f64_16(const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method) { return ui_hslider_at_g<char16_t, double>(id_text, value, min, max, step, window_relative_pos, size, confirm_method); }

///////////////////////////////////////////

template<typename C, typename N>
bool32_t ui_hslider_g(const C *name, N &value, N min, N max, N step, float width, ui_confirm_ confirm_method) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz({width, 0}, false, &final_pos, &final_size);

	return ui_hslider_at_g<C, N>(name, value, min, max, step, final_pos, final_size, confirm_method);
}

bool32_t ui_hslider   (const char     *name, float &value, float min, float max, float step, float width, ui_confirm_ confirm_method) { return ui_hslider_g<char, float>(name, value, min, max, step, width, confirm_method); }
bool32_t ui_hslider_16(const char16_t *name, float &value, float min, float max, float step, float width, ui_confirm_ confirm_method) { return ui_hslider_g<char16_t, float>(name, value, min, max, step, width, confirm_method); }

bool32_t ui_hslider_f64   (const char     *name, double &value, double min, double max, double step, float width, ui_confirm_ confirm_method) { return ui_hslider_g<char, double>(name, value, min, max, step, width, confirm_method); }
bool32_t ui_hslider_f64_16(const char16_t *name, double &value, double min, double max, double step, float width, ui_confirm_ confirm_method) { return ui_hslider_g<char16_t, double>(name, value, min, max, step, width, confirm_method); }

///////////////////////////////////////////

bool32_t _ui_handle_begin(uint64_t id, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type) {
	bool result = false;
	float color = 1;

	skui_last_element = id;

	matrix to_local = *hierarchy_to_local();
	matrix to_world = *hierarchy_to_world();
	ui_push_surface(movement);

	// If the handle is scale of zero, we don't actually want to draw or interact with it
	if (handle.dimensions.x == 0 || handle.dimensions.y == 0 || handle.dimensions.z == 0)
		return false;

	if (skui_preserve_keyboard_stack.last()) {
		skui_preserve_keyboard_ids_write->add(id);
	}

	vec3     box_start = handle.center;//   +vec3{ skui_settings.padding, skui_settings.padding, skui_settings.padding };
	vec3     box_size  = handle.dimensions + vec3{ skui_settings.padding, skui_settings.padding, skui_settings.padding } *2;
	bounds_t box       = bounds_t{box_start, box_size};

	static vec3 start_2h_pos = {};
	static quat start_2h_rot = { quat_identity };
	static vec3 start_2h_handle_pos = {};
	static quat start_2h_handle_rot = { quat_identity };
	static vec3 start_handle_pos[2] = {};
	static quat start_handle_rot[2] = { quat_identity,quat_identity };
	static vec3 start_palm_pos  [2] = {};
	static quat start_palm_rot  [2] = { quat_identity,quat_identity };

	if (!skui_interact_enabled) {
		ui_focus_set(-1, id, false, 0);
	} else {
		vec3 finger_pos[2] = {};
		for (int32_t i = 0; i < handed_max; i++) {
			// Skip this if something else has some focus!
			if (ui_is_hand_preoccupied((handed_)i, id, false))
				continue;

			const hand_t *hand = input_hand((handed_)i);
			finger_pos[i] = matrix_transform_pt( to_local, hand->pinch_pt );

			// Check to see if the handle has focus
			vec3  from_pt             = finger_pos[i];
			bool  has_hand_attention  = skui_hand[i].active_prev == id;
			float hand_attention_dist = 0;
			if (ui_in_box(skui_hand[i].pinch_pt, skui_hand[i].pinch_pt_prev, skui_finger_radius, box)) {
				has_hand_attention = true;
			} else if (skui_hand[i].ray_enabled && skui_enable_far_interact) {
				pointer_t *ptr = input_get_pointer(input_hand_pointer_id[i]);
				if (ptr->tracked & button_state_active) {
					vec3  at;
					ray_t far_ray = { hierarchy_to_local_point    (ptr->ray.pos), 
									  hierarchy_to_local_direction(ptr->ray.dir)};
					if (bounds_ray_intersect(box, far_ray, &at)) {
						vec3  window_world = hierarchy_to_world_point(at);
						float curr_mag     = vec3_magnitude_sq(input_head_pose_world.position - window_world);
						float hand_dist    = vec3_magnitude_sq(hand->palm.position - window_world);
					
						if (curr_mag < 0.65f * 0.65f || hand_dist < 0.2f * 0.2f) {
							// Reset id to zero if we found a window that's within touching distance
							ui_focus_set(i, 0, true, 10);
							skui_hand[i].ray_discard = true;
						} else {
							has_hand_attention  = true;
							hand_attention_dist = curr_mag + 10;
						}
					}
				}
			}
			button_state_ focused = ui_focus_set(i, id, has_hand_attention, hand_attention_dist);

			// If this is the second frame this window has focus for, and it's at
			// a distance, then draw a line to it.
			if (hand_attention_dist && focused & button_state_active && !(focused & button_state_just_active)) {
				pointer_t *ptr   = input_get_pointer(input_hand_pointer_id[i]);
				vec3       start = hierarchy_to_local_point(ptr->ray.pos);
				line_add(start*0.75f, vec3_zero, { 50,50,50,0 }, { 255,255,255,255 }, 0.002f);
				from_pt = matrix_transform_pt(to_local, hierarchy_to_world_point(vec3_zero));
			}

			// This waits until the window has been focused for a frame,
			// otherwise the handle UI may try and use a frame of focus to move
			// around a bit.
			if (skui_hand[i].focused_prev ==  id) {
				color = 1.5f;
				if (hand->pinch_state & button_state_just_active) {
					sound_play(skui_snd_grab, skui_hand[i].finger_world, 1);

					skui_hand[i].active = id;
					start_handle_pos[i] = movement.position;
					start_handle_rot[i] = movement.orientation;
					start_palm_pos  [i] = from_pt;
					start_palm_rot  [i] = matrix_transform_quat( to_local, hand->palm.orientation);
				}
				if (skui_hand[i].active_prev == id || skui_hand[i].active == id) {
					result = true;
					skui_hand[i].active = id;
					skui_hand[i].focused = id;

					quat dest_rot;
					vec3 dest_pos;

					// If both hands are interacting with this handle, then we do
					// a two handed interaction from the second hand.
					if (skui_hand[0].active_prev == id && skui_hand[1].active_prev == id || (skui_hand[0].active == id && skui_hand[1].active == id)) {
						if (i == 1) {
							dest_rot = quat_lookat(finger_pos[0], finger_pos[1]);
							dest_pos = finger_pos[0]*0.5f + finger_pos[1]*0.5f;

							if ((input_hand(handed_left)->pinch_state & button_state_just_active) || (input_hand(handed_right)->pinch_state & button_state_just_active)) {
								start_2h_pos = dest_pos;
								start_2h_rot = dest_rot;
								start_2h_handle_pos = movement.position;
								start_2h_handle_rot = movement.orientation;
							}

							switch (move_type) {
							case ui_move_exact: {
								dest_rot = quat_lookat(finger_pos[0], finger_pos[1]);
								dest_rot = quat_difference(start_2h_rot, dest_rot);
							} break;
							case ui_move_face_user: {
								dest_rot = quat_lookat(finger_pos[0], finger_pos[1]);
								dest_rot = quat_difference(start_2h_rot, dest_rot);
							} break;
							case ui_move_pos_only: {
								dest_rot = quat_identity;
							} break;
							case ui_move_none: {
								dest_rot = quat_identity;
							} break;
							default: dest_rot = quat_identity; log_err("Unimplemented move type!"); break;
							}

							hierarchy_set_enabled(false);
							line_add(matrix_transform_pt(to_world, finger_pos[0]), matrix_transform_pt(to_world, dest_pos), { 255,255,255,0 }, {255,255,255,128}, 0.001f);
							line_add(matrix_transform_pt(to_world, dest_pos), matrix_transform_pt(to_world, finger_pos[1]), { 255,255,255,128 }, {255,255,255,0}, 0.001f);
							hierarchy_set_enabled(true);

							dest_pos = dest_pos + dest_rot * (start_2h_handle_pos - start_2h_pos);
							dest_rot = start_2h_handle_rot * dest_rot;
							if (move_type == ui_move_none) {
								dest_pos = movement.position;
								dest_rot = movement.orientation;
							}

							movement.position    = vec3_lerp (movement.position,    dest_pos, 0.6f);
							movement.orientation = quat_slerp(movement.orientation, dest_rot, 0.4f);
						}

						// If one of the hands just let go, reset their starting
						// locations so the handle doesn't 'pop' when switching
						// back to 1-handed interaction.
						if ((input_hand(handed_left)->pinch_state & button_state_just_inactive) || (input_hand(handed_right)->pinch_state & button_state_just_inactive)) {
							start_handle_pos[i] = movement.position;
							start_handle_rot[i] = movement.orientation;
							start_palm_pos  [i] = from_pt;
							start_palm_rot  [i] = matrix_transform_quat( to_local, hand->palm.orientation);
						}
					} else {
						switch (move_type) {
						case ui_move_exact: {
							dest_rot = matrix_transform_quat(to_local, hand->palm.orientation);
							dest_rot = quat_difference(start_palm_rot[i], dest_rot);
						} break;
						case ui_move_face_user: {
							vec3 look_from = vec3{ movement.position.x, finger_pos[i].y, movement.position.z };
							dest_rot = quat_lookat_up(look_from, matrix_transform_pt(to_local, input_head()->position), matrix_transform_dir(to_local, vec3_up));
							dest_rot = quat_difference(start_handle_rot[i], dest_rot);
						} break;
						case ui_move_pos_only: {
							dest_rot = quat_identity;
						} break;
						case ui_move_none: {
							dest_rot = quat_identity;
						} break;
						default: dest_rot = quat_identity; log_err("Unimplemented move type!"); break;
						}

						vec3 curr_pos = finger_pos[i];
						dest_pos = curr_pos + dest_rot * (start_handle_pos[i] - start_palm_pos[i]);
						if (move_type == ui_move_none) dest_pos = movement.position;

						movement.position    = vec3_lerp (movement.position,    dest_pos, 0.6f);
						movement.orientation = quat_slerp(movement.orientation, start_handle_rot[i] * dest_rot, 0.4f); 
					}

					if (hand->pinch_state & button_state_just_inactive) {
						skui_hand[i].active = 0;
						sound_play(skui_snd_ungrab, skui_hand[i].finger_world, 1);
					}
					ui_pop_surface();
					ui_push_surface(movement);
				}
			}
		}
	}

	if (draw) {
		ui_draw_el(ui_vis_handle,
			handle.center+handle.dimensions/2, 
			handle.dimensions,
			ui_color_primary,
			color);
		ui_nextline();
	}
	return result;
}

///////////////////////////////////////////

bool32_t ui_handle_begin(const char *text, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type) {
	return _ui_handle_begin(ui_stack_hash(text), movement, handle, draw, move_type);
}
bool32_t ui_handle_begin_16(const char16_t *text, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type) {
	return _ui_handle_begin(ui_stack_hash(text), movement, handle, draw, move_type);
}

///////////////////////////////////////////

void ui_handle_end() {
	ui_pop_surface();
}

///////////////////////////////////////////

template<typename C, vec2 (*text_size_t)(const C *text, text_style_t style)>
void ui_window_begin_g(const C *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	uint64_t id = ui_push_id(text);
	int32_t index = skui_sl_windows.binary_search(&ui_window_t::hash, id);
	if (index < 0) {
		index = ~index;
		ui_window_t new_window = {};
		new_window.hash = id;
		new_window.size = window_size;
		skui_sl_windows.insert(index, new_window);
	}
	ui_window_t &window = skui_sl_windows[index];
	window.type = window_type;
	
	// figure out the size of it, based on its window type
	vec3 box_start = {}, box_size = {};
	if (window.type & ui_win_head) {
		float line = ui_line_height();
		box_start = vec3{ 0, line/2, skui_settings.depth/2 };
		box_size  = vec3{ window.size.x, line, skui_settings.depth*2 };
	} 
	if (window.type & ui_win_body) {
		box_start.z = skui_settings.depth/2;
		box_start.y -= window.size.y / 2;
		box_size.x  = window.size.x;
		box_size.y += window.size.y;
		box_size.z  = skui_settings.depth * 2;
	}

	// Set up window handle and layout area
	_ui_handle_begin(id, pose, { box_start, box_size }, false, move_type);
	ui_layout_area(window, { window.size.x / 2,0,0 }, window_size);

	// draw label
	if (window.type & ui_win_head) {
		vec2 size = text_size_t(text, skui_font_stack.last());
		vec3 at   = skui_layers.last().offset - vec3{ skui_settings.padding, -ui_line_height(), 2*mm2m };
		ui_text_in(at, size, text, text_align_top_left, text_align_center_left);

		if (skui_layers.last().max_x > at.x - size.x - skui_settings.padding)
			skui_layers.last().max_x = at.x - size.x - skui_settings.padding;
	}
	window.pose = pose;
}
void ui_window_begin(const char *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char, text_size>(text, pose, window_size, window_type, move_type);
}
void ui_window_begin_16(const char16_t *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char16_t, text_size_16>(text, pose, window_size, window_type, move_type);
}

///////////////////////////////////////////

void ui_window_end() {
	layer_t &layer = skui_layers.last();

	// Move to next line if we're still on a previous line
	if (layer.offset.x != layer.offset_initial.x - skui_settings.padding)
		ui_nextline();
	
	vec3 start = layer.offset_initial + vec3{0,0,skui_settings.depth};
	vec3 end   = { layer.max_x, layer.offset.y - (layer.line_height-skui_settings.padding),  layer.offset_initial.z};
	vec3 size  = start - end;
	size = { fmaxf(size.x+skui_settings.padding, layer.size.x), fmaxf(size.y+skui_settings.padding, layer.size.y), size.z };
	layer.window->size.x = size.x;
	layer.window->size.y = size.y;

	float line_height = ui_line_height();
	if (layer.window->type & ui_win_head) {
		float glow = 1;
		if (skui_hand[0].focused_prev == layer.window->hash || skui_hand[1].focused_prev == layer.window->hash)
			glow = 1.5f;
		ui_draw_el(layer.window->type == ui_win_head ? ui_vis_window_head_only : ui_vis_window_head, start + vec3{0,line_height,0}, { size.x, line_height, size.z }, ui_color_primary, glow);
	}
	if (layer.window->type & ui_win_body) {
		ui_draw_el(layer.window->type == ui_win_body ? ui_vis_window_body_only : ui_vis_window_body, start, size, ui_color_background, 1);
	}
	ui_handle_end();
	ui_pop_id();
}

///////////////////////////////////////////

void ui_panel_at(vec3 start, vec2 size, ui_pad_ padding) {
	vec3 start_offset = vec3_zero;
	vec3 size_offset  = vec3_zero;
	if (padding == ui_pad_outside) {
		float gutter  = skui_settings.gutter / 2;
		float gutter2 = skui_settings.gutter;
		start_offset = { gutter,  gutter,  0 };
		size_offset  = { gutter2, gutter2, 0 };
	}
	ui_draw_el(ui_vis_button, start+start_offset, vec3{ size.x, size.y, skui_settings.depth* 0.1f }+size_offset, ui_color_complement, 1);
}

///////////////////////////////////////////

struct panel_stack_data_t {
	vec3    at;
	float   max_x;
	ui_pad_ padding;
};
array_t<panel_stack_data_t> skui_panel_stack = {};
void ui_panel_begin(ui_pad_ padding) {
	layer_t           &layer = skui_layers.last();
	panel_stack_data_t data;
	data.at      = layer.offset;
	data.max_x   = layer.max_x;
	data.padding = padding;

	skui_panel_stack.add( data );
	layer.max_x = layer.offset_initial.x;

	if (padding == ui_pad_inside) {
		float gutter = skui_settings.gutter / 2;
		layer.offset_initial.x -= gutter;
		layer.offset_initial.y -= gutter;
		layer.offset.x -= gutter;
		layer.offset.y -= gutter;
	}
}

///////////////////////////////////////////

void ui_panel_end() {
	ui_sameline();

	float              gutter = skui_settings.gutter / 2;
	layer_t           &layer  = skui_layers.last();
	panel_stack_data_t start  = skui_panel_stack.last();
	if (start.padding == ui_pad_inside) layer.max_x -= gutter;

	vec3 curr = vec3{layer.max_x, layer.offset.y - (layer.line_height + (start.padding == ui_pad_inside?gutter:0)), layer.offset.z};

	ui_panel_at(start.at, {fabsf(curr.x-start.at.x), start.at.y-curr.y}, start.padding);

	layer.max_x = fminf(layer.max_x, start.max_x);
	if (start.padding == ui_pad_inside) {
		layer.offset_initial.x += gutter;
		layer.offset_initial.y += gutter;
		layer.line_height += gutter * 2;
		layer.offset.y += gutter;
		layer.offset.x -= gutter;
	}
	skui_panel_stack.pop();
	ui_nextline();
}

} // namespace sk
