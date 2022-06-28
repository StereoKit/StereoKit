#include "sound.h"
#include "assets.h"
#include "../stereokit.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../platforms/platform_utils.h"
#include "../systems/audio.h"

#include <string.h>

namespace sk {

///////////////////////////////////////////

sound_t sound_find(const char *id) {
	sound_t result = (sound_t)assets_find(id, asset_type_sound);
	if (result != nullptr) {
		sound_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void sound_set_id(sound_t sound, const char *id) {
	assets_set_id(&sound->header, id);
}

///////////////////////////////////////////

sound_t sound_create(const char *filename) {
	sound_t result = sound_find(filename);
	if (result != nullptr)
		return result;

	const char *sound_file = assets_file(filename);
	void       *data;
	size_t      length;
	if (!platform_read_file(sound_file, &data, &length)) {
		log_warnf("Sound file failed to load: %s", filename);
		return nullptr;
	}

	result = (_sound_t*)assets_allocate(asset_type_sound);
	result->type = sound_type_decode;
	sound_set_id(result, filename);

	ma_decoder_config config = ma_decoder_config_init(AU_SAMPLE_FORMAT, 1, AU_SAMPLE_RATE);
	if (ma_decoder_init_memory(data, length, &config, &result->decoder) != MA_SUCCESS) {
		log_errf("Failed to parse sound '%s'.", sound_file);
		assets_releaseref(&result->header);
		return nullptr;
	}
	return result;
}

///////////////////////////////////////////

sound_t sound_create_stream(float buffer_duration) {
	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);

	result->type = sound_type_stream;
	result->buffer.capacity = (uint64_t)((double)buffer_duration * AU_SAMPLE_RATE);
	result->buffer.data     = sk_malloc_t(float, (size_t)result->buffer.capacity);
	memset(result->buffer.data, 0, (size_t)(result->buffer.capacity * sizeof(float)));
	mtx_init(&result->data_lock, mtx_plain);

	return result;
}

///////////////////////////////////////////

sound_t sound_create_samples(const float *samples_at_48000s, uint64_t sample_count) {
	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);

	result->type = sound_type_buffer;
	result->buffer.capacity = sample_count;
	result->buffer.count    = sample_count;
	result->buffer.data     = sk_malloc_t(float, (size_t)sample_count);
	memcpy(result->buffer.data, samples_at_48000s, (size_t)(sample_count * sizeof(float)));

	return result;
}

///////////////////////////////////////////

sound_t sound_generate(float (*function)(float sample_time), float duration) {
	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);

	result->type = sound_type_buffer;
	result->buffer.capacity = (uint64_t)((double)duration * AU_SAMPLE_RATE);
	result->buffer.count    = result->buffer.capacity;
	result->buffer.data     = sk_malloc_t(float, (size_t)result->buffer.capacity);
	for (uint64_t i = 0, s = result->buffer.capacity; i < s; i += 1) {
		result->buffer.data[i] = function((float)i / (float)AU_SAMPLE_RATE);
	}

	return result;
}

///////////////////////////////////////////

void sound_write_samples(sound_t sound, const float *samples, uint64_t sample_count) {
	if (sound->type != sound_type_stream) { log_err("Sound read/write is only supported for streaming type sounds!"); return; }

	mtx_lock(&sound->data_lock);
	ring_buffer_write(&sound->buffer, samples, sample_count);
	mtx_unlock(&sound->data_lock);
}

///////////////////////////////////////////

uint64_t sound_read_samples(sound_t sound, float *out_samples, uint64_t sample_count) {
	if (sound->type != sound_type_stream) { log_err("Sound read/write is only supported for streaming type sounds!"); return 0; }

	mtx_lock(&sound->data_lock);
	uint64_t result = ring_buffer_read(&sound->buffer, out_samples, sample_count);
	mtx_unlock(&sound->data_lock);

	return result == 1
		? 0 
		: result;
}

///////////////////////////////////////////

uint64_t sound_unread_samples(sound_t sound) {
	if (sound->type != sound_type_stream) { return 0; }
	return sound->buffer.cursor >= sound->buffer.start
		? sound->buffer.count - ( sound->buffer.cursor - sound->buffer.start)
		: sound->buffer.count - ((sound->buffer.capacity-sound->buffer.start) + sound->buffer.cursor);
}

///////////////////////////////////////////

sound_inst_t sound_play(sound_t sound, vec3 at, float volume) {
	ma_decoder_seek_to_pcm_frame(&sound->decoder, 0);
	sound->buffer.cursor = sound->buffer.start;

	sound_inst_t result;
	result._id   = 0;
	result._slot = -1;

	for (size_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == sound) {
			au_active_sounds[i] = {sound, au_active_sounds[i].id, at, volume };

			result._id   = au_active_sounds[i].id;
			result._slot = (int16_t)i;
			return result;
		}
	}

	for (size_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == nullptr) {
			sound_addref(sound);
			au_active_sounds[i] = { sound, (uint16_t)(au_active_sounds[i].id+1), at, volume };

			result._id   = au_active_sounds[i].id;
			result._slot = (int16_t)i;
			return result;
		}
	}
	return result;
}
///////////////////////////////////////////

uint64_t sound_total_samples(sound_t sound) {
	switch (sound->type) {
	case sound_type_decode: {
		ma_uint64 sample_count = 0;
		if (ma_decoder_get_length_in_pcm_frames(&sound->decoder, &sample_count) != MA_SUCCESS) {
			log_err("Failed to get sample count of a decode sound!");
			return 0;
		}
		return sample_count;
	}
	case sound_type_buffer:
	case sound_type_stream: return sound->buffer.count;
	default: return 0;
	}
}

///////////////////////////////////////////

uint64_t sound_cursor_samples(sound_t sound) {
	return sound->buffer.cursor >= sound->buffer.start
		? sound->buffer.cursor - sound->buffer.start
		: (sound->buffer.capacity - sound->buffer.start) + sound->buffer.cursor;
}

///////////////////////////////////////////

float sound_duration(sound_t sound) {
	switch (sound->type) {
	case sound_type_decode: {
		ma_uint64 sample_count = 0;
		if (ma_decoder_get_length_in_pcm_frames(&sound->decoder, &sample_count) != MA_SUCCESS) {
			log_err("Failed to get sample duration of a decode sound!");
			return 0;
		}
		return (float)sample_count / (float)sound->decoder.outputSampleRate;
	}
	case sound_type_buffer:
	case sound_type_stream: return (float)sound->buffer.count / (float)AU_SAMPLE_RATE;
	default: return 0;
	}
}

///////////////////////////////////////////

void sound_addref(sound_t sound) {
	assets_addref(&sound->header);
}

///////////////////////////////////////////

void sound_release(sound_t sound) {
	if (sound == nullptr)
		return;

	assets_releaseref(&sound->header);
}

///////////////////////////////////////////

void sound_destroy(sound_t sound) {
	ma_decoder_uninit(&sound->decoder);
	if (sound->buffer.data) {
		sk_free    ( sound->buffer.data);
		mtx_destroy(&sound->data_lock);
	}
	memset(sound, 0, sizeof(_sound_t));
}

///////////////////////////////////////////

void sound_inst_stop(sound_inst_t sound_inst) {
	if (sound_inst._slot < 0 || au_active_sounds[sound_inst._slot].id != sound_inst._id) return;

	sound_t sound = au_active_sounds[sound_inst._slot].sound;
	au_active_sounds[sound_inst._slot].sound = nullptr;
	sound_release(sound);
}

///////////////////////////////////////////

bool32_t sound_inst_is_playing(sound_inst_t sound_inst) {
	if (sound_inst._slot < 0 || au_active_sounds[sound_inst._slot].id != sound_inst._id) return false;
	return au_active_sounds[sound_inst._slot].sound != nullptr;
}

///////////////////////////////////////////
void sound_inst_set_pos(sound_inst_t sound_inst, vec3 pos) {
	if (sound_inst._slot < 0 || au_active_sounds[sound_inst._slot].id != sound_inst._id) return;
	au_active_sounds[sound_inst._slot].position = pos;
}

///////////////////////////////////////////

vec3 sound_inst_get_pos(sound_inst_t sound_inst) {
	if (sound_inst._slot < 0 || au_active_sounds[sound_inst._slot].id != sound_inst._id) return vec3_zero;
	return au_active_sounds[sound_inst._slot].position;
}

///////////////////////////////////////////

void sound_inst_set_volume(sound_inst_t sound_inst, float volume) {
	if (sound_inst._slot < 0 || au_active_sounds[sound_inst._slot].id != sound_inst._id) return;
	au_active_sounds[sound_inst._slot].volume = volume;
}

///////////////////////////////////////////

float sound_inst_get_volume(sound_inst_t sound_inst) {
	if (sound_inst._slot < 0 || au_active_sounds[sound_inst._slot].id != sound_inst._id) return 0;
	return au_active_sounds[sound_inst._slot].volume;
}

///////////////////////////////////////////

void ring_buffer_write(ring_buffer_t *buffer, const float *data, uint64_t data_size) {
	// Special case, if the input is larger than the entire ring buffer, just
	// reset the whole thing.
	if (data_size >= buffer->capacity) {
		data_size = buffer->capacity;
		memcpy(buffer->data, data, (size_t)(data_size * sizeof(float)));
		buffer->start  = 0;
		buffer->cursor = 0;
		buffer->count  = data_size;
		return;
	}

	uint64_t write_at = (buffer->start + buffer->count) % buffer->capacity;
	buffer->count = mini(buffer->capacity, buffer->count+data_size);

	if (write_at + data_size > buffer->capacity) {
		// If the write loops around the end of the ring, we need to do the
		// copy as two pieces.
		uint64_t first_copy_size  = buffer->capacity - write_at;
		uint64_t second_copy_size = data_size - first_copy_size;
		memcpy(buffer->data + write_at, data,                   (size_t)(first_copy_size  * sizeof(float)));
		memcpy(buffer->data,            data + first_copy_size, (size_t)(second_copy_size * sizeof(float)));

		// Update the starting point and the cursor if we copied over the
		// area where they were.
		if (buffer->start  < second_copy_size || buffer->start  >= write_at) buffer->start  = second_copy_size;
		if (buffer->cursor < second_copy_size || buffer->cursor >= write_at) buffer->cursor = second_copy_size;
	} else {
		// The simple, most frequent case, where we just need a single copy.
		memcpy(buffer->data + write_at, data, (size_t)(data_size * sizeof(float)));

		// Update the starting point and the cursor if we copied over the
		// area where they were.
		uint64_t write_end = write_at + data_size;
		if (buffer->start  >= write_at && buffer->start  < write_end) buffer->start  = write_end % buffer->count;
		if (buffer->cursor >  write_at && buffer->cursor < write_end) buffer->cursor = write_end % buffer->count;
	}
}

///////////////////////////////////////////

uint64_t ring_buffer_read(ring_buffer_t *buffer, float *out_data, uint64_t out_data_size) {
	if (out_data_size == 0) return 0;
	uint64_t cursor_start    = buffer->cursor+1;
	uint64_t cursor_relative = buffer->cursor < buffer->start
		? cursor_start + (buffer->capacity-buffer->start)
		: cursor_start - buffer->start;
	uint64_t frames_read = mini(out_data_size, buffer->count - cursor_relative);

	if (cursor_start + frames_read > buffer->capacity) {
		// if the frames_read wraps past the end of the ring buffer,
		// then we'll need two copies
		uint64_t first_half = buffer->capacity - cursor_start;
		memcpy(out_data,            buffer->data + cursor_start, (size_t)(sizeof(float) *  first_half));
		memcpy(out_data+first_half, buffer->data,                (size_t)(sizeof(float) * (frames_read-first_half)));
		buffer->cursor = mini(frames_read-first_half, buffer->start-1);
	} else {
		// The starting point of the ring buffer is after the cursor,
		// so we'll only need a single copy.
		memcpy(out_data, buffer->data + cursor_start, (size_t)(frames_read*sizeof(float)));
		buffer->cursor += frames_read;
		if (buffer->cursor == buffer->start)
			buffer->cursor -= 1;
	}
	return frames_read;
}

}