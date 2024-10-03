/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2019-2024 Nick Klingensmith
 * Copyright (c) 2024 Qualcomm Technologies, Inc.
 */

#include "sound.h"
#include "assets.h"
#include "../stereokit.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../platforms/platform.h"
#include "../systems/audio.h"
#include "../libraries/ferr_thread.h"

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

const char* sound_get_id(const sound_t sound) {
	return sound->header.id_text;
}

///////////////////////////////////////////

sound_t sound_create(const char *filename) {
	sound_t result = sound_find(filename);
	if (result != nullptr)
		return result;

	
	void*    data;
	size_t   length;
	bool32_t loaded     = platform_read_file(filename, &data, &length);
	if (!loaded) {
		log_warnf("Sound file failed to load: %s", filename);
		return nullptr;
	}

	result = (_sound_t*)assets_allocate(asset_type_sound);
	result->type = sound_type_decode;
	sound_set_id(result, filename);

	ma_decoder_config config = ma_decoder_config_init(AU_SAMPLE_FORMAT, 1, AU_SAMPLE_RATE);
	if (ma_decoder_init_memory(data, length, &config, &result->decoder) != MA_SUCCESS) {
		log_errf("Failed to parse sound '%s'.", filename);
		assets_releaseref(&result->header);
		return nullptr;
	}

	result->decibels = 80;
	return result;
}

///////////////////////////////////////////

sound_t sound_create_stream(float buffer_duration) {
	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);

	result->type      = sound_type_stream;
	result->data_lock = ft_mutex_create();
	result->buffer.capacity = (uint64_t)((double)buffer_duration * AU_SAMPLE_RATE);
	result->buffer.data     = sk_malloc_t(float, (size_t)result->buffer.capacity);
	memset(result->buffer.data, 0, (size_t)(result->buffer.capacity * sizeof(float)));
	ma_pcm_rb_init(AU_SAMPLE_FORMAT, 1, (ma_uint32)result->buffer.capacity, result->buffer.data, nullptr, &result->stream_buffer);

	result->decibels = 80;
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

	result->decibels = 80;
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

	result->decibels = 80;
	return result;
}

///////////////////////////////////////////

void sound_write_samples(sound_t sound, const float *samples, uint64_t sample_count) {
	if (sound->type != sound_type_stream) { log_err("Sound read/write is only supported for streaming type sounds!"); return; }

	sample_count = mini((uint32_t)sample_count, ma_pcm_rb_available_write(&sound->stream_buffer));
	
	ma_uint32 written  = 0;
	ft_mutex_lock(sound->data_lock);
	ma_uint32 writable = 0;
	void*     write_to = nullptr;
	while (written < sample_count) {
		writable = (ma_uint32)sample_count - written;
		
		ma_result res = ma_pcm_rb_acquire_write(&sound->stream_buffer, &writable, &write_to);
		if (res != MA_SUCCESS) { break; }
		memcpy(write_to, samples+written, (size_t)(writable * sizeof(float)));
		
		res = ma_pcm_rb_commit_write(&sound->stream_buffer, writable);
		if (res != MA_SUCCESS) { break;}
		
		written += writable;
	}
	ft_mutex_unlock(sound->data_lock);

	sound->buffer.count = mini(sound->buffer.count + written, sound->buffer.capacity);
}

///////////////////////////////////////////

uint64_t sound_read_samples(sound_t sound, float *out_samples, uint64_t sample_count) {
	if (sound->type != sound_type_stream) { log_err("Sound read/write is only supported for streaming type sounds!"); return 0; }

	ma_uint32 available = ma_pcm_rb_available_read(&sound->stream_buffer);
	sample_count = mini((uint32_t)sample_count, available);

	ma_uint32 read  = 0;
	ft_mutex_lock(sound->data_lock);
	ma_uint32 readable  = 0;
	void*     read_from = nullptr;
	while (read < sample_count) {
		readable = (ma_uint32)sample_count - read;
		
		ma_result res = ma_pcm_rb_acquire_read(&sound->stream_buffer, &readable, &read_from);
		if (res != MA_SUCCESS) { break; }
		
		memcpy(out_samples+read, read_from, (size_t)(readable * sizeof(float)));
		
		res = ma_pcm_rb_commit_read(&sound->stream_buffer, readable);
		if (res != MA_SUCCESS && res != MA_AT_END) { break; }
		
		read += readable;
	}
	ft_mutex_unlock(sound->data_lock);

	return read;
}

///////////////////////////////////////////

uint64_t sound_unread_samples(sound_t sound) {
	return sound->type == sound_type_stream
		? ma_pcm_rb_available_read(&sound->stream_buffer)
		: 0;
}

///////////////////////////////////////////

float sound_get_decibels(sound_t sound) {
	return sound->decibels;
}

///////////////////////////////////////////

void sound_set_decibels(sound_t sound, float decibels) {
	sound->decibels = decibels;
}

///////////////////////////////////////////

float decibels_to_signal(float decibel) {
	// North American standard uses reference levels of 83db as -20dbfs, which
	// basically means that an 83db sound will result in a dbfs of -20, which
	// in turn is a digital signal value of 0.1. A dbfs of 0 will result in a
	// digital signal value of 1, and a -inf will result in a digital signal of
	// 0. The -20 reference level then provides a 20db headroom above 83 before
	// clipping starts happening.
	const float reference_dbfs = -20;
	const float reference_db   =  83;
	float dbfs = reference_dbfs + (decibel - reference_db);

	// This converts dbfs to a digital signal +-1 representation
	// return powf(10, dbfs / 20.0f);

	// This is the same as the powf call, but faster
	const float LN10_DIV_20 = 0.115129254f;
	return expf(dbfs * LN10_DIV_20);
}

///////////////////////////////////////////

sound_inst_t sound_play(sound_t sound, vec3 at, float volume) {
	ma_decoder_seek_to_pcm_frame(&sound->decoder, 0);
	sound->buffer.cursor = 0;

	int16_t slot = -1;
	for (int32_t i = 0; i < audio_get_inst_count(); i++) {
		_sound_inst_t* inst = audio_get_inst_slot(i);
		if (              inst->sound == sound)   { slot = i; break; }
		if (slot == -1 && inst->sound == nullptr) { slot = i; }
	}
	if (slot == -1) return sound_inst_t{0, -1};

	_sound_inst_t* dest = audio_get_inst_slot(slot);
	sound_addref (sound);
	sound_release(dest->sound);

	//*dest = {sound, (uint16_t)(dest->id+1), at, decibels_to_signal(sound->decibels * volume) };
	*dest = { sound, (uint16_t)(dest->id + 1), at, volume };
	return sound_inst_t{dest->id, slot};
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
	return sound->type == sound_type_stream
		? sound->buffer.count-ma_pcm_rb_pointer_distance(&sound->stream_buffer)
		: sound->buffer.cursor;
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
	if (sound->type == sound_type_stream) {
		ma_pcm_rb_uninit(&sound->stream_buffer);
	}
	if (sound->buffer.data) {
		sk_free         ( sound->buffer.data);
		ft_mutex_destroy(&sound->data_lock);
	}
	memset(sound, 0, sizeof(_sound_t));
}

///////////////////////////////////////////

void sound_inst_stop(sound_inst_t sound_inst) {
	_sound_inst_t* inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	if (!inst) return;

	sound_t sound = inst->sound;
	inst->sound = nullptr;
	sound_release(sound);
}

///////////////////////////////////////////

bool32_t sound_inst_is_playing(sound_inst_t sound_inst) {
	_sound_inst_t *inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	return inst
		? inst->sound != nullptr
		: false;
}

///////////////////////////////////////////
void sound_inst_set_pos(sound_inst_t sound_inst, vec3 pos) {
	_sound_inst_t* inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	if (inst)
		inst->position = pos;
}

///////////////////////////////////////////

vec3 sound_inst_get_pos(sound_inst_t sound_inst) {
	_sound_inst_t* inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	return inst
		? inst->position
		: vec3_zero;
}

///////////////////////////////////////////

void sound_inst_set_volume(sound_inst_t sound_inst, float volume) {
	_sound_inst_t* inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	if (inst)
		inst->volume = volume;
}

///////////////////////////////////////////

float sound_inst_get_volume(sound_inst_t sound_inst) {
	_sound_inst_t* inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	return inst
		? inst->volume
		: 0;
}

///////////////////////////////////////////

float sound_inst_get_intensity(sound_inst_t sound_inst) {
	_sound_inst_t* inst = audio_get_inst_gen(sound_inst._slot, sound_inst._id);
	return inst
		? inst->intensity_max_frame
		: 0;
}

}