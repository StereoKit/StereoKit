#pragma once

#include "../libraries/miniaudio.h"
#include "assets.h"

namespace sk {

struct _sound_t {
	asset_header_t header;
	ma_decoder decoder;
	void *sound_data;
};

struct sound_inst_t {
	sound_t sound;
	vec3    position;
	float   volume;
};

bool sound_init();
void sound_update();
void sound_shutdown();

void sound_destroy(sound_t sound);

}