#pragma once

#include "../libraries/miniaudio.h"
#include "assets.h"

namespace sk {

struct _sound_t {
	asset_header_t header;
	ma_decoder     decoder;
	void          *sound_data;
};

void sound_destroy(sound_t sound);

}