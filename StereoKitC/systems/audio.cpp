#include "audio.h"
#include "../asset_types/sound.h"

#include "../sk_memory.h"
#include "../sk_math.h"
#include "../platforms/platform_utils.h"

#include "../libraries/miniaudio.h"
#include "../libraries/stref.h"
#include "../libraries/isac_spatial_sound.h"

#include <string.h>
#include <assert.h>

namespace sk {

_sound_inst_t     au_active_sounds[8] = {};
float             au_mix_temp[4096];
matrix            au_head_transform;

ma_context        au_context        = {};
ma_decoder_config au_decoder_config = {};
ma_device_config  au_config         = {};
ma_device         au_device         = {};
ma_device         au_mic_device     = {};
sound_t           au_mic_sound      = {};
char             *au_mic_name       = nullptr;
bool              au_recording      = false;

///////////////////////////////////////////

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
ma_device_id au_default_device_out_id = {};
ma_device_id au_default_device_in_id  = {};
void audio_set_default_device_out(const wchar_t *device_id) {
	wcscpy_s(au_default_device_out_id.wasapi, device_id);
}
void audio_set_default_device_in(const wchar_t *device_id) {
	wcscpy_s(au_default_device_in_id.wasapi, device_id);
}
#endif

///////////////////////////////////////////

ma_uint32 read_and_mix_pcm_frames_f32(_sound_inst_t &inst, float *output, ma_uint64 frame_count) {
	const uint64_t channel_count = 2;

	// The way mixing works is that we just read into a temporary buffer, 
	// then take the contents of that buffer and mix it with the contents of
	// the output buffer by simply adding the samples together.
	vec3      head_pos          = input_head()->position;
	vec3      head_right        = vec3_normalize(input_head()->orientation * vec3_right);
	ma_uint64 frame_cap         = _countof(au_mix_temp) / channel_count;
	ma_uint64 total_frames_read = 0;

	while (total_frames_read < frame_count) {
		ma_uint64 frames_remaining = frame_count - total_frames_read;
		ma_uint64 frames_to_read   = frame_cap;
		if (frames_to_read > frames_remaining) {
			frames_to_read = frames_remaining;
		}

		// Grab sound samples!
		ma_uint64 frames_read = 0;
		switch (inst.sound->type) {
		case sound_type_decode: {
			if (ma_decoder_read_pcm_frames(&inst.sound->decoder, au_mix_temp, frames_to_read, &frames_read) != MA_SUCCESS) {
				log_err("Failed to read PCM frames for mixing!");
			}
		} break;
		case sound_type_stream: {
			frames_read = sound_read_samples(inst.sound, au_mix_temp, frames_to_read);
		} break;
		case sound_type_buffer: {
			frames_read = mini(frames_to_read, inst.sound->buffer.count - inst.sound->buffer.cursor);
			memcpy(au_mix_temp, inst.sound->buffer.data+inst.sound->buffer.cursor, (size_t)frames_read * sizeof(float));
			inst.sound->buffer.cursor += frames_read;
		} break;
		case sound_type_none: {
			log_errf("Got a sound_type_none?");
		} break;
		}
		if (frames_read <= 1) break;

		//// Mix the sound samples in

		// Calculate the volume based on distance using the 1/d^2 law
		vec3  dir    = head_pos - inst.position;
		float dist2  = vec3_magnitude_sq(dir);
		float volume = fminf(1,(1.f / dist2) * inst.volume);

		// Find the direction of the sound in relation to the head
		dir = dir / sqrtf(dist2);
		float dot = vec3_dot(dir, head_right);

		// Calculate a panning volume where a sound source directly in front
		// will have a volume of 1 for both ears, and a sound directly to 
		// either side will have a volume of zero opposite it
		float channel[2] = { fminf(1,dot+1), fminf(1,2-(dot+1)) };

		// Create a sample offset to simulate sound arrival time difference
		// between left and right ears.
		// NOTE: This needs a buffer of 10 samples before and after the
		// relevant range, otherwise it sparkles!!
		// The speed of sound is 343 m/s, and average head width is .15m
		// (head_width/speed_of_sound)*48,000 samples/s = 21 audio samples wide
		//int64_t offset[2] = { (int64_t)(10 * dot), (int64_t)(-10 * dot) };

		// Mix the frames together.
		for (ma_uint64 sample = 0; sample < frames_read; ++sample) {
			ma_uint64 i = total_frames_read * channel_count + sample*channel_count;
			float     s = au_mix_temp[sample] * volume;

			// Right channel
			//float     s = au_mix_temp[mini((int64_t)_countof(au_mix_temp)-1,maxi((int64_t)0,sample+offset[c]))] * volume *  channel[0];
			output[i] = fmaxf(-1, fminf(1, output[i] + s*channel[0]));

			// Left channel
			//s = au_mix_temp[mini((int64_t)_countof(au_mix_temp)-1,maxi((int64_t)0,sample+offset[c]))] * volume *  channel[1];
			i += 1;
			output[i] = fmaxf(-1, fminf(1, output[i] + s*channel[1]));
		}

		total_frames_read += frames_read;
		if (frames_read < frames_to_read) {
			break;  // Reached EOF.
		}
	}

	return (ma_uint32)total_frames_read;
}

///////////////////////////////////////////

void data_callback(ma_device*, void* output, const void*, ma_uint32 frame_count) {
	float* output_f = (float*)output;

	for (uint32_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == nullptr)
			continue;

		ma_uint32 framesRead = read_and_mix_pcm_frames_f32(au_active_sounds[i], output_f, frame_count);
		if (framesRead < frame_count && au_active_sounds[i].sound->type != sound_type_stream) {
			sound_release(au_active_sounds[i].sound);
			au_active_sounds[i].sound = nullptr;
		}
	}
}

///////////////////////////////////////////

ma_uint64 read_data_for_isac(_sound_inst_t& inst, float* output, ma_uint64 frame_count, vec3* position, float* volume) {
	// Set the position and volume for this object. ISAC applies this directly for us
	*position = matrix_transform_pt(au_head_transform, inst.position);
	*volume   = inst.volume;

	ma_uint64 frame_cap         = _countof(au_mix_temp);
	ma_uint64 total_frames_read = 0;

	while (total_frames_read < frame_count) {
		ma_uint64 frames_remaining = frame_count - total_frames_read;
		ma_uint64 frames_to_read = frame_cap;
		if (frames_to_read > frames_remaining) {
			frames_to_read = frames_remaining;
		}

		ma_uint64 frames_read = 0;
		switch (inst.sound->type) {
		case sound_type_decode: {
			if (ma_decoder_read_pcm_frames(&inst.sound->decoder, au_mix_temp, frames_to_read, &frames_read) != MA_SUCCESS) {
				log_err("Failed to read PCM frames for ISAC!");
			}
		} break;
		case sound_type_stream: {
			frames_read = sound_read_samples(inst.sound, au_mix_temp, frames_to_read);
		} break;
		case sound_type_buffer: {
			frames_read = mini(frames_to_read, inst.sound->buffer.count - inst.sound->buffer.cursor);
			memcpy(au_mix_temp, inst.sound->buffer.data+inst.sound->buffer.cursor, (size_t)frames_read * sizeof(float));
			inst.sound->buffer.cursor += frames_read;
		} break;
		case sound_type_none: {
			log_errf("Got a sound_type_none?");
		} break;
		}
		if (frames_read <= 1) break;

		// Read the data into the buffer provided by ISAC
		memcpy(&output[total_frames_read], au_mix_temp, (size_t)frames_read * sizeof(float));

		total_frames_read += frames_read;
		if (frames_read < frames_to_read) {
			break;  // Reached EOF.
		}
	}

	return total_frames_read;
}

///////////////////////////////////////////

void isac_data_callback(float** sourceBuffers, uint32_t numSources, uint32_t numFrames, vec3* positions, float* volumes) {
	// Assert on debug builds, eliminate warning on release builds
	//UNREFERENCED_PARAMETER(numSources);
	assert(numSources == _countof(au_active_sounds));

	for (uint32_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == nullptr)
			continue;

		ma_uint64 framesRead = read_data_for_isac(au_active_sounds[i], sourceBuffers[i], numFrames, &positions[i], &volumes[i]);
		if (framesRead < numFrames && au_active_sounds[i].sound->type != sound_type_stream) {
			sound_release(au_active_sounds[i].sound);
			au_active_sounds[i].sound = nullptr;
		}
	}
}

///////////////////////////////////////////

int32_t mic_device_count() {
	ma_uint32 capture_count = 0;
	if (ma_context_get_devices(&au_context, nullptr, nullptr, nullptr, &capture_count) != MA_SUCCESS) {
		return 0;
	}
	return capture_count;
}

///////////////////////////////////////////

const char *mic_device_name(int32_t index) {
	ma_device_info *capture_devices = nullptr;
	ma_uint32       capture_count   = 0;
	if (ma_context_get_devices(&au_context, nullptr, nullptr, &capture_devices, &capture_count) != MA_SUCCESS) {
		return nullptr;
	}
	if (index >= 0 && index < (int32_t)capture_count)
		return capture_devices[index].name;
	return nullptr;
}

///////////////////////////////////////////

void mic_callback(ma_device*, void*, const void* input, ma_uint32 frame_count) {
	if (input == nullptr || au_mic_sound == nullptr) return;

	sound_write_samples(au_mic_sound, (float*)input, frame_count);
}

///////////////////////////////////////////

bool32_t mic_start(const char *device_name) {
	// Make sure we're not starting up an already recording mic
	if (au_recording) {
		if (device_name == nullptr) {
			if (au_mic_name == nullptr)
				return true;
		} else if (au_mic_name != nullptr && strcmp(device_name, au_mic_name) == 0) {
			return true;
		}
		mic_stop();
	}
	au_mic_name = device_name == nullptr 
		? nullptr
		: string_copy(device_name);

	// Find the id of the mic based on the given name
	ma_device_id *id = nullptr;
	if (device_name != nullptr) {
		ma_device_info *capture_devices = nullptr;
		ma_uint32       capture_count   = 0;
		if (ma_context_get_devices(&au_context, nullptr, nullptr, &capture_devices, &capture_count) != MA_SUCCESS) {
			return false;
		}
		for (ma_uint32 i = 0; i < capture_count; i++) {
			if (strcmp(capture_devices[i].name, au_mic_name) == 0) {
				id = &capture_devices[i].id;
				break;
			}
		}
	}

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	if (id == nullptr && au_default_device_in_id.wasapi[0] != '\0')
		id = &au_default_device_in_id;
#endif

	// Start up the mic
	ma_device_config config   = ma_device_config_init(ma_device_type_capture);
	config.capture.pDeviceID  = id;
	config.capture.format     = AU_SAMPLE_FORMAT;
	config.capture.channels   = 1;
	config.sampleRate         = AU_SAMPLE_RATE;
	config.dataCallback       = mic_callback;
	config.pUserData          = nullptr;
	ma_result result = ma_device_init(&au_context, &config, &au_mic_device);
	if (result != MA_SUCCESS) {
		log_warnf("Mic start failed, '%s'", ma_result_description(result));
		return false;
	}
	ma_device_start(&au_mic_device);

	// And make sure we have a streaming sound to store mic data in
	mic_get_stream();

	au_recording = true;
	return true;
}

///////////////////////////////////////////

void mic_stop() {
	if (!au_recording) return;

	sk_free(au_mic_name);
	au_mic_name = nullptr;
	ma_device_stop  (&au_mic_device);
	ma_device_uninit(&au_mic_device);
	memset(&au_mic_device, 0, sizeof(au_mic_device));
	au_recording = false;
	sound_release(au_mic_sound);
}

///////////////////////////////////////////

sound_t mic_get_stream() {
	if (au_mic_sound == nullptr) {
		au_mic_sound = sound_create_stream(0.5f);
		sound_set_id(au_mic_sound, "sk/mic_sound");
	}
	sound_addref(au_mic_sound);
	return au_mic_sound;
}

///////////////////////////////////////////

bool32_t mic_is_recording() {
	return au_recording;
}

///////////////////////////////////////////

bool audio_init() {
	memset(au_mix_temp, 0, sizeof(au_mix_temp));

	if (ma_context_init(nullptr, 0, nullptr, &au_context) != MA_SUCCESS) {
		return false;
	}

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	if (au_default_device_out_id.wasapi[0] == '\0') {
		HRESULT hr = isac_activate(_countof(au_active_sounds), isac_data_callback);

		if (SUCCEEDED(hr)) {
			log_info("Using audio backend: ISAC");
			return true;
		} else if (hr == E_NOT_VALID_STATE) {
			log_diag("ISAC audio backend not available, falling back to miniaudio! It's likely the device doesn't have Windows Sonic enabled, which can be found under Settings->Sound->Device Properties->Spatial Sound.");
		} else {
			log_warnf("ISAC audio backend failed 0x%X, falling back to miniaudio!", hr);
		}
	}
#endif

	au_config = ma_device_config_init(ma_device_type_playback);
	au_config.playback.format   = AU_SAMPLE_FORMAT;
	au_config.playback.channels = 2;
	au_config.sampleRate        = AU_SAMPLE_RATE;
	au_config.dataCallback      = data_callback;
	au_config.pUserData         = nullptr;

	// If we've requested a specific output device, like Oculus requires,
	// we set that up here.
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	if (au_default_device_out_id.wasapi[0] != '\0') {
		au_config.playback.pDeviceID = &au_default_device_out_id;
	}
#endif

	ma_result result = ma_device_init(&au_context, &au_config, &au_device);
	if (result != MA_SUCCESS) {
		log_warnf("Failed to open audio playback device, '%s'.", ma_result_description(result));
		
		// Make a desperate attempt to fall back to a null device.
		ma_backend backend = ma_backend_null;
		if (ma_context_init(&backend, 1, nullptr, &au_context) != MA_SUCCESS) {
			return false;
		}
		result = ma_device_init(&au_context, &au_config, &au_device);
		
		// Even the null device failed, so let's stop.
		if (result != MA_SUCCESS) {
			log_errf("Failed to open null audio playback device, '%s'.", ma_result_description(result));
			return false;
		}
	}

	result = ma_device_start(&au_device);
	if (result != MA_SUCCESS) {
		log_errf("Failed to start audio playback device, '%s'.", ma_result_description(result));
		ma_device_uninit(&au_device);
		return false;
	}

	au_mic_name = nullptr;

	log_infof("Using audio backend: %s", ma_get_backend_name(au_device.pContext->backend));
	return true;
}

///////////////////////////////////////////

void audio_update() {
	matrix head = pose_matrix(*input_head());
	matrix_inverse(head, au_head_transform);
}

///////////////////////////////////////////

void audio_shutdown() {
	// Stop any sounds that are still playing
	for (int32_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound != nullptr) {
			sound_inst_t inst;
			inst._id   = au_active_sounds[i].id;
			inst._slot = (int16_t)i;
			sound_inst_stop(inst);
		}
	}

	mic_stop();
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	isac_destroy();
#endif
	ma_device_uninit (&au_device);
	ma_context_uninit(&au_context);

	sound_release(au_mic_sound);
	au_mic_sound = nullptr;
}

}