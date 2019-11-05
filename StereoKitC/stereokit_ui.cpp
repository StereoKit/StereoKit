#include "stereokit_ui.h"
#include "_stereokit_ui.h"

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
material_t      skui_mat;
font_t          skui_font;
text_style_t    skui_font_style;
material_t      skui_font_mat;
vec3            skui_fingertip[2];
vec3            skui_fingergrip[2];

float skui_padding = 10*mm2m;
float skui_gutter  = 20*mm2m;
float skui_depth   = 20*mm2m;
float skui_fontsize= 20*mm2m;

vec3  skui_prev_offset;
float skui_prev_line_height;

uint64_t skui_control_focused[2] = {};
uint64_t skui_control_active [2] = {};

///////////////////////////////////////////

void ui_push_pose(pose_t pose, vec2 size);
void ui_pop_pose ();

bool32_t ui_in_box             (vec3 pt, vec3 box_start, vec3 box_size);
int32_t  ui_box_interaction_1h(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size);

void ui_box      (vec3 start, vec3 size, material_t material, color128 color);
void ui_text     (vec3 start, const char *text, text_align_ position = text_align_x_left | text_align_y_top);

///////////////////////////////////////////

// djb2 hash: http://www.cse.yorku.ca/~oz/hash.html
uint64_t sk_ui_hash(const char *string) {
	unsigned long hash = 5381;
	int c;
	while (c = *string++)
		hash = ((hash << 5) + hash) + c; // hash * 33 + c
	return hash;
}

///////////////////////////////////////////

int32_t ui_box_interaction_1h(uint64_t id, vec3 box_unfocused_start, vec3 box_unfocused_size, vec3 box_focused_start, vec3 box_focused_size) {
	int32_t hand = -1;

	for (int32_t i = 0; i < handed_max; i++) {
		bool focused   = skui_control_focused[i] == id;
		vec3 box_start = box_unfocused_start;
		vec3 box_size  = box_unfocused_size;
		if (focused) {
			box_start = box_focused_start;
			box_size  = box_focused_size;
		}

		if (ui_in_box(skui_fingertip[i], box_start, box_size)) {
			skui_control_focused[i] = id;
			hand = i;
		} else if (focused) {
			skui_control_focused[i] = 0;
		}
	}
	return hand;
}

///////////////////////////////////////////

bool32_t ui_in_box(vec3 pt, vec3 box_start, vec3 box_size) {
	pt -= box_start;
	return
		pt.x >= 0 && pt.x <=  box_size.x &&
		pt.y <= 0 && pt.y >= -box_size.y &&
		pt.z >= 0 && pt.z <=  box_size.z;
}

///////////////////////////////////////////

bool ui_init() {
	skui_box = mesh_gen_cube(vec3_one);
	skui_mat = material_copy_id("default/material");

	skui_font_mat   = material_find("default/material_font");
	skui_font       = font_find("default/font");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, color32{255,255,255,255});

	return true;
}

///////////////////////////////////////////

void ui_push_pose(pose_t pose, vec2 size) {
	vec3   right = pose.orientation * vec3_right; // Use the position as the center of the window.
	matrix trs = matrix_trs(pose.position - right*(size.x/2), pose.orientation);

	// In a right-handed coordinate system, a forward (0,0,-1) facing UI would start at 1,1 in the top left
	// and -1,-1 in the bottom right. This feels profoundly wrong to me, so I set the root of all UI windows 
	// to a 180 rotation on the Y axis to switch it to -1,1 -> 1,-1 instead, yet still retain the benefit
	// of having a 'forward facing' UI.
	// TODO: Review this later, see how it turns out over time.
	if (skui_layers.size() == 0)
		trs = matrix_trs(vec3_zero, quat_euler({ 0, 180, 0 }), vec3_one) * trs;
	else
		trs = skui_layers.back().transform * trs;

	matrix trs_inverse;
	matrix_inverse(trs, trs_inverse);

	skui_layers.push_back(layer_t{
		trs, trs_inverse,
		vec3{skui_padding, -skui_padding}, size, 0, 0
	});

	for (size_t i = 0; i < handed_max; i++) {
		const hand_t &hand = input_hand((handed_)i);
		skui_fingertip[i] = hand.fingers[1][4].position;
		skui_fingertip[i] = matrix_mul_point(skui_layers.back().inverse, skui_fingertip[i]);

		skui_fingergrip[i] = (hand.fingers[1][4].position + hand.fingers[0][4].position)/2.f;
		skui_fingergrip[i] = matrix_mul_point(skui_layers.back().inverse, skui_fingergrip[i]);
	}
}

///////////////////////////////////////////

void ui_pop_pose() {
	skui_layers.pop_back();
}

///////////////////////////////////////////

void ui_box(vec3 start, vec3 size, material_t material, color128 color) {
	vec3 pos = start + (vec3{ size.x, -size.y, size.z } / 2);
	matrix mx = matrix_trs(pos, quat_identity, size);
	matrix_mul(mx, skui_layers.back().transform, mx);

	render_add_mesh(skui_box, material, mx, color);
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

void ui_reserve_box(vec2 size) {
	if (skui_layers.back().max_x < skui_layers.back().offset.x + size.x + skui_padding)
		skui_layers.back().max_x = skui_layers.back().offset.x + size.x + skui_padding;
	if (skui_layers.back().line_height < size.y)
		skui_layers.back().line_height = size.y;

	skui_layers.back().offset += vec3{ size.x + skui_gutter, 0, 0 };
}

///////////////////////////////////////////

void ui_nextline() {
	layer_t &layer = skui_layers.back();
	skui_prev_offset      = layer.offset;
	skui_prev_line_height = layer.line_height;

	layer.offset.x    = skui_padding;
	layer.offset.y   -= skui_layers.back().line_height + skui_gutter;
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
	if (skui_layers.back().offset.x == skui_padding)
		skui_layers.back().offset.y -= space;
	else
		skui_layers.back().offset.x += space;
}

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

bool32_t ui_button(const char *text) {
	uint64_t id = sk_ui_hash(text);
	bool result = false;
	color128 color = color128{ 1,1,1,1 };
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(text, skui_font_style);
	size += vec2{ skui_padding, skui_padding }*2;

	// If this is not the first element, and it goes outside the active window
	if (offset.x                  != skui_padding && 
		skui_layers.back().size.x != 0            && 
		offset.x + size.x > skui_layers.back().size.x - skui_padding)
	{
		ui_nextline();
		offset = skui_layers.back().offset;
	}

	// Button interaction focus is detected in the front half of the button to prevent 'reverse'
	// or 'side' presses where the finger comes from the back or side.
	//
	// Once focused is gained, interaction is tracked within a volume that extends from the 
	// front, to a good distance through the button's back. This is to help when the user's
	// finger inevitably goes completely through the button. May consider expanding the volume 
	// a bit too on the X/Y axes later.
	vec3    box_start = offset + vec3{ 0, 0, skui_depth/2.f };
	vec3    box_size  = vec3{ size.x, size.y, skui_depth/2.f };
	int32_t hand = ui_box_interaction_1h(id,
		box_start, box_size,
		box_start + vec3{ 0,0,-skui_depth * 4 },
		box_size  + vec3{ 0,0, skui_depth * 4 });

	// If a hand is interacting, adjust the button surface accordingly
	float finger_offset = skui_depth;
	if (hand != -1) {
		finger_offset = fmaxf(mm2m, skui_fingertip[hand].z - offset.z);
		if (finger_offset < skui_depth / 2) {
			if (skui_control_active[hand] != id) {
				skui_control_active[hand] = id;
				result = true;
			}
			color = color128{ 0.5f, 0.5f, 0.5f, 1 };
		}
	}

	ui_reserve_box(size);
	ui_box (offset, vec3{ size.x, size.y, finger_offset }, skui_mat, color);
	ui_text(offset + vec3{ size.x/2, -size.y/2, finger_offset + 2*mm2m }, text, text_align_center);
	ui_nextline();

	return result;
}

///////////////////////////////////////////

void ui_model(model_t model, vec2 ui_size, float model_scale) {
	vec3 offset = skui_layers.back().offset;
	vec2 size   = ui_size + vec2{ skui_padding, skui_padding }*2;

	ui_reserve_box(size);
	size = size / 2;
	ui_model_at(model, { offset.x + size.x, offset.y - size.y, offset.z }, vec3_one * model_scale, { 1,1,1,1 });
	ui_nextline();
}

///////////////////////////////////////////

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size) {
	uint64_t id_hash= sk_ui_hash(id);
	bool     result = false;
	bool     focused = false;
	vec3     offset = skui_layers.back().offset;
	vec2     size   = text_size(buffer, skui_font_style);
	size += vec2{ skui_padding, skui_padding } * 2;

	vec3 box_start = offset;
	vec3 box_size  = vec3{ size.x, size.y, skui_depth/2 };
	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_fingertip[i], box_start, box_size)) {
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
	ui_box (offset, vec3{ size.x, size.y, skui_depth/2 }, skui_mat, focused ? color128{0.5f, 0.5f, 0.5f, 1} : color128{1,1,1,1});
	ui_text(offset + vec3{ size.x/2, -size.y/2, skui_depth/2 + 2*mm2m }, buffer, text_align_center);
	ui_nextline();

	return result;
}

///////////////////////////////////////////

bool32_t ui_affordance(const char *text, pose_t &movement, vec3 at, vec3 size, bool32_t draw) {
	uint64_t id = sk_ui_hash(text);
	bool result = false;
	color128 color = { 1,1,1,1 };

	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_fingergrip[i], at, size)) {
			skui_control_focused[i] = id;
			color = { 0.75f,0.75f,0.75f,1 };
		} else if (skui_control_focused[i] == id) {
			skui_control_focused[i] = 0;
		}

		if (skui_control_focused[i] == id || skui_control_active[i] == id) {
			static vec3 start_aff_pos = vec3_zero;
			static quat start_aff_rot = quat_identity;
			static vec3 start_tip_pos = vec3_zero;
			static quat start_tip_rot = quat_identity;
			vec3 grip_pos = matrix_mul_point(skui_layers.back().transform, skui_fingergrip[i]);

			const hand_t &hand = input_hand((handed_)i);
			if (hand.state & input_state_justpinch) {
				skui_control_active[i] = id;
				start_aff_pos = movement.position;
				start_aff_rot = movement.orientation;
				start_tip_pos = grip_pos;
				start_tip_rot = input_hand((handed_)i).root.orientation;
			}
			if (skui_control_active[i] == id) {
				color = { 0.5f,0.5f,0.5f,1 };
				result = true;
				movement.position    = start_aff_pos + (grip_pos - start_tip_pos);
				//movement.orientation = quat_mul(start_aff_rot, quat_difference(start_tip_rot, input_hand((handed_)i).root.orientation));
				if (hand.state & input_state_unpinch) {
					skui_control_active[i] = 0;
				}
			}
		}
	}

	if (draw) {
		ui_box(at, size, skui_mat, color);
		ui_nextline();
	}

	return result;
}

///////////////////////////////////////////

bool32_t ui_hslider(const char *name, float &value, float min, float max, float step, float width) {
	uint64_t   id     = sk_ui_hash(name);
	bool       result = false;
	color128   color  = {1,1,1,1};
	vec3       offset = skui_layers.back().offset;

	// Find sizes of slider elements
	if (width == 0)
		width = skui_layers.back().size.x == 0 ? 0.1f : (skui_layers.back().size.x - skui_padding) - skui_layers.back().offset.x;
	vec2 size = { width, skui_fontsize };
	float rule_size = size.y / 6.f;

	// Interaction code
	vec3 box_start = offset + vec3{ 0, 0, -skui_depth };
	vec3 box_size  = vec3{ size.x, size.y, skui_depth*2 };
	for (size_t i = 0; i < handed_max; i++) {
		if (ui_in_box(skui_fingertip[i], box_start, box_size)) {
			skui_control_focused[i] = id;
			color = { 0.5f,0.5f,0.5f,1 };
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
	ui_box(vec3{ offset.x, offset.y - size.y / 2.f + rule_size / 2.f, offset.z }, vec3{ size.x, rule_size, rule_size }, skui_mat, color);
	ui_box(vec3{ offset.x + ((value-min)/(max-min))*size.x - rule_size/2.f, offset.y, offset.z}, vec3{rule_size, size.y, skui_depth}, skui_mat, color);
	ui_nextline();
	
	return result;
}

///////////////////////////////////////////

void ui_window_begin(const char *text, pose_t &pose, vec2 window_size) {
	uint64_t id = sk_ui_hash(text);

	if (window_size.x == 0) window_size.x = 1;

	ui_push_pose(pose, window_size);

	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(text, skui_font_style);
	vec3 box_start = vec3{ 0, 0, -skui_depth };
	vec3 box_size  = vec3{ window_size.x, size.y+skui_padding*2, skui_depth };

	ui_reserve_box(size);
	ui_text(box_start + vec3{skui_padding,-skui_padding, skui_depth + 2*mm2m}, text);
	ui_affordance(text, pose, box_start, box_size, true);

	ui_nextline();
}

///////////////////////////////////////////

void ui_window_end() {
	ui_pop_pose();
}

} // namespace sk