#pragma once

#include "../libraries/miniaudio.h"
#include "../libraries/ferr_thread.h"
#include "assets.h"

namespace sk {

typedef enum sound_data_ {
	sound_data_none = 0,
	sound_data_pcm,         // Fully decoded/generated samples in memory
	sound_data_stream_file, // Compressed source bytes, decoded per-voice
	sound_data_ring,        // Live ring buffer, mic or user fed
} sound_data_;

struct _sound_t {
	asset_header_t  header;
	sound_data_     data_type;
	sound_channels_ channels;   // Interleaved when more than one
	float           decibels;
	float           norm_gain;  // Loudness normalization from load-time RMS
	int32_t         sample_rate;
	bool32_t        fuma;       // Stream bytes are FuMa order, convert on read

	// sound_data_pcm
	float*         pcm;
	uint64_t       pcm_count;   // Frames

	// sound_data_stream_file, bytes owned by the sound
	void*          file_data;
	size_t         file_size;
	uint64_t       file_frames; // Decoded length

	// sound_data_ring: a broadcast ring. The write head is monotonic and
	// never wraps; writers publish it with a release store. Playing voices
	// read lock-free from their own absolute cursors without consuming
	// anything, so every voice hears the whole stream. The lock serializes
	// writers and the public API's consume-style reads only, the audio
	// thread never takes it.
	float*         ring_data;
	uint64_t       ring_capacity; // Frames
	uint64_t       ring_write_at; // Total frames ever written, atomic
	uint64_t       ring_read_at;  // Public consume cursor, under ring_lock
	ft_mutex_t     ring_lock;
};

void  sound_destroy      (sound_t sound);
float decibels_to_signal (float decibel);
void  sound_fuma_to_ambix(float* frames, uint64_t frame_count);

// Main thread, once per frame: submit plays that were waiting on their
// sound's async decode, catching the cursor up to the elapsed wait.
void  sound_play_pending_step();

// Audio thread: lock-free broadcast read from a ring sound at an absolute
// frame cursor, which advances (and may snap forward past overwritten
// frames). Returns frames read; never blocks, never consumes.
uint64_t sound_ring_read_at(sound_t sound, uint64_t* cursor, float* dest, uint64_t frames);

inline int32_t sound_channel_count(sound_channels_ channels) {
	switch (channels) {
	case sound_channels_stereo:     return 2;
	case sound_channels_ambisonic1: return 4;
	default:                        return 1;
	}
}

} // namespace sk
