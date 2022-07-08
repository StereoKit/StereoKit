#pragma once

#include "../libraries/miniaudio.h"
#include "../libraries/tinycthread.h"
#include "assets.h"

namespace sk {

struct buffer_t {
	float*   data;
	uint64_t capacity;
	uint64_t cursor;
	uint64_t count;
};

typedef enum sound_type_ {
	sound_type_none,
	sound_type_decode,
	sound_type_buffer,
	sound_type_stream,
} sound_type_;

struct _sound_t {
	asset_header_t header;
	sound_type_    type;
	ma_decoder     decoder;
	buffer_t       buffer;
	ma_pcm_rb      stream_buffer;
	mtx_t          data_lock;
};

void sound_destroy(sound_t sound);

}