#include "stereokit_ui.h"
#include "_stereokit_ui.h"
#include "math.h"
#include "systems/input.h"
#include "libraries/stref.h"

#define SL_IMPLEMENTATION
#include "libraries/sort_list.h"

#include <DirectXMath.h>
using namespace DirectX;
#include <vector>
using namespace std;

///////////////////////////////////////////

namespace sk {

struct ui_window_t {
	pose_t  pose;
	vec2    size;
	ui_win_ type;
};

struct layer_t {
	ui_window_t *window;
	vec3   offset_initial;
	vec3   offset;
	vec2   size;
	float  line_height;
	float  max_x;
	vec3 finger_pos[handed_max];
	vec3 finger_prev[handed_max];
};
struct ui_hand_t {
	vec3            finger;
	vec3            finger_prev;
	vec3            finger_world;
	vec3            finger_world_prev;
	bool            tracked;
	uint64_t        focused_prev = {};
	uint64_t        focused = {};
	uint64_t        active_prev = {};
	uint64_t        active = {};
};

struct ui_id_t {
	uint64_t id;
};

ui_window_t    *skui_sl_windows = nullptr;
vector<ui_id_t> skui_id_stack;
vector<layer_t> skui_layers;
mesh_t          skui_box = nullptr;
vec3            skui_box_min = {};
mesh_t          skui_box_dbg = nullptr;
vec3            skui_box_dbg_min = {};
mesh_t          skui_cylinder;
vec3            skui_cylinder_min = {};
material_t      skui_mat;
material_t      skui_mat_quad;
material_t      skui_mat_dbg;
font_t          skui_font;
text_style_t    skui_font_style;
material_t      skui_font_mat;
ui_hand_t       skui_hand[2];
bool32_t        skui_show_volumes = false;
uint64_t        skui_input_target = 0;

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

uint64_t skui_anim_id;
float    skui_anim_time;

const color128 skui_color_border = { 1,1,1,1 };
color128 skui_palette[5];

///////////////////////////////////////////

uint64_t ui_hash(const char *string, uint64_t start_hash = 14695981039346656037);
uint64_t ui_stack_hash(const char *string);

// Layout
void ui_push_pose  (ui_window_t &window, vec3 offset);
void ui_pop_pose   ();
void ui_layout_box (vec2 content_size, vec3 &out_position, vec2 &out_final_size, bool32_t use_content_padding = true);
void ui_reserve_box(vec2 size);

void ui_nextline    ();
void ui_sameline    ();
void ui_reserve_box (vec2 size);
void ui_space       (float space);

// Interaction
bool32_t ui_in_box            (vec3 pt1, vec3 pt2, bounds_t box);
void     ui_box_interaction_1h(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t &out_hand);
void     ui_button_behavior   (vec3 window_relative_pos, vec2 size, uint64_t id, float& finger_offset, button_state_& button_state, button_state_& focus_state);

// Base render types
void ui_box      (vec3 start, vec3 size, material_t material, color128 color);
void ui_text     (vec3 start, vec2 size, const char *text, text_align_ position, text_align_ align);

///////////////////////////////////////////

void quadrantify(vert_t *verts, int32_t count, bool uvs) {
	float left   =  1000;
	float right  = -1000;
	float top    = -1000;
	float bottom =  1000;

	for (int32_t i = 0; i < count; i++) {
		if (verts[i].pos.x < left  ) left   = verts[i].pos.x;
		if (verts[i].pos.x > right ) right  = verts[i].pos.x;
		if (verts[i].pos.y < bottom) bottom = verts[i].pos.y;
		if (verts[i].pos.y > top   ) top    = verts[i].pos.y;
	}

	for (int32_t i = 0; i < count; i++) {
		float q_x = verts[i].pos.x / fabsf(verts[i].pos.x);
		float q_y = verts[i].pos.y / fabsf(verts[i].pos.y);
		if (verts[i].pos.x == 0) q_x = 0;
		if (verts[i].pos.y == 0) q_y = 0;
		if (uvs)
			verts[i].uv = { q_x, q_y };
		if      (q_x < 0) verts[i].pos.x -= left;
		else if (q_x > 0) verts[i].pos.x -= right;
		if      (q_y < 0) verts[i].pos.y -= bottom;
		else if (q_y > 0) verts[i].pos.y -= top;
	}
}

void ui_quadrant_mesh(float padding) {
	if (skui_box == nullptr)
		skui_box = mesh_create();
	
	float radius = padding / 2;
	skui_box_min = { padding, padding, 0 };

	vind_t subd = (vind_t)3*4;
	int vert_count = subd * 5 + 2;
	int ind_count  = subd * 18;
	vert_t *verts = (vert_t *)malloc(vert_count * sizeof(vert_t));
	vind_t *inds  = (vind_t *)malloc(ind_count  * sizeof(vind_t));

	vind_t ind = 0;

	for (vind_t i = 0; i < subd; i++) {
		float u = 0, v = 0;

		float ang = ((float)i / subd + (0.5f/subd)) * (float)3.14159f * 2;
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
	quadrantify(verts, vert_count, true);

	mesh_set_verts(skui_box, verts, vert_count);
	mesh_set_inds (skui_box, inds,  ind_count);
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

void ui_settings(ui_settings_t settings) {
	if (settings.backplate_border == 0) settings.backplate_border = 0.5f * mm2m;
	if (settings.backplate_depth  == 0) settings.backplate_depth  = 0.4f;
	if (settings.depth            == 0) settings.depth   = 10 * mm2m;
	if (settings.gutter           == 0) settings.gutter  = 20 * mm2m;
	if (settings.padding          == 0) settings.padding = 10 * mm2m;
	skui_settings = settings; 

	ui_quadrant_mesh(settings.padding);
}

///////////////////////////////////////////

void ui_set_color(color128 color) {
	vec3 hsv = color_to_hsv(color);
	
	skui_palette[0] = color;
	skui_palette[1] = color_hsv(hsv.x, hsv.y * 0.5f,   hsv.z * 0.3f, color.a);
	skui_palette[2] = color_hsv(hsv.x,                  hsv.y * 0.075f, hsv.z * 0.7f, color.a);
	skui_palette[3] = color_hsv(fmodf(hsv.x + 0.5f, 1), hsv.y * 0.075f, hsv.z * 0.7f, color.a);
	skui_palette[4] = color128{1, 1, 1, 1};
}

///////////////////////////////////////////

bool ui_init() {
	ui_set_color(color_hsv(0.07f, 0.8f, 0.5f, 1));

	ui_quadrant_mesh(skui_settings.padding);
	skui_box_dbg  = mesh_gen_cube(vec3_one);
	//skui_box      = mesh_gen_cube(vec3{skui_settings.padding * 2,skui_settings.padding * 2,1});//mesh_gen_cylinder(skui_settings.padding * 2, 1,{ 0,0,1 }, 24);// mesh_gen_rounded_cube(vec3_one * skui_settings.padding * 2, skui_settings.padding * 2 * 0.2f, 2); // mesh_gen_cube(vec3_one);
	skui_cylinder = mesh_gen_cylinder(1, 1, {0,0,1}, 24);
	skui_mat      = material_find("default/material_ui");
	skui_mat_quad = material_find("default/material_ui_quadrant");
	//material_set_wireframe(skui_mat_quad, true);
	skui_mat_dbg  = material_copy(skui_mat);
	material_set_transparency(skui_mat_dbg, transparency_blend);
	material_set_color(skui_mat_dbg, "color", { 0,1,0,0.25f });

	skui_font_mat   = material_find("default/material_font");
	material_set_queue_offset(skui_font_mat, -12);
	skui_font       = font_find("default/font");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, color_to_32( skui_palette[4] ));
	
	skui_layers.push_back({});
	skui_id_stack.push_back({ STREF_HASH_START });

	skui_snd_interact   = sound_find("default/sound_click");
	skui_snd_uninteract = sound_find("default/sound_unclick");
	skui_snd_grab   = sound_find("default/sound_grab");
	skui_snd_ungrab = sound_find("default/sound_ungrab");

	return true;
}

///////////////////////////////////////////

void ui_update() {
	if (skui_layers.size() > 1 || skui_layers.size() == 0)
		log_err("ui: Mismatching number of Begin/End calls!");
	if (skui_id_stack.size() > 1 || skui_id_stack.size() == 0)
		log_err("ui: Mismatching number of id push/pop calls!");

	skui_layers[0] = {};

	for (size_t i = 0; i < handed_max; i++) {
		const hand_t &hand = input_hand((handed_)i);

		skui_hand[i].finger_world_prev = skui_hand[i].finger_world;
		skui_hand[i].finger_world      = hand.fingers[1][4].position;
		skui_hand[i].focused_prev = skui_hand[i].focused;
		skui_hand[i].active_prev  = skui_hand[i].active;

		skui_hand[i].focused = 0;
		skui_hand[i].active  = 0;
		skui_hand[i].finger       = matrix_mul_point(hierarchy_to_local(), skui_hand[i].finger_world);
		skui_hand[i].finger_prev  = matrix_mul_point(hierarchy_to_local(), skui_hand[i].finger_world_prev);
		skui_hand[i].tracked      = hand.tracked_state & button_state_active;

		skui_layers[0].finger_pos [i] = skui_hand[i].finger;
		skui_layers[0].finger_prev[i] = skui_hand[i].finger_prev;
	}
}

///////////////////////////////////////////

void ui_shutdown() {
	sl_free(skui_sl_windows);

	sound_release(skui_snd_interact);
	sound_release(skui_snd_uninteract);
	sound_release(skui_snd_grab);
	sound_release(skui_snd_ungrab);
	mesh_release(skui_box);
	mesh_release(skui_cylinder);
	material_release(skui_mat);
	material_release(skui_mat_dbg);
	material_release(skui_mat_quad);
	material_release(skui_font_mat);
	font_release(skui_font);
}

///////////////////////////////////////////

uint64_t ui_stack_hash(const char *string) {
	return skui_id_stack.size() > 0 
		? string_hash(string, skui_id_stack.back().id) 
		: string_hash(string);
}

///////////////////////////////////////////

uint64_t ui_push_id(const char *id) {
	uint64_t result = ui_stack_hash(id);
	skui_id_stack.push_back({ result });
	return result;
}

///////////////////////////////////////////

void ui_pop_id() {
	skui_id_stack.pop_back();
}

///////////////////////////////////////////

bool32_t ui_is_interacting(handed_ hand) {
	return skui_hand[hand].active_prev != 0 || skui_hand[hand].focused_prev != 0;
}

///////////////////////////////////////////
//////////////   Layout!   ////////////////
///////////////////////////////////////////

void ui_push_pose(pose_t &pose, vec3 offset) {
	vec3   right = pose.orientation * vec3_right;
	matrix trs   = matrix_trs(pose.position + right*offset, pose.orientation);
	hierarchy_push(trs);

	skui_layers.push_back(layer_t{
		nullptr,
		vec3{skui_settings.padding, -skui_settings.padding}, 
		vec3{skui_settings.padding, -skui_settings.padding}, {0,0}, 0, 0
	});

	layer_t &layer = skui_layers.back();
	for (size_t i = 0; i < handed_max; i++) {
		layer.finger_pos [i] = skui_hand[i].finger      = matrix_mul_point(hierarchy_to_local(), skui_hand[i].finger_world);
		layer.finger_prev[i] = skui_hand[i].finger_prev = matrix_mul_point(hierarchy_to_local(), skui_hand[i].finger_world_prev);
	}
}

///////////////////////////////////////////

void ui_pop_pose() {
	hierarchy_pop();
	skui_layers.pop_back();

	layer_t &layer = skui_layers.back();
	for (size_t i = 0; i < handed_max; i++) {
		skui_hand[i].finger      = layer.finger_pos[i];
		skui_hand[i].finger_prev = layer.finger_prev[i];
	}
}

///////////////////////////////////////////

void ui_layout_area(vec3 start, vec2 dimensions) {
	layer_t &layer = skui_layers.back();
	layer.window         = layer.window;
	layer.offset_initial = start;
	layer.offset         = layer.offset_initial - vec3{ skui_settings.padding, skui_settings.padding };
	layer.size           = dimensions;
	layer.max_x          = 0;
	layer.line_height    = 0;
}

///////////////////////////////////////////

void ui_layout_area(ui_window_t &window, vec3 start, vec2 dimensions) {
	layer_t &layer = skui_layers.back();
	layer.window         = &window;
	ui_layout_area(start, dimensions);
}

///////////////////////////////////////////

vec2 ui_area_remaining() {
	layer_t &layer = skui_layers.back();
	return vec2{
		fmaxf(0, layer.size.x - layer.offset.x),
		fmaxf(0, layer.size.y - layer.offset.y)
	};
}

///////////////////////////////////////////

void ui_layout_exact(vec2 content_size, vec3 &out_position) {
	out_position = skui_layers.back().offset;

	// If this is not the first element, and it goes outside the active window
	if (out_position.x            != -skui_settings.padding &&
		skui_layers.back().size.x != 0                      &&
		out_position.x - content_size.x < skui_layers.back().offset_initial.x - skui_layers.back().size.x + skui_settings.padding)
	{
		ui_nextline();
		out_position = skui_layers.back().offset;
	}
}

///////////////////////////////////////////

void ui_layout_box(vec2 content_size, vec3 &out_position, vec2 &out_final_size, bool32_t use_content_padding) {
	out_final_size = content_size;
	if (use_content_padding)
		out_final_size += vec2{ skui_settings.padding, skui_settings.padding }*2;

	ui_layout_exact(out_final_size, out_position);
}

///////////////////////////////////////////

void ui_reserve_box(vec2 size) {
	if (skui_layers.back().max_x > skui_layers.back().offset.x - size.x - skui_settings.padding)
		skui_layers.back().max_x = skui_layers.back().offset.x - size.x - skui_settings.padding;
	if (skui_layers.back().line_height < size.y)
		skui_layers.back().line_height = size.y;

	skui_layers.back().offset -= vec3{ size.x + skui_settings.gutter, 0, 0 };
}

///////////////////////////////////////////

void ui_nextline() {
	layer_t &layer = skui_layers.back();
	skui_prev_offset      = layer.offset;
	skui_prev_line_height = layer.line_height;

	layer.offset.x    = skui_layers.back().offset_initial.x - skui_settings.padding;
	layer.offset.y   -= skui_layers.back().line_height + skui_settings.gutter;
	layer.line_height = 0;
}

///////////////////////////////////////////

void ui_sameline() {
	layer_t &layer = skui_layers.back();
	layer.offset      = skui_prev_offset;
	layer.line_height = skui_prev_line_height;
}

///////////////////////////////////////////

float ui_line_height() {
	return skui_settings.padding * 2 + skui_fontsize;
}

///////////////////////////////////////////

void ui_space(float space) {
	if (skui_layers.back().offset.x == skui_layers.back().offset_initial.x - skui_settings.padding)
		skui_layers.back().offset.y -= space;
	else
		skui_layers.back().offset.x += space;
}

///////////////////////////////////////////
///////////   Interaction!   //////////////
///////////////////////////////////////////

void ui_box_interaction_1h(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state, int32_t &hand) {
	hand = -1;
	if (out_focus_state != nullptr)
		*out_focus_state = button_state_inactive;

	for (int32_t i = 0; i < handed_max; i++) {
		bool was_focused = skui_hand[i].focused_prev == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (was_focused) {
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		if (skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, ui_size_box(box_start, box_size))) {
			hand = i;
			skui_hand[i].focused = id;
			button_state_ focus_state = button_state_active;
			if (!was_focused)
				focus_state |= button_state_just_active;

			if (out_focus_state != nullptr)
				*out_focus_state = focus_state;
			
		} else if (was_focused) {
			hand = i;
			if (out_focus_state != nullptr)
				*out_focus_state = button_state_inactive | button_state_just_inactive;
		}
	}
}

///////////////////////////////////////////

bool32_t ui_in_box(vec3 pt, vec3 pt_prev, bounds_t box) {
	if (skui_show_volumes)
		render_add_mesh(skui_box_dbg, skui_mat_dbg, matrix_trs(box.center, quat_identity, box.dimensions));
	return bounds_line_contains(box, pt, pt_prev);
}

///////////////////////////////////////////

void ui_button_behavior(vec3 window_relative_pos, vec2 size, uint64_t id, float &finger_offset, button_state_ &button_state, button_state_ &focus_state) {
	button_state = button_state_inactive;
	focus_state  = button_state_inactive;
	int32_t hand = -1;

	// Button interaction focus is detected in the front half of the button to prevent 'reverse'
	// or 'side' presses where the finger comes from the back or side.
	//
	// Once focused is gained, interaction is tracked within a volume that extends from the 
	// front, to a good distance through the button's back. This is to help when the user's
	// finger inevitably goes completely through the button. May consider expanding the volume 
	// a bit too on the X/Y axes later.
	vec3    box_start = window_relative_pos + vec3{ 0, 0, -skui_settings.depth/2.f };
	vec3    box_size  = vec3{ size.x, size.y, skui_settings.depth/2.f };
	ui_box_interaction_1h(id,
		box_start, box_size,
		box_start + vec3{ 0,0, skui_settings.depth * 4 },
		box_size  + vec3{ 0,0, skui_settings.depth * 4 },
		&focus_state, hand);

	// If a hand is interacting, adjust the button surface accordingly
	finger_offset = skui_settings.depth;
	if (focus_state & button_state_active) {
		finger_offset = -skui_hand[hand].finger.z - window_relative_pos.z;
		if (finger_offset < skui_settings.depth / 2) {
			button_state = button_state_active;
			skui_hand[hand].active = id;
			if (skui_hand[hand].active_prev != id) {
				button_state |= button_state_just_active;
			}
		} else if (skui_hand[hand].active_prev == id) {
			button_state |= button_state_just_inactive;
		}
		finger_offset = fmaxf(2*mm2m, finger_offset);
	} else if (focus_state & button_state_just_inactive) {
		if (skui_hand[hand].active_prev == id) {
			button_state |= button_state_just_inactive;
		}
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

	render_add_mesh(skui_box, material, mx, color);
}

///////////////////////////////////////////

void ui_cylinder(vec3 start, float radius, float depth, material_t material, color128 color) {
	vec3   pos = start - (vec3{ radius, radius, depth } / 2);
	matrix mx  = matrix_trs(pos, quat_identity, {radius, radius, depth});

	render_add_mesh(skui_cylinder, material, mx, color);
}

///////////////////////////////////////////

void ui_model_at(model_t model, vec3 start, vec3 size, color128 color) {
	matrix mx = matrix_trs(start, quat_identity, size);
	render_add_model(model, mx, color);
}

///////////////////////////////////////////

bool32_t ui_volume_at(const char *id, bounds_t bounds) {
	uint64_t id_hash = ui_stack_hash(id);
	bool     result  = false;

	for (int32_t i = 0; i < handed_max; i++) {
		bool     was_focused = skui_hand[i].focused_prev == id_hash;
		bounds_t size        = bounds;
		if (was_focused) {
			size.dimensions = bounds.dimensions + vec3_one*skui_settings.padding;
		}

		if (skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, size)) {
			skui_hand[i].focused = id_hash;
			if (!was_focused)
				result = true;
		}
	}

	return result;
}

///////////////////////////////////////////

button_state_ ui_interact_volume_at(bounds_t bounds, handed_ &out_hand) {
	button_state_ result  = button_state_inactive;

	for (int32_t i = 0; i < handed_max; i++) {
		bool     was_active  = skui_hand[i].active_prev  != 0;
		bool     was_focused = skui_hand[i].focused_prev != 0 || was_active;
		if (was_active || was_focused)
			continue;

		if (skui_hand[i].tracked && ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, bounds)) {
			button_state_ state = input_hand((handed_)i).pinch_state;
			if (state != button_state_active) {
				result = state;
				out_hand = (handed_)i;
			}
		}
	}

	return result;
}

///////////////////////////////////////////

void ui_text(vec3 start, vec2 size, const char *text, text_align_ position, text_align_ align) {
	text_add_in(text, matrix_identity, size, text_fit_squeeze, skui_font_style, position, align, start.x, start.y, start.z);
}

///////////////////////////////////////////
///////////   UI Components   /////////////
///////////////////////////////////////////

void ui_label_sz(const char *text, vec2 size) {
	vec3 offset;
	ui_layout_exact(size, offset);
	ui_reserve_box(size);
	ui_nextline();

	ui_text(offset, size, text, text_align_x_left | text_align_y_top, text_align_x_left | text_align_y_center);
}

///////////////////////////////////////////

void ui_label(const char *text, bool32_t use_padding) {
	vec3  offset   = skui_layers.back().offset;
	vec2  txt_size = text_size(text, skui_font_style);
	vec2  size     = txt_size;
	float pad      = use_padding ? skui_settings.padding : 0;

	ui_layout_box (size, offset, size, use_padding);
	ui_reserve_box(size);
	ui_nextline();
	ui_text(offset - vec3{pad, pad, 2*mm2m }, txt_size, text, text_align_x_left | text_align_y_top, text_align_x_left | text_align_y_center);
}

///////////////////////////////////////////

void ui_image(sprite_t image, vec2 size) {
	float aspect     = sprite_get_aspect(image);
	vec3  offset     = skui_layers.back().offset;
	vec2  final_size = vec2{
		size.x==0 ? size.y*aspect : size.x, 
		size.y==0 ? size.x/aspect : size.y };

	vec2 layout_size;
	ui_layout_box (final_size, offset, layout_size, false);
	ui_reserve_box(layout_size);
	ui_nextline();
	
	sprite_draw(image, matrix_trs(offset - vec3{ final_size.x, 0, 2*mm2m }, quat_identity, vec3{ final_size.x, final_size.y, 1 }));
}

///////////////////////////////////////////

bool32_t ui_button_at(const char *text, vec3 window_relative_pos, vec2 size) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = state & button_state_active || focus & button_state_active ? 2.f : 1;
	float back_size   = skui_settings.backplate_border;
	if (ui_anim_has(id, .1f)) {
		float t     = ui_anim_elapsed    (id, .1f);
		back_size   = math_ease_hop      (back_size, fmaxf(mm2m*2, back_size*2.f), t);
		color_blend = math_ease_overshoot(1, 2.f, 10, t);
	}

	ui_box (window_relative_pos,  vec3{ size.x,   size.y,   finger_offset }, skui_mat_quad, skui_palette[2] * color_blend);
	//ui_box (window_relative_pos + vec3{back_size, back_size, mm2m}, vec3{ size.x+back_size*2, size.y+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m }, skui_mat_quad, skui_color_border * color_blend);
	ui_text(window_relative_pos - vec3{ size.x/2, size.y/2, finger_offset + 2*mm2m }, vec2{size.x-skui_settings.padding*2, size.y-skui_settings.padding*2}, text, text_align_center, text_align_center);

	return state & button_state_just_active;
}

///////////////////////////////////////////

bool32_t ui_button_sz(const char *text, vec2 size) {
	vec3 offset;
	ui_layout_exact(size, offset);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_button_at(text, offset, size);
}

///////////////////////////////////////////

bool32_t ui_button(const char *text) {
	vec3 offset;
	vec2 size;
	ui_layout_box (text_size(text, skui_font_style), offset, size);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_button_at(text, offset, size);
}

///////////////////////////////////////////

bool32_t ui_toggle_at(const char *text, bool32_t &pressed, vec3 window_relative_pos, vec2 size) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, size, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = pressed || focus & button_state_active ? 2.f : 1;
	float back_size   = skui_settings.backplate_border;
	if (ui_anim_has(id, .1f)) {
		float t     = ui_anim_elapsed    (id, .1f);
		back_size   = math_ease_hop      (back_size, fmaxf(mm2m*2, back_size*2.f), t);
		color_blend = math_ease_overshoot(1, 2.f, 10, t);
	}

	if (state & button_state_just_active) {
		pressed = !pressed;
	}
	finger_offset = pressed ? fminf(skui_settings.backplate_depth*skui_settings.depth + mm2m, finger_offset) : finger_offset;

	ui_box (window_relative_pos,  vec3{ size.x,    size.y,   finger_offset }, skui_mat_quad, skui_palette[2] * color_blend);
	//ui_box (window_relative_pos + vec3{ back_size, back_size, mm2m}, vec3{ size.x+back_size*2, size.y+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m }, skui_mat_quad, skui_color_border * color_blend);
	ui_text(window_relative_pos - vec3{ size.x/2,  size.y/2, finger_offset + 2*mm2m }, vec2{size.x-skui_settings.padding*2, size.y-skui_settings.padding*2}, text, text_align_center, text_align_center);

	return state & button_state_just_active;
}

///////////////////////////////////////////

bool32_t ui_toggle(const char *text, bool32_t &pressed) {
	vec3 offset;
	vec2 size;
	ui_layout_box (text_size(text, skui_font_style), offset, size);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_toggle_at(text, pressed, offset, size);
}

///////////////////////////////////////////

bool32_t ui_toggle_sz(const char *text, bool32_t &pressed, vec2 size) {
	vec3 offset;
	ui_layout_exact(size, offset);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_toggle_at(text, pressed, offset, size);
}

///////////////////////////////////////////

bool32_t ui_button_round_at(const char *text, sprite_t image, vec3 window_relative_pos, float diameter) {
	uint64_t      id = ui_stack_hash(text);
	float         finger_offset;
	button_state_ state, focus;
	ui_button_behavior(window_relative_pos, { diameter,diameter }, id, finger_offset, state, focus);

	if (state & button_state_just_active)
		ui_anim_start(id);
	float color_blend = state & button_state_active || focus & button_state_active ? 2.f : 1;
	float back_size   = skui_settings.backplate_border;
	if (ui_anim_has(id, .1f)) {
		float t     = ui_anim_elapsed    (id, .1f);
		back_size   = math_ease_hop      (back_size, fmaxf(mm2m*2, back_size*2.f), t);
		color_blend = math_ease_overshoot(1, 2.f, 10, t);
	}

	ui_cylinder(window_relative_pos, diameter, finger_offset, skui_mat, skui_palette[2] * color_blend);
	ui_cylinder(window_relative_pos + vec3{back_size, back_size, mm2m}, diameter+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m, skui_mat, skui_color_border * color_blend);
	sprite_draw(image, matrix_trs(window_relative_pos + vec3{ -diameter, 0, -(finger_offset + 2*mm2m) }, quat_identity, vec3{ diameter, diameter, 1 }));

	return state & button_state_just_active;
}

///////////////////////////////////////////

bool32_t ui_button_round(const char *id, sprite_t image, float diameter) {
	if (diameter == 0)
		diameter = ui_line_height();
	vec3 offset;
	vec2 size = vec2{diameter, diameter};
	size = vec2_one * fmaxf(size.x, size.y);
	ui_layout_box (size, offset, size, false);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_button_round_at(id, image, offset, size.x);
}

///////////////////////////////////////////

void ui_model(model_t model, vec2 ui_size, float model_scale) {
	vec3 offset = skui_layers.back().offset;
	vec2 size   = ui_size + vec2{ skui_settings.padding, skui_settings.padding }*2;

	ui_reserve_box(size);
	size = size / 2;
	ui_model_at(model, { offset.x + size.x, offset.y - size.y, offset.z }, vec3_one * model_scale, { 1,1,1,1 });
	ui_nextline();
}

///////////////////////////////////////////

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size, vec2 size) {
	uint64_t id_hash = ui_stack_hash(id);
	bool     result  = false;
	vec3     offset  = skui_layers.back().offset;
	vec3 box_size = vec3{ size.x, size.y, skui_settings.depth/2 };

	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, ui_size_box(offset, box_size))) {
			skui_hand[i].focused = id_hash;
			skui_input_target = id_hash;
		}
	}
	if (skui_input_target == id_hash) {
		char add = '\0';
		bool shift = input_key(key_shift) & button_state_active;
		if (input_key(key_backspace) & button_state_just_active) {
			size_t len = strlen(buffer);
			if (len >= 0) {
				buffer[len - 1] = '\0';
				result = true;
			}
		}
		if (input_key(key_space) & button_state_just_active) add = ' ';
		for (int32_t k = 0; k < 26; k++) {
			if (input_key((key_)(key_a + k)) & button_state_just_active) {
				add = (char)((shift ? 'A' : 'a') + k);
			}
		}
		for (int32_t k = 0; k < 10; k++) {
			if (input_key((key_)(key_0 + k)) & button_state_just_active) {
				const char *nums = ")!@#$%^&*(";
				add = (char)(shift ? nums[k] : '0'+k);
			}
		}
		if (add != '\0') {
			size_t len = strlen(buffer);
			if (len + 2 < buffer_size) {
				buffer[len] = add;
				buffer[len + 1] = '\0';
				result = true;
			}
		}
	}

	ui_reserve_box(size);
	ui_box (offset, vec3{ size.x, size.y, skui_settings.depth/2 }, skui_mat_quad, skui_palette[2] * (skui_input_target == id_hash ? 0.5f : 1.f) );
	ui_text(offset - vec3{ skui_settings.padding, skui_settings.padding, skui_settings.depth/2 + 2*mm2m }, {size.x-skui_settings.padding*2,size.y-skui_settings.padding*2}, buffer, text_align_x_left | text_align_y_top, text_align_x_left | text_align_y_center);
	ui_nextline();

	return result;
}

///////////////////////////////////////////

bool32_t ui_hslider_at(const char *id_text, float &value, float min, float max, float step, vec3 window_relative_pos, vec2 size) {
	uint64_t   id     = ui_stack_hash(id_text);
	bool       result = false;
	float      color  = 1;

	// Find sizes of slider elements
	float rule_size = fmax(skui_settings.padding, size.y / 6.f);
	vec3  box_start = window_relative_pos + vec3{ 0, 0, skui_settings.depth };
	vec3  box_size  = vec3{ size.x, size.y, skui_settings.depth*2 };

	button_state_ focus_state;
	int32_t hand = -1;
	ui_box_interaction_1h(id,
		box_start, box_size,
		box_start + vec3{ 0.5f,0.5f,0.5f } * cm2m,
		box_size  + vec3{ 1,1,1 } * cm2m,
		&focus_state, hand);

	if (focus_state & button_state_active) {
		float new_val = min + fminf(1, fmaxf(0, (fabsf(skui_hand[hand].finger.x - window_relative_pos.x) / size.x))) * (max - min);
		if (step != 0) {
			new_val = ((int)(((new_val - min) / step) + 0.5f)) * step;
		}
		result = value != new_val;
		value = new_val;

		skui_hand[hand].active = id;
	}

	// Draw the UI
	float back_size   = skui_settings.backplate_border;
	float x           = window_relative_pos.x;
	float line_y      = window_relative_pos.y - size.y / 2.f + rule_size / 2.f;
	float slide_x_rel = ((value - min) / (max - min)) * size.x;
	float slide_y     = window_relative_pos.y - size.y / 4;
	// Slide line
	ui_box(
		vec3{ x, line_y, window_relative_pos.z }, 
		vec3{ size.x, rule_size, rule_size*skui_settings.backplate_depth+mm2m+mm2m }, 
		skui_mat_quad, skui_palette[2] * color);
	ui_cylinder(
		vec3{ x - slide_x_rel + size.y/4.f, slide_y, window_relative_pos.z}, 
		size.y/2.f, rule_size+mm2m, skui_mat, skui_palette[0] * color);
	ui_cylinder(
		vec3{ x+back_size - slide_x_rel + size.y/4.f, slide_y + back_size, window_relative_pos.z}, 
		size.y/2.f+back_size*2, rule_size*skui_settings.backplate_depth+mm2m, skui_mat, skui_color_border * color);
	

	if (focus_state & button_state_just_active)
		sound_play(skui_snd_interact, skui_hand[hand].finger_world, 1);
	else if (focus_state & button_state_just_inactive)
		sound_play(skui_snd_uninteract, skui_hand[hand].finger_world, 1);

	return result;
}

///////////////////////////////////////////

bool32_t ui_hslider(const char *name, float &value, float min, float max, float step, float width) {
	vec3 offset = skui_layers.back().offset;
	if (width == 0)
		width = skui_layers.back().size.x == 0 ? 0.1f : (skui_layers.back().size.x - skui_settings.padding) - offset.x;
	vec2 size = { width, ui_line_height() };

	// Draw the UI
	ui_reserve_box(size);
	ui_nextline();
	
	return ui_hslider_at(name, value, min, max, step, offset, size);
}

///////////////////////////////////////////

bool32_t _ui_handle_begin(uint64_t id, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type) {
	bool result = false;
	float color = 1;

	matrix to_local = hierarchy_to_local();
	ui_push_pose(movement, vec3{ 0,0,0 });

	// If the handle is scale of zero, we don't actually want to draw or interact with it
	if (handle.dimensions.x == 0 || handle.dimensions.y == 0 || handle.dimensions.z == 0)
		return false;

	vec3     box_start = handle.center;//   +vec3{ skui_settings.padding, skui_settings.padding, skui_settings.padding };
	vec3     box_size  = handle.dimensions + vec3{ skui_settings.padding, skui_settings.padding, skui_settings.padding } *2;
	bounds_t box       = bounds_t{box_start, box_size};
	
	static vec3 start_aff_pos[2] = {};
	static quat start_aff_rot[2] = { quat_identity,quat_identity };
	static vec3 start_palm_pos[2] = {};
	static quat start_palm_rot[2] = { quat_identity,quat_identity };
	for (size_t i = 0; i < handed_max; i++) {
		// Skip this if something else has some focus!
		if (!skui_hand[i].tracked || (skui_hand[i].focused_prev != 0 && skui_hand[i].focused_prev != id))
			continue;

		const hand_t &hand = input_hand((handed_)i);
		vec3 finger_pos_world = vec3_lerp(
			hand.fingers[0][4].position,
			hand.fingers[1][4].position, 0.3f);
		vec3 finger_pos = matrix_mul_point( to_local, finger_pos_world );

		vec3 from_pt = finger_pos;
		if (ui_in_box(skui_hand[i].finger, skui_hand[i].finger_prev, box)) {
			skui_hand[i].focused = id;
		} else {
			vec3  head_pos = hierarchy_to_local_point(input_head_pose.position);
			vec3  ray_to   = hierarchy_to_local_point(finger_pos_world);
			ray_t far_ray  = { head_pos, ray_to - head_pos };
			vec3  at;
			if (bounds_ray_intersect(box, far_ray, &at) && vec3_magnitude_sq(at - head_pos) > 0.7f * 0.7f) {
				skui_hand[i].focused = id;
				from_pt = matrix_mul_point(to_local, hierarchy_to_world_point( vec3_zero ));
				line_add(ray_to, vec3_zero, { 50,50,50,255 }, { 255,255,255,255 }, 0.002f);
			}
		}

		if (skui_hand[i].focused == id || skui_hand[i].active_prev == id) {
			
			const hand_t &hand = input_hand((handed_)i);
			if (hand.pinch_state & button_state_just_active) {
				sound_play(skui_snd_grab, skui_hand[i].finger_world, 1);

				skui_hand[i].active = id;
				start_aff_pos[i] = movement.position;
				start_aff_rot[i] = movement.orientation;
				start_palm_pos[i] = from_pt;
				start_palm_rot[i] = matrix_mul_rotation( to_local, hand.palm.orientation);
			}
			if (skui_hand[i].active_prev == id || skui_hand[i].active == id) {
				color = 1.5f;
				result = true;
				skui_hand[i].active = id;

				quat dest_rot;
				vec3 dest_pos;
				
				switch (move_type) {
				case ui_move_exact: {
					dest_rot = matrix_mul_rotation(to_local, hand.palm.orientation);
					dest_rot = quat_difference(start_palm_rot[i], dest_rot);
				} break;
				case ui_move_face_user: {
					dest_rot = quat_lookat(movement.position, matrix_mul_point(to_local, input_head().position));
					dest_rot = quat_difference(start_aff_rot[i], dest_rot);
				} break;
				case ui_move_pos_only: {
					dest_rot = quat_identity;
				} break;
				default: log_err("Unimplemented move type!"); break;
				}

				vec3 curr_pos = finger_pos;
				dest_pos = curr_pos + dest_rot * (start_aff_pos[i] - start_palm_pos[i]);
				movement.position    = vec3_lerp (movement.position,    dest_pos, 0.6f);
				movement.orientation = quat_slerp(movement.orientation, start_aff_rot[i] * dest_rot, 0.4f); 

				if (hand.pinch_state & button_state_just_inactive) {
					skui_hand[i].active = 0;
					sound_play(skui_snd_ungrab, skui_hand[i].finger_world, 1);
				}
				ui_pop_pose();
				ui_push_pose(movement, vec3{ 0,0,0 });
			}
		}
	}

	if (draw) {
		ui_box(
			handle.center+handle.dimensions/2, 
			handle.dimensions, 
			skui_mat_quad, skui_palette[0] * color);
		ui_nextline();
	}
	return result;
}

///////////////////////////////////////////

bool32_t ui_handle_begin(const char *text, pose_t &movement, bounds_t handle, bool32_t draw, ui_move_ move_type) {
	return _ui_handle_begin(ui_stack_hash(text), movement, handle, draw, move_type);
}

///////////////////////////////////////////

void ui_handle_end() {
	ui_pop_pose();
}

///////////////////////////////////////////

void ui_window_begin(const char *text, pose_t &pose, vec2 window_size, ui_win_ window_type, ui_move_ move_type) {
	uint64_t id = ui_push_id(text);

	int32_t index = sl_indexof(skui_sl_windows, id);
	if (index < 0) {
		pose_t      head_pose  = input_head();
		ui_window_t new_window = {};
		new_window.size = window_size;
		index = sl_insert(skui_sl_windows, index, id, new_window);
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
		vec2 size = text_size(text, skui_font_style);
		vec3 at   = skui_layers.back().offset - vec3{ skui_settings.padding, -ui_line_height(), 2*mm2m };
		ui_text(at, size, text, text_align_x_left | text_align_y_top, text_align_x_left | text_align_y_center);
	}
	window.pose = pose;
}

///////////////////////////////////////////

void ui_window_end() {
	layer_t &layer = skui_layers.back();
	vec3 start = layer.offset_initial + vec3{0,0,skui_settings.depth};
	vec3 end   = { layer.max_x, skui_layers.back().offset.y - skui_layers.back().line_height,  layer.offset_initial.z};
	vec3 size  = start - end;
	size = { fmaxf(size.x+skui_settings.padding, layer.size.x), fmaxf(size.y+skui_settings.padding, layer.size.y), size.z };
	layer.window->size.x = size.x;
	layer.window->size.y = size.y;

	float line_height = ui_line_height();
	if (layer.window->type & ui_win_head) {
		ui_box(start + vec3{0,line_height,0}, { size.x, line_height, size.z }, skui_mat_quad, skui_palette[0]);
	}
	if (layer.window->type & ui_win_body) {
		ui_box(start, size, skui_mat_quad, skui_palette[1]);
	}
	ui_handle_end();
	ui_pop_id();
}

} // namespace sk