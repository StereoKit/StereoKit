#include "audio_tests.h"

#include <stereokit.h>
#include <systems/audio.h>

#include <math.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
static void ar_sleep_ms(int32_t ms) { Sleep(ms); }
#else
#include <unistd.h>
static void ar_sleep_ms(int32_t ms) { usleep(ms * 1000); }
#endif

using namespace sk;

// Offline listening renders for spatializer tuning, not a test suite: stereo
// float32 wavs for headphone A/B comparison. The interesting pairs are
// *_direct vs *_bus, the same orbiting source through both render paths.

#define AR_BLOCK 480 // 10ms, a typical device period

///////////////////////////////////////////

static FILE* ar_wav_open(const char* path, uint32_t frames) {
	FILE* file = fopen(path, "wb");
	if (file == nullptr) { log_errf("[audio_render] can't write %s", path); return nullptr; }

	uint32_t data_size  = frames * 2 * sizeof(float);
	uint32_t chunk_size = 36 + data_size;
	uint32_t fmt_size   = 16;
	uint16_t fmt_float  = 3, channels  = 2, align = 8, bits = 32;
	uint32_t rate       = AU_SAMPLE_RATE, byte_rate = rate * align;
	fwrite("RIFF", 1, 4, file); fwrite(&chunk_size, 4, 1, file); fwrite("WAVE", 1, 4, file);
	fwrite("fmt ", 1, 4, file); fwrite(&fmt_size,   4, 1, file);
	fwrite(&fmt_float, 2, 1, file); fwrite(&channels,  2, 1, file);
	fwrite(&rate,      4, 1, file); fwrite(&byte_rate, 4, 1, file);
	fwrite(&align,     2, 1, file); fwrite(&bits,      2, 1, file);
	fwrite("data", 1, 4, file); fwrite(&data_size, 4, 1, file);
	return file;
}

// Ring down the decode's filter/delay state between renders.
static void ar_flush() {
	static float block[AR_BLOCK * 2];
	for (int32_t i = 0; i < 16; i++) {
		audio_render_block(block, AR_BLOCK);
		audio_test_step();
	}
}

///////////////////////////////////////////

// Deterministic white noise from the absolute frame index.
static float ar_noise(uint64_t i) {
	uint32_t h = (uint32_t)i * 2654435761u;
	h ^= h >> 16; h *= 2246822519u; h ^= h >> 13;
	return ((int32_t)(h >> 9) - 4194304) * (1.0f / 4194304.0f);
}

// Noise bursts, 100ms with 5ms edge fades, 4 per second. Broadband sharp
// onsets are the easiest content to localize, so decode blur shows most.
static void ar_gen_bursts(float* out, uint64_t start, uint64_t frames) {
	const uint64_t period = AU_SAMPLE_RATE / 4;
	const uint64_t burst  = AU_SAMPLE_RATE / 10;
	const float    fade   = AU_SAMPLE_RATE * 0.005f;
	for (uint64_t i = 0; i < frames; i++) {
		uint64_t at  = (start + i) % period;
		float    env = 0;
		if (at < burst) {
			env = fminf(1.0f, fminf((float)at / fade, (float)(burst - at) / fade));
		}
		out[i] = ar_noise(start + i) * env;
	}
}

///////////////////////////////////////////

// An orbiting burst source, one revolution per 4s. Horizontal circles the
// head, vertical loops the median plane: front, overhead, behind, below.
static void ar_render_orbit(const char* path, sound_t sound, float radius, bool vertical, bool force_bus) {
	const uint32_t total = AU_SAMPLE_RATE * 8;
	FILE* file = ar_wav_open(path, total);
	if (file == nullptr) return;

	audio_test_force_bus(force_bus);
	sound_play_t settings = {}; settings.flags = sound_flags_loop;
	sound_inst_t inst     = sound_play(sound, vec3{0, 0, -radius}, &settings);

	static float block[AR_BLOCK * 2];
	for (uint32_t at = 0; at < total; at += AR_BLOCK) {
		float ang = ((float)at / AU_SAMPLE_RATE) * (6.2831853f / 4.0f);
		vec3  pos = vertical
			? vec3{0, sinf(ang) * radius, -cosf(ang) * radius}
			: vec3{sinf(ang) * radius, 0, -cosf(ang) * radius};
		sound_inst_set_pos(inst, pos);
		audio_render_block(block, AR_BLOCK);
		fwrite(block, sizeof(float), AR_BLOCK * 2, file);
		audio_test_step();
	}
	fclose(file);

	sound_inst_stop(inst);
	audio_test_force_bus(false);
	ar_flush();
	log_infof("[audio_render] wrote %s", path);
}

// Environment values mirroring the C# AudioEnvironment preset constants.
//                                       wet    decay  damp  size scatter reflect
static const audio_env_t ar_env_off    = {};
static const audio_env_t ar_env_room   = { 0.17f, 0.4f,  0.55f,  7, 0.6f, 0.55f };
static const audio_env_t ar_env_hall   = { 0.22f, 1.4f,  0.45f, 16, 0.7f, 0.55f };
static const audio_env_t ar_env_forest = { 0.11f, 0.5f,  0.9f,  12, 0.9f, 0.12f };
static const audio_env_t ar_env_field  = { 0.05f, 0.25f, 0.9f,   8, 0.7f, 0.06f };

// Bursts orbiting at 3m for 4s, then spiraling out to 12m. Listen for tail
// character, and the direct/reverb balance carrying distance on the way out.
static void ar_render_env(const char* path, sound_t sound, audio_env_t env) {
	const uint32_t total = AU_SAMPLE_RATE * 10;
	const uint32_t hold  = AU_SAMPLE_RATE * 4;
	FILE* file = ar_wav_open(path, total);
	if (file == nullptr) return;

	audio_set_env(env);
	sound_play_t settings = {}; settings.flags = sound_flags_loop;
	sound_inst_t inst     = sound_play(sound, vec3{0, 0, -3}, &settings);

	static float block[AR_BLOCK * 2];
	for (uint32_t at = 0; at < total; at += AR_BLOCK) {
		float ang  = ((float)at / AU_SAMPLE_RATE) * (6.2831853f / 4.0f);
		float dist = at <= hold ? 3.0f
			: 3.0f + 9.0f * (float)(at - hold) / (float)(total - hold);
		sound_inst_set_pos(inst, vec3{sinf(ang) * dist, 0, -cosf(ang) * dist});
		audio_render_block(block, AR_BLOCK);
		fwrite(block, sizeof(float), AR_BLOCK * 2, file);
		audio_test_step();
	}
	fclose(file);

	sound_inst_stop(inst);
	audio_set_env(ar_env_off);
	ar_flush();
	log_infof("[audio_render] wrote %s", path);
}

// An ambisonic bed at the head: 2 seconds facing forward, then a slow full
// turn - the field should stay planted in the world while the head moves.
static void ar_render_ambi(const char* path, sound_t bed) {
	float duration = sound_duration(bed);
	const uint32_t total = (uint32_t)(fminf(duration, 16.0f) * AU_SAMPLE_RATE);
	FILE* file = ar_wav_open(path, total);
	if (file == nullptr) return;

	sound_play_t settings = {}; settings.flags = sound_flags_loop;
	sound_inst_t inst     = sound_play(bed, vec3{0, 0, 0}, &settings);

	const uint32_t hold = AU_SAMPLE_RATE * 2;
	static float block[AR_BLOCK * 2];
	for (uint32_t at = 0; at < total; at += AR_BLOCK) {
		float  yaw  = at <= hold ? 0 : 360.0f * (float)(at - hold) / (float)(total - hold);
		pose_t pose = {{0, 0, 0}, quat_from_angles(0, yaw, 0)};
		audio_set_listener(&pose);
		audio_render_block(block, AR_BLOCK);
		fwrite(block, sizeof(float), AR_BLOCK * 2, file);
		audio_test_step();
	}
	fclose(file);

	sound_inst_stop(inst);
	audio_set_listener(nullptr);
	ar_flush();
	log_infof("[audio_render] wrote %s", path);
}

///////////////////////////////////////////

int audio_render_run(const char* out_dir, const char* ambi_file) {
	audio_test_offline(true);

	sk_settings_t settings = {};
	settings.app_name     = "StereoKitC Audio Render";
	settings.mode         = app_mode_offscreen;
	settings.standby_mode = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[audio_render] sk_init failed");
		return 1;
	}

	sound_t bursts = sound_generate(ar_gen_bursts, 1.0f, sound_channels_mono);
	sound_set_decibels(bursts, 83);

	char path[512];
	snprintf(path, sizeof(path), "%s/orbit_direct.wav", out_dir);
	ar_render_orbit(path, bursts, 2.0f, false, false);
	snprintf(path, sizeof(path), "%s/orbit_bus.wav", out_dir);
	ar_render_orbit(path, bursts, 2.0f, false, true);
	snprintf(path, sizeof(path), "%s/vertical_direct.wav", out_dir);
	ar_render_orbit(path, bursts, 2.0f, true, false);
	snprintf(path, sizeof(path), "%s/vertical_bus.wav", out_dir);
	ar_render_orbit(path, bursts, 2.0f, true, true);
	snprintf(path, sizeof(path), "%s/orbit_near.wav", out_dir);
	ar_render_orbit(path, bursts, 0.35f, false, false);
	snprintf(path, sizeof(path), "%s/env_off.wav", out_dir);
	ar_render_env(path, bursts, ar_env_off);
	snprintf(path, sizeof(path), "%s/env_room.wav", out_dir);
	ar_render_env(path, bursts, ar_env_room);
	snprintf(path, sizeof(path), "%s/env_hall.wav", out_dir);
	ar_render_env(path, bursts, ar_env_hall);
	snprintf(path, sizeof(path), "%s/env_forest.wav", out_dir);
	ar_render_env(path, bursts, ar_env_forest);
	snprintf(path, sizeof(path), "%s/env_field.wav", out_dir);
	ar_render_env(path, bursts, ar_env_field);
	sound_release(bursts);

	if (ambi_file != nullptr) {
		sound_t bed = sound_create(ambi_file);
		for (int32_t i = 0; i < 500 && sound_duration(bed) == 0; i++) ar_sleep_ms(10);
		if (sound_duration(bed) > 0) {
			sound_set_decibels(bed, 83);
			snprintf(path, sizeof(path), "%s/ambi_yaw.wav", out_dir);
			ar_render_ambi(path, bed);
		} else {
			log_errf("[audio_render] couldn't load ambisonic file %s", ambi_file);
		}
		sound_release(bed);
	}

	sk_shutdown();
	return 0;
}
