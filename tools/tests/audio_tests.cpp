#include "audio_tests.h"

#include <stereokit.h>

// Internal headers, reached through StereoKitC's public include root. The
// hooks used here are exported specifically for this harness.
#include <systems/audio.h>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined(_WIN32)
#include <windows.h>
static void at_sleep_ms(int32_t ms) { Sleep(ms); }
#else
#include <unistd.h>
#include <time.h>
static void at_sleep_ms(int32_t ms) { usleep(ms * 1000); }
#endif

// Big enough for an OS temp directory (Windows' MAX_PATH is 260) plus the
// longest filename this file builds ("audio_test_ambi_fuma.wav" and friends).
#define AT_PATH_MAX 320

// Scratch wav files for round-trip tests need an absolute path. A relative
// name would get resolved through StereoKit's "Assets" folder convention,
// which these files aren't part of.
static void at_temp_path(char* out, size_t out_size, const char* name) {
#if defined(_WIN32)
	char dir[AT_PATH_MAX];
	GetTempPathA((DWORD)sizeof(dir), dir);
	snprintf(out, out_size, "%s%s", dir, name);
#else
	snprintf(out, out_size, "/tmp/%s", name);
#endif
}

using namespace sk;

///////////////////////////////////////////

static int at_failures = 0;

#define AT_CHECK(condition, description) do { \
	if (condition) { log_infof("[audio_test] pass: %s", description); } \
	else           { log_errf ("[audio_test] FAIL: %s", description); at_failures += 1; } \
	} while (0)

#define AT_BLOCK 1024

// The C API takes raw environment parameters; these values mirror the C#
// AudioEnvironment.Room preset.       wet   decay damp  size scatter reflect
static const audio_env_t at_env_room = { 0.17f, 0.4f, 0.55f, 7, 0.6f, 0.55f };
static const audio_env_t at_env_off  = {};

///////////////////////////////////////////

// Renders `frames` of audio in blocks, running the main-thread step between
// blocks like a frame loop would. Returns summed per-ear energy.
static void at_render(int32_t frames, double* out_energy_l, double* out_energy_r) {
	static float block[AT_BLOCK * 2];
	double energy_l = 0;
	double energy_r = 0;
	while (frames > 0) {
		int32_t count = frames < AT_BLOCK ? frames : AT_BLOCK;
		audio_render_block(block, count);
		for (int32_t i = 0; i < count; i++) {
			energy_l += (double)block[i*2  ] * block[i*2  ];
			energy_r += (double)block[i*2+1] * block[i*2+1];
		}
		audio_test_step();
		frames -= count;
	}
	if (out_energy_l) *out_energy_l = energy_l;
	if (out_energy_r) *out_energy_r = energy_r;
}

static double at_render_energy(int32_t frames) {
	double l = 0, r = 0;
	at_render(frames, &l, &r);
	return l + r;
}

///////////////////////////////////////////

static float at_sine(float t) { return sinf(t * 440.0f * 6.2831853f) * 0.1f; }
static float at_dc  (float  ) { return 0.5f; }

// Pan/elevation assertions use high frequency content: real heads only
// shadow highs - low frequencies localize by arrival time, ~1dB of level.
static float at_sine8k(float t) { return sinf(t * 8000.0f * 6.2831853f) * 0.1f; }

// The decode has filter/delay state that rings down briefly after content
// stops, silence assertions flush it out first.
static void at_flush() {
	static float block[AT_BLOCK * 2];
	audio_render_block(block, AT_BLOCK);
	audio_render_block(block, AT_BLOCK);
	audio_test_step();
}

// With loudness normalization, 83dB declared renders any measurable content
// at -20dBFS RMS at 1m - amplitude ~0.14 for the sine sources here.
#define AT_UNIT_GAIN_DB 83.0f

// Adapts the per-sample helpers to the batch generate signature.
static float (*at_gen_fn)(float) = nullptr;
static void at_gen_batch(float* out, uint64_t start, uint64_t count) {
	for (uint64_t i = 0; i < count; i++)
		out[i] = at_gen_fn((float)(start + i) / (float)AU_SAMPLE_RATE);
}

static sound_t at_generate(float (*fn)(float), float duration) {
	at_gen_fn = fn;
	sound_t sound = sound_generate(at_gen_batch, duration);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	return sound;
}

///////////////////////////////////////////

static void at_test_polyphony() {
	sound_t sound = at_generate(at_sine, 0.25f);

	sound_inst_t inst = sound_play(sound, vec3{0,0,-1});
	double energy_one = at_render_energy(AU_SAMPLE_RATE / 2);
	AT_CHECK(energy_one > 0.001, "single voice produces audio");
	AT_CHECK(sound_inst_is_playing(inst) == false, "voice finishes at end of data");

	sound_inst_t insts[3];
	for (int32_t i = 0; i < 3; i++) insts[i] = sound_play(sound, vec3{0,0,-1});
	bool all_playing = sound_inst_is_playing(insts[0]) && sound_inst_is_playing(insts[1]) && sound_inst_is_playing(insts[2]);
	AT_CHECK(all_playing, "the same sound plays three times at once");
	AT_CHECK(insts[0]._slot != insts[1]._slot && insts[1]._slot != insts[2]._slot, "overlapping plays get distinct voices");

	// Three identical in-phase sources should sum to ~3x amplitude, ~9x energy.
	double energy_three = at_render_energy(AU_SAMPLE_RATE / 2);
	AT_CHECK(energy_three > energy_one * 7 && energy_three < energy_one * 11, "three coherent voices give ~9x the energy");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_stop() {
	sound_t      sound = at_generate(at_sine, 2.0f);
	sound_inst_t inst  = sound_play(sound, vec3{0,0,-1});

	at_render(AT_BLOCK, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(inst), "long voice is playing after a block");

	// The mixer needs a block to act on the stop, but the handle is dead as
	// soon as the caller asks: polling it in the same frame must not lie.
	sound_inst_stop(inst);
	AT_CHECK(sound_inst_is_playing(inst) == false, "stopped voice reports not playing right away");

	at_render(AT_BLOCK * 2, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(inst) == false, "stopped voice stays stopped once the mixer catches up");

	double energy_after = at_render_energy(AT_BLOCK * 4);
	AT_CHECK(energy_after < 0.0001, "stopped voice is silent");

	sound_release(sound);
}

///////////////////////////////////////////

// Stopping mid-waveform ramps the voice down over AU_FADE_FRAMES instead of
// cutting, so the output can't step to zero and click. DC content through the
// head-locked path is flat, so any step shows as a sample-to-sample delta.
static void at_test_stop_declick() {
	sound_t sound = at_generate(at_dc, 2.0f);

	sound_play_t play = {}; play.flags = sound_flags_head_locked;
	sound_inst_t inst = sound_play(sound, vec3{0,0,0}, &play);
	at_render(AT_BLOCK * 2, nullptr, nullptr);

	static float block[AT_BLOCK * 2];
	audio_render_block(block, AT_BLOCK);
	audio_test_step();
	float level = block[(AT_BLOCK-1) * 2];

	sound_inst_stop(inst);
	audio_render_block(block, AT_BLOCK);
	audio_test_step();

	float max_delta = 0;
	float prev      = level;
	for (int32_t i = 0; i < AT_BLOCK; i++) {
		float delta = fabsf(block[i*2] - prev);
		if (delta > max_delta) max_delta = delta;
		prev = block[i*2];
	}
	AT_CHECK(level     > 0.01f,          "declick voice renders at a measurable level");
	AT_CHECK(max_delta < level * 0.05f,  "stop ramps out instead of stepping to zero");
	AT_CHECK(fabsf(block[(AT_BLOCK-1)*2]) < 0.000001f, "stop block ends silent");
	AT_CHECK(audio_test_voice_state(inst._slot) == au_voice_free, "ramped-out voice frees its slot");

	// A paused voice is already silent, nothing to declick - its stop must
	// finish immediately rather than wait on a ramp that never advances.
	sound_inst_t paused = sound_play(sound, vec3{0,0,0}, &play);
	at_render(AT_BLOCK, nullptr, nullptr);
	sound_inst_set_paused(paused, true);
	at_render(AT_BLOCK, nullptr, nullptr);
	sound_inst_stop(paused);
	at_render(AT_BLOCK, nullptr, nullptr);
	AT_CHECK(audio_test_voice_state(paused._slot) == au_voice_free, "stopping a paused voice frees its slot");

	sound_release(sound);
}

///////////////////////////////////////////

// Swings the voices through the near field with per-frame position jumps,
// like a hand-held emitter, and reports the peak level and the largest
// sample-to-sample step in the left ear across the whole run.
static void at_zipper_measure(sound_inst_t* insts, int32_t count, float* out_level, float* out_delta) {
	static float block[AT_BLOCK * 2];
	float level = 0, max_delta = 0, prev = 0;
	bool  first = true;
	for (int32_t f = 0; f < 40; f++) {
		// ~4.5m/s through the min-distance clamp region, side to side.
		float ph = (float)f / 8.0f;
		vec3  at = vec3{sinf(ph) * 0.3f, 0, -0.25f - 0.25f * fabsf(cosf(ph))};
		for (int32_t v = 0; v < count; v++) sound_inst_set_pos(insts[v], at);
		audio_render_block(block, AT_BLOCK);
		audio_test_step();
		for (int32_t i = 0; i < AT_BLOCK; i++) {
			float s = block[i*2];
			if (!first && fabsf(s - prev) > max_delta) max_delta = fabsf(s - prev);
			if (fabsf(s) > level) level = fabsf(s);
			prev = s; first = false;
		}
	}
	*out_level = level;
	*out_delta = max_delta;
}

// Voicing coefficients, shadow wet, and shoulder gain are direction-driven
// and invisible to DC content - a sine orbiting fast through azimuth and
// elevation exercises them, bounded by the sine's own per-sample delta.
static void at_test_voicing_zipper_measure(float* out_level, float* out_delta) {
	sound_t      sound = at_generate(at_sine, 2.0f);
	sound_play_t loop  = {}; loop.flags = sound_flags_loop;
	sound_inst_t inst  = sound_play(sound, vec3{0, 0, -0.3f}, &loop);
	at_render(AT_BLOCK * 4, nullptr, nullptr);

	static float block[AT_BLOCK * 2];
	float level = 0, max_delta = 0, prev = 0;
	bool  first = true;
	for (int32_t f = 0; f < 60; f++) {
		float az = (float)f * 0.35f;
		float el = 0.9f * sinf((float)f * 0.55f);
		vec3  at = vec3{sinf(az) * 0.3f * cosf(el), sinf(el) * 0.3f, -cosf(az) * 0.3f * cosf(el)};
		sound_inst_set_pos(inst, at);
		audio_render_block(block, AT_BLOCK);
		audio_test_step();
		for (int32_t i = 0; i < AT_BLOCK; i++) {
			float s = block[i*2];
			if (!first && fabsf(s - prev) > max_delta) max_delta = fabsf(s - prev);
			if (fabsf(s) > level) level = fabsf(s);
			prev = s; first = false;
		}
	}
	sound_inst_stop(inst);
	at_flush();
	sound_release(sound);
	*out_level = level;
	*out_delta = max_delta;
}

// The mixer ramps applied gains across each block and smooths position, so
// a loud source moving fast near the head can't step at block boundaries.
// Flat DC content makes any step show as a sample-to-sample delta.
static void at_test_gain_zipper() {
	sound_t      sound = at_generate(at_dc, 2.0f);
	sound_play_t loop  = {}; loop.flags = sound_flags_loop;
	float        level, delta;

	// One voice takes the scalar direct path.
	sound_inst_t inst = sound_play(sound, vec3{0,0,-0.5f}, &loop);
	at_render(AT_BLOCK * 4, nullptr, nullptr);
	at_zipper_measure(&inst, 1, &level, &delta);
	AT_CHECK(level > 0.05f,         "zipper voice reaches near-field level");
	AT_CHECK(delta < level * 0.02f, "fast near-field movement stays step-free");
	sound_inst_stop(inst);
	at_flush();

	// Four voices exercise the same ramps in the batched direct path.
	sound_inst_t four[4];
	for (int32_t i = 0; i < 4; i++) four[i] = sound_play(sound, vec3{0,0,-0.5f}, &loop);
	at_render(AT_BLOCK * 4, nullptr, nullptr);
	at_zipper_measure(four, 4, &level, &delta);
	AT_CHECK(delta < level * 0.02f, "batched near-field movement stays step-free");
	for (int32_t i = 0; i < 4; i++) sound_inst_stop(four[i]);
	at_flush();

	// 8% splits a 440Hz sine's natural ~5.8% per-sample delta from the
	// ~11.5% that per-block parameter snapping produces on this orbit.
	at_test_voicing_zipper_measure(&level, &delta);
	AT_CHECK(delta < level * 0.08f, "direction-driven DSP params stay step-free");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_gen_ambi_w(float* out, uint64_t start, uint64_t frames);

static void at_test_steal() {
	sound_t sound = at_generate(at_sine, 2.0f);

	// Fill the pool with one very quiet far voice among moderate ones, all
	// *activated* - reserved slots are mid-handoff and can't be reclaimed.
	sound_play_t quiet_play = {}; quiet_play.volume = 0.05f;
	sound_play_t mid_play   = {}; mid_play  .volume = 0.5f;
	sound_inst_t quiet = sound_play(sound, vec3{0, 0, -50}, &quiet_play);
	sound_inst_t mid[AU_VOICE_COUNT - 1];
	for (int32_t i = 0; i < AU_VOICE_COUNT - 1; i++)
		mid[i] = sound_play(sound, vec3{0,0,-4}, &mid_play);
	at_render(AT_BLOCK, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(quiet), "pool fills to capacity");

	// A clearly louder play displaces the quietest voice.
	sound_inst_t extra = sound_play(sound, vec3{0,0,-1});
	AT_CHECK(extra._slot >= 0 && sound_inst_is_playing(extra), "a louder play steals from a full pool");
	AT_CHECK(sound_inst_is_playing(quiet) == false, "the quietest voice was the one stolen");
	AT_CHECK(extra._slot == quiet._slot, "the stolen slot was reused");

	// A play *no louder* than the quietest survivor refuses instead: it
	// would be the least audible thing in the pool.
	sound_inst_t refuse = sound_play(sound, vec3{0,0,-50}, &quiet_play);
	AT_CHECK(refuse._slot < 0, "a play quieter than everything refuses instead of stealing");

	int32_t still_playing = 0;
	for (int32_t i = 0; i < AU_VOICE_COUNT - 1; i++)
		if (sound_inst_is_playing(mid[i])) still_playing += 1;
	AT_CHECK(still_playing == AU_VOICE_COUNT - 1, "louder voices survive the steal");

	// Cleanup
	sound_inst_stop(extra);
	for (int32_t i = 0; i < AU_VOICE_COUNT - 1; i++) sound_inst_stop(mid[i]);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_virtual_voices() {
	// A quiet cricket loop in the distance: stealable in principle, but
	// when outranked it should go dormant and *resume*, not die.
	sound_t cricket_snd = at_generate(at_sine8k, 0.5f);
	sound_t loud_snd    = at_generate(at_sine,   0.4f);
	sound_t bed_snd     = sound_generate(at_gen_ambi_w, 0.5f, sound_channels_ambisonic1);
	sound_set_decibels(bed_snd, AT_UNIT_GAIN_DB);

	sound_play_t loop = {}; loop.flags = sound_flags_loop; loop.volume = 0.3f;
	sound_inst_t cricket = sound_play(cricket_snd, vec3{0,0,-30}, &loop);
	sound_play_t bedloop = {}; bedloop.flags = sound_flags_loop;
	sound_inst_t bed     = sound_play(bed_snd, vec3{0,0,0}, &bedloop);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(cricket) && sound_inst_is_playing(bed), "distant loop and ambisonic bed play");

	// A swarm of louder one-shots overruns the mix budget.
	for (int32_t i = 0; i < AU_MIX_VOICES + 16; i++)
		sound_play(loud_snd, vec3{0,0,-1});
	at_render(AT_BLOCK * 2, nullptr, nullptr);

	AT_CHECK(sound_inst_is_playing(cricket), "the outranked loop stays alive, dormant");
	AT_CHECK(sound_inst_is_playing(bed),     "the distance-less bed is never displaced");
	uint64_t frozen = sound_inst_get_cursor(cricket);
	at_render(AT_BLOCK * 4, nullptr, nullptr);
	AT_CHECK(sound_inst_get_cursor(cricket) == frozen, "a dormant voice's cursor is frozen");

	// The swarm dies down, the cricket resumes right where it froze.
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(cricket), "the loop survives the whole storm");
	AT_CHECK(sound_inst_get_cursor(cricket) != frozen, "the dormant loop resumes when the pool quiets");

	sound_inst_stop(cricket);
	sound_inst_stop(bed);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(cricket_snd);
	sound_release(loud_snd);
	sound_release(bed_snd);
}

///////////////////////////////////////////

static void at_test_cmd_overflow() {
	sound_t sound = at_generate(at_sine, 2.0f);

	// Without pumping blocks nothing drains the command ring, so play spam
	// must eventually refuse instead of corrupting anything.
	bool refused = false;
	sound_inst_t alive[512];
	int32_t      alive_ct = 0;
	for (int32_t i = 0; i < 512; i++) {
		sound_inst_t inst = sound_play(sound, vec3{0,0,-1});
		if (inst._slot < 0) { refused = true; break; }
		if (alive_ct < 512) alive[alive_ct++] = inst;
	}
	AT_CHECK(refused, "command ring overflow refuses plays");

	// After a block drains the ring, a *louder* play works again - equal
	// loudness refuses by the audibility rule, so step closer.
	at_render(AT_BLOCK, nullptr, nullptr);
	sound_inst_t retry = sound_play(sound, vec3{0,0,-0.3f});
	AT_CHECK(retry._slot >= 0, "plays succeed again after the ring drains");

	sound_inst_stop(retry);
	for (int32_t i = 0; i < alive_ct; i++) sound_inst_stop(alive[i]);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_pan() {
	sound_t sound = at_generate(at_sine8k, 0.5f);

	sound_play(sound, vec3{2,0,0}); // Right of the identity listener
	double l = 0, r = 0;
	at_render(AU_SAMPLE_RATE / 2, &l, &r);
	AT_CHECK(r > l * 1.5, "a source on the right is right-ear dominant");

	sound_play(sound, vec3{-2,0,0}); // And the mirror
	at_render(AU_SAMPLE_RATE / 2, &l, &r);
	AT_CHECK(l > r * 1.5, "a source on the left is left-ear dominant");

	// Turning the head around swaps the ears.
	pose_t flipped = {{0,0,0}, quat_from_angles(0, 180, 0)};
	audio_set_listener(&flipped);
	audio_test_step();
	sound_play(sound, vec3{2,0,0});
	at_render(AU_SAMPLE_RATE / 2, &l, &r);
	AT_CHECK(l > r * 1.5, "yawing the listener 180 flips the pan");
	audio_set_listener(nullptr);
	audio_test_step();

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_ring_stream() {
	sound_t stream = sound_create_stream(0.5f);
	sound_set_decibels(stream, AT_UNIT_GAIN_DB);

	// 0.1s of tone, then let the voice idle past the end of its data.
	float samples[AU_SAMPLE_RATE / 10];
	for (int32_t i = 0; i < AU_SAMPLE_RATE / 10; i++) samples[i] = at_sine((float)i / AU_SAMPLE_RATE);
	sound_write_samples(stream, samples, AU_SAMPLE_RATE / 10);

	sound_inst_t inst   = sound_play(stream, vec3{0,0,-1});
	double energy_data  = at_render_energy(AU_SAMPLE_RATE / 10);
	at_flush(); // Decode filter tails ring down after the data ends
	double energy_after = at_render_energy(AU_SAMPLE_RATE / 10);
	AT_CHECK(energy_data > 0.001,   "stream voice plays its buffered data");
	AT_CHECK(energy_after < 0.0001, "idle stream voice is silent");
	AT_CHECK(sound_inst_is_playing(inst), "stream voice idles instead of finishing");

	// New data picks right back up.
	sound_write_samples(stream, samples, AU_SAMPLE_RATE / 10);
	double energy_resume = at_render_energy(AU_SAMPLE_RATE / 10);
	AT_CHECK(energy_resume > 0.001, "idle stream voice resumes on new data");

	sound_inst_stop(inst);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(stream);
}

///////////////////////////////////////////

static void at_test_write_overwrites_oldest() {
	sound_t stream = sound_create_stream(0.5f); // 24000 sample capacity

	float* ramp = (float*)malloc(sizeof(float) * 30000);
	for (int32_t i = 0; i < 30000; i++) ramp[i] = (float)i;
	sound_write_samples(stream, ramp, 30000);

	float* read    = (float*)malloc(sizeof(float) * 30000);
	int64_t count  = (int64_t)sound_read_samples(stream, read, 30000);
	AT_CHECK(count == 24000, "full ring holds exactly its capacity");
	AT_CHECK(count > 0 && read[0] == 6000.0f && read[count-1] == 29999.0f, "overflowing writes keep the newest samples");

	free(ramp);
	free(read);
	sound_release(stream);
}

///////////////////////////////////////////

// Writes a float32 sine wav with the tone on masked channels. Files over 10s
// take the per-voice streaming decoder path rather than predecode.
static bool at_write_wav_ch(const char* path, float seconds, float amplitude, int16_t channels, int32_t active_mask, float hz) {
	int32_t frames    = (int32_t)(AU_SAMPLE_RATE * seconds);
	int32_t data_size = frames * channels * sizeof(float);
	FILE* file = fopen(path, "wb");
	if (file == nullptr) return false;

	int32_t chunk_size = 36 + data_size;
	int16_t format = 3, block_align = (int16_t)(channels * 4), bits = 32;
	int32_t rate = AU_SAMPLE_RATE, byte_rate = AU_SAMPLE_RATE * channels * 4, fmt_size = 16;
	fwrite("RIFF", 1, 4, file); fwrite(&chunk_size, 4, 1, file); fwrite("WAVE", 1, 4, file);
	fwrite("fmt ", 1, 4, file); fwrite(&fmt_size,   4, 1, file);
	fwrite(&format,    2, 1, file); fwrite(&channels,  2, 1, file);
	fwrite(&rate,      4, 1, file); fwrite(&byte_rate, 4, 1, file);
	fwrite(&block_align,2,1, file); fwrite(&bits,      2, 1, file);
	fwrite("data", 1, 4, file); fwrite(&data_size, 4, 1, file);
	for (int32_t i = 0; i < frames; i++) {
		float t      = (float)i / AU_SAMPLE_RATE;
		float sample = sinf(t * hz * 6.2831853f) * amplitude;
		for (int16_t c = 0; c < channels; c++) {
			float value = (active_mask & (1 << c)) ? sample : 0.0f;
			fwrite(&value, 4, 1, file);
		}
	}
	fclose(file);
	return true;
}

static bool at_write_wav(const char* path, float seconds, float amplitude) {
	return at_write_wav_ch(path, seconds, amplitude, 1, 1, 440.0f);
}

// Writes a FuMa-tagged wav: WAVE_FORMAT_EXTENSIBLE with the ambisonic float
// GUID, FuMa channel order W,X,Y,Z at the given amplitudes.
static bool at_write_wav_fuma(const char* path, float seconds, float w_amp, float x_amp, float hz) {
	int32_t frames    = (int32_t)(AU_SAMPLE_RATE * seconds);
	int32_t data_size = frames * 4 * sizeof(float);
	FILE* file = fopen(path, "wb");
	if (file == nullptr) return false;

	int32_t chunk_size = 60 + data_size;
	int32_t fmt_size = 40, rate = AU_SAMPLE_RATE, byte_rate = rate * 16, mask = 0;
	int16_t tag = (int16_t)0xFFFE, channels = 4, align = 16, bits = 32, cb = 22, valid = 32;
	uint8_t guid[16] = {0x03,0,0,0, 0x21,0x07,0xd3,0x11, 0x86,0x44, 0xc8,0xc1,0xca,0,0,0};
	fwrite("RIFF", 1, 4, file); fwrite(&chunk_size, 4, 1, file); fwrite("WAVE", 1, 4, file);
	fwrite("fmt ", 1, 4, file); fwrite(&fmt_size,   4, 1, file);
	fwrite(&tag,   2, 1, file); fwrite(&channels,   2, 1, file);
	fwrite(&rate,  4, 1, file); fwrite(&byte_rate,  4, 1, file);
	fwrite(&align, 2, 1, file); fwrite(&bits,       2, 1, file);
	fwrite(&cb,    2, 1, file); fwrite(&valid,      2, 1, file);
	fwrite(&mask,  4, 1, file); fwrite(guid,        1, 16, file);
	fwrite("data", 1, 4, file); fwrite(&data_size,  4, 1, file);
	for (int32_t i = 0; i < frames; i++) {
		float s    = sinf((float)i / AU_SAMPLE_RATE * hz * 6.2831853f);
		float v[4] = { s * w_amp, s * x_amp, 0, 0 };
		fwrite(v, 4, 4, file);
	}
	fclose(file);
	return true;
}

// Loads a wav and waits out the async decode. Null on timeout.
static sound_t at_load_wav(const char* path) {
	sound_t sound = sound_create(path);
	if (sound == nullptr) return nullptr;
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++)
		at_sleep_ms(10);
	return sound;
}

static void at_test_file_streaming() {
	char path[AT_PATH_MAX];
	at_temp_path(path, sizeof(path), "audio_test_long.wav");
	if (!at_write_wav(path, 12, 0.1f)) {
		log_warnf("[audio_test] skipping file streaming test, can't write %s", path);
		return;
	}

	sound_t sound = at_load_wav(path);
	AT_CHECK(sound != nullptr, "long wav file loads");
	if (sound == nullptr) return;
	AT_CHECK(sound_duration(sound) > 11.9f && sound_duration(sound) < 12.1f, "long wav reports its duration");

	sound_inst_t inst = sound_play(sound, vec3{0,0,-1});
	AT_CHECK(sound_inst_is_playing(inst), "streaming voice starts");

	// Play through a few seconds in chunks, prefetch keeps it fed.
	double energy = at_render_energy(AU_SAMPLE_RATE * 3);
	AT_CHECK(energy > 0.01, "streaming voice renders continuous audio");
	AT_CHECK(sound_inst_is_playing(inst), "streaming voice is still going mid-file");

	// Render past the end, it should finish on its own.
	at_render(AU_SAMPLE_RATE * 10, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(inst) == false, "streaming voice finishes at end of file");

	sound_release(sound);
	remove(path);
}

///////////////////////////////////////////

// Plays made while a sound is still decoding hold their voice and catch up
// to real time on load. sound_create sets asset_state_loading before the
// decode task can run, so playing on the next line always defers. The
// offline clock only moves on audio_test_step, so elapsed time is exact.
static void at_test_deferred_play() {
	char base[AT_PATH_MAX];
	at_temp_path(base, sizeof(base), "audio_test_defer");
	char path[AT_PATH_MAX];

	// Loads inside the grace window play from the top, as if nothing
	// happened. One step is 16ms of clock, well under the grace.
	snprintf(path, sizeof(path), "%s1.wav", base);
	if (!at_write_wav(path, 2, 0.5f)) {
		log_warnf("[audio_test] skipping deferred play test, can't write %s", path);
		return;
	}
	sound_t sound = sound_create(path);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	sound_inst_t inst = sound_play(sound, vec3{0,0,-1});
	AT_CHECK(sound_inst_is_playing(inst), "deferred play hands back a live handle");
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++) at_sleep_ms(10);
	audio_test_step();
	double energy = at_render_energy(AU_SAMPLE_RATE / 4);
	AT_CHECK(energy > 0.001, "grace window play renders audio");
	AT_CHECK(sound_inst_get_cursor(inst) <= (uint64_t)(AU_SAMPLE_RATE / 4 + AU_PLAY_GRACE * AU_SAMPLE_RATE),
		"grace window play starts from the top");
	sound_inst_stop(inst);
	at_flush();
	sound_release(sound);
	remove(path);

	// A long wait catches the cursor up and ramps the onset in; a
	// mid-waveform start would land as a click otherwise.
	snprintf(path, sizeof(path), "%s2.wav", base);
	at_write_wav(path, 2, 0.5f);
	sound = sound_create(path);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	inst = sound_play(sound, vec3{0,0,-1});
	audio_test_advance(0.7f); // The load "takes" 0.7s
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++) at_sleep_ms(10);
	audio_test_step();
	static float block[AT_BLOCK * 2];
	audio_render_block(block, AT_BLOCK);
	uint64_t cursor = sound_inst_get_cursor(inst);
	AT_CHECK(cursor > (uint64_t)(0.6 * AU_SAMPLE_RATE) && cursor < (uint64_t)(0.8 * AU_SAMPLE_RATE),
		"catch-up play resumes near the elapsed time");
	double head = 0, tail = 0;
	for (int32_t i = 0;   i < 100;  i++) head += (double)block[i*2] * block[i*2];
	for (int32_t i = 500; i < 1000; i++) tail += (double)block[i*2] * block[i*2];
	AT_CHECK(head < tail * 0.5, "catch-up onset fades in instead of clicking");
	sound_inst_stop(inst);
	at_flush();
	sound_release(sound);
	remove(path);

	// A one-shot whose moment has passed never plays at all.
	snprintf(path, sizeof(path), "%s3.wav", base);
	at_write_wav(path, 0.25f, 0.5f);
	sound = sound_create(path);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	inst = sound_play(sound, vec3{0,0,-1});
	audio_test_advance(3);
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++) at_sleep_ms(10);
	audio_test_step();
	AT_CHECK(sound_inst_is_playing(inst) == false, "expired one-shot never plays");
	at_flush();
	AT_CHECK(at_render_energy(AU_SAMPLE_RATE / 4) < 0.0001, "expired one-shot renders silence");
	sound_release(sound);
	remove(path);

	// A stop while the sound is still decoding takes effect immediately, it
	// doesn't wait around for a load whose play was already cancelled.
	snprintf(path, sizeof(path), "%s5.wav", base);
	at_write_wav(path, 0.5f, 0.5f);
	sound = sound_create(path);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	inst = sound_play(sound, vec3{0,0,-1});
	sound_inst_stop(inst);
	AT_CHECK(sound_inst_is_playing(inst) == false, "stop during decode reports not playing right away");
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++) at_sleep_ms(10);
	audio_test_step();
	AT_CHECK(sound_inst_is_playing(inst) == false, "a play stopped during decode never starts");
	at_flush();
	AT_CHECK(at_render_energy(AU_SAMPLE_RATE / 4) < 0.0001, "a play stopped during decode renders silence");
	sound_release(sound);
	remove(path);

	// The same wait on a looping play just wraps.
	snprintf(path, sizeof(path), "%s4.wav", base);
	at_write_wav(path, 0.25f, 0.5f);
	sound = sound_create(path);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	sound_play_t loop_play = {}; loop_play.flags = sound_flags_loop;
	inst = sound_play(sound, vec3{0,0,-1}, &loop_play);
	audio_test_advance(3);
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++) at_sleep_ms(10);
	audio_test_step();
	AT_CHECK(sound_inst_is_playing(inst), "expired loop keeps playing");
	AT_CHECK(at_render_energy(AU_SAMPLE_RATE / 4) > 0.001, "expired loop renders audio");
	sound_inst_stop(inst);
	at_flush();
	sound_release(sound);
	remove(path);
}

///////////////////////////////////////////

// Streams are broadcast: each voice reads at its own cursor, so several
// voices hear the whole stream, and playback never consumes the samples
// the public ReadSamples API sees.
static void at_test_ring_shared() {
	sound_t stream = sound_create_stream(0.5f);
	sound_set_decibels(stream, AT_UNIT_GAIN_DB);

	float samples[AU_SAMPLE_RATE / 5];
	for (int32_t i = 0; i < AU_SAMPLE_RATE / 5; i++) samples[i] = at_sine((float)i / AU_SAMPLE_RATE);
	sound_write_samples(stream, samples, AU_SAMPLE_RATE / 5);

	sound_inst_t a = sound_play(stream, vec3{0,0,-1});
	sound_inst_t b = sound_play(stream, vec3{0,0,-1});
	at_render(AU_SAMPLE_RATE / 4, nullptr, nullptr);

	AT_CHECK(sound_inst_is_playing(a) && sound_inst_is_playing(b), "two voices share one stream");
	AT_CHECK(sound_inst_get_cursor(a) == (uint64_t)(AU_SAMPLE_RATE / 5), "first voice hears the whole stream");
	AT_CHECK(sound_inst_get_cursor(b) == (uint64_t)(AU_SAMPLE_RATE / 5), "second voice hears the whole stream");
	AT_CHECK(sound_unread_samples(stream) == (uint64_t)(AU_SAMPLE_RATE / 5), "playback doesn't consume ReadSamples data");

	sound_inst_stop(a);
	sound_inst_stop(b);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(stream);
}

///////////////////////////////////////////

static void at_test_pitch() {
	sound_t sound = at_generate(at_sine, 0.5f);

	sound_play_t fast = {}; fast.pitch = 2;
	sound_inst_t inst_fast = sound_play(sound, vec3{0,0,-1}, &fast);
	sound_inst_t inst_norm = sound_play(sound, vec3{0,0,-1}, nullptr);

	// At 2x rate a 0.5s sound is done in 0.25s, the 1x control isn't.
	at_render(AU_SAMPLE_RATE/4 + AT_BLOCK*2, nullptr, nullptr);
	AT_CHECK(!sound_inst_is_playing(inst_fast), "pitch 2 finishes in half the time");
	AT_CHECK( sound_inst_is_playing(inst_norm), "pitch 1 control is still playing");

	sound_inst_stop(inst_norm);
	at_render(AT_BLOCK*2, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_loop() {
	sound_t sound = at_generate(at_sine, 0.25f);

	sound_play_t looped = {}; looped.flags = sound_flags_loop;
	sound_inst_t inst = sound_play(sound, vec3{0,0,-1}, &looped);

	// A 0.25s sound with loop still has energy in every quarter of a
	// full second, and never finishes on its own.
	bool all_quarters = true;
	for (int32_t q = 0; q < 4; q++)
		if (at_render_energy(AU_SAMPLE_RATE/4) < 0.001) all_quarters = false;
	AT_CHECK(all_quarters, "looping voice renders energy past its length");
	AT_CHECK(sound_inst_is_playing(inst), "looping voice doesn't finish");

	sound_inst_stop(inst);
	at_render(AT_BLOCK*2, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_pause_seek() {
	sound_t      sound = at_generate(at_sine, 2.0f);
	sound_inst_t inst  = sound_play(sound, vec3{0,0,-1}, nullptr);

	at_render(AT_BLOCK*2, nullptr, nullptr);
	sound_inst_set_paused(inst, true);
	at_render(AT_BLOCK, nullptr, nullptr); // Pause lands at next block edge
	uint64_t frozen = sound_inst_get_cursor(inst);
	double   energy = at_render_energy(AT_BLOCK*4);
	AT_CHECK(sound_inst_get_paused(inst),                "voice reports paused");
	AT_CHECK(sound_inst_get_cursor(inst) == frozen,      "paused voice's cursor is frozen");
	AT_CHECK(energy < 0.0001,                            "paused voice is silent");
	AT_CHECK(sound_inst_is_playing(inst),                "paused voice stays alive");

	sound_inst_set_paused(inst, false);
	at_render(AT_BLOCK*2, nullptr, nullptr);
	AT_CHECK(sound_inst_get_cursor(inst) > frozen,       "resumed voice's cursor advances");

	// Seek close to the end, the voice should finish almost immediately.
	sound_inst_seek(inst, sound_total_samples(sound) - AT_BLOCK);
	at_render(AT_BLOCK*3, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(inst) == false,       "seek near the end finishes the voice");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_gen_stereo_seek(float* out, uint64_t start, uint64_t frames) {
	for (uint64_t i = 0; i < frames; i++) {
		float t = (float)(start + i) / AU_SAMPLE_RATE;
		out[i*2  ] = at_sine(t);
		out[i*2+1] = at_sine(t * 1.01f);
	}
}

// Cursor and seek speak interleaved samples, the mixer counts frames - they
// differ by channel count, and a mono-only suite never exercises that.
static void at_test_seek_multichannel() {
	sound_t      sound = sound_generate(at_gen_stereo_seek, 2.0f, sound_channels_stereo);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	sound_inst_t inst  = sound_play(sound, vec3{0,0,0}, nullptr);

	int32_t rendered = AT_BLOCK * 2;
	at_render(rendered, nullptr, nullptr);

	// A stereo voice advances two samples per frame, so the cursor clearly
	// outpaces frames rendered - half that (the missing conversion) fails.
	uint64_t cursor = sound_inst_get_cursor(inst);
	AT_CHECK(cursor > (uint64_t)rendered + rendered/2, "stereo cursor counts interleaved samples");

	// Seek to the sample midpoint: read as frames (the bug) it lands past the
	// end and finishes the voice, read as samples playback continues.
	uint64_t total = sound_total_samples(sound);
	sound_inst_seek(inst, total / 2);
	at_render(AT_BLOCK, nullptr, nullptr);
	AT_CHECK(sound_inst_is_playing(inst), "stereo seek to the sample midpoint stays in bounds");

	uint64_t after = sound_inst_get_cursor(inst);
	AT_CHECK(after >= total/2 && after < total/2 + AT_BLOCK*8, "stereo cursor reflects the sample-domain seek");

	sound_inst_stop(inst);
	at_render(AT_BLOCK, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

// Renders and returns the index of the first frame with audible output,
// -1 if the whole span is silent.
static int64_t at_first_audible(int32_t frames) {
	static float block[AT_BLOCK * 2];
	int64_t base = 0;
	while (frames > 0) {
		int32_t count = frames < AT_BLOCK ? frames : AT_BLOCK;
		audio_render_block(block, count);
		for (int32_t i = 0; i < count; i++)
			if (fabsf(block[i*2]) > 0.0001f || fabsf(block[i*2+1]) > 0.0001f)
				return base + i;
		audio_test_step();
		base   += count;
		frames -= count;
	}
	return -1;
}

static void at_test_delay() {
	sound_t sound = at_generate(at_dc, 0.25f);

	// A 0.1s delay lands on exactly sample 4800; DC 0.5 makes the first mixed
	// sample audible. Stop + drain each measurement so nothing bleeds.
	sound_play_t delayed = {}; delayed.delay = 0.1f;
	sound_inst_t inst = sound_play(sound, vec3{0,0,-1}, &delayed);
	int64_t first = at_first_audible(AU_SAMPLE_RATE/2);
	AT_CHECK(first == 4800, "onset delay is sample exact");
	sound_inst_stop(inst);
	at_render(AT_BLOCK*2, nullptr, nullptr);

	// Propagation delay adds round(dist/343*48000) on top: at 2m, 280.
	sound_play_t prop = {}; prop.delay = 0.1f; prop.flags = sound_flags_propagation_delay;
	inst  = sound_play(sound, vec3{0,0,-2}, &prop);
	first = at_first_audible(AU_SAMPLE_RATE/2);
	AT_CHECK(first == 4800 + 280, "propagation delay is sample exact");
	sound_inst_stop(inst);
	at_render(AT_BLOCK*2, nullptr, nullptr);

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_attenuation() {
	sound_t sound = at_generate(at_sine, 0.25f);

	sound_play(sound, vec3{0,0,-1}, nullptr);
	double near_e = at_render_energy(AU_SAMPLE_RATE/2);
	sound_play(sound, vec3{0,0,-2}, nullptr);
	double far_e  = at_render_energy(AU_SAMPLE_RATE/2);
	double ratio  = far_e > 0 ? near_e / far_e : 0;
	AT_CHECK(ratio > 3.6 && ratio < 4.4, "doubling distance drops output by 6dB");

	// +20dB declared loudness is 10x amplitude, 100x energy.
	sound_set_decibels(sound, AT_UNIT_GAIN_DB - 20);
	sound_play(sound, vec3{0,0,-1}, nullptr);
	double quiet_e = at_render_energy(AU_SAMPLE_RATE/2);
	ratio = quiet_e > 0 ? near_e / quiet_e : 0;
	AT_CHECK(ratio > 85 && ratio < 115, "20dB declared difference is 100x energy");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_normalization() {
	char path_a[AT_PATH_MAX], path_b[AT_PATH_MAX];
	at_temp_path(path_a, sizeof(path_a), "audio_test_quiet.wav");
	at_temp_path(path_b, sizeof(path_b), "audio_test_hot.wav");
	if (!at_write_wav(path_a, 1, 0.05f) || !at_write_wav(path_b, 1, 0.5f)) {
		log_warnf("[audio_test] skipping normalization test, can't write files");
		return;
	}
	sound_t quiet = at_load_wav(path_a);
	sound_t hot   = at_load_wav(path_b);
	AT_CHECK(quiet != nullptr && hot != nullptr, "normalization wavs load");
	if (quiet == nullptr || hot == nullptr) return;

	sound_play(quiet, vec3{0,0,-1}, nullptr);
	double quiet_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
	sound_play(hot,   vec3{0,0,-1}, nullptr);
	double hot_e   = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
	double ratio   = hot_e > 0 ? quiet_e / hot_e : 0;
	AT_CHECK(ratio > 0.9 && ratio < 1.1, "equal declared dB renders equal energy regardless of recording level");

	// Absolute calibration: 83dB declared renders -20dBFS, RMS 0.1 per ear.
	// Head-locked pins the pure gain path; the spatial decode gets a looser
	// check, ITD phase interference costs ~1dB (real ears comb too).
	sound_set_decibels(hot, 83);
	sound_play_t locked = {}; locked.flags = sound_flags_head_locked;
	sound_play(hot, vec3{0,0,-1}, &locked);
	double e_l = 0, e_r = 0;
	at_render(AU_SAMPLE_RATE, &e_l, &e_r);
	double rms = sqrt(e_l / AU_SAMPLE_RATE);
	AT_CHECK(rms > 0.09 && rms < 0.11, "83dB renders at the calibrated -20dBFS through the gain path");

	sound_play(hot, vec3{0,0,-1}, nullptr);
	at_render(AU_SAMPLE_RATE, &e_l, &e_r);
	rms = sqrt(e_l / AU_SAMPLE_RATE);
	AT_CHECK(rms > 0.077 && rms < 0.115, "the spatial decode stays within ~2dB of calibration");

	sound_release(quiet);
	sound_release(hot);
	remove(path_a);
	remove(path_b);
}

///////////////////////////////////////////

static void at_test_norm_gating() {
	// The same content with and without trailing silence must render equally
	// loud - gated measurement ignores padding instead of diluting RMS.
	int32_t tone_ct = AU_SAMPLE_RATE / 10;
	int32_t pad_ct  = tone_ct * 5;
	float*  padded  = (float*)malloc(sizeof(float) * pad_ct);
	memset(padded, 0, sizeof(float) * pad_ct);
	for (int32_t i = 0; i < tone_ct; i++)
		padded[i] = at_sine((float)i / AU_SAMPLE_RATE);

	sound_t trimmed = sound_create_samples(padded, tone_ct);
	sound_t sound_p = sound_create_samples(padded, pad_ct);

	sound_play(trimmed, vec3{0,0,-1});
	double trim_e = at_render_energy(pad_ct + AT_BLOCK);
	sound_play(sound_p, vec3{0,0,-1});
	double pad_e  = at_render_energy(pad_ct + AT_BLOCK);
	double ratio  = pad_e > 0 ? trim_e / pad_e : 0;
	AT_CHECK(ratio > 0.9 && ratio < 1.1, "silence padding doesn't change rendered loudness");

	free(padded);
	sound_release(trimmed);
	sound_release(sound_p);
}

///////////////////////////////////////////

static void at_test_norm_peak_cap() {
	// A near-silent tone with one full-scale sample: the crest allowance caps
	// the boost far below what raw RMS normalization would demand.
	int32_t count   = AU_SAMPLE_RATE / 2;
	float*  samples = (float*)malloc(sizeof(float) * count);
	for (int32_t i = 0; i < count; i++)
		samples[i] = sinf((float)i / AU_SAMPLE_RATE * 440.0f * 6.2831853f) * 0.01f;
	samples[0] = 1.0f;

	sound_t sound = sound_create_samples(samples, count);
	sound_set_decibels(sound, AT_UNIT_GAIN_DB);
	sound_play(sound, vec3{0,0,-1});
	double e_l = 0, e_r = 0;
	at_render(count, &e_l, &e_r);
	double rms = sqrt(e_l / count);
	AT_CHECK(rms > 0.003 && rms < 0.02, "peak cap limits normalization of pathological content");

	free(samples);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_decode_direction() {
	sound_t sound = at_generate(at_sine8k, 0.5f);

	// Measured pinna acoustics are deeply notched below and nearly smooth
	// overhead (Blauert's 8kHz "above" band agrees), so at 8kHz an overhead
	// source reads *brighter* than one underneath. Ordering, not golden.
	sound_play(sound, vec3{0, 2, 0});
	double above_e = at_render_energy(AU_SAMPLE_RATE / 2);
	sound_play(sound, vec3{0, -2, 0});
	double below_e = at_render_energy(AU_SAMPLE_RATE / 2);
	AT_CHECK(above_e > below_e * 1.2, "elevation notches separate above from below");

	// Behind loses sparkle relative to front.
	sound_play(sound, vec3{0, 0, -2});
	double front_e = at_render_energy(AU_SAMPLE_RATE / 2);
	sound_play(sound, vec3{0, 0, 2});
	double back_e  = at_render_energy(AU_SAMPLE_RATE / 2);
	AT_CHECK(front_e > back_e * 1.2, "back sources are duller than front");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_spread() {
	sound_t sound = at_generate(at_sine8k, 0.5f);

	// A hard-side point source is strongly lateral, the same source fully
	// spread becomes direction-free and lands even in both ears.
	sound_play_t wide = {}; wide.spread = 1;
	sound_play(sound, vec3{2,0,0}, &wide);
	double l = 0, r = 0;
	at_render(AU_SAMPLE_RATE / 2, &l, &r);
	double balance = r > 0 ? l / r : 0;
	AT_CHECK(balance > 0.8 && balance < 1.25, "full spread is direction-free");

	sound_release(sound);
}

///////////////////////////////////////////

// Deterministic white noise from the absolute sample index.
static float at_noise(float t) {
	uint32_t h = (uint32_t)(t * (float)AU_SAMPLE_RATE + 0.5f) * 2654435761u;
	h ^= h >> 16; h *= 2246822519u; h ^= h >> 13;
	return ((int32_t)(h >> 9) - 4194304) * (1.0f / 4194304.0f) * 0.1f;
}

static void at_gen_noise_w(float* out, uint64_t start, uint64_t frames) {
	for (uint64_t i = 0; i < frames; i++) {
		out[i*4] = at_noise((float)(start + i) / AU_SAMPLE_RATE);
		out[i*4+1] = out[i*4+2] = out[i*4+3] = 0;
	}
}

// Renders like at_render, but returns the normalized interaural correlation.
static double at_render_corr(int32_t frames) {
	static float block[AT_BLOCK * 2];
	double ll = 0, rr = 0, lr = 0;
	while (frames > 0) {
		int32_t count = frames < AT_BLOCK ? frames : AT_BLOCK;
		audio_render_block(block, count);
		for (int32_t i = 0; i < count; i++) {
			double l = block[i*2], r = block[i*2+1];
			ll += l*l; rr += r*r; lr += l*r;
		}
		audio_test_step();
		frames -= count;
	}
	return ll > 0 && rr > 0 ? lr / sqrt(ll * rr) : 1.0;
}

static void at_test_decode_decorrelation() {
	// A diffuse field reaches real ears decorrelated above ~1kHz. The bed is
	// W-only noise - without the decorrelator, interaural correlation is 1.0.
	sound_t bed = sound_generate(at_gen_noise_w, 0.5f, sound_channels_ambisonic1);
	sound_set_decibels(bed, AT_UNIT_GAIN_DB);
	sound_play(bed, vec3{0,0,0});
	double diffuse = at_render_corr(AU_SAMPLE_RATE / 2);
	AT_CHECK(diffuse < 0.7, "a diffuse noise bed decorrelates between the ears");
	at_flush();

	// A frontal point source is one wavefront from one direction - the
	// direct path must keep it fully coherent or transients smear.
	sound_t point = at_generate(at_noise, 0.5f);
	sound_play(point, vec3{0,0,-2});
	double coherent = at_render_corr(AU_SAMPLE_RATE / 2);
	AT_CHECK(coherent > 0.9, "a frontal point source stays coherent");

	sound_release(bed);
	sound_release(point);
}

///////////////////////////////////////////

static void at_test_limiter() {
	// 110dB right at the head clamps way past full scale, the soft limiter
	// must squash it below 1.0 without ever letting it through.
	sound_t sound = at_generate(at_sine, 0.5f);
	sound_set_decibels(sound, 110);
	sound_play(sound, vec3{0, 0, -0.1f});

	static float block[AT_BLOCK * 2];
	float peak = 0;
	for (int32_t f = 0; f < AU_SAMPLE_RATE / 2; f += AT_BLOCK) {
		audio_render_block(block, AT_BLOCK);
		for (int32_t i = 0; i < AT_BLOCK * 2; i++)
			if (fabsf(block[i]) > peak) peak = fabsf(block[i]);
		audio_test_step();
	}
	// tanh saturates to exactly 1.0f in float math, so the bound is
	// inclusive: the guarantee is never *exceeding* full scale.
	AT_CHECK(peak > 0.89f && peak <= 1.0f, "the limiter squashes hot mixes to full scale at most");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_headlocked() {
	sound_t sound = at_generate(at_sine, 0.25f);

	// Way off to the right, but head-locked: both ears equal, and no
	// distance attenuation at all.
	sound_play_t locked = {}; locked.flags = sound_flags_head_locked;
	sound_play(sound, vec3{50,0,0}, &locked);
	double l = 0, r = 0;
	at_render(AU_SAMPLE_RATE/2, &l, &r);
	AT_CHECK(l > 0.001, "head-locked voice ignores distance");
	AT_CHECK(fabs(l - r) < l * 0.01, "head-locked voice is equal in both ears");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_master_bus_meter() {
	sound_t sound = at_generate(at_sine, 0.25f);

	// Read the meter mid-sound, the last block of a longer render window
	// would already be silence.
	sound_play_t music = {}; music.bus = sound_bus_music;
	sound_play(sound, vec3{0,0,-1}, &music);
	double base_e = at_render_energy(AU_SAMPLE_RATE/8);
	float  dbfs   = audio_get_output_decibels();
	AT_CHECK(dbfs > -120 && dbfs < 0, "output meter reads during playback");
	base_e += at_render_energy(AU_SAMPLE_RATE/2 - AU_SAMPLE_RATE/8);

	audio_set_bus_volume(sound_bus_music, 0.5f);
	sound_play(sound, vec3{0,0,-1}, &music);
	double bus_e = at_render_energy(AU_SAMPLE_RATE/2);
	audio_set_bus_volume(sound_bus_music, 1);
	double ratio = bus_e > 0 ? base_e / bus_e : 0;
	AT_CHECK(ratio > 3.6 && ratio < 4.4, "bus volume 0.5 is -6dB");

	audio_set_volume(0.5f);
	sound_play(sound, vec3{0,0,-1}, &music);
	double master_e = at_render_energy(AU_SAMPLE_RATE/2);
	audio_set_volume(1);
	ratio = master_e > 0 ? base_e / master_e : 0;
	AT_CHECK(ratio > 3.6 && ratio < 4.4, "master volume 0.5 is -6dB");
	AT_CHECK(audio_get_volume() == 1 && audio_get_bus_volume(sound_bus_music) == 1, "volume getters round-trip");

	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_shapes() {
	sound_t sound = at_generate(at_sine8k, 2.0f);

	// A sphere emitter's spread is its subtended angle, evaluated
	// synchronously at play: r=1 at 2m is asin(0.5) -> ~0.33.
	vec3 center = {0, 0, -2};
	sound_play_t ball = {}; ball.flags = sound_flags_loop;
	ball.shape_points = &center; ball.shape_point_count = 1; ball.shape_radius = 1;
	sound_inst_t sphere = sound_play(sound, vec3{0,0,0}, &ball);
	float sphere_spread = sound_inst_get_spread(sphere);
	AT_CHECK(sphere_spread > 0.28f && sphere_spread < 0.38f, "sphere spread matches its subtended angle at play time");
	sound_inst_stop(sphere);
	at_render(AT_BLOCK * 2, nullptr, nullptr);

	// A stream: polyline along X, 3m in front, half meter radius.
	vec3 line[2] = {{-10, 0, -3}, {10, 0, -3}};
	sound_play_t shaped = {}; shaped.flags = sound_flags_loop;
	shaped.shape_points = line; shaped.shape_point_count = 2; shaped.shape_radius = 0.5f;
	sound_inst_t inst = sound_play(sound, vec3{0,0,0}, &shaped);

	// The emitter sits on the shape's surface, radius shy of the core.
	vec3 pos = sound_inst_get_pos(inst);
	AT_CHECK(fabsf(pos.x) < 0.01f && fabsf(pos.z + 2.5f) < 0.01f, "shaped play resolves its emit point immediately");

	// Walking along the bank, the emit point follows.
	pose_t walk = {{5, 0, 0}, {0,0,0,1}};
	audio_set_listener(&walk);
	for (int32_t i = 0; i < 60; i++) audio_test_step();
	pos = sound_inst_get_pos(inst);
	AT_CHECK(fabsf(pos.x - 5) < 0.1f && fabsf(pos.z + 2.5f) < 0.1f, "the emit point follows the listener along the polyline");

	// Far away the stream is narrow, close up it fills the view.
	pose_t far_p = {{5, 0, 17}, {0,0,0,1}};
	audio_set_listener(&far_p);
	for (int32_t i = 0; i < 60; i++) audio_test_step();
	float spread_far = sound_inst_get_spread(inst);
	pose_t near_p = {{5, 0, -2.4f}, {0,0,0,1}};
	audio_set_listener(&near_p);
	for (int32_t i = 0; i < 60; i++) audio_test_step();
	float spread_near = sound_inst_get_spread(inst);
	AT_CHECK(spread_near > spread_far + 0.3f, "spread grows approaching the shape");

	// 0.1m outside the surface the emitter is 0.1m from the head, so the
	// step inside (emitter at the head) is continuous, not a volume pop.
	pos = sound_inst_get_pos(inst);
	AT_CHECK(vec3_magnitude(pos - near_p.position) < 0.15f, "just outside, the emitter sits on the surface by the listener");

	// Standing in the stream it surrounds you completely.
	pose_t inside = {{5, 0, -3}, {0,0,0,1}};
	audio_set_listener(&inside);
	for (int32_t i = 0; i < 60; i++) audio_test_step();
	pos = sound_inst_get_pos(inst);
	AT_CHECK(sound_inst_get_spread(inst) > 0.99f, "inside the shape is fully diffuse");
	AT_CHECK(vec3_magnitude(pos - inside.position) < 0.05f, "inside the shape the emitter is at the head");
	AT_CHECK(at_render_energy(AU_SAMPLE_RATE / 4) > 0.001, "a fully diffuse inside-shape voice renders energy");

	// A teleport while inside carries the emitter with the head immediately -
	// smoothing is listener-relative, so there's no world-space lag to fade
	// back from (a lagged emitter reads as a dropout in the ambient bed).
	pose_t teleport = {{-8, 0, -3}, {0,0,0,1}};
	audio_set_listener(&teleport);
	audio_test_step();
	pos = sound_inst_get_pos(inst);
	AT_CHECK(vec3_magnitude(pos - teleport.position) < 0.05f, "a teleport inside the shape moves the emitter instantly");

	// An explicit position clears the shape and stays put.
	sound_inst_set_pos(inst, vec3{1, 2, 3});
	for (int32_t i = 0; i < 5; i++) audio_test_step();
	pos = sound_inst_get_pos(inst);
	AT_CHECK(pos.x == 1 && pos.y == 2 && pos.z == 3, "set_pos turns a shaped emitter back into a point");

	audio_set_listener(nullptr);
	audio_test_step();
	sound_inst_stop(inst);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_test_channel_formats() {
	char path[AT_PATH_MAX];

	// A stereo file with tone on the left only: it plays head-locked with
	// its image intact, even when "positioned" hard right.
	at_temp_path(path, sizeof(path), "audio_test_stereo.wav");
	if (at_write_wav_ch(path, 1, 0.25f, 2, 1, 440.0f)) {
		sound_t stereo = at_load_wav(path);
		AT_CHECK(stereo != nullptr && sound_get_channels(stereo) == sound_channels_stereo, "stereo wav loads as stereo");
		sound_play(stereo, vec3{5,0,0});
		double l = 0, r = 0;
		at_render(AU_SAMPLE_RATE + AT_BLOCK, &l, &r);
		AT_CHECK(l > r * 20, "stereo content keeps its image and ignores position");
		sound_release(stereo);
		remove(path);
	}

	// A 4 channel file loads as an ambisonic bed with no separate loader
	// call, and a W-only bed is omni: head rotation must not change energy.
	at_temp_path(path, sizeof(path), "audio_test_ambi_w.wav");
	if (at_write_wav_ch(path, 1, 0.25f, 4, 1, 8000.0f)) {
		sound_t bed = sound_create(path);
		for (int32_t i = 0; i < 500 && sound_duration(bed) == 0; i++) at_sleep_ms(10);
		AT_CHECK(sound_get_channels(bed) == sound_channels_ambisonic1, "4ch wav loads as ambisonic");

		sound_play(bed, vec3{0,0,0});
		double id_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
		pose_t flipped = {{0,0,0}, quat_from_angles(0, 180, 0)};
		audio_set_listener(&flipped);
		audio_test_step();
		sound_play(bed, vec3{0,0,0});
		double flip_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
		double ratio  = flip_e > 0 ? id_e / flip_e : 0;
		AT_CHECK(ratio > 0.95 && ratio < 1.05, "an omni ambisonic bed is rotation invariant");
		audio_set_listener(nullptr);
		audio_test_step();
		sound_release(bed);
		remove(path);
	}

	// W+X together make a forward-facing cardioid field: turning around
	// pushes its energy through the duller back corners.
	at_temp_path(path, sizeof(path), "audio_test_ambi_wx.wav");
	if (at_write_wav_ch(path, 1, 0.25f, 4, 1 | 8, 8000.0f)) {
		sound_t front = sound_create(path);
		for (int32_t i = 0; i < 500 && sound_duration(front) == 0; i++) at_sleep_ms(10);

		sound_play(front, vec3{0,0,0});
		double id_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
		pose_t flipped = {{0,0,0}, quat_from_angles(0, 180, 0)};
		audio_set_listener(&flipped);
		audio_test_step();
		sound_play(front, vec3{0,0,0});
		double flip_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
		AT_CHECK(id_e > flip_e * 1.1, "a forward ambisonic field counter-rotates against the head");
		audio_set_listener(nullptr);
		audio_test_step();
		sound_release(front);
		remove(path);
	}

	// A FuMa-tagged file converts to ambiX on load: its X channel sits in
	// FuMa slot 1, which would read as ambiX Y (left/right, yaw-symmetric)
	// without the reorder. Converted, it's the same forward cardioid as the
	// ambiX W+X case - front-heavy, so a 180 yaw drops energy.
	at_temp_path(path, sizeof(path), "audio_test_ambi_fuma.wav");
	if (at_write_wav_fuma(path, 1, 0.177f, 0.25f, 8000.0f)) {
		sound_t fuma = sound_create(path);
		for (int32_t i = 0; i < 500 && sound_duration(fuma) == 0; i++) at_sleep_ms(10);
		AT_CHECK(sound_get_channels(fuma) == sound_channels_ambisonic1, "a FuMa-tagged wav loads as ambisonic");

		sound_play(fuma, vec3{0,0,0});
		double id_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
		pose_t flipped = {{0,0,0}, quat_from_angles(0, 180, 0)};
		audio_set_listener(&flipped);
		audio_test_step();
		sound_play(fuma, vec3{0,0,0});
		double flip_e = at_render_energy(AU_SAMPLE_RATE + AT_BLOCK);
		AT_CHECK(id_e > flip_e * 1.1, "FuMa channel order converts to ambiX on load");
		audio_set_listener(nullptr);
		audio_test_step();
		sound_release(fuma);
		remove(path);
	}
}

///////////////////////////////////////////

static void at_test_create_mem() {
	char path[AT_PATH_MAX];
	at_temp_path(path, sizeof(path), "audio_test_mem.wav");
	if (!at_write_wav(path, 1, 0.25f)) return;

	FILE* file = fopen(path, "rb");
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	void* bytes = malloc(size);
	size_t read = fread(bytes, 1, size, file);
	fclose(file);
	remove(path);
	if (read != (size_t)size) { free(bytes); return; }

	sound_t sound = sound_create_mem("test/mem_wav", bytes, size);
	free(bytes); // create_mem copies, the caller's buffer stays ours
	for (int32_t i = 0; i < 500 && sound_duration(sound) == 0; i++) at_sleep_ms(10);
	AT_CHECK(sound_duration(sound) > 0.9f && sound_duration(sound) < 1.1f, "sounds load from memory");

	sound_play(sound, vec3{0,0,-1});
	double energy = at_render_energy(AU_SAMPLE_RATE / 4);
	AT_CHECK(energy > 0.001, "memory-loaded sounds play");
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);
	sound_release(sound);
}

///////////////////////////////////////////

static void at_gen_stereo_left(float* out, uint64_t start, uint64_t frames) {
	for (uint64_t i = 0; i < frames; i++) {
		out[i*2  ] = at_sine8k((float)(start + i) / AU_SAMPLE_RATE);
		out[i*2+1] = 0;
	}
}

static void at_gen_ambi_w(float* out, uint64_t start, uint64_t frames) {
	for (uint64_t i = 0; i < frames; i++) {
		out[i*4] = at_sine8k((float)(start + i) / AU_SAMPLE_RATE);
		out[i*4+1] = out[i*4+2] = out[i*4+3] = 0;
	}
}

static void at_test_generate_channels() {
	// Generated stereo keeps its image, exactly like loaded stereo.
	sound_t stereo = sound_generate(at_gen_stereo_left, 0.5f, sound_channels_stereo);
	AT_CHECK(sound_get_channels(stereo) == sound_channels_stereo, "generated stereo reports its format");
	sound_play(stereo, vec3{5,0,0});
	double l = 0, r = 0;
	at_render(AU_SAMPLE_RATE / 2 + AT_BLOCK, &l, &r);
	AT_CHECK(l > r * 20, "generated stereo keeps its image and ignores position");
	sound_release(stereo);

	// A generated W-only ambisonic bed is rotation invariant.
	sound_t bed = sound_generate(at_gen_ambi_w, 0.5f, sound_channels_ambisonic1);
	AT_CHECK(sound_get_channels(bed) == sound_channels_ambisonic1, "generated ambisonics report their format");
	sound_play(bed, vec3{0,0,0});
	double id_e = at_render_energy(AU_SAMPLE_RATE / 2 + AT_BLOCK);
	pose_t flipped = {{0,0,0}, quat_from_angles(0, 180, 0)};
	audio_set_listener(&flipped);
	audio_test_step();
	sound_play(bed, vec3{0,0,0});
	double flip_e = at_render_energy(AU_SAMPLE_RATE / 2 + AT_BLOCK);
	double ratio  = flip_e > 0 ? id_e / flip_e : 0;
	AT_CHECK(ratio > 0.95 && ratio < 1.05, "a generated omni ambisonic bed is rotation invariant");
	audio_set_listener(nullptr);
	audio_test_step();
	sound_release(bed);

	// Interleaved samples work for FromSamples-style creation too.
	float* frames = (float*)malloc(sizeof(float) * 4800 * 2);
	for (int32_t i = 0; i < 4800; i++) { frames[i*2] = at_sine8k(i / 48000.0f); frames[i*2+1] = 0; }
	sound_t from = sound_create_samples(frames, 4800 * 2, sound_channels_stereo);
	AT_CHECK(sound_get_channels(from) == sound_channels_stereo && sound_total_samples(from) == 4800 * 2, "create_samples takes interleaved stereo");
	free(frames);
	sound_release(from);
}

///////////////////////////////////////////

static void at_test_stream_rate() {
	// A 16kHz stream: 0.1s of source data should render as 0.1s of output
	// through the rate converter, and report duration at its own rate.
	sound_t stream = sound_create_stream(0.5f, sound_channels_mono, sound_sample_rate_speech);
	sound_set_decibels(stream, AT_UNIT_GAIN_DB);

	float samples[1600];
	for (int32_t i = 0; i < 1600; i++)
		samples[i] = sinf((float)i / 16000.0f * 440.0f * 6.2831853f) * 0.1f;
	sound_write_samples(stream, samples, 1600);
	AT_CHECK(sound_duration(stream) > 0.09f && sound_duration(stream) < 0.11f, "stream duration uses its own sample rate");

	sound_inst_t inst = sound_play(stream, vec3{0,0,-1});
	double during = at_render_energy(AU_SAMPLE_RATE / 10);
	at_flush();
	double after  = at_render_energy(AU_SAMPLE_RATE / 10);
	AT_CHECK(during > 0.001,  "a 16khz stream renders through the rate converter");
	AT_CHECK(after  < 0.0001, "16khz source data lasts its real duration");

	sound_inst_stop(inst);
	at_render(AT_BLOCK * 2, nullptr, nullptr);
	sound_release(stream);
}

///////////////////////////////////////////

static void at_gen_impulse(float* out, uint64_t start, uint64_t count) {
	for (uint64_t i = 0; i < count; i++)
		out[i] = (start + i) == 0 ? 1.0f : 0.0f;
}

static int32_t at_argmax_ear(const float* block, int32_t frames, int32_t ear) {
	int32_t best = 0;
	float   bv   = -1;
	for (int32_t i = 0; i < frames; i++) {
		float v = fabsf(block[i*2 + ear]);
		if (v > bv) { bv = v; best = i; }
	}
	return best;
}

// The direct path gives each point source its own exact ITD - the azimuth
// precision cue - so it gets asserted at the sample level.
static void at_test_direct_itd() {
	sound_t imp = sound_generate(at_gen_impulse, 0.05f, sound_channels_mono);
	sound_set_decibels(imp, AT_UNIT_GAIN_DB);
	static float block[AT_BLOCK * 2];

	// Hard left: the right ear lags by the full interaural delay, ~31.5
	// samples at 48kHz. The near ear is normalized to zero delay.
	sound_inst_t inst = sound_play(imp, vec3{-3, 0, 0});
	audio_render_block(block, AT_BLOCK);
	audio_test_step();
	int32_t lag = at_argmax_ear(block, AT_BLOCK, 1) - at_argmax_ear(block, AT_BLOCK, 0);
	AT_CHECK(lag >= 28 && lag <= 35, "hard-left source: right ear lags ~31 samples");
	sound_inst_stop(inst);
	at_flush();

	// Frontal: both ears aligned.
	inst = sound_play(imp, vec3{0, 0, -3});
	audio_render_block(block, AT_BLOCK);
	audio_test_step();
	lag = at_argmax_ear(block, AT_BLOCK, 1) - at_argmax_ear(block, AT_BLOCK, 0);
	AT_CHECK(lag >= -2 && lag <= 2, "frontal source: ears aligned");
	sound_inst_stop(inst);
	at_flush();

	sound_release(imp);
}

// Four+ point voices render through the SIMD batch path, fewer take the
// scalar path. They must agree: four coherent copies sum to exactly 4x the
// amplitude (+12dB), and batched ear taps keep the sample-exact ITD.
static void at_test_direct_batch() {
	sound_t      sine = at_generate(at_sine8k, 0.25f);
	vec3         pos  = vec3{-2, 0.3f, -1};
	sound_play_t loop = {}; loop.flags = sound_flags_loop;

	sound_inst_t one = sound_play(sine, pos, &loop);
	double       e1  = at_render_energy(AT_BLOCK * 4);
	sound_inst_stop(one);
	at_flush();

	sound_inst_t four[4];
	for (int32_t i = 0; i < 4; i++) four[i] = sound_play(sine, pos, &loop);
	double e4 = at_render_energy(AT_BLOCK * 4);
	for (int32_t i = 0; i < 4; i++) sound_inst_stop(four[i]);
	at_flush();
	AT_CHECK(fabs(10.0 * log10(e4 / e1) - 12.04) < 1.0, "4 coherent batched voices sum to 4x amplitude");

	// Batched ITD: four hard-left impulses, right ear still lags ~31.
	sound_t imp = sound_generate(at_gen_impulse, 0.05f, sound_channels_mono);
	sound_set_decibels(imp, AT_UNIT_GAIN_DB);
	static float block[AT_BLOCK * 2];
	for (int32_t i = 0; i < 4; i++) four[i] = sound_play(imp, vec3{-3, 0, 0});
	audio_render_block(block, AT_BLOCK);
	audio_test_step();
	int32_t lag = at_argmax_ear(block, AT_BLOCK, 1) - at_argmax_ear(block, AT_BLOCK, 0);
	AT_CHECK(lag >= 28 && lag <= 35, "batched hard-left voices keep the ~31 sample ITD");
	for (int32_t i = 0; i < 4; i++) sound_inst_stop(four[i]);
	at_flush();

	sound_release(imp);
	sound_release(sine);
}

// Direct and bus rendering must hold the same loudness, or spread
// animation (and the A/B toggle) would double as a volume knob.
static void at_test_direct_bus_level() {
	sound_t      sine = at_generate(at_sine, 0.25f);
	vec3         pos  = vec3{2, 0.5f, -2};
	sound_play_t loop = {}; loop.flags = sound_flags_loop;

	sound_inst_t inst     = sound_play(sine, pos, &loop);
	double       e_direct = at_render_energy(AT_BLOCK * 4);
	sound_inst_stop(inst);
	at_flush();

	audio_test_force_bus(true);
	inst = sound_play(sine, pos, &loop);
	double e_bus = at_render_energy(AT_BLOCK * 4);
	sound_inst_stop(inst);
	at_flush();
	audio_test_force_bus(false);

	double ratio_db = 10.0 * log10(e_direct / e_bus);
	AT_CHECK(fabs(ratio_db) < 1.5, "direct and bus paths render within 1.5dB");

	// Mid-crossfade holds level too - a spread sweep shouldn't dip.
	sound_play_t mid = loop; mid.spread = 0.25f;
	inst = sound_play(sine, pos, &mid);
	double e_mid = at_render_energy(AT_BLOCK * 4);
	sound_inst_stop(inst);
	at_flush();
	AT_CHECK(fabs(10.0 * log10(e_mid / e_direct)) < 2.0, "spread crossfade holds level within 2dB");

	sound_release(sine);
}

///////////////////////////////////////////

static float at_sine18k(float t) { return sinf(t * 18000.0f * 6.2831853f) * 0.1f; }

static void at_test_near_field() {
	// Near-field ILD: inside ~1m the per-ear path lengths add broadband ILD,
	// so a hard-left source splits far more at 30cm than at 3m.
	sound_t sound = at_generate(at_sine8k, 0.25f);
	double  l = 0, r = 0;
	sound_play(sound, vec3{-0.3f, 0, 0});
	at_render(AU_SAMPLE_RATE / 2, &l, &r);
	double near_ild = r > 0 ? l / r : 0;
	sound_play(sound, vec3{-3, 0, 0});
	at_render(AU_SAMPLE_RATE / 2, &l, &r);
	double far_ild = r > 0 ? l / r : 0;
	AT_CHECK(near_ild > far_ild * 2, "near sources carry extra broadband ILD");
	sound_release(sound);

	// Within ~4.5m the air filter fades to a true bypass - a bright close
	// source keeps its top octave, within ~1dB of head-locked.
	sound_t bright = at_generate(at_sine18k, 0.25f);
	sound_play_t locked = {}; locked.flags = sound_flags_head_locked;
	sound_play(bright, vec3{0, 0, -1}, &locked);
	double locked_e  = at_render_energy(AU_SAMPLE_RATE / 2);
	sound_play(bright, vec3{0, 0, -1});
	double spatial_e = at_render_energy(AU_SAMPLE_RATE / 2);
	double ratio_db  = spatial_e > 0 ? 10.0 * log10(locked_e / spatial_e) : 99;
	AT_CHECK(ratio_db < 2.0, "the air filter is a true bypass at close range");
	sound_release(bright);
}

///////////////////////////////////////////

static void at_test_environment() {
	sound_t burst = at_generate(at_noise, 0.15f);

	// Environment off - the default - leaves true silence after a voice
	// finishes and its decode state rings down.
	sound_play(burst, vec3{0, 0, -2});
	at_render(AU_SAMPLE_RATE / 2, nullptr, nullptr);
	double dry_tail = at_render_energy(AU_SAMPLE_RATE / 4);
	AT_CHECK(dry_tail < 1e-7, "no environment leaves no tail");

	// A room leaves a decaying tail behind the same burst.
	audio_set_env(at_env_room);
	sound_play(burst, vec3{0, 0, -2});
	at_render(AU_SAMPLE_RATE / 4, nullptr, nullptr);
	double tail_a = at_render_energy(AU_SAMPLE_RATE / 4);
	double tail_b = at_render_energy(AU_SAMPLE_RATE / 4);
	AT_CHECK(tail_a > 1e-6, "an environment leaves a reverb tail");
	AT_CHECK(tail_b < tail_a * 0.7, "the tail decays");
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);

	// The send ignores distance while the source shares the space - similar
	// tails under direct signals ~10dB apart is the absolute distance cue.
	// Past ~half the size the send parallels the direct falloff instead.
	sound_play(burst, vec3{0, 0, -1});
	at_render(AU_SAMPLE_RATE / 4, nullptr, nullptr);
	double tail_near = at_render_energy(AU_SAMPLE_RATE / 4);
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);
	sound_play(burst, vec3{0, 0, -3});
	at_render(AU_SAMPLE_RATE / 4, nullptr, nullptr);
	double tail_mid = at_render_energy(AU_SAMPLE_RATE / 4);
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);
	sound_play(burst, vec3{0, 0, -12});
	at_render(AU_SAMPLE_RATE / 4, nullptr, nullptr);
	double tail_far = at_render_energy(AU_SAMPLE_RATE / 4);
	AT_CHECK(tail_mid > tail_near * 0.4 && tail_mid < tail_near * 2.5, "the tail is distance-constant within the space");
	AT_CHECK(tail_far < tail_near * 0.4 && tail_far > tail_near * 0.01, "the send falls off beyond the space");
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);

	// With reflect up, an impulse grows wall bounce copies in the 10-110ms
	// window that a reflection-free space lacks. Wet is near zero to keep the
	// tail away, and the always-on floor tap (~5ms) lands inside the skip.
	sound_t imp = sound_generate(at_gen_impulse, 0.05f, sound_channels_mono);
	sound_set_decibels(imp, AT_UNIT_GAIN_DB);
	audio_env_t er_env = { 0.01f, 0.3f, 0.5f, 8, 0.5f, 1.0f };
	audio_set_env(er_env);
	sound_play(imp, vec3{0, 0, -2});
	at_render(480, nullptr, nullptr);
	double er_e = at_render_energy(AU_SAMPLE_RATE / 10);
	at_render(AU_SAMPLE_RATE / 2, nullptr, nullptr);
	er_env.reflect = 0;
	audio_set_env(er_env);
	sound_play(imp, vec3{0, 0, -2});
	at_render(480, nullptr, nullptr);
	double flat_e = at_render_energy(AU_SAMPLE_RATE / 10);
	AT_CHECK(er_e > flat_e * 2, "reflections add early bounce copies");
	sound_release(imp);

	audio_set_env(at_env_off);
	at_render(AU_SAMPLE_RATE, nullptr, nullptr);
	sound_release(burst);
}

///////////////////////////////////////////
// Mixer performance benchmark. Times audio_render_block - the audio
// thread's entire per-block cost - in real-time sized blocks, so results
// read directly as "% of one core at device rate".

#if defined(_WIN32)
static int64_t at_time_ns() {
	static LARGE_INTEGER freq = {};
	if (freq.QuadPart == 0) QueryPerformanceFrequency(&freq);
	LARGE_INTEGER t; QueryPerformanceCounter(&t);
	return (int64_t)((double)t.QuadPart * (1000000000.0 / (double)freq.QuadPart));
}
#else
static int64_t at_time_ns() {
	timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
	return (int64_t)t.tv_sec * 1000000000 + t.tv_nsec;
}
#endif

#define AT_BENCH_FRAMES 480 // 10ms at 48kHz, a typical device period
#define AT_BENCH_BLOCKS 1000

// The fastest block is the true hardware cost - frequency scaling and
// scheduler noise only slow blocks down. Median is typical, worst is spikes.
static void at_bench(const char* name) {
	static float   block[AT_BENCH_FRAMES * 2];
	static int64_t times[AT_BENCH_BLOCKS];
	for (int32_t i = 0; i < 50; i++) { // Settle ranking, ramp the clocks
		audio_render_block(block, AT_BENCH_FRAMES);
		audio_test_step();
	}

	for (int32_t i = 0; i < AT_BENCH_BLOCKS; i++) {
		int64_t t0 = at_time_ns();
		audio_render_block(block, AT_BENCH_FRAMES);
		times[i] = at_time_ns() - t0;
		audio_test_step();
	}
	for (int32_t i = 1; i < AT_BENCH_BLOCKS; i++) { // Insertion sort
		int64_t v = times[i];
		int32_t j = i - 1;
		while (j >= 0 && times[j] > v) { times[j+1] = times[j]; j -= 1; }
		times[j+1] = v;
	}
	double min_us = (double)times[0]                   / 1000.0;
	double med_us = (double)times[AT_BENCH_BLOCKS / 2] / 1000.0;
	double rt_us  = 1000000.0 * AT_BENCH_FRAMES / AU_SAMPLE_RATE;
	log_infof("[audio_bench] %-30s min %7.1fus  med %7.1fus  worst %7.1fus  %5.2f%% of realtime",
		name, min_us, med_us, (double)times[AT_BENCH_BLOCKS-1] / 1000.0, 100.0 * min_us / rt_us);
}

// Golden-angle ring at 1-8m, so distance gain and air absorption vary
// across voices the way a real scene's would.
static vec3 at_bench_pos(int32_t i) {
	float ang  = (float)i * 0.618034f * 6.2831853f;
	float dist = 1.0f + (float)(i % 8);
	return vec3{ sinf(ang) * dist, (float)(i % 5 - 2) * 0.4f, cosf(ang) * dist };
}

static void at_bench_stop(sound_inst_t* insts, int32_t count) {
	for (int32_t i = 0; i < count; i++) sound_inst_stop(insts[i]);
	at_flush();
}

static void at_gen_stereo_pair(float* out, uint64_t start, uint64_t frames) {
	for (uint64_t i = 0; i < frames; i++) {
		float t = (float)(start + i) / AU_SAMPLE_RATE;
		out[i*2  ] = at_sine(t);
		out[i*2+1] = at_sine(t * 1.01f);
	}
}

int audio_bench_run() {
	audio_test_offline(true);

	sk_settings_t settings = {};
	settings.app_name     = "StereoKitC Audio Bench";
	settings.mode         = app_mode_offscreen;
	settings.standby_mode = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[audio_bench] sk_init failed");
		return 1;
	}

	sound_t mono   = at_generate(at_sine, 0.25f);
	sound_t ambi   = sound_generate(at_gen_ambi_w,      0.25f, sound_channels_ambisonic1);
	sound_t stereo = sound_generate(at_gen_stereo_pair, 0.25f, sound_channels_stereo);
	sound_set_decibels(ambi,   AT_UNIT_GAIN_DB);
	sound_set_decibels(stereo, AT_UNIT_GAIN_DB);

	sound_play_t loop    = {}; loop.flags    = sound_flags_loop;
	sound_play_t pitched = {}; pitched.flags = sound_flags_loop; pitched.pitch = 1.31f;
	sound_play_t locked  = {}; locked.flags  = (sound_flags_)(sound_flags_loop | sound_flags_head_locked);

	sound_inst_t insts[AU_VOICE_COUNT] = {};
	log_infof("[audio_bench] %d blocks of %d frames, %d voice budget",
		AT_BENCH_BLOCKS, AT_BENCH_FRAMES, AU_MIX_VOICES);

	at_bench("fixed cost, 0 voices");

	// One voice keeps the FOA decode awake: this row is the decode's cost.
	insts[0] = sound_play(mono, at_bench_pos(0), &loop);
	at_bench("1 spatial voice");
	at_bench_stop(insts, 1);

	for (int32_t i = 0; i < 64; i++) insts[i] = sound_play(mono, at_bench_pos(i), &loop);
	at_bench("64 spatial voices");
	at_bench_stop(insts, 64);

	// Full-spread voices skip the direct path and render through the bus.
	sound_play_t wide = {}; wide.flags = sound_flags_loop; wide.spread = 1.0f;
	for (int32_t i = 0; i < 64; i++) insts[i] = sound_play(mono, at_bench_pos(i), &wide);
	at_bench("64 diffuse voices, bus");
	at_bench_stop(insts, 64);

	for (int32_t i = 0; i < 64; i++) insts[i] = sound_play(mono, at_bench_pos(i), &pitched);
	at_bench("64 spatial voices, pitched");
	at_bench_stop(insts, 64);

	// The full environment: per-voice sends and reflection taps, plus the
	// shared FDN. The delta against plain 64-voice is the env's price.
	audio_set_env(at_env_room);
	for (int32_t i = 0; i < 64; i++) insts[i] = sound_play(mono, at_bench_pos(i), &loop);
	at_bench("64 spatial voices, environment");
	at_bench_stop(insts, 64);
	audio_set_env(at_env_off);
	at_bench_stop(insts, 0); // Just settles the wet ramp-down

	// A demo-shaped blend: beds and music alongside the transient swarm.
	for (int32_t i = 0; i <  4; i++) insts[i] = sound_play(ambi,   vec3_zero,       &loop);
	for (int32_t i = 4; i <  8; i++) insts[i] = sound_play(stereo, vec3_zero,       &loop);
	for (int32_t i = 8; i < 16; i++) insts[i] = sound_play(mono,   vec3_zero,       &locked);
	for (int32_t i =16; i < 64; i++) insts[i] = sound_play(mono,   at_bench_pos(i), &pitched);
	at_bench("64 voice blend");
	at_bench_stop(insts, 64);

	// Oversubscribed pool: dormant voices should cost the audio thread
	// nothing, this should match the plain 64-voice number.
	for (int32_t i = 0; i < AU_VOICE_COUNT; i++) insts[i] = sound_play(mono, at_bench_pos(i), &loop);
	{ // Sanity: the budget should be fully granted, the rest dormant.
		static float block[AT_BENCH_FRAMES * 2];
		for (int32_t i = 0; i < 5; i++) { audio_render_block(block, AT_BENCH_FRAMES); audio_test_step(); }
		int32_t playing = 0, mixing = 0;
		for (int32_t i = 0; i < AU_VOICE_COUNT; i++) {
			if (sound_inst_is_playing   (insts[i]))     playing += 1;
			if (sound_inst_get_intensity(insts[i]) > 0) mixing  += 1;
		}
		log_infof("[audio_bench] probe: %d playing, %d mixed last block", playing, mixing);
	}
	at_bench("128 voices, 64 dormant");

	// Main-thread cost with a full pool: ranking, smoothing, drains.
	{
		static float block[AT_BENCH_FRAMES * 2];
		int64_t total = 0, worst = 0;
		for (int32_t i = 0; i < AT_BENCH_BLOCKS; i++) {
			audio_render_block(block, AT_BENCH_FRAMES);
			int64_t t0 = at_time_ns();
			audio_test_step();
			int64_t dt = at_time_ns() - t0;
			total += dt;
			if (dt > worst) worst = dt;
		}
		log_infof("[audio_bench] %-30s avg %8.1fus  worst %8.1fus  (per main frame)",
			"main step, 128 voices", (double)total / AT_BENCH_BLOCKS / 1000.0, (double)worst / 1000.0);
	}
	at_bench_stop(insts, AU_VOICE_COUNT);

	sound_release(mono);
	sound_release(ambi);
	sound_release(stereo);
	sk_shutdown();
	return 0;
}

///////////////////////////////////////////

int audio_stress_run() {
	// Live device on purpose: this hammers the real audio callback. Explicit
	// so the mode is declared rather than inherited from the global default.
	audio_test_offline(false);

	sk_settings_t settings = {};
	settings.app_name     = "StereoKitC Audio Stress";
	settings.mode         = app_mode_offscreen;
	settings.standby_mode = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[audio_stress] sk_init failed");
		return 1;
	}

	sound_t snd    = at_generate(at_sine, 0.3f);
	sound_t stream = sound_create_stream(0.5f);
	float   chunk[512];
	for (int32_t i = 0; i < 512; i++) chunk[i] = at_sine((float)i / AU_SAMPLE_RATE);

	// More handles than voices keeps the steal path hot the whole run.
	sound_inst_t insts[256] = {};
	uint32_t     rng        = 12345;

	for (int32_t frame = 0; frame < 600; frame++) {
		for (int32_t i = 0; i < 8; i++) {
			rng = rng*1664525u + 1013904223u; uint32_t pick = rng % 256;
			if (sound_inst_is_playing(insts[pick])) {
				rng = rng*1664525u + 1013904223u; uint32_t op = rng % 3;
				if      (op == 0) sound_inst_stop      (insts[pick]);
				else if (op == 1) sound_inst_set_pos   (insts[pick], vec3{(float)(rng%10)-5, 0, (float)(rng%7)-3});
				else              sound_inst_set_volume(insts[pick], (rng%100)*0.01f);
			} else {
				rng = rng*1664525u + 1013904223u;
				insts[pick] = sound_play(rng%4 == 0 ? stream : snd, vec3{(float)(rng%10)-5, 0, -1});
			}
		}
		sound_write_samples(stream, chunk, 512);
		sk_step(nullptr);
	}

	sound_release(snd);
	sound_release(stream);
	sk_shutdown();
	log_info("[audio_stress] completed");
	return 0;
}

///////////////////////////////////////////

int audio_tests_run() {
	audio_test_offline(true);

	sk_settings_t settings = {};
	settings.app_name     = "StereoKitC Audio Tests";
	settings.mode         = app_mode_offscreen;
	settings.standby_mode = standby_mode_none;
	if (!sk_init(settings)) {
		log_err("[audio_test] sk_init failed");
		return 1;
	}

	at_test_polyphony();
	at_test_stop();
	at_test_stop_declick();
	at_test_gain_zipper();
	at_test_steal();
	at_test_virtual_voices();
	at_test_cmd_overflow();
	at_test_pan();
	at_test_ring_stream();
	at_test_ring_shared();
	at_test_write_overwrites_oldest();
	at_test_file_streaming();
	at_test_deferred_play();
	at_test_pitch();
	at_test_loop();
	at_test_pause_seek();
	at_test_seek_multichannel();
	at_test_delay();
	at_test_attenuation();
	at_test_normalization();
	at_test_norm_gating();
	at_test_norm_peak_cap();
	at_test_headlocked();
	at_test_master_bus_meter();
	at_test_decode_direction();
	at_test_spread();
	at_test_decode_decorrelation();
	at_test_limiter();
	at_test_shapes();
	at_test_channel_formats();
	at_test_generate_channels();
	at_test_create_mem();
	at_test_stream_rate();
	at_test_direct_itd();
	at_test_direct_bus_level();
	at_test_direct_batch();
	at_test_near_field();
	at_test_environment();

	sk_shutdown();

	if (at_failures == 0) log_info("[audio_test] all tests passed!");
	else                  log_errf("[audio_test] %d checks failed!", at_failures);
	return at_failures;
}
