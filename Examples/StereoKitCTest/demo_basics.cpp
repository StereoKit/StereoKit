#include "demo_basics.h"

#include "../../StereoKitC/stereokit.h"
#include <vector>
using namespace std;

///////////////////////////////////////////

vector<solid_t> phys_objs;



transform_t tr;
model_t     gltf;

font_t       font;
text_style_t font_style;
material_t   font_mat;
transform_t  text_tr;

///////////////////////////////////////////

void demo_basics_init() {
	font_mat   = material_create("app/font_segoe", shader_find("default/shader_font"));
	font       = font_create("C:/Windows/Fonts/segoeui.ttf");
	font_style = text_make_style(font, 1, font_mat, text_align_x_right | text_align_y_center);
	transform_set(text_tr, vec3_up*0.1f, vec3_one*unit_cm(5), quat_identity);

	// Load a gltf model
	gltf = model_create_file("../../Examples/Assets/DamagedHelmet.gltf");
}

///////////////////////////////////////////

void demo_basics_update() {
	// Do hand input
	if (input_hand(handed_right).state & input_state_justpinch) {
		solid_t new_obj = solid_create({ 0,3,0 }, quat_identity);
		solid_add_sphere(new_obj, 0.45f, 40);
		solid_add_box   (new_obj, vec3_one*0.35f, 40);
		phys_objs.push_back(new_obj);
	}

	// Render solid helmets
	transform_set_scale(tr, vec3_one*0.25f);
	for (size_t i = 0; i < phys_objs.size(); i++) {
		solid_get_transform(phys_objs[i], tr);
		render_add_model   (gltf, transform_matrix(tr));
	}

	transform_set_position(text_tr, input_hand(handed_right).root.position);
	transform_set_rotation(text_tr, input_hand(handed_right).root.orientation);
	if (cosf(sk_timef()*10) > 0) {
		const char *txt = "Testing spaces!!!\n\tAnd newlines?\tAnd Tabs.\nAnother line\n<3";
		vec2 txt_size = text_size(font_style, txt);
		text_add_at(font_style, transform_matrix(text_tr), txt);
	}
}

///////////////////////////////////////////

void demo_basics_shutdown() {
	// Release everything
	for (size_t i = 0; i < phys_objs.size(); i++)
		solid_release(phys_objs[i]);
	model_release(gltf);
}