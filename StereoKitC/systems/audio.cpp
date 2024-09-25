#include "audio.h"
#include "../asset_types/sound.h"

#include "../sk_memory.h"
#include "../sk_math.h"
#include "../platforms/platform.h"

#include "../libraries/stref.h"
#include "../libraries/isac_spatial_sound.h"

#include <string.h>
#include <assert.h>

namespace sk {

_sound_inst_t     au_active_sounds[8] = {};
matrix            au_head_transform;
int32_t           au_mix_temp_size = 4096;
float*            au_mix_temp;

ma_context        au_context        = {};
ma_decoder_config au_decoder_config = {};
ma_device_config  au_config         = {};
ma_device         au_device         = {};
ma_device         au_mic_device     = {};
sound_t           au_mic_sound      = {};
char             *au_mic_name       = nullptr;
bool              au_recording      = false;
bool              au_paused         = false;

///////////////////////////////////////////

#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

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

float low_pass_filter(float input, float cutoff_freq, float previous_output) {
	float RC = 1.0f / (2.0f * 3.14159265359f * cutoff_freq);
	float dt = 1.0f / AU_SAMPLE_RATE;
	float alpha = dt / (RC + dt);

	// Calculate the new output based on the previous output
	float output = previous_output + alpha * (input - previous_output);

	return output;
}

///////////////////////////////////////////

ma_uint32 read_and_mix_pcm_frames_f32(_sound_inst_t &inst, float *output, ma_uint64 frame_count) {
	// The way mixing works is that we just read into a temporary buffer,
	// then take the contents of that buffer and mix it with the contents of
	// the output buffer by simply adding the samples together.
	vec3 head_pos     = input_head()->position;
	vec3 head_right   = vec3_normalize(input_head()->orientation * vec3_right);
	vec3 head_forward = vec3_normalize(input_head()->orientation * vec3{0,0,1});

	// Calculate the volume based on distance using 1/d. While sound
	// "intensity" is best modeled with 1/d^2, sound percieved loudness
	// comes from sound amplitude/pressure, which falls off as 1/d.
	vec3  dir    = head_pos - inst.position;
	float dist2  = vec3_magnitude_sq(dir);
	float dist   = fmaxf(0.001f, sqrtf(dist2));
	float volume = fminf(1, (1.f / dist) * inst.volume);

	// Find the direction of the sound in relation to the head, these vectors
	// are normalized so the result is -1 to +1, and made to be more "linear"
	float dot_pan   = asinf(vec3_dot(dir / dist, head_right  )) / (3.14159f*0.5f);
	float dot_front = asinf(vec3_dot(dir / dist, head_forward)) / (3.14159f*0.5f);

	// Calculate a panning volume where a sound source directly in front
	// will have a volume of 1 for both ears, and a sound directly to
	// either side will have a volume of 40% opposite it. Technically low
	// frequencies and high frequencies drop at different rates in head shadow,
	// but we're using a single simple approximate instead. (low freq 10-20%
	// loss, high freq 50-60% loss?)
	const float pan_loss = 0.6f;
	float pan_volume[2] = { fminf(1,1+(dot_pan*pan_loss)) * volume, fminf(1,1-(dot_pan*pan_loss)) * volume };

	// Make sure we have enough room for all our samples.
	if (au_mix_temp_size < frame_count + AU_SAMPLE_BUFFER_SIZE * 2) {
		au_mix_temp_size = frame_count + AU_SAMPLE_BUFFER_SIZE * 2;
		sk_free(au_mix_temp);
		au_mix_temp = sk_malloc_t(float, au_mix_temp_size);
	}

	// Reading samples is a bit odd here because we can only read forward from
	// some of our audio sources, and we need about 10 extra samples on either
	// side of our audio in order to do proper 3D audio panning. So we cache a
	// few samples from the previous read, and read a bit extra at the end as
	// well.
	//
	// Here, | is the audio start/end, O is the extra padding at the end, and v
	// is pulled from the last step's cache.
	// 
	// |====O         |
	// |   vv===O     |
	// |       vv===O |
	// |           vv=|
	int32_t new_at   = 0;
	int32_t start_at = 0;
	if (inst.prev_buffer_ct > 0) {
		memcpy(au_mix_temp, inst.prev_buffer, sizeof(float) * inst.prev_buffer_ct);
		new_at   = inst.prev_buffer_ct;
		start_at = mini(inst.prev_buffer_ct, AU_SAMPLE_BUFFER_SIZE);
	}

	// Grab sound samples!
	ma_uint64 frames_read   = 0;
	ma_uint64 frames_needed = (frame_count+AU_SAMPLE_BUFFER_SIZE) - (new_at-start_at);
	switch (inst.sound->type) {
	case sound_type_decode: {
		if (ma_decoder_read_pcm_frames(&inst.sound->decoder, &au_mix_temp[new_at], frames_needed, &frames_read) != MA_SUCCESS) {
			log_err("Failed to read PCM frames for mixing!");
		}
	} break;
	case sound_type_stream: {
		frames_read = sound_read_samples(inst.sound, &au_mix_temp[new_at], frames_needed);
	} break;
	case sound_type_buffer: {
		frames_read = mini(frames_needed, inst.sound->buffer.count - inst.sound->buffer.cursor);
		memcpy(&au_mix_temp[new_at], inst.sound->buffer.data+inst.sound->buffer.cursor, (size_t)frames_read * sizeof(float));
		inst.sound->buffer.cursor += frames_read;
	} break;
	case sound_type_none: {
		log_errf("Got a sound_type_none?");
	} break;
	}

	int32_t mix_count = mini((int32_t)frame_count, (int32_t)(new_at+frames_read) - start_at);
	int32_t end_at    = start_at + mix_count;
	int32_t total     = new_at + frames_read;

	// Create a sample offset to simulate sound arrival time difference
	// between left and right ears.
	// NOTE: This needs a buffer of 10 samples before and after the
	// relevant range, otherwise it sparkles!!
	// The speed of sound is 343 m/s, and average head width is .15m
	// (head_width/speed_of_sound)*48,000 samples/s = 21 audio samples wide
	int32_t offset[2] = { (int32_t)(10 * dot_pan), (int32_t)(-10 * dot_pan) };
	int32_t start [2] = { maxi(0,     start_at + inst.prev_offset[0]), maxi(0,     start_at + inst.prev_offset[1]) };
	int32_t end   [2] = { mini(total, end_at   + offset[0]),           mini(total, end_at   + offset[1]) };
	float   step  [2] = { (float)(end[0]-start[0]) / frame_count, (float)(end[1]-start[1]) / frame_count };
	float   curr  [2] = { (float)start[0], (float)start[1] };

	// Sounds behind the user should get a low pass filter
	float prev   = au_mix_temp[maxi(0, new_at - 1)];
	float cutoff = fminf(
		fmaxf(1000, -4000.f * log(fmaxf(1,dist-3.5f)) + 22200), // distance diminishes higher frequencies: https://www.desmos.com/calculator/h5tssewqbl
		math_lerp(2500, 30000, fminf(1, 1 + dot_front))); // So does being behind the listener
	float RC     = 1.0f / (2.0f * 3.14159265359f * cutoff);
	float dt     = 1.0f / AU_SAMPLE_RATE;
	float alpha  = dt / (RC + dt);
	for (int32_t i = new_at; i < total; i++) {
		if (inst.intensity_max_last_read < au_mix_temp[i])
			inst.intensity_max_last_read = au_mix_temp[i];

		au_mix_temp[i] = prev + alpha * (au_mix_temp[i] - prev);
		prev = au_mix_temp[i];
	}

	// Mix the frames together.
	for (int32_t sample = 0; sample < mix_count; sample+=1) {
		// Right channel
		float   sr = au_mix_temp[(int32_t)curr[0]];
		int32_t ir = sample * 2;
		output[ir] = fmaxf(-1, fminf(1, output[ir] + sr*pan_volume[0]));

		// Left channel
		float   sl = au_mix_temp[(int32_t)curr[1]];
		int32_t il = sample * 2 + 1;
		output[il] = fmaxf(-1, fminf(1, output[il] + sl*pan_volume[1]));

		curr[0] += step[0];
		curr[1] += step[1];
	}

	// Cache the last few samples for implementing our sample buffer
	int64_t count = mini((int64_t)AU_SAMPLE_BUFFER_SIZE*2, (int64_t)(new_at+frames_read));
	memcpy(inst.prev_buffer, &au_mix_temp[(new_at + frames_read) - count], count * sizeof(float));
	inst.prev_buffer_ct = count;
	inst.prev_offset[0] = offset[0];
	inst.prev_offset[1] = offset[1];

	return (ma_uint32)mix_count;
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

	ma_uint64 frame_cap         = au_mix_temp_size;
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
	au_mix_temp = sk_malloc_t(float, au_mix_temp_size);
	memset(au_mix_temp, 0, sizeof(float) * au_mix_temp_size);

	if (ma_context_init(nullptr, 0, nullptr, &au_context) != MA_SUCCESS) {
		return false;
	}

#if defined(_MSC_VER)
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

void audio_step() {
	matrix head = pose_matrix(*input_head());
	matrix_inverse(head, au_head_transform);

	for (size_t i = 0; i < _countof(au_active_sounds); i++) {
		au_active_sounds[i].intensity_max_frame     = au_active_sounds[i].intensity_max_last_read;
		au_active_sounds[i].intensity_max_last_read = 0;
	}
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
#if defined(_MSC_VER)
	isac_destroy();
#endif
	ma_device_uninit (&au_device);
	ma_context_uninit(&au_context);

	sound_release(au_mic_sound);
	au_mic_sound = nullptr;
	sk_free(au_mix_temp);
}

///////////////////////////////////////////

void audio_pause() {
	if (au_paused) return;

	au_paused = true;
	ma_device_stop  (&au_device);
	ma_device_uninit(&au_device);
}

///////////////////////////////////////////

void audio_resume() {
	if (!au_paused) return;

	au_paused = false;
	ma_device_init (&au_context, &au_config, &au_device);
	ma_device_start(&au_device);
}

}