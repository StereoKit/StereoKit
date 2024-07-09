#include <stereokit.h>
#include <stereokit_ui.h>
#include <vector>
using namespace sk;

#include "scene.h"
#include "demo_ui.h"
#include "demo_ui_layout.h"
#include "demo_mic.h"
#include "demo_sprites.h"
#include "demo_lines.h"
#include "demo_picker.h"
#include "demo_world.h"
#include "demo_anchors.h"
#include "demo_lighting.h"
#include "demo_draw.h"
#include "demo_envmap.h"
#include "demo_windows.h"
#include "demo_desktop.h"
#include "demo_bvh.h"
#include "demo_aliasing.h"

#include <stdio.h>

#include <string>
#include <list>

matrix      floor_tr;
material_t  floor_mat;
model_t     floor_model;

scene_t demos[] = {
	{
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
		"Anchors",
		demo_anchors_init,
		demo_anchors_update,
		demo_anchors_shutdown,
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
	}, {
		"Aliasing",
		demo_aliasing_init,
		demo_aliasing_update,
		demo_aliasing_shutdown,
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

void on_log(void*, log_ log_level, const char* log_c_str) {
	if (log_level == log_error) {
		log_level = log_level;
	}
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
	settings.app_name      = "StereoKit C";
	settings.assets_folder = "Assets";
	settings.mode          = app_mode_xr;
	if (!sk_init(settings))
		return 1;

	common_init();

	scene_set_active(demos[8]);

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

	// Build a floor!
	floor_tr = matrix_trs(vec3{ 0,-1.5f,0 }, quat_identity, vec3{ 5,1,5 });

	demo_select_pose.position = vec3{0, 0, -0.4f};
	demo_select_pose.orientation = quat_lookat(vec3_forward, vec3_zero);

		// Log some system info
	const char* text = "";
	log_infof("Device:   %s", device_get_name());
	log_infof("GPU:      %s", device_get_gpu());
	switch (device_get_tracking()) { case device_tracking_none: text = "none"; break; case device_tracking_3dof: text = "3dof"; break; case device_tracking_6dof: text = "6dof"; break; }
	log_infof("Tracking: %s", text);
	log_infof("Hands:    %s", device_has_hand_tracking()?"true":"false");
	log_infof("Eyes:     %s", device_has_eye_gaze()?"true":"false");
	switch (device_display_get_type()) { case display_type_none: text = "none"; break; case display_type_flatscreen: text = "flatscreen"; break; case display_type_stereo: text = "stereo"; break; }
	log_infof("Display Type:  %s", text);
	switch (device_display_get_blend()) { case display_blend_additive: text = "additive"; break; case display_blend_blend: text = "blend"; break; case display_blend_opaque: text = "opaque"; break; case display_blend_none: text = "none"; break; default: text = "unknown";  break; }
	log_infof("Display Blend: %s", text);
	log_infof("Display FoV:   %.0f, %.0f, %.0f, %.0f",
		device_display_get_fov().right,
		device_display_get_fov().left,
		device_display_get_fov().top,
		device_display_get_fov().bottom);
	log_infof("Display Hz:    %.1f", device_display_get_refresh_rate());
	log_infof("Display Size:  %d<~BLK>x<~clr>%d", device_display_get_width(), device_display_get_height());
}

void common_update() {
	static app_focus_ prev_focus = app_focus_hidden;
	app_focus_        curr_focus = sk_app_focus();
	if (curr_focus != prev_focus) {
		switch (curr_focus) {
		case app_focus_active:     log_write(log_diagnostic, "Focus: Active");     break;
		case app_focus_background: log_write(log_diagnostic, "Focus: Background"); break;
		case app_focus_hidden:     log_write(log_diagnostic, "Focus: Hidden");     break;
		}
		prev_focus = curr_focus;
	}

	scene_update();

	// Render floor
	if (device_display_get_blend() == display_blend_opaque)
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
	if (ui_button("Exit") || (input_key(key_esc) & button_state_just_active)) sk_quit();
#endif
	ui_window_end();

	ruler_window();
	log_window();
}

void common_shutdown() {
	scene_shutdown();

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
