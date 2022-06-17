#include "demo_basics.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;
#include <vector>
using namespace std;

///////////////////////////////////////////

struct phys_obj_t {
	solid_t solid;
	float   scale;
};
vector<phys_obj_t> phys_objs;

pose_t  tr;
model_t gltf;

///////////////////////////////////////////

void demo_basics_init() {

	// Load a gltf model
	gltf = model_create_file("DamagedHelmet.gltf");
}

///////////////////////////////////////////

void demo_basics_update() {
	// Do hand input

	static pose_t window_pose = //pose_t{ vec3{1,1,1} * 0.9f, quat_lookat({1,1,1}, {0,0,0}) };
		pose_t{ {0,0,-0.25f}, quat_lookat({0,0,-0.25f}, {0,0,0}) };
	ui_window_begin("Options", window_pose, vec2{ 24 }*cm2m);
	
	static float scale = 0.25f;
	ui_hslider("scale", scale, 0.05f, 0.25f, 0, 72 * mm2m); ui_sameline();
	ui_model(gltf, vec2{ 40,10 }*mm2m, scale*0.1f);

	static bool pressed = false;
	if (ui_button("Spawn")) {
		if (!pressed) {
			pressed = true;
			solid_t new_obj = solid_create({ 0,1,0 }, quat_identity);
			solid_add_sphere(new_obj, 1.8f * scale, 40);
			solid_add_box(new_obj, vec3_one * 1.4f * scale, 40);
			phys_objs.push_back({ new_obj, scale });
		}
	} else {
		pressed = false;
	}
	ui_sameline();
	if (ui_button("Clear")) {
		for (size_t i = 0; i < phys_objs.size(); i++)
			solid_release(phys_objs[i].solid);
		phys_objs.clear();
	}

	ui_window_end();

	// Render solid helmets
	for (size_t i = 0; i < phys_objs.size(); i++) {
		solid_get_pose  (phys_objs[i].solid, tr);
		model_draw(gltf, pose_matrix(tr, vec3_one * phys_objs[i].scale));
	}
}

///////////////////////////////////////////

void demo_basics_shutdown() {
	// Release everything
	for (size_t i = 0; i < phys_objs.size(); i++)
		solid_release(phys_objs[i].solid);
	phys_objs.clear();
	model_release(gltf); gltf = nullptr;
}