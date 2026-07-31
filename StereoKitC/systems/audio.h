#pragma once

#include "../stereokit.h"
#include "../libraries/miniaudio.h"

namespace sk {

#define AU_SAMPLE_RATE        48000
#define AU_SAMPLE_BUFFER_SIZE 10 // ITD padding samples on each side
#define AU_SAMPLE_FORMAT      ma_format_f32
// More slots than the mixer renders: the top AU_MIX_VOICES by audibility
// render, the rest go dormant *in place* and resume when they rank back in.
#define AU_VOICE_COUNT        128
#define AU_MIX_VOICES         64
#define AU_CMD_RING_SIZE      256
#define AU_PREDECODE_MAX      (AU_SAMPLE_RATE*10)  // Frames, larger streams
#define AU_STREAM_PREFETCH    (AU_SAMPLE_RATE/4)   // Per-voice ring, frames
#define AU_PITCH_MIN          0.25f
#define AU_PITCH_MAX          4.0f
#define AU_MIN_DISTANCE       0.25f  // Attenuation clamp, ~inside the head
#define AU_ITD_MAX            64     // Ear tap ring, samples - pow2 masked wrap
#define AU_VOICING_STAGES     4      // Voicing biquads: N1, N2, P1, shelf
#define AU_DIRECT_SPREAD      0.5f   // Spread where a voice is fully bus-rendered
#define AU_SEEK_NONE          UINT64_MAX
#define AU_BUS_COUNT          4
#define AU_SHAPE_MAX_POINTS   32
// Plays made while the sound was still decoding catch up to real time on
// load; loads faster than the grace start from the top instead.
#define AU_PLAY_GRACE         0.1f
// A mid-waveform start or seek is a step function, an audible click, so
// those onsets ramp in over this many frames (~5ms).
#define AU_FADE_FRAMES        240
#define AU_ER_TAPS            6      // Environment reflection images per voice
// Reflection taps go to the most audible spatial voices only - quiet voices'
// bounces are masked anyway, and the cap bounds cost under voice swarms.
#define AU_ER_VOICES          16
#define AU_SMOOTH_TIME        0.05f  // Emit point smoothing constant, sec

// Voice lifecycle, the value is atomic. Main reserves free slots, audio
// activates them on the play command, finished voices wait for a main drain.
// Stealing moves playing back to reserved, resources hand off at activation.
typedef enum au_voice_state_ {
	au_voice_free = 0,
	au_voice_reserved,
	au_voice_playing,
	au_voice_finished,
} au_voice_state_;

// Cross-thread parameter snapshot, each field individually atomic. A torn
// *set* of fields is fine, single fields must not tear.
struct au_voice_params_t {
	float    pos_x, pos_y, pos_z;
	float    volume;   // 0-1 trim over the sound's decibel loudness
	float    pitch;    // Playback rate, AU_PITCH_MIN..AU_PITCH_MAX
	float    spread;   // 0 = point source, 1 = fully diffuse
	float    cutoff;   // Low-pass Hz override, 0 = automatic model
	int32_t  flags;    // sound_flags_
	int32_t  paused;
	int32_t  stop_request;
	uint64_t seek_request; // Target frame, AU_SEEK_NONE when empty
};

struct au_voice_t {
	// Main-thread-owned. Shapes are evaluated on main each frame, the
	// audio thread only ever consumes the resolved position/spread params.
	uint16_t          id;              // Generation, increments on reserve
	float             intensity_frame;
	vec3              shape_points[AU_SHAPE_MAX_POINTS];
	int32_t           shape_count;     // 0 = plain point source
	float             shape_radius;
	float             base_spread;     // The play settings' spread floor
	vec3              smooth_offset;   // Emit smoothing, listener-relative
	float             smooth_spread;
	bool              smooth_init;
	bool              wait_load;       // Reserved, submits once the sound loads
	double            wait_started;    // au_main_clock at the play call

	// Cross-thread, atomic access only
	int32_t           state;           // au_voice_state_
	int32_t           audible;         // In the mix budget this frame
	int32_t           er_grant;        // In the reflection budget this frame
	au_voice_params_t params;
	float             intensity;       // Peak |sample| since last frame
	int32_t           stream_eof;      // Prefetch decoder hit end of data

	// Written by main only while reserved, consumed by the audio thread at
	// activation. The command ring publish makes them visible.
	sound_t           pending_sound;
	uint64_t          pending_cursor;
	uint64_t          pending_delay;   // Onset delay in frames
	sound_bus_        pending_bus;
	int32_t           pending_fade;    // Mid-waveform start, ramp the onset
	ma_decoder*       pending_decoder;
	ma_pcm_rb*        pending_ring;
	float*            pending_ring_data;

	// Audio-thread-owned once playing. Main only reads where a race is
	// provably benign, and frees through return ring entries, never the
	// voice. cursor writes are relaxed atomics for the main-thread getter.
	sound_t           sound;
	uint64_t          cursor;          // Frames, in *source* samples
	uint64_t          delay_left;      // Onset frames remaining
	sound_bus_        bus;
	float             fade_gain;       // Onset/seek ramp, 1 = no fade
	float             resample_frac;   // Pitch resampler phase, 0-1
	float             resample_last[3][4]; // Last 3 consumed frames, per channel
	ma_decoder*       stream_decoder;  // sound_data_stream_file voices only
	ma_pcm_rb*        stream_ring;
	float*            stream_ring_data;
	float             lpf_state[4];    // Air absorption 4-pole cascade state

	// Direct binaural state for point sources, audio thread owned. The
	// ring holds the voice's filtered mono history for the two ear taps.
	float             dir_ring[AU_ITD_MAX];
	int32_t           dir_ring_at;
	float             dir_shoulder;    // Shoulder bounce delay, samples; -1 = snap
	float             er_delay[AU_ER_TAPS]; // Slewed tap delay per surface; -1 = snap
	float             er_lp, er_lp2;   // Shared surface-absorption two-pole
	float             dir_delay[2];    // Smoothed per-ear delay, samples; -1 = snap
	float             dir_shadow[2];   // Per-ear head shadow one-pole state
	float             dir_filter[AU_VOICING_STAGES][4]; // Voicing biquad states, [stage][x1 x2 y1 y2]
};

bool audio_init    ();
void audio_step    ();
void audio_shutdown();

void audio_pause ();
void audio_resume();

#if defined(SK_OS_WINDOWS)
void audio_set_default_device_in (const wchar_t *id);
void audio_set_default_device_out(const wchar_t *id);
#endif

// Voice pool interface, main thread only. Stopping isn't here: setting a
// voice's stop_request param is complete on its own, so it can't fail.
int16_t audio_voice_reserve(sound_t sound, vec3 at, float volume); // Slot, or -1
bool    audio_voice_submit (int16_t slot);          // False = ring full, undo
void    audio_voice_prefetch();                     // Top up streaming rings
void    audio_voice_rank   ();                      // Audibility -> mix budget
void    audio_voice_shapes_step(vec3 listener_pos, float dt);
void    audio_voice_shape_set  (au_voice_t* voice, const vec3* points, int32_t count, float radius, vec3 listener_pos); // Copy + immediate eval

// Listener pose snapshot, published by audio_step, read by the mixer.
void   audio_listener_publish(pose_t pose);
pose_t audio_listener_get    ();

// audio_set_listener's stored override, lives in audio.cpp. The offline
// harness publishes it so listener-dependent tests can steer the head.
extern pose_t au_listener_override;
extern bool   au_listener_has_override;

// Main-thread wall clock in seconds, stepped by audio_step (and the offline
// test step, at a fixed rate for determinism). Times deferred play catch-up.
extern double au_main_clock;

// Offline harness for deterministic tests: enable offline *before* sk_init to
// skip device creation, then pump blocks with render_block and main-thread
// work with test_step. Exported so test executables can reach them.
SK_API void audio_render_block(float* out_stereo, int32_t frame_count);
SK_API void audio_test_offline(bool32_t enable);
SK_API void audio_test_step   ();
// Advances the main-thread clock without a step, so deferred play catch-up
// tests can simulate a slow asset load.
SK_API void audio_test_advance(float seconds);
// A/B hook: force point sources through the FOA bus (the pre-direct-
// binaural render path), switchable live for listening comparisons.
SK_API void audio_test_force_bus(bool32_t enable);
extern bool32_t   au_offline;
extern au_voice_t au_voices[AU_VOICE_COUNT];

void audio_mix_init    (int32_t period_frames);
void audio_mix_shutdown();  // Call with the device stopped, drains everything
void audio_mix_drain_returns();

void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count);

} // namespace sk
