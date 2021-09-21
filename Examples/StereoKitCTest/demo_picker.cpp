#include "demo_lines.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;

#include <malloc.h>
#include <string.h>

///////////////////////////////////////////

pose_t        picker_win_pose = { {0.5f, 0, -0.5f}, quat_lookat(vec3_zero, {-1,0,1}) };
char         *picker_filename = nullptr;
const char   *picker_displayname = nullptr;
file_filter_t picker_filter[] = { {".glb"}, {".gltf"} };

model_t       picker_model = nullptr;
pose_t        picker_pose  =  { {0.7f, 0, -0.3f}, quat_lookat(vec3_zero, {-1,0,1}) };
bounds_t      picker_bounds;
float         picker_scale;

///////////////////////////////////////////

void demo_picker_on_pick(void *, bool32_t confirmed, const char *filename) {
	if (!confirmed) {
		log_info("User cancelled the file picker");
		return;
	}
	free(picker_filename);
	picker_filename = (char*)malloc(sizeof(char)*(strlen(filename)+1));
	if (picker_filename != nullptr)
		memcpy(picker_filename, filename, strlen(filename)+1);

	picker_model  = model_create_file(filename);
	picker_bounds = model_get_bounds(picker_model);

	float mag = vec3_magnitude(picker_bounds.dimensions);
	picker_scale = (1.0f / mag) * 0.2f;
	picker_bounds.center     *= picker_scale;
	picker_bounds.dimensions *= picker_scale;

	picker_displayname = picker_filename;
	const char *curr = picker_displayname;
	while (*curr != '\0') {
		if (*curr == '/' || *curr == '\\')
			picker_displayname = curr + 1;
		curr++;
	}
}

///////////////////////////////////////////

void demo_picker_init() {
}

///////////////////////////////////////////

void show_node(model_t model, model_node_id node, int32_t tab) {
	node = model_node_child(picker_model, node);

	while (node != -1) {
		for (size_t i = 0; i < tab; i++) { ui_label("-"); ui_sameline(); }
		ui_label(model_node_get_name(picker_model, node));
		show_node(model, node, tab + 1);
		node = model_node_sibling(picker_model, node);
	}
}

void demo_picker_update() {
	ui_window_begin("File Pickers", picker_win_pose);
	if (ui_button("Open") && !platform_file_picker_visible())
		platform_file_picker(picker_mode_open, nullptr, demo_picker_on_pick, picker_filter, sizeof(picker_filter) / sizeof(file_filter_t));
	ui_sameline();
	if (ui_button("Close")) {
		model_release(picker_model);
		picker_model = nullptr;
	}
	ui_sameline();
	if (ui_button("(Save)") && !platform_file_picker_visible())
		platform_file_picker(picker_mode_save, nullptr, demo_picker_on_pick, picker_filter, sizeof(picker_filter) / sizeof(file_filter_t));

	if (picker_model) {
		ui_label(picker_displayname == nullptr ? " " : picker_displayname);
		ui_hseparator();
		show_node(picker_model, -1, 0);
	}

	ui_window_end();
	if (picker_model) {
		ui_handle_begin("picker model", picker_pose, picker_bounds, false);
		render_add_model(picker_model, matrix_trs(vec3_zero, quat_identity, vec3_one * picker_scale));
		ui_handle_end();
	}
}

///////////////////////////////////////////

void demo_picker_shutdown() {
	free(picker_filename);
	picker_filename = nullptr;

	platform_file_picker_close();
}