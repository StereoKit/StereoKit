#include "audio.h"
#include "../asset_types/sound.h"

#include "../sk_memory.h"
#include "../systems/platform/platform_utils.h"

#define DR_WAV_IMPLEMENTATION
#include "../libraries/dr_wav.h"   /* Enables WAV decoding. */

#define MA_NO_OPENSL
#define MINIAUDIO_IMPLEMENTATION
#include "../libraries/miniaudio.h"

#include "../libraries/isac_spatial_sound.h"

namespace sk {

sound_inst_t      au_active_sounds[8];
float             au_mix_temp[4096];
matrix            au_head_transform;

ma_decoder_config au_decoder_config = {};
ma_device_config  au_config         = {};
ma_device         au_device         = {};
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
IsacAdapter*      isac_adapter      = nullptr;
#endif

///////////////////////////////////////////

ma_uint32 read_and_mix_pcm_frames_f32(sound_inst_t &inst, float* pOutputF32, ma_uint32 frame_count) {
	// The way mixing works is that we just read into a temporary buffer, then take the contents of that buffer and mix it with the
	// contents of the output buffer by simply adding the samples together. You could also clip the samples to -1..+1, but I'm not
	// doing that in this example.

	vec3 head_pos = input_head()->position;

	ma_uint32 tempCapInFrames = _countof(au_mix_temp) / AU_CHANNEL_COUNT;
	ma_uint32 total_frames_read = 0;

	while (total_frames_read < frame_count) {
		ma_uint32 frames_remaining = frame_count - total_frames_read;
		ma_uint32 frames_to_read   = tempCapInFrames;
		if (frames_to_read > frames_remaining) {
			frames_to_read = frames_remaining;
		}

		ma_uint32 frames_read = (ma_uint32)ma_decoder_read_pcm_frames(&inst.sound->decoder, au_mix_temp, frames_to_read);
		if (frames_read == 0) {
			break;
		}

		float dist   = vec3_magnitude(inst.position - head_pos);
		float volume = fminf(1,(1.f / dist) * inst.volume);

		// Mix the frames together.
		for (ma_uint32 sample = 0; sample < frames_read*AU_CHANNEL_COUNT; ++sample) {
			int i = total_frames_read * AU_CHANNEL_COUNT + sample;
			pOutputF32[i] = fmaxf(-1, fminf(1, pOutputF32[i] + au_mix_temp[sample] * volume));
		}

		total_frames_read += frames_read;
		if (frames_read < frames_to_read) {
			break;  // Reached EOF.
		}
	}

	return total_frames_read;
}

///////////////////////////////////////////

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	float* pOutputF32 = (float*)pOutput;

	ma_assert(pDevice->playback.format == AU_SAMPLE_FORMAT);   // <-- Important for this example.

	for (uint32_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == nullptr)
			continue;

		ma_uint32 framesRead = read_and_mix_pcm_frames_f32(au_active_sounds[i], pOutputF32, frameCount);
		if (framesRead < frameCount) {
			sound_release(au_active_sounds[i].sound);
			au_active_sounds[i].sound = nullptr;
		}
	}

	(void)pInput;
}

///////////////////////////////////////////

ma_uint32 readDataForIsac(sound_inst_t& inst, float* pOutputF32, ma_uint32 frame_count, vec3* position, float* volume) {
	// Set the position and volume for this object. ISAC applies this directly for us
	*position = matrix_mul_point(au_head_transform, inst.position);
	*volume   = inst.volume;

	ma_uint32 tempCapInFrames = _countof(au_mix_temp) / AU_CHANNEL_COUNT;
	ma_uint32 total_frames_read = 0;

	while (total_frames_read < frame_count) {
		ma_uint32 frames_remaining = frame_count - total_frames_read;
		ma_uint32 frames_to_read = tempCapInFrames;
		if (frames_to_read > frames_remaining) {
			frames_to_read = frames_remaining;
		}

		ma_uint32 frames_read = (ma_uint32)ma_decoder_read_pcm_frames(&inst.sound->decoder, au_mix_temp, frames_to_read);
		if (frames_read == 0) {
			break;
		}

		// Read the data into the buffer provided by ISAC
		memcpy(&pOutputF32[total_frames_read * AU_CHANNEL_COUNT], au_mix_temp, frames_read * AU_CHANNEL_COUNT * sizeof(float));

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

		ma_uint32 framesRead = readDataForIsac(au_active_sounds[i], sourceBuffers[i], numFrames, &positions[i], &volumes[i]);
		if (framesRead < numFrames) {
			sound_release(au_active_sounds[i].sound);
			au_active_sounds[i].sound = nullptr;
		}
	}
}

///////////////////////////////////////////

bool audio_init() {
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	isac_adapter = new IsacAdapter(_countof(au_active_sounds));
	HRESULT hr = isac_adapter->Activate(isac_data_callback);

	if (SUCCEEDED(hr)) {
		log_diag("Using audio backend: ISAC");
		return true;
	} else if (hr == E_NOT_VALID_STATE){
		log_diag("ISAC not available, falling back to miniaudio! It's likely the device doesn't have Windows Sonic enabled, which can be found under Settings->Sound->Device Properties->Spatial Sound.");
	} else {
		log_warnf("ISAC failed 0x%X, falling back to miniaudio!", hr);
	}
	delete isac_adapter;
	isac_adapter = nullptr;
#endif

	au_config = ma_device_config_init(ma_device_type_playback);
	au_config.playback.format   = AU_SAMPLE_FORMAT;
	au_config.playback.channels = AU_CHANNEL_COUNT;
	au_config.sampleRate        = AU_SAMPLE_RATE;
	au_config.dataCallback      = data_callback;
	au_config.pUserData         = nullptr;

	if (ma_device_init(NULL, &au_config, &au_device) != MA_SUCCESS) {
		log_err("miniaudio: Failed to open playback device.");
		return false;
	}

	if (ma_device_start(&au_device) != MA_SUCCESS) {
		log_err("miniaudio: Failed to start playback device.");
		ma_device_uninit(&au_device);
		return false;
	}

	log_diagf("miniaudio: using backend %s", ma_get_backend_name(au_device.pContext->backend));
	return true;
}

///////////////////////////////////////////

void audio_update() {
	matrix head = pose_matrix(*input_head());
	matrix_inverse(head, au_head_transform);
}

///////////////////////////////////////////

void audio_shutdown() {
	ma_device_uninit (&au_device);
#if defined(SK_OS_WINDOWS) || defined(SK_OS_WINDOWS_UWP)
	if (isac_adapter)
		delete isac_adapter;
#endif
}

}