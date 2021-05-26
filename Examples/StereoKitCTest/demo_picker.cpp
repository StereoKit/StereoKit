#include "demo_lines.h"

#include "../../StereoKitC/stereokit.h"
#include "../../StereoKitC/stereokit_ui.h"
using namespace sk;

#include <malloc.h>
#include <string.h>

///////////////////////////////////////////

pose_t        picker_win_pose = { {0.5f, 0, -0.5f}, quat_lookat(vec3_zero, {-1,0,1}) };
char         *picker_filename = nullptr;
file_filter_t picker_filter[] = { {".glb"}, {".gltf"} };

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
}

///////////////////////////////////////////

void demo_picker_init() {
}

///////////////////////////////////////////

void demo_picker_update() {
	ui_window_begin("File Pickers", picker_win_pose);
	if (ui_button("Open") && !platform_file_picker_visible())
		platform_file_picker(picker_mode_open, nullptr, demo_picker_on_pick, picker_filter, sizeof(picker_filter) / sizeof(file_filter_t));
	ui_sameline();
	if (ui_button("Save") && !platform_file_picker_visible())
		platform_file_picker(picker_mode_save, nullptr, demo_picker_on_pick, picker_filter, sizeof(picker_filter) / sizeof(file_filter_t));

	ui_label(picker_filename == nullptr ? " " : picker_filename);

	ui_window_end();
}

///////////////////////////////////////////

void demo_picker_shutdown() {
	free(picker_filename);
	picker_filename = nullptr;

	platform_file_picker_close();
}