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
	vec2     prev_size;
	vec2     curr_size;
	vec3     layout_start;
	vec2     layout_size;
	ui_win_  type;
	uint64_t hash;
};

struct ui_layout_t {
	ui_window_t* window;
	vec3         offset_initial;
	vec3         offset;
	vec2         size;
	float        line_height;
	float        max_x;
	vec3         prev_offset;
	float        prev_line_height;
};

struct layer_t {
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
array_t<ui_layout_t> skui_layouts    = {};
array_t<text_style_t>skui_font_stack = {};
array_t<color128>    skui_tint_stack = {};
array_t<bool32_t>    skui_enabled_stack = {};
array_t<bool>        skui_preserve_keyboard_stack = {};
array_t<uint64_t>    skui_preserve_keyboard_ids[2] = {};
array_t<uint64_t>   *skui_preserve_keyboard_ids_read;
array_t<uint64_t>   *skui_preserve_keyboard_ids_write;

ui_el_visual_t  skui_visuals[ui_vis_max] = {};
mesh_t          skui_box_top      = nullptr;
mesh_t          skui_box_bot      = nullptr;
mesh_t          skui_box          = nullptr;
vec2            skui_box_min      = {};
mesh_t          skui_small_left   = nullptr;
mesh_t          skui_small_right  = nullptr;
mesh_t          skui_small        = nullptr;
vec2            skui_small_min    = {};
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
ui_move_        skui_system_move_type = ui_move_face_user;
uint64_t        skui_input_target = 0;
color128        skui_tint = {1,1,1,1};
bool32_t        skui_interact_enabled = true;
uint64_t        skui_last_element = 0xFFFFFFFFFFFFFFFF;
sprite_t        skui_toggle_off = nullptr;
sprite_t        skui_toggle_on = nullptr;

sound_t         skui_snd_interact;
sound_t         skui_snd_uninteract;
sound_t         skui_snd_grab;
sound_t         skui_snd_ungrab;
sound_t         skui_snd_tick;

ui_settings_t skui_settings = {};
float skui_fontsize = 10*mm2m;

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

float ui_text_in(const char*     text_utf8,  text_align_ position, text_align_ align, text_fit_ fit, vec3 start, vec2 size);
float ui_text_in(const char16_t* text_utf16, text_align_ position, text_align_ align, text_fit_ fit, vec3 start, vec2 size);
vec2  text_size (const char16_t* text_utf16, text_style_t style) { return text_size_16(text_utf16, style); }

///////////////////////////////////////////

tex_t ui_create_sdf_tex(int32_t width, int32_t height, float (*sdf)(float x, float y), float scale) {
	color32 *data     = sk_malloc_t(color32, width * height);
	float    center_x = width  / 2.0f;
	float    center_y = height / 2.0f;
	for (int32_t y = 0; y < height; y++) {
		int32_t yoff = y * width;
		for (int32_t x = 0; x < width; x++) {
			float dist = sdf(x - center_x, y - center_y);
			float lerp = 1-math_saturate(dist * scale);

			data[x + yoff] = {255,255,255,(uint8_t)(lerp * 255)};
		}
	}
	return tex_create_color32(data, width, height, false);
}

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

inline bounds_t ui_size_box(vec3 top_left, vec3 dimensions) {
	return { top_left - dimensions/2, dimensions };
}

///////////////////////////////////////////

inline void ui_anim_start(uint64_t id) {
	if (skui_anim_id != id) {
		skui_anim_id = id;
		skui_anim_time = time_totalf_unscaled();
	}
}

///////////////////////////////////////////

inline bool ui_anim_has(uint64_t id, float duration) {
	if (id == skui_anim_id) {
		if ((time_totalf_unscaled() - skui_anim_time) < duration)
			return true;
		skui_anim_id = 0;
	}
	return false;
}

///////////////////////////////////////////

inline float ui_anim_elapsed(uint64_t id, float duration = 1, float max = 1) {
	return skui_anim_id == id ? fminf(max, (time_totalf_unscaled() - skui_anim_time) / duration) : 0;
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

ui_move_ ui_system_get_move_type() {
	return skui_system_move_type;
}

///////////////////////////////////////////

void ui_system_set_move_type(ui_move_ move_type) {
	skui_system_move_type = move_type;
}

///////////////////////////////////////////

void ui_settings(ui_settings_t settings) {
	if (settings.backplate_border == 0) settings.backplate_border = 0.5f * mm2m;
	if (settings.backplate_depth  == 0) settings.backplate_depth  = 0.4f;
	if (settings.depth            == 0) settings.depth   = 10 * mm2m;
	if (settings.gutter           == 0) settings.gutter  = 10 * mm2m;
	if (settings.padding          == 0) settings.padding = 10 * mm2m;
	if (settings.rounding         == 0) settings.rounding= 7.5f * mm2m;

	bool rebuild_meshes = skui_settings.rounding != settings.rounding;
	skui_settings = settings;

	if (rebuild_meshes) {
		int32_t slices  = 3;//  settings.rounding > 20 * mm2m ? 4 : 3;
		bool    set_ids = skui_box == nullptr;
		ui_default_mesh     (&skui_box,       true, settings.rounding*2, 1.25f*mm2m, slices);
		ui_default_mesh_half(&skui_box_top,   true, settings.rounding*2,       1.25f*mm2m, slices, 0);
		ui_default_mesh_half(&skui_box_bot,   true, settings.rounding*2,       1.25f*mm2m, slices, 180 * deg2rad);

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

ui_settings_t ui_get_settings() {
	return skui_settings;
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
	// TODO: v0.4, this sets up default values when zeroed out, with a
	// ui_get_settings, this isn't really necessary anymore!
	ui_settings_t settings = {};
	ui_settings(settings);

	ui_default_mesh(&skui_cylinder, false, 1, 4 * cm2m, 5);
	mesh_set_id(skui_box_bot, "sk/ui/cylinder_mesh");

	// Create default sprites for the toggles
	tex_t toggle_tex_on = ui_create_sdf_tex(64, 64, [](float x, float y) {
		float dist = vec2_magnitude({ x, y });
		return fminf(dist - 20, fmaxf(-(dist - 27), dist - 31)) / 32.0f;
	}, 40);
	tex_set_address(toggle_tex_on, tex_address_clamp);
	tex_set_id     (toggle_tex_on, "sk/ui/toggle_on_tex");
	tex_t toggle_tex_off = ui_create_sdf_tex(64, 64, [](float x, float y) {
		float dist = vec2_magnitude({ x, y });
		return fmaxf(-(dist - 27), dist - 31) / 32.0f;
	}, 40);
	tex_set_address(toggle_tex_off, tex_address_clamp);
	tex_set_id     (toggle_tex_off, "sk/ui/toggle_off_tex");
	skui_toggle_on  = sprite_create(toggle_tex_on,  sprite_type_single);
	skui_toggle_off = sprite_create(toggle_tex_off, sprite_type_single);
	sprite_set_id(skui_toggle_on,  "sk/ui/toggle_on_spr");
	sprite_set_id(skui_toggle_off, "sk/ui/toggle_off_spr");
	tex_release(toggle_tex_on);
	tex_release(toggle_tex_off);

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

	skui_mat = material_copy_id(default_id_material_ui);
	material_set_bool(skui_mat, "ui_tint", true);
	skui_mat_quad = material_find(default_id_material_ui_quadrant);
	//material_set_wireframe(skui_mat_quad, true);
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
			20.0f * time_stepf_unscaled());
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
	skui_finger_radius /= (float)handed_max;

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
	sound_release(skui_snd_tick);       skui_snd_tick       = nullptr;
	mesh_release(skui_box);         skui_box         = nullptr;
	mesh_release(skui_box_top);     skui_box_top     = nullptr;
	mesh_release(skui_box_bot);     skui_box_bot     = nullptr;
	mesh_release(skui_small);       skui_small       = nullptr;
	mesh_release(skui_small_left);  skui_small_left  = nullptr;
	mesh_release(skui_small_right); skui_small_right = nullptr;
	mesh_release(skui_cylinder);    skui_cylinder    = nullptr;
	mesh_release(skui_box_dbg);     skui_box_dbg     = nullptr;
	material_release(skui_mat);      skui_mat      = nullptr;
	material_release(skui_mat_quad); skui_mat_quad = nullptr;
	material_release(skui_mat_dbg);  skui_mat_dbg  = nullptr;
	material_release(skui_font_mat); skui_font_mat = nullptr;
	sprite_release(skui_toggle_off); skui_toggle_off = nullptr;
	sprite_release(skui_toggle_on);  skui_toggle_on  = nullptr;
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

	skui_layers.add(layer_t{});
	ui_layout_push(layout_start, layout_dimensions);

	layer_t*      layer    = &skui_layers.last();
	const matrix *to_local = hierarchy_to_local();
	for (int32_t i = 0; i < handed_max; i++) {
		layer->finger_pos   [i] = skui_hand[i].finger        = matrix_transform_pt(*to_local, skui_hand[i].finger_world);
		layer->finger_prev  [i] = skui_hand[i].finger_prev   = matrix_transform_pt(*to_local, skui_hand[i].finger_world_prev);
		layer->pinch_pt_pos [i] = skui_hand[i].pinch_pt      = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_world);
		layer->pinch_pt_prev[i] = skui_hand[i].pinch_pt_prev = matrix_transform_pt(*to_local, skui_hand[i].pinch_pt_world_prev);
	}
}

///////////////////////////////////////////

void ui_pop_surface() {
	hierarchy_pop();
	skui_layers.pop();
	ui_layout_pop();

	if (skui_layers.count <= 0) {
		for (int32_t i = 0; i < handed_max; i++) {
			skui_hand[i].finger        = skui_hand[i].finger_world;
			skui_hand[i].finger_prev   = skui_hand[i].finger_world_prev;
			skui_hand[i].pinch_pt      = skui_hand[i].pinch_pt_world;
			skui_hand[i].pinch_pt_prev = skui_hand[i].pinch_pt_world_prev;
		}
	} else {
		layer_t *layer = &skui_layers.last();
		for (int32_t i = 0; i < handed_max; i++) {
			skui_hand[i].finger        = layer->finger_pos[i];
			skui_hand[i].finger_prev   = layer->finger_prev[i];
			skui_hand[i].pinch_pt      = layer->pinch_pt_pos[i];
			skui_hand[i].pinch_pt_prev = layer->pinch_pt_prev[i];
		}
	}
}

///////////////////////////////////////////

void ui_layout_area(vec3 start, vec2 dimensions) {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset_initial   = start;
	layout->offset           = start - vec3{ skui_settings.padding, skui_settings.padding };
	layout->size             = dimensions;
	layout->max_x            = 0;
	layout->line_height      = 0;
	layout->prev_offset      = layout->offset;
	layout->prev_line_height = layout->line_height;
}

///////////////////////////////////////////

void ui_layout_area(ui_window_t &window, vec3 start, vec2 dimensions) {
	ui_layout_area(start, dimensions);
	skui_layouts.last().window = &window;
	window.layout_start = start;
	window.layout_size  = dimensions;
	
	window.curr_size = {};
	if (window.layout_size.x != 0) window.curr_size.x = dimensions.x;
	if (window.layout_size.y != 0) window.curr_size.y = dimensions.y;
}

///////////////////////////////////////////

vec2 ui_layout_remaining() {
	ui_layout_t *layout = &skui_layouts.last();
	float size_x = layout->size.x != 0
		? layout->size.x
		: (layout->window ? layout->window->prev_size.x : 0);
	float size_y = layout->size.y != 0
		? layout->size.y
		: (layout->window ? layout->window->prev_size.y : 0);
	float max_x = size_x == 0 ? -layout->max_x : layout->offset_initial.x - (size_x - skui_settings.padding);
	return vec2{
		fmaxf(max_x, size_x - (layout->offset_initial.x - layout->offset.x) - skui_settings.padding),
		fmaxf(0,     size_y + (layout->offset.y + layout->offset_initial.y) - skui_settings.padding)
	};
}

///////////////////////////////////////////

vec2 ui_area_remaining() {
	return ui_layout_remaining();
}

///////////////////////////////////////////

vec3 ui_layout_at() {
	return skui_layouts.last().offset;
}

///////////////////////////////////////////

bounds_t ui_layout_last() {
	return skui_recent_layout;
}

///////////////////////////////////////////

void ui_layout_reserve_sz(vec2 size, bool32_t add_padding, vec3 *out_position, vec2 *out_size) {
	if (size.x == 0) size.x = ui_layout_remaining().x - (add_padding ? skui_settings.padding*2 : 0);
	if (size.y == 0) size.y = ui_line_height();

	ui_layout_t *layout = &skui_layouts.last();

	vec3 final_pos  = layout->offset;
	vec2 final_size = add_padding
		? size + vec2{ skui_settings.padding, skui_settings.padding }*2
		: size;

	// If this is not the first element, and it goes outside the active window,
	// then we'll want to start this element on the next line down
	if (final_pos.x  != layout->offset_initial.x-skui_settings.padding &&
		layout->size.x != 0                                            &&
		final_pos.x - final_size.x < (layout->offset_initial.x - (layout->size.x + skui_settings.padding)))
	{
		ui_nextline();
		final_pos = layout->offset;
	}

	// Track the sizes for this line, for ui_layout_remaining, as well as
	// window auto-sizing.
	if (layout->max_x > layout->offset.x - final_size.x)
		layout->max_x = layout->offset.x - final_size.x;
	if (layout->line_height < final_size.y)
		layout->line_height = final_size.y;

	// Advance the UI layout position
	layout->offset -= vec3{ final_size.x + skui_settings.gutter, 0, 0 };

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

void ui_layout_push(vec3 start, vec2 dimensions) {
	ui_layout_t layout = {};
	layout.offset_initial   = start;
	layout.prev_offset      = start;
	layout.offset           = start - vec3{ skui_settings.padding, skui_settings.padding, 0 };
	layout.size             = dimensions;
	layout.line_height      = 0;
	layout.max_x            = 0;
	layout.prev_line_height = 0;
	skui_layouts.add(layout);
}

///////////////////////////////////////////

void ui_layout_push_cut(ui_cut_ cut_to, float size) {
	ui_layout_t* curr = &skui_layouts.last();
	if      (cut_to == ui_cut_bottom && curr->size.y == 0) log_warn("Can't cut bottom for layout with a height of 0!");
	else if (cut_to == ui_cut_right  && curr->size.x == 0) log_warn("Can't cut right for layout with an width of 0!");

	vec3 cut_start   = {};
	vec2 cut_size    = {};
	vec3 curr_offset = {};
	switch (cut_to) {
	case ui_cut_left:
		curr_offset   = {-size, 0, 0 };
		cut_start     = curr->offset_initial;
		cut_size      = { size, curr->size.y };
		curr->size.x -= size;
		break;
	case ui_cut_right:
		curr_offset   = {0, 0, 0 };
		cut_start     = curr->offset_initial - vec3{curr->size.x-size, 0, 0};
		cut_size      = { size, curr->size.y };
		curr->size.x -= size;break;
	case ui_cut_top:
		curr_offset   = {0, -size, 0 };
		cut_start     = curr->offset_initial;
		cut_size      = { curr->size.x, size };
		curr->size.y -= size;
		break;
	case ui_cut_bottom:
		curr_offset   = {0, 0, 0 };
		cut_start     = curr->offset_initial - vec3{0,curr->size.y-size,0};
		cut_size      = { curr->size.x, size };
		curr->size.y -= size;
		break;
	}
	curr->offset         += curr_offset;
	curr->offset_initial += curr_offset;
	curr->prev_offset    += curr_offset;
	ui_layout_push(cut_start, cut_size);
	skui_layouts.last().window = curr->window;
}

///////////////////////////////////////////

void ui_layout_pop() {
	ui_layout_t* layout = &skui_layouts.last();

	// Move to next line if we're still on a previous line
	if (layout->offset.x != layout->offset_initial.x - skui_settings.padding)
		ui_nextline();

	if (layout->window) {
		vec3 start = layout->window->layout_start;// layout->offset_initial + vec3{0,0,skui_settings.depth};
		vec3 end   = { layout->max_x, layout->offset.y - (layout->line_height-skui_settings.gutter),  layout->offset_initial.z};
		vec3 size  = start - end;
		size = { fmaxf(size.x+skui_settings.padding, layout->size.x), fmaxf(size.y+skui_settings.padding, layout->size.y), size.z };
		if (layout->window->layout_size.x == 0)
			layout->window->curr_size.x = fmaxf(size.x, layout->window->curr_size.x);
		if (layout->window->layout_size.y == 0)
			layout->window->curr_size.y = fmaxf(size.y, layout->window->curr_size.y);
	}

	skui_layouts.pop();
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
	ui_layout_t *layout = &skui_layouts.last();
	layout->prev_offset      = layout->offset;
	layout->prev_line_height = layout->line_height;

	layout->offset.x    = layout->offset_initial.x - skui_settings.padding;
	layout->offset.y   -= layout->line_height      + skui_settings.gutter;
	layout->line_height = 0;
}

///////////////////////////////////////////

void ui_sameline() {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset      = layout->prev_offset;
	layout->line_height = layout->prev_line_height;
}

///////////////////////////////////////////

float ui_line_height() {
	return skui_settings.padding * 2 + skui_fontsize;
}

///////////////////////////////////////////

void ui_space(float space) {
	ui_layout_t *layout = &skui_layouts.last();
	if (layout->offset.x == layout->offset_initial.x - skui_settings.padding)
		layout->offset.y -= space;
	else
		layout->offset.x -= space;
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

	ui_text_at(text, text_align_center_left, text_fit_squeeze, final_pos - vec3{0,0,skui_settings.depth/2}, final_size);
}
void ui_label_sz   (const char     *text, vec2 size) { ui_label_sz_g<char    >(text, size); }
void ui_label_sz_16(const char16_t *text, vec2 size) { ui_label_sz_g<char16_t>(text, size); }

///////////////////////////////////////////

template<typename C>
void ui_label_g(const C *text, bool32_t use_padding) {
	vec3 final_pos;
	vec2 final_size;
	vec2 txt_size = text_size(text, skui_font_stack.last());
	txt_size += use_padding
		? vec2{skui_settings.padding, skui_settings.padding}*2
		: vec2{0, skui_settings.padding}*2;
	ui_layout_reserve_sz(txt_size, false, &final_pos, &final_size);

	ui_text_at(text, text_align_center, text_fit_squeeze, final_pos - vec3{0,0,skui_settings.depth/2}, final_size);
}
void ui_label   (const char     *text, bool32_t use_padding) { ui_label_g<char    >(text, use_padding); }
void ui_label_16(const char16_t *text, bool32_t use_padding) { ui_label_g<char16_t>(text, use_padding); }

///////////////////////////////////////////

float ui_text_in  (const char*     text, text_align_ position, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { return text_add_in   (text, matrix_identity, size, fit, skui_font_stack.last(), position, align, start.x, start.y, start.z, skui_enabled_stack.last() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
float ui_text_in  (const char16_t* text, text_align_ position, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { return text_add_in_16(text, matrix_identity, size, fit, skui_font_stack.last(), position, align, start.x, start.y, start.z, skui_enabled_stack.last() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }

///////////////////////////////////////////

void ui_text_at   (const char*     text, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { text_add_in   (text, matrix_identity, size, fit, skui_font_stack.last(), text_align_top_left, align, start.x, start.y, start.z, skui_enabled_stack.last() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
void ui_text_at   (const char16_t* text, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { text_add_in_16(text, matrix_identity, size, fit, skui_font_stack.last(), text_align_top_left, align, start.x, start.y, start.z, skui_enabled_stack.last() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }
void ui_text_at_16(const char16_t* text, text_align_ align, text_fit_ fit, vec3 start, vec2 size) { text_add_in_16(text, matrix_identity, size, fit, skui_font_stack.last(), text_align_top_left, align, start.x, start.y, start.z, skui_enabled_stack.last() ? color128{ 1, 1, 1, 1 } : color128{ .5f, .5f, .5f, 1 }); }

///////////////////////////////////////////

template<typename C>
void ui_text_sz_g(const C *text, text_align_ text_align, text_fit_ fit, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	ui_text_at(text, text_align, fit, final_pos - vec3{ 0, 0, skui_settings.depth/2 }, final_size);
}
void ui_text_sz   (const char     *text, text_align_ text_align, text_fit_ fit, vec2 size) { ui_text_sz_g<char    >(text, text_align, fit, size); }
void ui_text_sz_16(const char16_t *text, text_align_ text_align, text_fit_ fit, vec2 size) { ui_text_sz_g<char16_t>(text, text_align, fit, size); }

///////////////////////////////////////////

template<typename C>
void ui_text_g(const C *text, text_align_ text_align) {
	vec3 offset = ui_layout_at();
	vec2 size   = { ui_layout_remaining().x, 0 };
	vec3 at     = offset - vec3{ 0, 0, skui_settings.depth/2 };
	size.y = ui_text_in(text, text_align_top_left, text_align, text_fit_wrap, at, size);

	ui_layout_reserve(size);
}
void ui_text   (const char     *text, text_align_ text_align) { ui_text_g<char    >(text, text_align); }
void ui_text_16(const char16_t *text, text_align_ text_align) { ui_text_g<char16_t>(text, text_align); }

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
void _ui_button_img_surface(const C* text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size, float finger_offset) {
	float pad2       = skui_settings.padding * 2;
	float pad2gutter = pad2 + skui_settings.gutter;
	float depth      = finger_offset + 2 * mm2m;
	vec3  image_at;
	float image_size;
	text_align_ image_align;
	vec3  text_at;
	vec2  text_size;
	text_align_ text_align;
	float aspect = image != nullptr ? sprite_get_aspect(image) : 1.0f;
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
	case ui_btn_layout_none:
		image_size = 0;

		text_align = text_align_top_left;
		text_at    = window_relative_pos - vec3{ skui_settings.padding, skui_settings.padding, depth };
		text_size  = vec2{ size.x - pad2, size.y - pad2 };
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

	if (image_size>0 && image) {
		color128 final_color = skui_tint;
		if (!skui_enabled_stack.last()) final_color = final_color * color128{ .5f, .5f, .5f, 1 };
	
		sprite_draw_at(image, matrix_ts(image_at, { image_size, image_size, image_size }), image_align, color_to_32( final_color ));
	}
	if (image_layout != ui_btn_layout_center_no_text)
		ui_text_in(text, text_align, text_align_center, text_fit_squeeze, text_at, text_size);
}

///////////////////////////////////////////

template<typename C>
vec2 _ui_button_img_size(const C* text, sprite_t image, ui_btn_layout_ image_layout) {
	vec2 size = {};
	if (image_layout == ui_btn_layout_center || image_layout == ui_btn_layout_center_no_text) {
		size = { skui_fontsize, skui_fontsize };
	} else if (image_layout == ui_btn_layout_none) {
		size = text_size(text, skui_font_stack.last());
	} else {
		vec2  txt_size   = text_size(text, skui_font_stack.last());
		float aspect     = sprite_get_aspect(image);
		float image_size = skui_fontsize * aspect;
		size = vec2{ txt_size.x + image_size + skui_settings.gutter, skui_fontsize };
	}
	return size;
}

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
	_ui_button_img_surface(text, image, image_layout, window_relative_pos, size, finger_offset);

	return state & button_state_just_active;
}
bool32_t ui_button_img_at   (const char     *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char    >(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at   (const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size); }
bool32_t ui_button_img_at_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_g<char16_t>(text, image, image_layout, window_relative_pos, size); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_button_img_g(const C *text, sprite_t image, ui_btn_layout_ image_layout) {
	vec3 final_pos;
	vec2 final_size;
	vec2 size = _ui_button_img_size(text, image, image_layout);

	ui_layout_reserve_sz(size, true, &final_pos, &final_size);
	return ui_button_img_at(text, image, image_layout, final_pos, final_size);
}
bool32_t ui_button_img   (const char     *text, sprite_t image, ui_btn_layout_ image_layout) { return ui_button_img_g<char    >(text, image, image_layout); }
bool32_t ui_button_img_16(const char16_t *text, sprite_t image, ui_btn_layout_ image_layout) { return ui_button_img_g<char16_t>(text, image, image_layout); }

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

bool32_t ui_button_at   (const char     *text, vec3 window_relative_pos, vec2 size) { return ui_button_img_at   (text, nullptr, ui_btn_layout_none, window_relative_pos, size); }
bool32_t ui_button_at   (const char16_t *text, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_16(text, nullptr, ui_btn_layout_none, window_relative_pos, size); }
bool32_t ui_button_at_16(const char16_t *text, vec3 window_relative_pos, vec2 size) { return ui_button_img_at_16(text, nullptr, ui_btn_layout_none, window_relative_pos, size); }

///////////////////////////////////////////

bool32_t ui_button_sz   (const char     *text, vec2 size) { return ui_button_img_sz   (text, nullptr, ui_btn_layout_none, size); }
bool32_t ui_button_sz_16(const char16_t *text, vec2 size) { return ui_button_img_sz_16(text, nullptr, ui_btn_layout_none, size); }

///////////////////////////////////////////

bool32_t ui_button   (const char     *text) { return ui_button_img   (text, nullptr, ui_btn_layout_none); }
bool32_t ui_button_16(const char16_t *text) { return ui_button_img_16(text, nullptr, ui_btn_layout_none); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_at_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) {
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

	if (state & button_state_just_active) {
		pressed = pressed ? false : true;
	}
	finger_offset = pressed ? fminf(skui_settings.backplate_depth * skui_settings.depth + mm2m, finger_offset) : finger_offset;

	float activation = 1 + 1 - (finger_offset / skui_settings.depth);
	ui_draw_el(ui_vis_button, window_relative_pos, vec3{ size.x,size.y,finger_offset }, ui_color_common, fmaxf(activation, color_blend));
	_ui_button_img_surface(text, pressed?toggle_on:toggle_off, image_layout, window_relative_pos, size, finger_offset);

	return state & button_state_just_active;
}
bool32_t ui_toggle_img_at   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_g<char    >(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size); }
bool32_t ui_toggle_img_at   (const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size); }
bool32_t ui_toggle_img_at_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, window_relative_pos, size); };

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout) {
	vec3 final_pos;
	vec2 final_size;
	vec2 size = _ui_button_img_size(text, toggle_off, image_layout);

	ui_layout_reserve_sz(size, true, &final_pos, &final_size);
	return ui_toggle_img_at(text, pressed, toggle_off, toggle_on, image_layout, final_pos, final_size);
}
bool32_t ui_toggle_img   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout) { return ui_toggle_img_g<char    >(text, pressed, toggle_off, toggle_on, image_layout); }
bool32_t ui_toggle_img_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout) { return ui_toggle_img_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout); }

///////////////////////////////////////////

template<typename C>
bool32_t ui_toggle_img_sz_g(const C* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, false, &final_pos, &final_size);

	return ui_toggle_img_at(text, pressed, toggle_off, toggle_on, image_layout, final_pos, final_size);
}
bool32_t ui_toggle_img_sz   (const char*     text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size) { return ui_toggle_img_sz_g<char    >(text, pressed, toggle_off, toggle_on, image_layout, size); }
bool32_t ui_toggle_img_sz_16(const char16_t* text, bool32_t& pressed, sprite_t toggle_off, sprite_t toggle_on, ui_btn_layout_ image_layout, vec2 size) { return ui_toggle_img_sz_g<char16_t>(text, pressed, toggle_off, toggle_on, image_layout, size); }

///////////////////////////////////////////

bool32_t ui_toggle_at   (const char     *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size); }
bool32_t ui_toggle_at   (const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size); }
bool32_t ui_toggle_at_16(const char16_t *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) { return ui_toggle_img_at_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, window_relative_pos, size); }

///////////////////////////////////////////

bool32_t ui_toggle   (const char     *text, bool32_t &pressed) { return ui_toggle_img   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left); }
bool32_t ui_toggle_16(const char16_t *text, bool32_t &pressed) { return ui_toggle_img_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left); }

///////////////////////////////////////////

bool32_t ui_toggle_sz   (const char     *text, bool32_t &pressed, vec2 size) { return ui_toggle_img_sz   (text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, size); }
bool32_t ui_toggle_sz_16(const char16_t* text, bool32_t& pressed, vec2 size) { return ui_toggle_img_sz_16(text, pressed, skui_toggle_off, skui_toggle_on, ui_btn_layout_left, size); }

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
	if (ui_anim_has(id, .2f)) {
		float t     = ui_anim_elapsed    (id, .2f);
		color_blend = math_ease_overshoot(1, 2.f, 40, t);
	}

	float activation = 1 + 1-(finger_offset / skui_settings.depth);
	ui_draw_el(ui_vis_button_round, window_relative_pos, { diameter, diameter, finger_offset }, ui_color_common, fmaxf(activation, color_blend));

	float sprite_scale = fmaxf(1, sprite_get_aspect(image));
	float sprite_size  = (diameter * 0.7f) / sprite_scale;
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

template<typename C>
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
		skui_input_blink  = time_totalf_unscaled();
		skui_input_target = id_hash;
		skui_input_carat  = skui_input_carat_end = (int32_t)utf_charlen(buffer);
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

			if (curr == key_backspace) {
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(utf_advance_chars(buffer, start), count);
					skui_input_carat_end = skui_input_carat = start;
					result = true;
				} else if (skui_input_carat > 0) {
					skui_input_carat_end = skui_input_carat = skui_input_carat - 1;
					utf_remove_chars(utf_advance_chars(buffer, skui_input_carat), 1);
					result = true;
				}
			} else if (curr == 0x7f) {
				if (skui_input_carat != skui_input_carat_end) {
					int32_t start = mini(skui_input_carat, skui_input_carat_end);
					int32_t count = maxi(skui_input_carat, skui_input_carat_end) - start;
					utf_remove_chars(utf_advance_chars(buffer, start), count);
					skui_input_carat_end = skui_input_carat = start;
					result = true;
				} else if (skui_input_carat >= 0) {
					utf_remove_chars(utf_advance_chars(buffer, skui_input_carat), 1);
					result = true;
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
					utf_remove_chars(utf_advance_chars(buffer, start), count);
					skui_input_carat_end = skui_input_carat = start;
				}
				utf_insert_char(buffer, buffer_size, utf_advance_chars(buffer, skui_input_carat), add);
				skui_input_carat += 1;
				skui_input_carat_end = skui_input_carat;
				result = true;
			}

			curr = input_text_consume();
		}
		if      (input_key(key_shift) & button_state_active && input_key(key_left ) & button_state_just_active) { skui_input_blink = time_totalf_unscaled(); skui_input_carat = maxi(0, skui_input_carat - 1); }
		else if (input_key(key_left ) & button_state_just_active)                                               { skui_input_blink = time_totalf_unscaled(); if (skui_input_carat_end == skui_input_carat) skui_input_carat = maxi(0, skui_input_carat - 1); skui_input_carat_end = skui_input_carat; }
		if      (input_key(key_shift) & button_state_active && input_key(key_right) & button_state_just_active) { skui_input_blink = time_totalf_unscaled(); skui_input_carat = mini((int32_t)utf_charlen(buffer), skui_input_carat + 1); }
		else if (input_key(key_right) & button_state_just_active)                                               { skui_input_blink = time_totalf_unscaled(); if (skui_input_carat_end == skui_input_carat) skui_input_carat = mini((int32_t)utf_charlen(buffer), skui_input_carat + 1); skui_input_carat_end = skui_input_carat; }
	}

	// Render the input UI
	vec2 text_bounds = { final_size.x - skui_settings.padding * 2,final_size.y };
	ui_draw_el(ui_vis_input, final_pos, vec3{ final_size.x, final_size.y, skui_settings.depth/2 }, ui_color_common, color_blend);

	// Swap out for a string of asterisks to hide any password
	const C* draw_text = buffer;
	if (type == text_context_password) {
		int32_t len          = utf_charlen(buffer);
		C*      password_txt = (C*)alloca(sizeof(C) * (len + 1));
		for (size_t i = 0; i < len; i++)
			password_txt[i] = '*';
		password_txt[len] = '\0';
		draw_text = password_txt;
	}

	// If the input is focused, display text selection information
	if (skui_input_target == id_hash) {
		// Advance the displayed text if it's off the right side of the input
		int32_t carat_at      = skui_input_carat;
		vec2    carat_pos     = text_char_at_o(draw_text, skui_font_stack.last(), carat_at, &text_bounds, text_fit_clip, text_align_top_left, text_align_center_left);
		float   scroll_margin = text_bounds.x - skui_fontsize;
		while (carat_pos.x < -scroll_margin && *draw_text != '\0' && carat_at >= 0) {
			draw_text += 1;
			carat_at  -= 1;
			carat_pos = text_char_at_o(draw_text, skui_font_stack.last(), carat_at, &text_bounds, text_fit_clip, text_align_top_left, text_align_center_left);
		}

		// Display a selection box for highlighted text
		float line = ui_line_height() * 0.5f;
		if (skui_input_carat != skui_input_carat_end) {
			int32_t end       = maxi(0, carat_at + (skui_input_carat_end - skui_input_carat));
			vec2    carat_end = text_char_at_o(draw_text, skui_font_stack.last(), end, &text_bounds, text_fit_clip, text_align_top_left, text_align_center_left);
			float   left      = fmaxf(carat_pos.x, carat_end.x);
			float   right     = fmax(fminf(carat_pos.x, carat_end.x), -text_bounds.x);

			vec3   sz  = vec3{ -(right - left), line, line * 0.01f };
			vec3   pos = (final_pos - vec3{ skui_settings.padding - left, -carat_pos.y, skui_settings.depth / 2 + 1 * mm2m }) - sz / 2;
			matrix mx  = matrix_trs(pos, quat_identity, sz);
			mesh_draw(skui_box_dbg, skui_mat, mx, skui_palette[3]*skui_tint);
		}

		// Show a blinking text carat
		if ((int)((time_totalf_unscaled()-skui_input_blink)*2)%2==0) {
			ui_draw_el(ui_vis_carat, final_pos - vec3{ skui_settings.padding - carat_pos.x, -carat_pos.y, skui_settings.depth/2 }, vec3{ line * 0.1f, line, line * 0.1f }, ui_color_text, 1);
		}
	}

	ui_text_at(draw_text, text_align_center_left, text_fit_clip, final_pos - vec3{ skui_settings.padding, 0, skui_settings.depth / 2 + 2 * mm2m }, text_bounds);

	return result;
}

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	return ui_input_g<char>(id, buffer, buffer_size, size, type);
}
bool32_t ui_input_16(const char16_t *id, char16_t *buffer, int32_t buffer_size, vec2 size, text_context_ type) {
	return ui_input_g<char16_t>(id, buffer, buffer_size, size, type);
}

///////////////////////////////////////////

bool32_t ui_has_keyboard_focus() {
	return skui_input_target != 0;
}

///////////////////////////////////////////

pose_t ui_popup_pose(vec3 shift) {
	vec3 at;
	if (ui_last_element_active() & button_state_active) {
		// If there was a UI element focused, we'll use that
		at = hierarchy_to_world_point( ui_layout_last().center );
	} else {
		bool active_left  = input_hand(handed_left )->tracked_state & button_state_active;
		bool active_right = input_hand(handed_right)->tracked_state & button_state_active;

		if (active_left && active_right) {
			// Both hands are active, pick the hand that's the most high
			// and outstretched.
			vec3  pl       = input_hand(handed_left )->fingers[1][4].position;
			vec3  pr       = input_hand(handed_right)->fingers[1][4].position;
			vec3  head     = input_head()->position;
			float dist_l   = vec3_distance(pl, head);
			float dist_r   = vec3_distance(pr, head);
			float height_l = pl.y - pr.y;
			float height_r = pr.y - pl.y;
			at = dist_l + height_l / 2.0f > dist_r + height_r / 2.0f ? pl : pr;
		} else if (active_left) {
			at = input_hand(handed_left)->fingers[1][4].position;
		} else if (active_right) {
			at = input_hand(handed_right)->fingers[1][4].position;
		} else {
			// Head based fallback!
			at = input_head()->position + input_head()->orientation * vec3_forward * 0.35f;
		}
	}

	vec3 dir = at - input_head()->position;
	at = input_head()->position + dir * 0.7f;

	matrix to_local = matrix_invert(render_get_cam_root());
	vec3   local_at = matrix_transform_pt(to_local, at);

	return pose_t {
		local_at + shift,
		quat_lookat(vec3_zero, matrix_transform_dir(to_local, -dir)) };
}

///////////////////////////////////////////

void ui_progress_bar_at_ex(float percent, vec3 start_pos, vec2 size, float focus, bool vertical) {
	// For a vertical progress bar, the easiest thing is to just rotate the
	// hierarchy 90, as this simplifies any issues with trying to rotate the
	// calls to draw the left and right line segments.
	hierarchy_push(vertical
		? matrix_trs(start_pos - vec3{size.x,0,0}, quat_from_angles(0, 0, 90))
		: matrix_t  (start_pos));
	if (vertical) size = { size.y, size.x };

	// Find sizes of bar elements
	float bar_height = fmaxf(skui_settings.padding, size.y / 6.f);
	float bar_depth  = bar_height * skui_settings.backplate_depth - mm2m;
	float bar_y      = -size.y / 2.f + bar_height / 2.f;

	// If the left or right side of the bar is too small, then we'll just draw
	// a single solid bar.
	float bar_length = math_saturate(percent) * size.x;
	vec2  min_size   = ui_get_mesh_minsize(ui_vis_slider_line_active);
	if (bar_length <= min_size.x) {
		ui_draw_el(ui_vis_slider_line,
			vec3{ 0,      bar_y,      0 },
			vec3{ size.x, bar_height, bar_depth },
			ui_color_common, focus);
		hierarchy_pop();
		return;
	} else if (bar_length >= size.x-min_size.x) {
		ui_draw_el(ui_vis_slider_line,
			vec3{ 0,      bar_y,      0 },
			vec3{ size.x, bar_height, bar_depth },
			ui_color_primary, focus);
		hierarchy_pop();
		return;
	}

	// Slide line
	ui_draw_el(ui_vis_slider_line_active,
		vec3{ 0,          bar_y,      0 },
		vec3{ bar_length, bar_height, bar_depth },
		ui_color_primary, focus);
	ui_draw_el(ui_vis_slider_line_inactive,
		vec3{        - bar_length, bar_y,      0 },
		vec3{ size.x - bar_length, bar_height, bar_depth },
		ui_color_common, focus);
	hierarchy_pop();
}

///////////////////////////////////////////

void ui_progress_bar_at(float percent, vec3 start_pos, vec2 size) {
	ui_progress_bar_at_ex(percent, start_pos, size, 1, false);
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
bool32_t ui_hslider_at_g(const C *id_text, N &value, N min, N max, N step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) {
	uint64_t id     = ui_stack_hash(id_text);
	bool     result = false;

	const float snap_scale = 1;
	const float snap_dist  = 7*cm2m;

	bool  vertical = size.y > size.x;
	float size_min = vertical ? size.x : size.y;

	// Find sizes of slider elements
	float percent      = (float)((value - min) / (max - min));
	float button_depth = confirm_method == ui_confirm_push ? skui_settings.depth : skui_settings.depth * 1.5f;
	float rule_size    = fmaxf(skui_settings.padding, size_min / 6.f);
	vec2  button_size  = confirm_method == ui_confirm_push
		? vec2{ size_min / 2, size_min / 2 }
		: (vertical
			? vec2{ size_min, size_min / 4 }
			: vec2{ size_min / 4, size_min } );

	// Activation bounds sizing
	float activation_plane = button_depth + skui_finger_radius;

	// Set up for getting the state of the sliders.
	button_state_ focus_state   = button_state_inactive;
	button_state_ button_state  = button_state_inactive;
	float         finger_offset = button_depth;
	float         finger_at     = 0;
	int32_t       hand          = -1;
	if (confirm_method == ui_confirm_push) {
		vec3  activation_start = vertical
			? window_relative_pos + vec3{ -(size.x / 2 - button_size.x / 2) + button_size.x / 2.0f, percent * -(size.y-button_size.y) + button_size.y/2.0f, -activation_plane }
			: window_relative_pos + vec3{ percent * -(size.x-button_size.x) + button_size.x/2.0f, -(size.y/2 - button_size.y/2) + button_size.y/2.0f, -activation_plane };
		vec3  activation_size = vec3{ button_size.x*2, button_size.y*2, 0.0001f };
		vec3  sustain_size    = vec3{ size.x + 2*skui_finger_radius, size.y + 2*skui_finger_radius, activation_plane + 6*skui_finger_radius  };
		vec3  sustain_start   = window_relative_pos + vec3{ skui_finger_radius, skui_finger_radius, -activation_plane + sustain_size.z };

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
			finger_at = vertical ? skui_hand[hand].finger.y : skui_hand[hand].finger.x;
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		vec3 activation_start;
		vec3 activation_size;
		if (vertical) {
			activation_start = window_relative_pos + vec3{ -(size.x / 2 - button_size.x / 2), percent * -(size.y - button_size.y) + button_size.y, button_depth };
			activation_size  = vec3{ button_size.x, button_size.y * 3, button_depth * 2 };
		} else {
			activation_start = window_relative_pos + vec3{ percent * -(size.x - button_size.x) + button_size.x, -(size.y / 2 - button_size.y / 2), button_depth };
			activation_size  = vec3{ button_size.x * 3, button_size.y, button_depth * 2 };
		}

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
			finger_at = vertical ? pinch_local.y : pinch_local.x;

			if (confirm_method == ui_confirm_variable_pinch && button_state & button_state_active && scale_step > 0) {
				finger_at = finger_at / (1 + scale_step * snap_scale);
			}
		}
	}

	if (button_state & button_state_active) {
		float pos_in_slider = vertical
			? (float)fmin(1, fmax(0, ((window_relative_pos.y-button_size.y/2)-finger_at) / (size.y-button_size.y)))
			: (float)fmin(1, fmax(0, ((window_relative_pos.x-button_size.x/2)-finger_at) / (size.x-button_size.x)));
		N new_val = (N)min + (N)pos_in_slider*(N)(max-min);
		if (step != 0) {
			new_val = min + ((int)(((new_val - min) / step) + (N)0.5)) * step;
		}
		result  = value != new_val;
		percent = (float)((new_val - min) / (max - min));

		// Play tick sound as the value updates
		if (result) {
			
			if (step != 0) {
				// Play on every change if there's a user specified step value
				sound_play(skui_snd_tick, skui_hand[hand].finger_world, 1);
			} else {
				// If no user specified step, then we'll do a set number of
				// clicks across the whole bar.
				const int32_t click_steps = 10;

				float   old_percent  = (float)((value - min) / (max - min));
				int32_t old_quantize = (int32_t)(old_percent * click_steps + 0.5f);
				int32_t new_quantize = (int32_t)(percent     * click_steps + 0.5f);

				if (old_quantize != new_quantize) {
					sound_play(skui_snd_tick, skui_hand[hand].finger_world, 1);
				}
			}
		}

		// Do this down here so we can calculate old_percent above
		value = new_val;
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
	float y           = window_relative_pos.y;
	float slide_x_rel = 0;
	float slide_y_rel = 0;
	if (vertical) {
		slide_x_rel = (size.x - button_size.x) / 2;
		slide_y_rel = (float)(percent * (size.y - button_size.y));
	} else {
		slide_x_rel = (float)(percent * (size.x - button_size.x));
		slide_y_rel = (size.y - button_size.y) / 2;
	}

	ui_progress_bar_at_ex(percent, window_relative_pos, size, color_blend, vertical);

	if (confirm_method == ui_confirm_push) {
		ui_draw_el(ui_vis_slider_push,
			vec3{x - slide_x_rel, y - slide_y_rel, window_relative_pos.z},
			vec3{button_size.x, button_size.y, fmaxf(finger_offset,rule_size*skui_settings.backplate_depth+mm2m)},
			ui_color_primary, color_blend);
	} else if (confirm_method == ui_confirm_pinch || confirm_method == ui_confirm_variable_pinch) {
		ui_draw_el(ui_vis_slider_pinch,
			vec3{x - slide_x_rel, y - slide_y_rel, window_relative_pos.z},
			vec3{button_size.x, button_size.y, button_depth},
			ui_color_primary, color_blend);

		vec3 pinch_local = hand < 0
			? vec3_zero
			: hierarchy_to_local_point(input_hand((handed_)hand)->pinch_pt);
		int32_t scale_step  = (int32_t)((-pinch_local.z-activation_plane) / snap_dist);
		if (confirm_method == ui_confirm_variable_pinch && button_state & button_state_active && scale_step > 0) {
			float scale     = 1 + scale_step * snap_scale;
			float z         = -activation_plane - (scale_step * snap_dist) + button_depth/2;
			float scaled_at = vertical
				? y+size.y*(scale-1)*0.5f
				: x+size.x*(scale-1)*0.5f;
			
			if (vertical) {
				float connector_x = (x-slide_x_rel) - size_min * 0.5f;
				line_add({ connector_x, y,        window_relative_pos.z}, { connector_x, scaled_at,              window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
				line_add({ connector_x, y-size.y, window_relative_pos.z}, { connector_x, scaled_at-size.y*scale, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
			} else {
				float connector_y = (y-slide_y_rel) - size_min * 0.5f;
				line_add({ x,        connector_y, window_relative_pos.z}, { scaled_at,              connector_y, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
				line_add({ x-size.x, connector_y, window_relative_pos.z}, { scaled_at-size.x*scale, connector_y, window_relative_pos.z + z}, {255,255,255,0}, {255,255,255,255}, rule_size*0.5f);
			}

			if (vertical) {
				ui_progress_bar_at_ex(percent,
					vec3{ x-slide_x_rel, scaled_at, window_relative_pos.z + z },
					vec2{ size.x, size.y*scale },
					color_blend, vertical);
			} else {
				ui_progress_bar_at_ex(percent,
					vec3{ scaled_at, y-slide_y_rel, window_relative_pos.z + z },
					vec2{ size.x*scale, size.y },
					color_blend, vertical);
			}
			ui_draw_el(ui_vis_slider_pinch,
				vertical
					? vec3{ x - slide_x_rel, scaled_at - slide_y_rel * scale, window_relative_pos.z + z }
					: vec3{ scaled_at - slide_x_rel * scale, y - slide_y_rel, window_relative_pos.z + z },
				vec3{ button_size.x, button_size.y, button_depth },
				ui_color_primary, color_blend);
		}
	}
	
	if (hand >= 0 && hand < 2) {
		if (button_state & button_state_just_active)
			sound_play(skui_snd_interact, skui_hand[hand].finger_world, 1);
		else if (button_state & button_state_just_inactive)
			sound_play(skui_snd_uninteract, skui_hand[hand].finger_world, 1);
	}

	if      (notify_on == ui_notify_change)   return result;
	else if (notify_on == ui_notify_finalize) return button_state & button_state_just_inactive;
	else                                      return result;
}
bool32_t ui_hslider_at   (const char     *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_at_g<char    , float>(id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at   (const char16_t *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_at_g<char16_t, float>(id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at_16(const char16_t *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_at_g<char16_t, float>(id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }

bool32_t ui_hslider_at_f64   (const char     *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_at_g<char    , double>(id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at_f64   (const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_at_g<char16_t, double>(id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }
bool32_t ui_hslider_at_f64_16(const char16_t *id_text, double &value, double min, double max, double step, vec3 window_relative_pos, vec2 size, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_at_g<char16_t, double>(id_text, value, min, max, step, window_relative_pos, size, confirm_method, notify_on); }

///////////////////////////////////////////

template<typename C, typename N>
bool32_t ui_hslider_g(const C *name, N &value, N min, N max, N step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz({width, 0}, false, &final_pos, &final_size);

	return ui_hslider_at_g<C, N>(name, value, min, max, step, final_pos, final_size, confirm_method, notify_on);
}

bool32_t ui_hslider   (const char     *name, float &value, float min, float max, float step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_g<char,     float>(name, value, min, max, step, width, confirm_method, notify_on); }
bool32_t ui_hslider_16(const char16_t *name, float &value, float min, float max, float step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_g<char16_t, float>(name, value, min, max, step, width, confirm_method, notify_on); }

bool32_t ui_hslider_f64   (const char     *name, double &value, double min, double max, double step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_g<char,     double>(name, value, min, max, step, width, confirm_method, notify_on); }
bool32_t ui_hslider_f64_16(const char16_t *name, double &value, double min, double max, double step, float width, ui_confirm_ confirm_method, ui_notify_ notify_on) { return ui_hslider_g<char16_t, double>(name, value, min, max, step, width, confirm_method, notify_on); }

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

template<typename C>
void ui_window_begin_g(const C *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	uint64_t id = ui_push_id(text);
	int32_t index = skui_sl_windows.binary_search(&ui_window_t::hash, id);
	if (index < 0) {
		index = ~index;
		ui_window_t new_window = {};
		new_window.hash = id;
		new_window.prev_size = window_size;
		new_window.curr_size = window_size;
		skui_sl_windows.insert(index, new_window);
	}
	ui_window_t &window = skui_sl_windows[index];
	window.type = window_type;
	
	// figure out the size of it, based on its window type
	vec3 box_start = {}, box_size = {};
	if (window.type & ui_win_head) {
		float line = ui_line_height();
		box_start = vec3{ 0, line/2, skui_settings.depth/2 };
		box_size  = vec3{ window.prev_size.x, line, skui_settings.depth*2 };
	} 
	if (window.type & ui_win_body) {
		box_start.z  = skui_settings.depth/2;
		box_start.y -= window.prev_size.y / 2;
		box_size.x   = window.prev_size.x;
		box_size.y  += window.prev_size.y;
		box_size.z   = skui_settings.depth * 2;
	}

	// Set up window handle and layout area
	_ui_handle_begin(id, pose, { box_start, box_size }, false, move_type);
	ui_layout_push({}, {});
	ui_layout_area(window, { window.prev_size.x / 2,0,0 }, window_size);

	// draw label
	if (window.type & ui_win_head) {
		ui_layout_t* layout = &skui_layouts.last();

		vec2 size = text_size(text, skui_font_stack.last());
		vec3 at   = layout->offset - vec3{ skui_settings.padding, -ui_line_height(), 2*mm2m };
		ui_text_at(text, text_align_center_left, text_fit_squeeze, at, size);

		if (layout->max_x > at.x - size.x - skui_settings.padding)
			layout->max_x = at.x - size.x - skui_settings.padding;
	}
	window.pose = pose;
}
void ui_window_begin(const char *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char>(text, pose, window_size, window_type, move_type);
}
void ui_window_begin_16(const char16_t *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	ui_window_begin_g<char16_t>(text, pose, window_size, window_type, move_type);
}

///////////////////////////////////////////

void ui_window_end() {
	ui_layout_t *layout = &skui_layouts.last();
	ui_layout_pop();
	layout->window->prev_size = layout->window->curr_size;

	vec3 start = layout->window->layout_start + vec3{ 0,0,skui_settings.depth };
	vec3 size  = { layout->window->curr_size.x, layout->window->curr_size.y, skui_settings.depth };

	float line_height = ui_line_height();
	if (layout->window->type & ui_win_head) {
		float glow = 1;
		if (skui_hand[0].focused_prev == layout->window->hash || skui_hand[1].focused_prev == layout->window->hash)
			glow = 1.5f;
		ui_draw_el(layout->window->type == ui_win_head ? ui_vis_window_head_only : ui_vis_window_head, start + vec3{0,line_height,0}, { size.x, line_height, size.z }, ui_color_primary, glow);
	}
	if (layout->window->type & ui_win_body) {
		ui_draw_el(layout->window->type == ui_win_body ? ui_vis_window_body_only : ui_vis_window_body, start, size, ui_color_background, 1);
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
	ui_draw_el(ui_vis_panel, start+start_offset, vec3{ size.x, size.y, skui_settings.depth* 0.1f }+size_offset, ui_color_complement, 1);
}

///////////////////////////////////////////

struct panel_stack_data_t {
	vec3    at;
	float   max_x;
	ui_pad_ padding;
};
array_t<panel_stack_data_t> skui_panel_stack = {};
void ui_panel_begin(ui_pad_ padding) {
	ui_layout_t*       layout = &skui_layouts.last();
	panel_stack_data_t data;
	data.at      = layout->offset;
	data.max_x   = layout->max_x;
	data.padding = padding;

	skui_panel_stack.add( data );
	layout->max_x = layout->offset_initial.x;

	if (padding == ui_pad_inside) {
		float gutter = skui_settings.gutter / 2;
		layout->offset_initial.x -= gutter;
		layout->offset_initial.y -= gutter;
		layout->offset.x -= gutter;
		layout->offset.y -= gutter;
	}
}

///////////////////////////////////////////

void ui_panel_end() {
	ui_sameline();

	float              gutter = skui_settings.gutter / 2;
	ui_layout_t*       layout = &skui_layouts.last();
	panel_stack_data_t start  = skui_panel_stack.last();
	if (start.padding == ui_pad_inside) layout->max_x -= gutter;

	vec3 curr = vec3{layout->max_x, layout->offset.y - (layout->line_height + (start.padding == ui_pad_inside?gutter:0)), layout->offset.z};

	ui_panel_at(start.at, {fabsf(curr.x-start.at.x), start.at.y-curr.y}, start.padding);

	layout->max_x = fminf(layout->max_x, start.max_x);
	if (start.padding == ui_pad_inside) {
		layout->offset_initial.x += gutter;
		layout->offset_initial.y += gutter;
		layout->line_height += gutter * 2;
		layout->offset.y += gutter;
		layout->offset.x -= gutter;
	}
	skui_panel_stack.pop();
	ui_nextline();
}

} // namespace sk
