#include <stereokit.h>
#include <stereokit_ui.h>
#include <vector>
using namespace sk;

#include "scene.h"
#include "demo_basics.h"
#include "demo_ui.h"
#include "demo_ui_layout.h"
#include "demo_mic.h"
#include "demo_sprites.h"
#include "demo_lines.h"
#include "demo_picker.h"
#include "demo_world.h"
#include "demo_lighting.h"
#include "demo_draw.h"
#include "demo_envmap.h"
#include "demo_windows.h"
#include "demo_desktop.h"
#include "demo_bvh.h"

#include <stdio.h>

#include <string>
#include <list>

solid_t     floor_solid;
matrix      floor_tr;
material_t  floor_mat;
model_t     floor_model;

scene_t demos[] = {
	{
		"Basics",
		demo_basics_init,
		demo_basics_update,
		demo_basics_shutdown,
	}, {
		"UI",
		demo_ui_init,
		demo_ui_update,
		demo_ui_shutdown,
	}, {
		"UI Layout",
		demo_ui_layout_init,
		demo_ui_layout_update,
		demo_ui_layout_shutdown,
	}, {
		"Microphone",
		demo_mic_init,
		demo_mic_update,
		demo_mic_shutdown,
	}, {
		"Sprites",
		demo_sprites_init,
		demo_sprites_update,
		demo_sprites_shutdown,
	}, {
		"Lines",
		demo_lines_init,
		demo_lines_update,
		demo_lines_shutdown,
	}, {
		"Picker",
		demo_picker_init,
		demo_picker_update,
		demo_picker_shutdown,
	}, {
		"World",
		demo_world_init,
		demo_world_update,
		demo_world_shutdown,
	}, {
		"Lighting",
		demo_lighting_init,
		demo_lighting_update,
		demo_lighting_shutdown,
	}, {
		"Draw",
		demo_draw_init,
		demo_draw_update,
		demo_draw_shutdown,
	}, {
		"Environment Map",
		demo_envmap_init,
		demo_envmap_update,
		demo_envmap_shutdown,
	}, {
        "BVH",
        demo_bvh_init,
        demo_bvh_update,
        demo_bvh_shutdown,
    },
#if defined(_WIN32) && !defined(WINDOWS_UWP)
	{
		"Windows",
		demo_windows_init,
		demo_windows_update,
		demo_windows_shutdown,
	}, {
		"Desktop",
		demo_desktop_init,
		demo_desktop_update,
		demo_desktop_shutdown,
	}
#endif
};


pose_t demo_select_pose;

void on_log(log_, const char*);
void log_window();
void common_init();
void common_update();
void common_shutdown();
void ruler_window();

pose_t log_pose = pose_t{vec3{0, -0.1f, 0.5f}, quat_lookat(vec3_zero, vec3_forward)};
std::list<std::string> log_list;

void on_log(log_ log_level, const char* log_c_str) {
	if (log_list.size() > 10) {
		log_list.pop_front();
	}
	std::string log_str(log_c_str);
	if (log_str.size() >= 100) {
		log_str.resize(100);
		log_str += "...";
	}
	log_list.push_back(log_str);
}

void log_window() {
	ui_window_begin("Log", log_pose, vec2{40*cm2m, 0*cm2m});
	for (auto &log_str : log_list) {
		ui_label(log_str.c_str(), false);
	}
	ui_window_end();
}

#ifndef WINDOWS_UWP
int main() {
#else
int __stdcall wWinMain(void*, void*, wchar_t*, int) {
#endif
	log_subscribe(on_log);
	log_set_filter(log_diagnostic);

	sk_settings_t settings = {};
	settings.app_name           = "StereoKit C";
	settings.assets_folder      = "Assets";
	settings.display_preference = display_mode_mixedreality;
	if (!sk_init(settings))
		return 1;

	common_init();

	scene_set_active(demos[1]);

	sk_run(common_update, common_shutdown);

	return 0;
}

void common_init() {
	// Create a PBR floor material
	tex_t tex_color = tex_create_file("test.png");
	//tex_t tex_norm  = tex_create_file("test_normal.png");
	floor_mat = material_copy_id(default_id_material);
	material_set_texture(floor_mat, "diffuse",   tex_color);
	//material_set_texture(floor_mat, "normal",    tex_norm);
	material_set_float  (floor_mat, "tex_scale", 6);
	material_set_float  (floor_mat, "roughness", 1.0f);
	material_set_float  (floor_mat, "metallic",  0.5f);
	material_set_queue_offset(floor_mat, 1);
	if (tex_color != nullptr) tex_release(tex_color);
	//if (tex_norm  != nullptr) tex_release(tex_norm);

	// Procedurally create a cube model
	mesh_t mesh_cube = mesh_gen_cube(vec3_one, 0);
	floor_model  = model_create_mesh(mesh_cube, floor_mat);
	mesh_release(mesh_cube);

	// Build a physical floor!
	vec3 pos   = vec3{ 0,-1.5f,0 };
	vec3 scale = vec3{ 5,1,5 };
	floor_tr    = matrix_trs(pos, quat_identity, scale);
	floor_solid = solid_create(pos, quat_identity, solid_type_immovable);
	solid_add_box (floor_solid, scale);

	demo_select_pose.position = vec3{0, 0, -0.4f};
	demo_select_pose.orientation = quat_lookat(vec3_forward, vec3_zero);
}

void common_update() {
	scene_update();

	// Render floor
	if (sk_system_info().display_type == display_opaque)
		render_add_model(floor_model, floor_tr);

	ui_window_begin("Demos", demo_select_pose, vec2{50*cm2m, 0*cm2m});
	for (int i = 0; i < sizeof(demos) / sizeof(scene_t); i++) {
		std::string &name = demos[i].name;

		ui_sameline();

		if (ui_button(name.c_str())) {
			log_write(log_inform, name.c_str());
			scene_set_active(demos[i]);
		}
	}
#if !defined(__EMSCRIPTEN__)
	ui_hseparator();
	if (ui_button("Exit")) sk_quit();
#endif
	ui_window_end();

	ruler_window();
	log_window();
}

void common_shutdown() {
	scene_shutdown();

	solid_release   (floor_solid);
	material_release(floor_mat);
	model_release   (floor_model);
}

void ruler_window() {
	static pose_t window_pose = pose_t{{0, 0, 0.5f}, quat_identity};
	ui_handle_begin("Ruler", window_pose,
					bounds_t{vec3_zero, vec3{30*cm2m, 4*cm2m, 1*cm2m}},
					true, ui_move_exact);
	color32 color = color_to_32(color_hsv(0.6f, 0.5f, 1, 1));
	text_add_at("Centimeters",
				matrix_trs(vec3{14.5f*cm2m, -1.5f*cm2m, -0.6f*cm2m},
						   quat_identity, vec3{0.3f, 0.3f, 0.3f}),
				0, text_align_bottom_left);
	for (int d = 0; d <= 60; d++) {
		float x = d / 2.0f;
		float size = (d % 2 == 0) ? 1.0f : 0.15f;
		line_add(vec3{(15 - x)*cm2m, 2*cm2m, -0.6f*cm2m},
				 vec3{(15 - x)*cm2m, (2 - size)*cm2m, -0.6f*cm2m},
				 color, color, 0.5f*mm2m);

		if (d % 2 == 0 && d / 2 != 30) {
			text_add_at(std::to_string(d / 2).c_str(),
						matrix_trs(vec3{(15 - x - 0.1f)*cm2m,
										(2 - size)*cm2m, -0.6f*cm2m},
									quat_identity, vec3{0.2f, 0.2f, 0.2f}),
						0, text_align_bottom_left);
		}
	}
	ui_handle_end();
}
