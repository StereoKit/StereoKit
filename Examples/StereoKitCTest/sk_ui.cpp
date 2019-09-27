#include "sk_ui.h"

#include <DirectXMath.h>
using namespace DirectX;
#include <vector>
using namespace std;

///////////////////////////////////////////

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

float skui_padding = 10*mm2m;
float skui_gutter  = 20*mm2m;
float skui_depth   = 20*mm2m;
float skui_fontsize= 20*mm2m;

uint64_t skui_control_focused[2] = {};
uint64_t skui_control_active [2] = {};

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

void sk_ui_init() {
	skui_box = mesh_gen_cube("sk_ui/box", vec3_one);

	material_t mat_default = material_find("default/material");
	skui_mat        = material_copy("sk_ui/mat", mat_default);
	skui_mat_active = material_copy("sk_ui/mat_active", mat_default);
	material_release(mat_default);
	material_set_color32(skui_mat_active, "color", { 128,128,128,255 });

	skui_font_mat   = material_create("app/font_segoe", shader_find("default/shader_font"));
	skui_font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, text_align_x_left | text_align_y_top);
}

///////////////////////////////////////////

void sk_ui_begin_frame() {
}

///////////////////////////////////////////

void sk_ui_push_pose(pose_t pose, vec2 size) {
	matrix trs = matrix_trs(pose.position, pose.orientation);
	matrix trs_inverse;
	matrix_inverse(trs, trs_inverse);

	skui_layers.push_back(layer_t{
		trs, trs_inverse,
		vec3{skui_padding, -skui_padding}, size, 0, 0
	});

	for (size_t i = 0; i < handed_max; i++) {
		skui_fingertip[i] = input_hand((handed_)i).fingers[1][4].position;
		skui_fingertip[i] = matrix_mul_point(skui_layers.back().inverse, skui_fingertip[i]);
	}
}

///////////////////////////////////////////

void sk_ui_pop_pose() {
	skui_layers.pop_back();
}

///////////////////////////////////////////

void sk_ui_box(vec3 start, vec3 size, material_t material) {
	vec3 pos = start + (vec3{ size.x, -size.y, size.z } / 2);
	matrix mx = matrix_trs(pos, quat_identity, size);
	matrix_mul(mx, skui_layers.back().transform, mx);

	render_add_mesh(skui_box, material, mx);
}

///////////////////////////////////////////

void sk_ui_text(vec3 start, const char *text, text_align_ position) {
	text_add_at(skui_font_style, skui_layers.back().transform, text, position, start.x, start.y, start.z);
}

///////////////////////////////////////////

void sk_ui_reserve_box(vec2 size) {
	if (skui_layers.back().max_x < skui_layers.back().offset.x + size.x + skui_padding)
		skui_layers.back().max_x = skui_layers.back().offset.x + size.x + skui_padding;
	if (skui_layers.back().line_height < size.y)
		skui_layers.back().line_height = size.y;

	skui_layers.back().offset += vec3{ size.x + skui_gutter, 0, 0 };
}

///////////////////////////////////////////

void sk_ui_nextline() {
	skui_layers.back().offset.x = skui_padding;
	skui_layers.back().offset.y -= skui_layers.back().line_height + skui_gutter;
	skui_layers.back().line_height = 0;
}

///////////////////////////////////////////

void sk_ui_space(float space) {
	if (skui_layers.back().offset.x == skui_padding)
		skui_layers.back().offset.y -= space;
	else
		skui_layers.back().offset.x += space;
}

///////////////////////////////////////////

bool sk_ui_inbox(vec3 pt, vec3 box_start, vec3 box_size) {
	pt -= box_start;
	return
		pt.x >= 0 && pt.x <=  box_size.x &&
		pt.y <= 0 && pt.y >= -box_size.y &&
		pt.z >= 0 && pt.z <=  box_size.z;
}

///////////////////////////////////////////

void sk_ui_label(const char *text) {
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(skui_font_style, text);
	sk_ui_reserve_box(size);
	sk_ui_text(offset + vec3{ 0, 0, 2*mm2m }, text);
}

///////////////////////////////////////////

void sk_ui_image(sprite_t image, vec2 size) {
	float aspect     = sprite_get_aspect(image);
	vec3  offset     = skui_layers.back().offset;
	vec2  final_size = vec2{
		size.x==0 ? size.y/aspect : size.x, 
		size.y==0 ? size.x*aspect : size.y };

	sk_ui_reserve_box(final_size);
	
	matrix result;
	matrix_mul(matrix_trs(offset, quat_identity, vec3{ final_size.x, final_size.y, 1 }), skui_layers.back().transform, result);
	sprite_draw(image, result);
}

///////////////////////////////////////////

bool sk_ui_button(const char *text) {
	uint64_t id = sk_ui_hash(text);
	bool result = false;
	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(skui_font_style, text);
	size += vec2{ skui_padding, skui_padding }*2;

	if (offset.x                  != skui_padding && 
		skui_layers.back().size.x != 0            && 
		offset.x + size.x > skui_layers.back().size.x - skui_padding)
	{
		sk_ui_nextline();
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

	sk_ui_reserve_box(size);
	sk_ui_box (offset, vec3{ size.x, size.y, finger_offset }, result ? skui_mat_active : skui_mat);
	sk_ui_text(offset + vec3{ size.x/2, -size.y/2, finger_offset + 2*mm2m }, text, text_align_x_center | text_align_y_center);

	return result;
}

///////////////////////////////////////////

bool sk_ui_affordance(const char *text, pose_t &movement, vec3 at, vec3 size) {
	uint64_t id = sk_ui_hash(text);
	bool result = false;

	for (size_t i = 0; i < handed_max; i++) {
		if (sk_ui_inbox(skui_fingertip[i], at, size)) {
			skui_control_focused[i] = id;
		} else if (skui_control_focused[i] == id) {
			skui_control_focused[i] = 0;
		}

		if (skui_control_focused[i] == id || skui_control_active[i] == id) {
			static vec3 start_aff_pos = vec3_zero;
			static quat start_aff_rot = quat_identity;
			static vec3 start_tip_pos = vec3_zero;
			static quat start_tip_rot = quat_identity;
			vec3 tip_pos = matrix_mul_point(skui_layers.back().transform, skui_fingertip[i]);

			const hand_t &hand = input_hand((handed_)i);
			if (hand.state & input_state_justpinch) {
				skui_control_active[i] = id;
				start_aff_pos = movement.position;
				start_aff_rot = movement.orientation;
				start_tip_pos = tip_pos;
				start_tip_rot = input_hand((handed_)i).root.orientation;
			}
			if (skui_control_active[i] == id) {
				result = true;
				movement.position    = start_aff_pos + (tip_pos - start_tip_pos);
				//movement.orientation = quat_mul(start_aff_rot, quat_difference(start_tip_rot, input_hand((handed_)i).root.orientation));
				if (hand.state & input_state_unpinch) {
					skui_control_active[i] = 0;
				}
			}
		}
	}

	sk_ui_box (at, size, result ? skui_mat_active : skui_mat);

	return result;
}

///////////////////////////////////////////

bool sk_ui_hslider(const char *name, float &value, float min, float max, float step, float width) {
	uint64_t   id     = sk_ui_hash(name);
	bool       result = false;
	material_t mat    = skui_mat;
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
			mat = skui_mat_active;
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
	sk_ui_reserve_box(size);
	sk_ui_box(vec3{ offset.x, offset.y - size.y / 2.f + rule_size / 2.f, offset.z }, vec3{ size.x, rule_size, rule_size }, mat);
	sk_ui_box(vec3{ offset.x + ((value-min)/(max-min))*size.x - rule_size/2.f, offset.y, offset.z}, vec3{rule_size, size.y, skui_depth}, mat);
	return result;
}

///////////////////////////////////////////

void sk_ui_window_begin(const char *text, pose_t &pose, vec2 window_size) {
	uint64_t id = sk_ui_hash(text);

	if (window_size.x == 0) window_size.x = 1;

	sk_ui_push_pose(pose, window_size);

	vec3 offset = skui_layers.back().offset;
	vec2 size   = text_size(skui_font_style, text);
	vec3 box_start = vec3{ 0, 0, -skui_depth };
	vec3 box_size  = vec3{ window_size.x, size.y+skui_padding*2, skui_depth };

	sk_ui_reserve_box(size);
	sk_ui_text(box_start + vec3{skui_padding,-skui_padding, skui_depth + 2*mm2m}, text);
	sk_ui_affordance(text, pose, box_start, box_size);

	sk_ui_nextline();
}

///////////////////////////////////////////

void sk_ui_window_end() {
	sk_ui_pop_pose();
}