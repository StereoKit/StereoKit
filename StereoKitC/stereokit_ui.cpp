#include "stereokit_ui.h"

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
material_t      skui_mat_active;
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

void ui_init() {
	skui_box = mesh_gen_cube(vec3_one);

	material_t mat_default = material_find("default/material");
	skui_mat        = material_copy(mat_default);
	skui_mat_active = material_copy(mat_default);
	material_release(mat_default);
	material_set_color(skui_mat_active, "color", { .5f,.5f,.5f,1 });

	skui_font_mat   = material_create(shader_find("default/shader_font"));
	skui_font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, text_align_x_left | text_align_y_top);
}

///////////////////////////////////////////

void ui_push_pose(pose_t pose, vec2 size) {
	matrix trs = matrix_trs(pose.position, pose.orientation);
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

void ui_text(vec3 start, const char *text, text_align_ position) {
	text_add_at(skui_font_style, skui_layers.back().transform, text, position, start.x, start.y, start.z);
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

bool32_t sk_ui_inbox(vec3 pt, vec3 box_start, vec3 box_size) {
	pt -= box_start;
	return
		pt.x >= 0 && pt.x <=  box_size.x &&
		pt.y <= 0 && pt.y >= -box_size.y &&
		pt.z >= 0 && pt.z <=  box_size.z;
}

///////////////////////////////////////////

void ui_label(const char *text) {
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(skui_font_style, text);
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
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(skui_font_style, text);
	size += vec2{ skui_padding, skui_padding }*2;

	if (offset.x                  != skui_padding && 
		skui_layers.back().size.x != 0            && 
		offset.x + size.x > skui_layers.back().size.x - skui_padding)
	{
		ui_nextline();
		offset = skui_layers.back().offset;
	}

	vec3 box_start = offset + vec3{ 0, 0, -skui_depth };
	vec3 box_size  = vec3{ size.x, size.y, skui_depth*2 };
	float finger_offset = skui_depth;
	for (size_t i = 0; i < handed_max; i++) {
		if (sk_ui_inbox(skui_fingertip[i], box_start, box_size)) {
			skui_control_focused[i] = id;
			finger_offset = fmaxf(mm2m,skui_fingertip[i].z-offset.z);
		}
		if (finger_offset < skui_depth / 2) {
			skui_control_active[i] = id;
			result = true;
		}
	}

	ui_reserve_box(size);
	ui_box (offset, vec3{ size.x, size.y, finger_offset }, skui_mat, result ? color128{0.5f, 0.5f, 0.5f, 1} : color128{1,1,1,1});
	ui_text(offset + vec3{ size.x/2, -size.y/2, finger_offset + 2*mm2m }, text, text_align_x_center | text_align_y_center);
	ui_nextline();

	return result;
}

///////////////////////////////////////////

bool32_t ui_input(const char *id, char *buffer, int32_t buffer_size) {
	uint64_t id_hash= sk_ui_hash(id);
	bool     result = false;
	bool     focused = false;
	vec3     offset = skui_layers.back().offset;
	vec2     size   = text_size(skui_font_style, buffer);
	size += vec2{ skui_padding, skui_padding } * 2;

	vec3 box_start = offset;
	vec3 box_size  = vec3{ size.x, size.y, skui_depth/2 };
	for (size_t i = 0; i < handed_max; i++) {
		if (sk_ui_inbox(skui_fingertip[i], box_start, box_size)) {
			skui_control_focused[i] = id_hash;
		}
		if (skui_control_focused[i] == id_hash)
			focused = true;
	}
	if (focused) {
		char add = '\0';
		bool shift = input_key(key_shift) & button_state_down;
		if (input_key(key_backspace) & button_state_just_down) {
			int len = strlen(buffer);
			if (len >= 0) {
				buffer[len - 1] = '\0';
				result = true;
			}
		}
		if (input_key(key_space) & button_state_just_down) add = ' ';
		for (int32_t k = 0; k < 26; k++) {
			if (input_key((key_)(key_a + k)) & button_state_just_down) {
				add = (shift ? 'A' : 'a') + k;
			}
		}
		for (int32_t k = 0; k < 10; k++) {
			if (input_key((key_)(key_0 + k)) & button_state_just_down) {
				const char *nums = ")!@#$%^&*(";
				add = (shift ? nums[k] : '0'+k);
			}
		}
		if (add != '\0') {
			int len = strlen(buffer);
			if (len + 2 < buffer_size) {
				buffer[len] = add;
				buffer[len + 1] = '\0';
				result = true;
			}
		}
	}

	ui_reserve_box(size);
	ui_box (offset, vec3{ size.x, size.y, skui_depth/2 }, skui_mat, focused ? color128{0.5f, 0.5f, 0.5f, 1} : color128{1,1,1,1});
	ui_text(offset + vec3{ size.x/2, -size.y/2, skui_depth/2 + 2*mm2m }, buffer, text_align_x_center | text_align_y_center);
	ui_nextline();

	return result;
}

///////////////////////////////////////////

bool32_t ui_affordance(const char *text, pose_t &movement, vec3 at, vec3 size, bool32_t draw) {
	uint64_t id = sk_ui_hash(text);
	bool result = false;
	color128 color = { 1,1,1,1 };

	for (size_t i = 0; i < handed_max; i++) {
		if (sk_ui_inbox(skui_fingergrip[i], at, size)) {
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
					log_info("unpinch");
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
		if (sk_ui_inbox(skui_fingertip[i], box_start, box_size)) {
			skui_control_focused[i] = id;
			color = { 0.5f,0.5f,0.5f,1 };
			float new_val = ((skui_fingertip[i].x - offset.x) / size.x) * (max - min);
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
	vec2 size   = text_size(skui_font_style, text);
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