#include "audio.h"
#include "input.h"
#include "../asset_types/sound.h"

#include "../sk_memory.h"
#include "../sk_math.h"
#include "../platforms/platform.h"

#include "../libraries/stref.h"
#include "../libraries/profiler.h"
#include "../libraries/atomic_util.h"

#include <string.h>

namespace sk {

ma_context        au_context        = {};
ma_decoder_config au_decoder_config = {};
ma_device_config  au_config         = {};
ma_device         au_device         = {};
ma_device         au_mic_device     = {};
sound_t           au_mic_sound      = {};
char             *au_mic_name       = nullptr;
bool              au_recording      = false;
bool              au_paused         = false;
pose_t            au_listener_override     = {};
bool              au_listener_has_override = false;
double            au_main_clock            = 0;

///////////////////////////////////////////

#if defined(SK_OS_WINDOWS)

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

int32_t mic_device_count() {
	if (au_offline) return 0;

	ma_uint32 capture_count = 0;
	if (ma_context_get_devices(&au_context, nullptr, nullptr, nullptr, &capture_count) != MA_SUCCESS) {
		return 0;
	}
	return capture_count;
}

///////////////////////////////////////////

const char *mic_device_name(int32_t index) {
	if (au_offline) return nullptr;

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

static sound_t mic_stream_ensure(int32_t sample_rate);

bool32_t mic_start(const char *device_name, sound_sample_rate_ sample_rate) {
	if (au_offline) return false;

	// The enum value is the rate in Hz - miniaudio resamples the device to it,
	// so no clamping here. 0 (the default) means StereoKit's native mix rate.
	int32_t rate = sample_rate == sound_sample_rate_default
		? AU_SAMPLE_RATE
		: (int32_t)sample_rate;

	permission_state_ state = permission_state(permission_type_microphone);
	if (state == permission_state_capable) {
		// We can record, but we need to ask permission first!
		permission_type_ mic_permission = permission_type_microphone;
		permission_request(&mic_permission, 1);
		// Chances are good that we'll fail this, mic permission is
		// interactive, and that takes time.
		if (permission_state(permission_type_microphone) != permission_state_granted)
			return false;
	} else if (state != permission_state_granted) {
		log_diag("Recording audio failed due to permissions.");
		return false;
	}

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
			sk_free(au_mic_name);
			au_mic_name = nullptr;
			return false;
		}
		for (ma_uint32 i = 0; i < capture_count; i++) {
			if (strcmp(capture_devices[i].name, au_mic_name) == 0) {
				id = &capture_devices[i].id;
				break;
			}
		}
	}

#if defined(SK_OS_WINDOWS)
	if (id == nullptr && au_default_device_in_id.wasapi[0] != '\0')
		id = &au_default_device_in_id;
#endif

	// Make sure we have a streaming sound to store mic data in. The ref
	// taken here pairs with mic_stop's release, same as the public getter.
	// If the stream already exists at a different rate, capture happens at
	// the stream's rate - a cached stream asset can't change format.
	sound_t stream = mic_stream_ensure(rate);
	if (stream->sample_rate != rate)
		log_warnf("Mic stream already exists at %dhz, ignoring the requested %dhz.", stream->sample_rate, rate);

	// Start up the mic
	ma_device_config config   = ma_device_config_init(ma_device_type_capture);
	config.capture.pDeviceID  = id;
	config.capture.format     = AU_SAMPLE_FORMAT;
	config.capture.channels   = 1;
	config.sampleRate         = (ma_uint32)stream->sample_rate;
	config.dataCallback       = mic_callback;
	config.pUserData          = nullptr;
	ma_result result = ma_device_init(&au_context, &config, &au_mic_device);
	if (result != MA_SUCCESS) {
		log_warnf("Mic start failed, '%s'", ma_result_description(result));
		sound_release(au_mic_sound);
		sk_free(au_mic_name);
		au_mic_name = nullptr;
		return false;
	}
	ma_device_start(&au_mic_device);

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

static sound_t mic_stream_ensure(int32_t sample_rate) {
	if (au_mic_sound == nullptr) {
		au_mic_sound = sound_create_stream(0.5f, sound_channels_mono, (sound_sample_rate_)sample_rate);
		sound_set_id(au_mic_sound, "sk/mic_sound");
	}
	sound_addref(au_mic_sound);
	return au_mic_sound;
}

sound_t mic_get_stream() {
	return mic_stream_ensure(0);
}

///////////////////////////////////////////

bool32_t mic_is_recording() {
	return au_recording;
}

///////////////////////////////////////////

bool audio_init() {
	profiler_zone();

	if (au_offline) {
		// Deterministic test mode: no device or context, blocks are pumped
		// manually through audio_render_block.
		audio_mix_init(4096);
		log_info("Using audio backend: offline");
		return true;
	}

	if (ma_context_init(nullptr, 0, nullptr, &au_context) != MA_SUCCESS) {
		return false;
	}

	au_config = ma_device_config_init(ma_device_type_playback);
	au_config.playback.format   = AU_SAMPLE_FORMAT;
	au_config.playback.channels = 2;
	au_config.sampleRate        = AU_SAMPLE_RATE;
	au_config.dataCallback      = data_callback;
	au_config.pUserData         = nullptr;

	// If we've requested a specific output device, like Oculus requires,
	// we set that up here.
#if defined(SK_OS_WINDOWS)
	if (au_default_device_out_id.wasapi[0] != '\0') {
		au_config.playback.pDeviceID = &au_default_device_out_id;
	}
#endif

	ma_result result = ma_device_init(&au_context, &au_config, &au_device);
	if (result != MA_SUCCESS) {
		log_warnf("Failed to open audio playback device, '%s'.", ma_result_description(result));

		// Make a desperate attempt to fall back to a null device.
		ma_context_uninit(&au_context);
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

	// The mix buffer must be sized before the callback can fire. Some
	// backends deliver more than a period per callback, so leave margin.
	int32_t period = (int32_t)au_device.playback.internalPeriodSizeInFrames;
	audio_mix_init(maxi(period * 2, 4096));

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

void audio_set_listener(const pose_t *opt_pose) {
	au_listener_has_override = opt_pose != nullptr;
	if (opt_pose != nullptr)
		au_listener_override = *opt_pose;
}

///////////////////////////////////////////

void audio_step() {
	profiler_zone();

	au_main_clock += time_stepf_unscaled();
	pose_t listener = au_listener_has_override ? au_listener_override : input_head();
	audio_listener_publish(listener);
	audio_mix_drain_returns();
	sound_play_pending_step();
	audio_voice_prefetch();
	audio_voice_shapes_step(listener.position, time_stepf_unscaled());
	audio_voice_rank();

	for (int32_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voices[i].intensity_frame = atomic_load_f32(&au_voices[i].intensity);
		atomic_store_f32(&au_voices[i].intensity, 0);
	}
}

///////////////////////////////////////////

void audio_shutdown() {
	if (!au_offline) {
		mic_stop();
		// Uninit stops the audio thread, after this the mix can be drained
		// from here safely.
		ma_device_uninit(&au_device);
	}

	audio_mix_shutdown();

	if (!au_offline)
		ma_context_uninit(&au_context);

	sound_release(au_mic_sound);
	au_mic_sound  = nullptr;
	au_main_clock = 0;
}

///////////////////////////////////////////

void audio_pause() {
	if (au_paused || au_offline) return;

	au_paused = true;
	ma_device_stop  (&au_device);
	ma_device_uninit(&au_device);
}

///////////////////////////////////////////

void audio_resume() {
	if (!au_paused || au_offline) return;

	// On failure au_paused stays true, so a later resume can retry.
	ma_result result = ma_device_init(&au_context, &au_config, &au_device);
	if (result != MA_SUCCESS) {
		log_warnf("Failed to resume audio device, '%s'.", ma_result_description(result));
		return;
	}
	result = ma_device_start(&au_device);
	if (result != MA_SUCCESS) {
		log_warnf("Failed to start audio device, '%s'.", ma_result_description(result));
		ma_device_uninit(&au_device);
		return;
	}
	au_paused = false;
}

}
