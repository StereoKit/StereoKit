#include "sk_ui.h"

#include <DirectXMath.h>
using namespace DirectX;
#include <vector>
using namespace std;

///////////////////////////////////////////

struct layer_t {
	XMMATRIX transform;
	XMMATRIX inverse;
	vec3     offset;
	vec2     size;
	float    line_height;
	float    max_x;
};

vector<layer_t> skui_layers;
mesh_t          skui_box;
material_t      skui_mat;
font_t          skui_font;
text_style_t    skui_font_style;
material_t      skui_font_mat;
transform_t     skui_tr;
vec3            skui_fingertip[2];

float skui_padding = 10*mm2m;
float skui_gutter  = 20*mm2m;
float skui_depth   = 20*mm2m;
float skui_fontsize= 20*mm2m;

///////////////////////////////////////////

void sk_ui_init() {
	skui_box = mesh_gen_cube("sk_ui/box", vec3_one);
	skui_mat = material_find("default/material");

	skui_font_mat   = material_create("app/font_segoe", shader_find("default/shader_font"));
	skui_font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	skui_font_style = text_make_style(skui_font, skui_fontsize, skui_font_mat, text_align_x_left | text_align_y_top);
}

///////////////////////////////////////////

void sk_ui_begin_frame() {
}

///////////////////////////////////////////

void sk_ui_push_pose(pose_t pose, vec2 size) {
	XMMATRIX mat = XMMatrixAffineTransformation(
		DirectX::g_XMOne, DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4 *)& pose.orientation),
		XMLoadFloat3((XMFLOAT3 *)& pose.position));

	skui_layers.push_back(layer_t{
		mat, XMMatrixInverse(nullptr, mat),
		vec3{skui_padding, -skui_padding}, size, 0, 0
	});

	for (size_t i = 0; i < handed_max; i++) {
		skui_fingertip[i] = input_hand((handed_)i).fingers[1][4].position;

		XMVECTOR resultXM = XMVector3Transform( XMLoadFloat3((XMFLOAT3 *)&skui_fingertip[i]), skui_layers.back().inverse);
		XMStoreFloat3((XMFLOAT3 *)& skui_fingertip[i], resultXM);
	}
}

///////////////////////////////////////////

void sk_ui_pop_pose() {
	skui_layers.pop_back();
}

///////////////////////////////////////////

void sk_ui_box(vec3 start, vec3 size) {
	vec3 pos = start + (vec3{ size.x, -size.y, size.z } / 2);
	XMMATRIX mat = XMMatrixAffineTransformation(
			XMLoadFloat3((XMFLOAT3 *)& size), DirectX::g_XMZero,
			DirectX::g_XMIdentityR3,
			XMLoadFloat3((XMFLOAT3 *)&pos));
	mat *= skui_layers.back().transform;

	render_add_mesh_mx(skui_box, skui_mat, mat);
}

///////////////////////////////////////////

void sk_ui_text(vec3 start, const char *text) {
	skui_tr._dirty     = false;
	skui_tr._transform = skui_layers.back().transform;

	text_add_at(skui_font_style, skui_tr, text, start.x, start.y, start.z);
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
}

///////////////////////////////////////////

void sk_ui_space(float space) {
	if (skui_layers.back().offset.x == skui_padding)
		skui_layers.back().offset.y -= space;
	else
		skui_layers.back().offset.x += space;
}

///////////////////////////////////////////
bool sk_ui_inbox(vec3 pt, vec3 box_start, vec3 box_end) {
	return
		pt.x >= box_start.x && pt.x <= box_end.x &&
		pt.y >= box_start.y && pt.y <= box_end.y &&
		pt.z >= box_start.z && pt.z <= box_end.z;
}
void sk_ui_button(const char *text) {
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

	vec3 box_start = offset;
	vec3 box_end   = offset + vec3{ size.x, size.y, skui_depth };
	float finger_offset = 0;
	for (size_t i = 0; i < handed_max; i++) {
		if (sk_ui_inbox(skui_fingertip[i], box_start, box_end)) {
			finger_offset = skui_fingertip[i].z-box_start.z;
		}
	}

	sk_ui_reserve_box(size);
	sk_ui_box (offset, vec3{ size.x, size.y, skui_depth+finger_offset });
	sk_ui_text(offset + vec3{ skui_padding, -skui_padding, (skui_depth + 2*mm2m)+finger_offset }, text);
}

///////////////////////////////////////////

void sk_ui_window_begin(const char *text, pose_t pose, vec2 size) {
	sk_ui_push_pose(pose, size);

	vec2 name_size = text_size(skui_font_style, text);
	sk_ui_text(vec3{ skui_padding, name_size.y, 2.f*mm2m }, text);
}

///////////////////////////////////////////

void sk_ui_window_end() {
	vec2 size = skui_layers.back().size;
	if (size.x == 0)
		size.x = skui_layers.back().max_x;
	if (size.y == 0)
		size.y = -skui_layers.back().offset.y + skui_layers.back().line_height + skui_padding;
	sk_ui_box(vec3{ 0,0,-skui_depth}, vec3{ size.x, size.y, skui_depth });

	sk_ui_pop_pose();
}