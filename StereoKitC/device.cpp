#include "device.h"
#include "stereokit.h"
#include "sk_memory.h"

namespace sk {

device_data_t device_data = {};

///////////////////////////////////////////

void device_data_free(device_data_t* data) {
	sk_free(data->name);
	sk_free(data->gpu);
	*data = {};
}

///////////////////////////////////////////

display_type_ device_display_get_type() {
	return device_data.display_type;
}

///////////////////////////////////////////

display_blend_ device_display_get_blend() {
	return device_data.display_blend;
}

///////////////////////////////////////////

float device_display_get_refresh_rate() {
	return device_data.display_refresh_rate;
}

///////////////////////////////////////////

int32_t device_display_get_width() {
	return device_data.display_width;
}

///////////////////////////////////////////

int32_t device_display_get_height() {
	return device_data.display_height;
}

///////////////////////////////////////////

fov_info_t device_display_get_fov() {
	return device_data.display_fov;
}

///////////////////////////////////////////

device_tracking_ device_get_tracking() {
	return device_data.tracking;
}

///////////////////////////////////////////

const char* device_get_name() {
	return device_data.name;
}

///////////////////////////////////////////

const char* device_get_gpu() {
	return device_data.gpu;
}

///////////////////////////////////////////

bool32_t device_has_eye_gaze() {
	return device_data.has_eye_gaze;
}

///////////////////////////////////////////

bool32_t device_has_hand_tracking() {
	return device_data.has_hand_tracking;
}

///////////////////////////////////////////

}