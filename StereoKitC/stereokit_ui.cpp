#include "stereokit_ui.h"
#include "_stereokit_ui.h"
#include "math.h"

#include <DirectXMath.h>
using namespace DirectX;
#include <vector>
using namespace std;

///////////////////////////////////////////

namespace sk {

struct layer_t {
	matrix transform;
	matrix inverse;
	vec3   offset;
	vec2   size;
	float  line_height;
	float  max_x;
};

vector<layer_t> skui_layers;
mesh_t          skui_box;
mesh_t          skui_cylinder;
material_t      skui_mat;
font_t          skui_font;
text_style_t    skui_font_style;
material_t      skui_font_mat;
vec3            skui_fingertip[2];
vec3            skui_fingertip_prev[2];
vec3            skui_fingertip_world[2];
vec3            skui_fingertip_world_prev[2];

ui_settings_t skui_settings = {
	10 * mm2m,
	20 * mm2m,
	15 * mm2m,
	.4f,
	0.5f * mm2m,
};
float skui_fontsize = 20*mm2m;

vec3  skui_prev_offset;
float skui_prev_line_height;

uint64_t skui_control_focused[2] = {};
uint64_t skui_control_active [2] = {};
float    skui_control_focused_time[2] = {};
float    skui_control_active_time [2] = {};

const color128 skui_color_base   = { .25f,.25f,.35f, 1 };
const color128 skui_color_border = { 1,1,1,1 };

///////////////////////////////////////////

uint64_t ui_hash(const char *string);

// Layout
void ui_push_pose  (pose_t pose, vec2 size);
void ui_pop_pose   ();
void ui_layout_box (vec2 content_size, vec3 &out_position, vec2 &out_final_size);
void ui_reserve_box(vec2 size);

void ui_nextline    ();
void ui_sameline    ();
void ui_reserve_box (vec2 size);
void ui_space       (float space);

// Interaction
bool32_t      ui_in_box            (vec3 pt1, vec3 pt2, bounds_t box);
int32_t       ui_box_interaction_1h(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state);
button_state_ ui_button_behavior   (vec3 window_relative_pos, vec2 size, const char *text, float &finger_offset, float &active_time);

// Base render types
void ui_box      (vec3 start, vec3 size, material_t material, color128 color);
void ui_text     (vec3 start, const char *text, text_align_ position = text_align_x_left | text_align_y_top);

///////////////////////////////////////////

inline bounds_t ui_size_box(vec3 top_left, vec3 dimensions) {
	return { top_left + vec3{dimensions.x / 2, dimensions.y / -2, dimensions.z / 2}, dimensions };
}

///////////////////////////////////////////

void ui_settings(ui_settings_t settings) {
	if (settings.backplate_border == 0) settings.backplate_border = 0.5f * mm2m;
	if (settings.backplate_depth  == 0) settings.backplate_depth  = 0.4f;
	if (settings.depth            == 0) settings.depth   = 15 * mm2m;
	if (settings.gutter           == 0) settings.gutter  = 20 * mm2m;
	if (settings.padding          == 0) settings.padding = 10 * mm2m;
	skui_settings = settings;
}

///////////////////////////////////////////

bool ui_init() {
	skui_box      = mesh_gen_cube(vec3_one);
	skui_cylinder = mesh_gen_cylinder(1, 1, {0,0,1}, 24);
	skui_mat      = material_find("default/material_ui");

	skui_font_mat   = material_find("default/material_font");
	skui_font       = font_find("default/font");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, color32{255,255,255,255});
	
	return true;
}

///////////////////////////////////////////

void ui_update() {
	skui_fingertip_world_prev[handed_right] = skui_fingertip_world[handed_right];
	skui_fingertip_world_prev[handed_left ] = skui_fingertip_world[handed_left];
	skui_fingertip_world[handed_right] = input_hand(handed_right).fingers[1][4].position;
	skui_fingertip_world[handed_left ] = input_hand(handed_left ).fingers[1][4].position;
}

///////////////////////////////////////////

void ui_shutdown() {
	mesh_release(skui_box);
	mesh_release(skui_cylinder);
	material_release(skui_mat);
	material_release(skui_font_mat);
	font_release(skui_font);
}

///////////////////////////////////////////

// djb2 hash: http://www.cse.yorku.ca/~oz/hash.html
uint64_t ui_hash(const char *string) {
	unsigned long hash = 5381;
	int c;
	while (c = *string++)
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	return hash;
}

///////////////////////////////////////////
//////////////   Layout!   ////////////////
///////////////////////////////////////////

void ui_push_pose(pose_t pose, vec2 size) {
	vec3   right = pose.orientation * vec3_right; // Use the position as the center of the window.
	matrix trs = matrix_trs(pose.position + right*(size.x/2), pose.orientation);

	// In a right-handed coordinate system, a forward (0,0,-1) facing UI would start at 1,1 in the top left
	// and -1,-1 in the bottom right. This feels profoundly wrong to me, so I set the root of all UI windows 
	// to a 180 rotation on the Y axis to switch it to -1,1 -> 1,-1 instead, yet still retain the benefit
	// of having a 'forward facing' UI.
	// TODO: Review this later, see how it turns out over time.
	if (skui_layers.size() == 0)
		trs = matrix_trs(vec3_zero, quat_from_angles(0, 180, 0), vec3_one) * trs;
	else
		trs = skui_layers.back().transform * trs;

	matrix trs_inverse;
	matrix_inverse(trs, trs_inverse);

	skui_layers.push_back(layer_t{
		trs, trs_inverse,
		vec3{skui_settings.padding, -skui_settings.padding}, size, 0, 0
		});

	for (size_t i = 0; i < handed_max; i++) {
		const hand_t &hand = input_hand((handed_)i);
		skui_fingertip     [i] = matrix_mul_point(skui_layers.back().inverse, skui_fingertip_world[i]);
		skui_fingertip_prev[i] = matrix_mul_point(skui_layers.back().inverse, skui_fingertip_world_prev[i]);
	}
}

///////////////////////////////////////////

void ui_pop_pose() {
	skui_layers.pop_back();
}

///////////////////////////////////////////

void ui_layout_box(vec2 content_size, vec3 &out_position, vec2 &out_final_size) {
	out_position   = skui_layers.back().offset;
	out_final_size = content_size;
	out_final_size += vec2{ skui_settings.padding, skui_settings.padding }*2;

	// If this is not the first element, and it goes outside the active window
	if (out_position.x            != skui_settings.padding &&
		skui_layers.back().size.x != 0                     && 
		out_position.x + out_final_size.x > skui_layers.back().size.x - skui_settings.padding)
	{
		ui_nextline();
		out_position = skui_layers.back().offset;
	}
}

///////////////////////////////////////////

void ui_reserve_box(vec2 size) {
	if (skui_layers.back().max_x < skui_layers.back().offset.x + size.x + skui_settings.padding)
		skui_layers.back().max_x = skui_layers.back().offset.x + size.x + skui_settings.padding;
	if (skui_layers.back().line_height < size.y)
		skui_layers.back().line_height = size.y;

	skui_layers.back().offset += vec3{ size.x + skui_settings.gutter, 0, 0 };
}

///////////////////////////////////////////

void ui_nextline() {
	layer_t &layer = skui_layers.back();
	skui_prev_offset      = layer.offset;
	skui_prev_line_height = layer.line_height;

	layer.offset.x    = skui_settings.padding;
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

void ui_space(float space) {
	if (skui_layers.back().offset.x == skui_settings.padding)
		skui_layers.back().offset.y -= space;
	else
		skui_layers.back().offset.x += space;
}

///////////////////////////////////////////
///////////   Interaction!   //////////////
///////////////////////////////////////////

int32_t ui_box_interaction_1h(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size, button_state_ *out_focus_state) {
	int32_t hand = -1;

	for (int32_t i = 0; i < handed_max; i++) {
		button_state_ focus_state = button_state_up;
		bool focused   = skui_control_focused[i] == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (focused) {
			focus_state = button_state_down;
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		if (ui_in_box(skui_fingertip[i], skui_fingertip_prev[i], ui_size_box(box_start, box_size))) {
			if (!focused) {
				skui_control_focused[i] = id;
				skui_control_focused_time[i] = time_getf();
				focus_state = button_state_down | button_state_just_down;
			} else {
				focus_state = button_state_down;
			}
			hand = i;
		} else {
			if (focused) {
				skui_control_focused[i] = 0;
				focus_state = button_state_up | button_state_just_up;
			}
		}

		if (hand == i && out_focus_state != nullptr)
			*out_focus_state = focus_state;
	}
	return hand;
}

///////////////////////////////////////////

bool32_t ui_in_box(vec3 pt, vec3 pt_prev, bounds_t box) {
	//bounds_t box = { box_start + box_size / 2, box_size };
	//render_add_mesh(skui_box, skui_mat, matrix_trs(box.center, quat_identity, box.dimensions) * skui_layers.back().transform);
	line_add(
		matrix_mul_point(skui_layers.back().transform, pt),
		matrix_mul_point(skui_layers.back().transform, pt_prev),
		{ 0,255,0,255 }, 0.01f);
	//return bounds_point_contains(box, pt);
	return bounds_line_contains(box, pt, pt_prev);
	/*pt -= box_start;
	return
		pt.x >= 0 && pt.x <=  box_size.x &&
		pt.y <= 0 && pt.y >= -box_size.y &&
		pt.z >= 0 && pt.z <=  box_size.z;*/
}

///////////////////////////////////////////

button_state_ ui_button_behavior(vec3 window_relative_pos, vec2 size, const char *text, float &finger_offset, float &active_time) {
	uint64_t      id     = ui_hash(text);
	button_state_ result = button_state_up;

	// Button interaction focus is detected in the front half of the button to prevent 'reverse'
	// or 'side' presses where the finger comes from the back or side.
	//
	// Once focused is gained, interaction is tracked within a volume that extends from the 
	// front, to a good distance through the button's back. This is to help when the user's
	// finger inevitably goes completely through the button. May consider expanding the volume 
	// a bit too on the X/Y axes later.
	vec3    box_start = window_relative_pos + vec3{ 0, 0, skui_settings.depth/2.f };
	vec3    box_size  = vec3{ size.x, size.y, skui_settings.depth/2.f };
	button_state_ focus;
	int32_t hand = ui_box_interaction_1h(id,
		box_start, box_size,
		box_start + vec3{ 0,0,-skui_settings.depth * 4 },
		box_size  + vec3{ 0,0, skui_settings.depth * 4 },
		&focus);

	// If a hand is interacting, adjust the button surface accordingly
	finger_offset = skui_settings.depth;
	active_time   = 0;
	if (hand != -1) {
		finger_offset = skui_fingertip[hand].z - window_relative_pos.z;
		if (finger_offset < skui_settings.depth / 2) {
			result = button_state_down;
			if (skui_control_active[hand] != id) {
				skui_control_active[hand] = id;
				skui_control_active_time[hand] = time_getf();
				result |= button_state_just_down;
			}
		} else if (skui_control_active[hand] == id) {
			skui_control_active[hand] = 0;
			result |= button_state_just_up;
		}
		finger_offset = fmaxf(skui_settings.backplate_depth*skui_settings.depth + mm2m, finger_offset);
		active_time   = time_getf() - skui_control_active_time[hand];
	} else if (focus & button_state_just_up && skui_control_active[hand] == id) {
		skui_control_active[hand] = 0;
		result |= button_state_just_up;
	}

	return result;
}

///////////////////////////////////////////
////////   Base Visual Elements   /////////
///////////////////////////////////////////

void ui_box(vec3 start, vec3 size, material_t material, color128 color) {
	vec3   pos = start + (vec3{ size.x, -size.y, size.z } / 2);
	matrix mx  = matrix_trs(pos, quat_identity, size);
	matrix_mul(mx, skui_layers.back().transform, mx);

	render_add_mesh(skui_box, material, mx, color);
}

///////////////////////////////////////////

void ui_cylinder(vec3 start, float radius, float depth, material_t material, color128 color) {
	vec3   pos = start + (vec3{ radius, -radius, depth } / 2);
	matrix mx  = matrix_trs(pos, quat_identity, {radius, radius, depth});
	matrix_mul(mx, skui_layers.back().transform, mx);

	render_add_mesh(skui_cylinder, material, mx, color);
}

///////////////////////////////////////////

void ui_model_at(model_t model, vec3 start, vec3 size, color128 color) {
	matrix mx = matrix_trs(start, quat_identity, size);
	matrix_mul(mx, skui_layers.back().transform, mx);
	render_add_model(model, mx, color);
}

///////////////////////////////////////////

void ui_text(vec3 start, const char *text, text_align_ position) {
	text_add_at(text, skui_layers.back().transform, skui_font_style, position, text_align_x_left | text_align_y_top, start.x, start.y, start.z);
}

///////////////////////////////////////////
///////////   UI Components   /////////////
///////////////////////////////////////////

void ui_label(const char *text) {
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(text, skui_font_style);
	ui_reserve_box(size);
	ui_text(offset + vec3{ 0, 0, 2*mm2m }, text);
	ui_nextline();
}

///////////////////////////////////////////

void ui_image(sprite_t image, vec2 size) {
	float aspect     = sprite_get_aspect(image);
	vec3  offset     = skui_layers.back().offset;
	vec2  final_size = vec2{
		size.x==0 ? size.y/aspect : size.x, 
		size.y==0 ? size.x*aspect : size.y };

	ui_reserve_box(final_size);
	
	matrix result;
	matrix_mul(matrix_trs(offset, quat_identity, vec3{ final_size.x, final_size.y, 1 }), skui_layers.back().transform, result);
	sprite_draw(image, result);
	ui_nextline();
}

///////////////////////////////////////////

bool32_t ui_button_at(vec3 window_relative_pos, vec2 size, const char *text) {
	float         finger_offset;
	float         active_time;
	button_state_ state = ui_button_behavior(window_relative_pos, size, text, finger_offset, active_time);

	float back_size   = skui_settings.backplate_border;
	float color_blend = 1;
	if (state & button_state_down) {
		float t = fminf(1, active_time * 12);
		back_size   = math_ease_hop(back_size, fmaxf(mm2m*2, back_size*2.f), t);
		color_blend = math_ease_overshoot(1, 1.5f, 10, t);
	}

	ui_box (window_relative_pos,  vec3{ size.x,    size.y,   finger_offset }, skui_mat, skui_color_base * color_blend);
	ui_box (window_relative_pos + vec3{-back_size, back_size, -mm2m}, vec3{ size.x+back_size*2, size.y+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m }, skui_mat, skui_color_border * color_blend);
	ui_text(window_relative_pos + vec3{ size.x/2, -size.y/2, finger_offset + 2*mm2m }, text, text_align_center);

	return state & button_state_just_down;
}

///////////////////////////////////////////

bool32_t ui_button(const char *text) {
	vec3 offset;
	vec2 size;
	ui_layout_box (text_size(text, skui_font_style), offset, size);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_button_at(offset, size, text);
}

///////////////////////////////////////////

bool32_t ui_toggle_at(vec3 window_relative_pos, vec2 size, const char *text, bool32_t &pressed) {
	float         finger_offset;
	float         active_time;
	button_state_ state = ui_button_behavior(window_relative_pos, size, text, finger_offset, active_time);

	if (state & button_state_just_down) {
		pressed = !pressed;
	}
	finger_offset = pressed ? fminf(skui_settings.backplate_depth*skui_settings.depth + mm2m, finger_offset) : finger_offset;

	float back_size   = skui_settings.backplate_border;
	float color_blend = pressed ? 1.5f : 1;
	if (state & button_state_down) {
		float t = fminf(1, active_time * 12);
		back_size   = math_ease_hop(back_size, fmaxf(mm2m*2, back_size*2.f), t);
		color_blend = math_ease_overshoot(1, 1.5f, 10, t);
	}

	ui_box (window_relative_pos,  vec3{ size.x,    size.y,   finger_offset }, skui_mat, skui_color_base * color_blend);
	ui_box (window_relative_pos + vec3{-back_size, back_size, -mm2m}, vec3{ size.x+back_size*2, size.y+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m }, skui_mat, skui_color_border * color_blend);
	ui_text(window_relative_pos + vec3{ size.x/2, -size.y/2, finger_offset + 2*mm2m }, text, text_align_center);

	return state & button_state_changed;
}

///////////////////////////////////////////

bool32_t ui_toggle(const char *text, bool32_t &pressed) {
	vec3 offset;
	vec2 size;
	ui_layout_box (text_size(text, skui_font_style), offset, size);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_toggle_at(offset, size, text, pressed);
}

///////////////////////////////////////////

bool32_t ui_button_round_at(vec3 window_relative_pos, float diameter, const char *text) {
	float         finger_offset;
	float         active_time;
	button_state_ state = ui_button_behavior(window_relative_pos, {diameter,diameter}, text, finger_offset, active_time);

	float back_size   = skui_settings.backplate_border;
	float color_blend = 1;
	if (state & button_state_down) {
		float t = fminf(1, active_time * 12);
		back_size   = math_ease_hop(back_size, fmaxf(mm2m*2, back_size*2.f), t);
		color_blend = math_ease_overshoot(1, 1.5f, 10, t);
	}

	ui_cylinder(window_relative_pos, diameter, finger_offset, skui_mat, skui_color_base * color_blend);
	ui_cylinder(window_relative_pos + vec3{-back_size, back_size, -mm2m}, diameter+back_size*2, skui_settings.backplate_depth*skui_settings.depth+mm2m, skui_mat, skui_color_border * color_blend);
	ui_text    (window_relative_pos + vec3{ diameter/2, -diameter/2, finger_offset + 2*mm2m }, text, text_align_center);

	return state & button_state_just_down;
}

///////////////////////////////////////////

bool32_t ui_button_round(const char *text, float diameter) {
	vec3 offset;
	vec2 size = diameter == 0 ? text_size(text, skui_font_style) : vec2{diameter, diameter};
	size = vec2_one * fmaxf(size.x, size.y);
	ui_layout_box (size, offset, size);
	ui_reserve_box(size);
	ui_nextline   ();

	return ui_button_round_at(offset, size.x, text);
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

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size) {
	uint64_t id_hash= ui_hash(id);
	bool     result = false;
	bool     focused = false;
	vec3     offset = skui_layers.back().offset;
	vec2     size   = text_size(buffer, skui_font_style);
	size += vec2{ skui_settings.padding, skui_settings.padding } * 2;
	vec3 box_size = vec3{ size.x, size.y, skui_settings.depth/2 };

	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_fingertip[i], skui_fingertip_prev[i], ui_size_box(offset, box_size))) {
			skui_control_focused[i] = id_hash;
		}
		if (skui_control_focused[i] == id_hash)
			focused = true;
	}
	if (focused) {
		char add = '\0';
		bool shift = input_key(key_shift) & button_state_down;
		if (input_key(key_backspace) & button_state_just_down) {
			size_t len = strlen(buffer);
			if (len >= 0) {
				buffer[len - 1] = '\0';
				result = true;
			}
		}
		if (input_key(key_space) & button_state_just_down) add = ' ';
		for (int32_t k = 0; k < 26; k++) {
			if (input_key((key_)(key_a + k)) & button_state_just_down) {
				add = (char)((shift ? 'A' : 'a') + k);
			}
		}
		for (int32_t k = 0; k < 10; k++) {
			if (input_key((key_)(key_0 + k)) & button_state_just_down) {
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
	ui_box (offset, vec3{ size.x, size.y, skui_settings.depth/2 }, skui_mat, skui_color_base * (focused ? 0.5f : 1.f) );
	ui_text(offset + vec3{ size.x/2, -size.y/2, skui_settings.depth/2 + 2*mm2m }, buffer, text_align_center);
	ui_nextline();

	return result;
}

///////////////////////////////////////////

bool32_t ui_affordance(const char *text, pose_t &movement, vec3 at, vec3 size, bool32_t draw) {
	uint64_t id = ui_hash(text);
	bool result = false;
	float color = 1;

	vec3     box_start = at   - vec3{ skui_settings.padding, -skui_settings.padding, skui_settings.padding * 3 };
	vec3     box_size  = size + vec3{ skui_settings.padding,  skui_settings.padding, skui_settings.padding * 3 } *2;
	bounds_t box       = ui_size_box(box_start, box_size);
	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_fingertip[i], skui_fingertip_prev[i], box)) {
			skui_control_focused[i] = id;
			color = 0.75f;
		} else if (skui_control_focused[i] == id) {
			skui_control_focused[i] = 0;
		}

		if (skui_control_focused[i] == id || skui_control_active[i] == id) {
			static vec3 start_aff_pos = vec3_zero;
			static quat start_aff_rot = quat_identity;
			static vec3 start_tip_pos = vec3_zero;
			static quat start_tip_rot = quat_identity;

			const hand_t &hand = input_hand((handed_)i);
			if (hand.state & input_state_justpinch) {
				skui_control_active[i] = id;
				start_aff_pos = movement.position;
				start_aff_rot = movement.orientation;
				start_tip_pos = input_hand((handed_)i).root.position;
				start_tip_rot = input_hand((handed_)i).root.orientation;
			}
			if (skui_control_active[i] == id) {
				color = 0.5f;
				result = true;

				quat rot = quat_difference(start_tip_rot, input_hand((handed_)i).root.orientation);

				movement.position    = input_hand((handed_)i).root.position + rot*(start_aff_pos - start_tip_pos);
				movement.orientation = start_aff_rot*rot;
				if (hand.state & input_state_unpinch) {
					skui_control_active[i] = 0;
				}
			}
		}
	}

	if (draw) {
		ui_box(at, size, skui_mat, skui_color_base * color);
		ui_nextline();
	}

	return result;
}

///////////////////////////////////////////

bool32_t ui_hslider(const char *name, float &value, float min, float max, float step, float width) {
	uint64_t   id     = ui_hash(name);
	bool       result = false;
	float      color  = 1;
	vec3       offset = skui_layers.back().offset;

	// Find sizes of slider elements
	if (width == 0)
		width = skui_layers.back().size.x == 0 ? 0.1f : (skui_layers.back().size.x - skui_settings.padding) - skui_layers.back().offset.x;
	vec2 size = { width, skui_fontsize };
	float rule_size = size.y / 6.f;

	// Interaction code
	vec3     box_start = offset + vec3{ 0, 0, -skui_settings.depth };
	vec3     box_size  = vec3{ size.x, size.y, skui_settings.depth*2 };
	bounds_t box       = ui_size_box(box_start, box_size);
	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_fingertip[i], skui_fingertip_prev[i], box)) {
			skui_control_focused[i] = id;
			color = 1.5f;
			float new_val = min + ((skui_fingertip[i].x - offset.x) / size.x) * (max - min);
			if (step != 0) {
				new_val = ((int)(((new_val - min) / step)+0.5f)) * step;
			}
			result = value != new_val;
			value  = new_val;

			if (result)
				skui_control_active[i] = id;
		}
	}

	// Draw the UI
	ui_reserve_box(size);
	float back_size = skui_settings.backplate_border;
	ui_box(vec3{ offset.x, offset.y - size.y / 2.f + rule_size / 2.f, offset.z }, vec3{ size.x, rule_size, rule_size }, skui_mat, skui_color_base * color);
	ui_box(vec3{ offset.x-back_size, offset.y - size.y / 2.f + rule_size / 2.f + back_size, offset.z-mm2m }, vec3{ size.x+back_size*2, rule_size+back_size*2, rule_size*skui_settings.backplate_depth+mm2m }, skui_mat, skui_color_border * color);
	ui_box(vec3{ offset.x + ((value-min)/(max-min))*size.x - rule_size/2.f, offset.y, offset.z}, vec3{rule_size, size.y, skui_settings.depth}, skui_mat, skui_color_base * color);
	ui_nextline();
	
	return result;
}

///////////////////////////////////////////

void ui_window_begin(const char *text, pose_t &pose, vec2 window_size, bool32_t show_header) {
	if (window_size.x == 0) window_size.x = 32*cm2m;

	ui_push_pose(pose, window_size);

	if (show_header) {
		vec3 offset = skui_layers.back().offset;
		vec2 size   = text_size(text, skui_font_style);
		vec3 box_start = vec3{ 0, 0, -skui_settings.depth };
		vec3 box_size  = vec3{ window_size.x, size.y+ skui_settings.padding*2, skui_settings.depth };

		ui_reserve_box(size);
		ui_text(box_start + vec3{skui_settings.padding,-skui_settings.padding, skui_settings.depth + 2*mm2m}, text);
		ui_affordance(text, pose, box_start, box_size, true);

		ui_nextline();
	}
}

///////////////////////////////////////////

void ui_window_end() {
	ui_pop_pose();
}

} // namespace sk