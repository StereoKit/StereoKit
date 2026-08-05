#include "sound.h"
#include "assets.h"
#include "../stereokit.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../platforms/platform.h"
#include "../systems/audio.h"
#include "../libraries/ferr_thread.h"
#include "../libraries/atomic_util.h"

#include <string.h>
#include <math.h>

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

struct sound_load_t {
	void*    file_data;
	size_t   file_size;
};

// FuMa B-format marker: the .amb convention tags its wav with an ambisonic
// subformat GUID (PCM 01 or float 03 variant) in a WAVE_FORMAT_EXTENSIBLE
// fmt chunk. ambiX carries no marker at all.
static bool sound_wav_is_fuma(const uint8_t* data, size_t size) {
	static const uint8_t guid_tail[12] = {
		0x21,0x07,0xd3,0x11, 0x86,0x44, 0xc8,0xc1,0xca,0x00,0x00,0x00 };
	if (size < 12 || memcmp(data, "RIFF", 4) != 0 || memcmp(data + 8, "WAVE", 4) != 0)
		return false;
	size_t at = 12;
	while (at + 8 <= size) {
		uint32_t chunk;
		memcpy(&chunk, data + at + 4, 4);
		if (memcmp(data + at, "fmt ", 4) == 0) {
			const uint8_t* fmt = data + at + 8;
			if (chunk < 40 || at + 8 + 40 > size) return false;
			return fmt[0] == 0xFE && fmt[1] == 0xFF // WAVE_FORMAT_EXTENSIBLE
			    && (fmt[24] == 0x01 || fmt[24] == 0x03)
			    && fmt[25] == 0 && fmt[26] == 0 && fmt[27] == 0
			    && memcmp(fmt + 28, guid_tail, 12) == 0;
		}
		at += 8 + chunk + (chunk & 1);
	}
	return false;
}

// The .amb extension implies FuMa even without the GUID - older tools wrote
// plain headers and relied on the name.
static bool sound_id_is_amb(const char* id) {
	size_t len = strlen(id);
	if (len < 4) return false;
	const char* ext = id + len - 4;
	return ext[0] == '.'
	    && (ext[1] == 'a' || ext[1] == 'A')
	    && (ext[2] == 'm' || ext[2] == 'M')
	    && (ext[3] == 'b' || ext[3] == 'B');
}

// FuMa B-format (W,X,Y,Z with W at -3dB) to ambiX (W,Y,Z,X SN3D), in place.
// First order maxN matches SN3D on the directional channels, so this is a
// reorder plus restoring W to unity weight.
void sound_fuma_to_ambix(float* frames, uint64_t frame_count) {
	for (uint64_t i = 0; i < frame_count; i++) {
		float* f = frames + i * 4;
		float  x = f[1];
		f[0] *= 1.4142135f;
		f[1]  = f[2];
		f[2]  = f[3];
		f[3]  = x;
	}
}

// Loudness normalization makes declared decibels truthful: the waveform
// is the *shape* of a sound, decibels are how loud it is, and measuring
// the content is what enforces that separation. Measurement is gated RMS
// in the style of EBU R128: 50ms blocks, and blocks more than 30dB below
// the loudest are ignored - silence and quiet tails don't dilute the
// measure, so padding a sound can't make it louder. The result scales
// content loudness to unity, decibels_to_signal provides the rest.
// Intra-sound dynamics are always preserved, this is one gain per asset.
// A generous 18dB crest allowance backstops pathological content (a
// near-silent file with one hot sample), while musical crest like drum
// or thunder transients rides declared loudness into the limiter instead.
// measure_ch selects one interleaved channel (ambisonics measure W, the
// omni loudness), or -1 for all of them.
static float sound_norm_gain(const float* samples, uint64_t frames, int32_t channels, int32_t measure_ch) {
	if (frames == 0) return 1;

	const uint64_t block_size = AU_SAMPLE_RATE / 20; // 50ms, in frames
	int32_t ch_start = measure_ch < 0 ? 0        : measure_ch;
	int32_t ch_end   = measure_ch < 0 ? channels : measure_ch + 1;
	int32_t ch_count = ch_end - ch_start;

	// One pass over the samples: each block's mean square plus the content
	// peak, then the gate reads from the cached blocks.
	uint64_t block_count = (frames + block_size - 1) / block_size;
	double*  block_ms    = sk_malloc_t(double, (size_t)block_count);
	uint64_t block       = 0;
	double   loudest     = 0;
	float    peak        = 0;
	for (uint64_t at = 0; at < frames; at += block_size) {
		uint64_t end = mini(at + block_size, frames);
		double   ms  = 0;
		for (uint64_t i = at; i < end; i++) {
			for (int32_t c = ch_start; c < ch_end; c++) {
				float s = samples[i*channels + c];
				ms += (double)s * s;
				if (fabsf(s) > peak) peak = fabsf(s);
			}
		}
		ms /= (double)((end - at) * ch_count);
		if (ms > loudest) loudest = ms;
		block_ms[block++] = ms;
	}
	if (loudest <= 0) { sk_free(block_ms); return 1; }

	// Average the blocks within 30dB of the loudest.
	double   gate = loudest * 0.001;
	double   sum  = 0;
	uint64_t n    = 0;
	for (uint64_t b = 0; b < block_count; b++)
		if (block_ms[b] >= gate) { sum += block_ms[b]; n += 1; }
	sk_free(block_ms);

	float norm = 1.0f / (float)sqrt(sum / (double)n);
	if (peak * norm > 8.0f) norm = 8.0f / peak;
	return fmaxf(0.3f, fminf(300.f, norm));
}

static bool32_t sound_load_decode(asset_task_t*, asset_header_t* asset, void* job_data) {
	sound_t       sound = (sound_t)asset;
	sound_load_t* data  = (sound_load_t*)job_data;

	// Probe with the file's own channel count, then constrain: 4 channels
	// reads as first order ambisonics - the only 4ch content XR apps see in
	// practice - and other surround layouts downmix to stereo.
	ma_decoder        decoder;
	ma_decoder_config config = ma_decoder_config_init(AU_SAMPLE_FORMAT, 0, AU_SAMPLE_RATE);
	if (ma_decoder_init_memory(data->file_data, data->file_size, &config, &decoder) != MA_SUCCESS) {
		log_errf("Failed to parse sound '%s'.", sound->header.id_text);
		atomic_store_i32_rel((int32_t*)&sound->header.state, asset_state_error_unsupported);
		return false;
	}
	ma_uint32 file_ch = decoder.outputChannels;
	bool      fuma    = false;
	if (file_ch == 4) {
		// FuMa-tagged files convert to the ambiX frame the mixer speaks;
		// untagged 4ch is ambiX already by the dominant convention.
		sound->channels = sound_channels_ambisonic1;
		fuma = sound_wav_is_fuma((const uint8_t*)data->file_data, data->file_size)
		    || sound_id_is_amb(sound->header.id_text);
	} else if (file_ch > 2) {
		log_warnf("Sound '%s' has %u channels, downmixing to stereo.", sound->header.id_text, file_ch);
		ma_decoder_uninit(&decoder);
		config = ma_decoder_config_init(AU_SAMPLE_FORMAT, 2, AU_SAMPLE_RATE);
		if (ma_decoder_init_memory(data->file_data, data->file_size, &config, &decoder) != MA_SUCCESS) {
			atomic_store_i32_rel((int32_t*)&sound->header.state, asset_state_error_unsupported);
			return false;
		}
		sound->channels = sound_channels_stereo;
	} else if (file_ch == 2) {
		sound->channels = sound_channels_stereo;
	} else {
		sound->channels = sound_channels_mono;
	}
	int32_t ch = sound_channel_count(sound->channels);

	ma_uint64 frames = 0;
	if (ma_decoder_get_length_in_pcm_frames(&decoder, &frames) != MA_SUCCESS)
		frames = 0;

	// data_type and state are the publication points for main thread
	// readers: their release stores pair with acquire loads in the getters,
	// making the plain field writes above them visible.
	if (frames > 0 && frames <= AU_PREDECODE_MAX) {
		float*    pcm  = sk_malloc_t(float, (size_t)(frames * ch));
		ma_uint64 read = 0;
		ma_decoder_read_pcm_frames(&decoder, pcm, frames, &read);
		ma_decoder_uninit(&decoder);
		if (fuma) sound_fuma_to_ambix(pcm, read);

		sound->pcm       = pcm;
		sound->pcm_count = read;
		sound->norm_gain = sound_norm_gain(pcm, read, ch, sound->channels == sound_channels_ambisonic1 ? 0 : -1);
		atomic_store_i32_rel((int32_t*)&sound->data_type, sound_data_pcm);
	} else {
		// Long or unknown-length sounds keep their compressed bytes, and
		// each playing voice gets its own decoder over them.
		ma_decoder_uninit(&decoder);
		sound->file_data   = data->file_data;
		sound->file_size   = data->file_size;
		sound->file_frames = frames;
		sound->fuma        = fuma; // Converted at prefetch instead
		data->file_data    = nullptr; // Ownership moved to the sound
		atomic_store_i32_rel((int32_t*)&sound->data_type, sound_data_stream_file);
	}
	atomic_store_i32_rel((int32_t*)&sound->header.state, asset_state_loaded);
	return true;
}

// The loading task publishes these fields from an asset thread, so readers
// that may touch a file sound mid-load go through this acquire.
static sound_data_ sound_data_type(sound_t sound) {
	return (sound_data_)atomic_load_i32_acq((int32_t*)&sound->data_type);
}

static void sound_load_free(asset_header_t*, void* job_data) {
	sound_load_t* data = (sound_load_t*)job_data;
	sk_free(data->file_data);
	sk_free(data);
}

static void sound_load_failure(asset_header_t* asset, void*) {
	if (asset->state >= asset_state_none)
		atomic_store_i32_rel((int32_t*)&asset->state, asset_state_error);
}

static asset_load_action_t sound_load_actions[] = {
	sound_load_decode,
};

///////////////////////////////////////////

// Shared async-decode creation, takes ownership of the data buffer.
static sound_t sound_create_data(const char *id, void *data, size_t length) {
	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);
	result->decibels     = 80;
	result->norm_gain    = 1;
	result->sample_rate  = AU_SAMPLE_RATE;
	result->header.state = asset_state_loading;
	sound_set_id(result, id);

	sound_load_t* load = sk_malloc_zero_t(sound_load_t, 1);
	load->file_data = data;
	load->file_size = length;

	asset_task_t task = {};
	task.asset        = &result->header;
	task.load_data    = load;
	task.free_data    = sound_load_free;
	task.on_failure   = sound_load_failure;
	task.actions      = sound_load_actions;
	task.action_count = 1;
	task.sort         = asset_sort(0, asset_complexity_bytes(length));
	assets_add_task(task);
	return result;
}

///////////////////////////////////////////

sound_t sound_create(const char *filename) {
	sound_t result = sound_find(filename);
	if (result != nullptr)
		return result;

	// A synchronous read keeps the null-on-missing-file contract, the
	// expensive decode happens on the asset threads.
	void*  data;
	size_t length;
	if (!platform_read_file(filename, &data, &length)) {
		log_warnf("Sound file failed to load: %s", filename);
		return nullptr;
	}
	return sound_create_data(filename, data, length);
}

///////////////////////////////////////////

sound_t sound_create_mem(const char *id, const void *data, size_t data_size) {
	sound_t result = sound_find(id);
	if (result != nullptr)
		return result;

	void* copy = sk_malloc(data_size);
	memcpy(copy, data, data_size);
	return sound_create_data(id, copy, data_size);
}

///////////////////////////////////////////

sound_t sound_create_stream(float buffer_duration, sound_channels_ channels, sound_sample_rate_ sample_rate) {
	// 0, or a nonsense negative from a bad enum cast, means the native mix
	// rate. Any other positive rate is honored - the mixer resamples as
	// needed - so we don't clamp to a range here.
	int32_t rate = sample_rate <= 0 ? AU_SAMPLE_RATE : (int32_t)sample_rate;
	int32_t ch   = sound_channel_count(channels);

	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);
	result->data_type     = sound_data_ring;
	result->channels      = channels;
	result->decibels      = 80;
	result->norm_gain     = 1;
	result->sample_rate   = rate;
	result->ring_lock     = ft_mutex_create();
	result->ring_capacity = maxi((int64_t)1, (int64_t)((double)buffer_duration * rate)); // Frames
	result->ring_data     = sk_malloc_t(float, (size_t)(result->ring_capacity * ch));
	memset(result->ring_data, 0, (size_t)(result->ring_capacity * ch * sizeof(float)));
	if (buffer_duration <= 0)
		log_warnf("sound_create_stream: a %.2fs buffer can't hold audio!", buffer_duration);

	result->header.state = asset_state_loaded;
	return result;
}

///////////////////////////////////////////

sound_t sound_create_samples(const float *samples_at_48000s, uint64_t sample_count, sound_channels_ channels) {
	int32_t ch = sound_channel_count(channels);

	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);
	result->data_type   = sound_data_pcm;
	result->channels    = channels;
	result->decibels    = 80;
	result->sample_rate = AU_SAMPLE_RATE;
	result->pcm_count   = sample_count / ch; // Interleaved samples -> frames
	result->pcm         = sk_malloc_t(float, (size_t)(result->pcm_count * ch));
	memcpy(result->pcm, samples_at_48000s, (size_t)(result->pcm_count * ch * sizeof(float)));
	result->norm_gain   = sound_norm_gain(result->pcm, result->pcm_count, ch, channels == sound_channels_ambisonic1 ? 0 : -1);

	result->header.state = asset_state_loaded;
	return result;
}

///////////////////////////////////////////

// The generator's counts are frames: fill frame_count * channel-count
// interleaved samples. For mono that's the same numbers as before.
sound_t sound_generate(void (*audio_generator)(float *out_samples, uint64_t frame_start, uint64_t frame_count), float duration, sound_channels_ channels) {
	int32_t ch = sound_channel_count(channels);

	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);
	result->data_type   = sound_data_pcm;
	result->channels    = channels;
	result->decibels    = 80;
	result->sample_rate = AU_SAMPLE_RATE;
	result->pcm_count   = (uint64_t)((double)duration * AU_SAMPLE_RATE);
	result->pcm         = sk_malloc_t(float, (size_t)(result->pcm_count * ch));
	audio_generator(result->pcm, 0, result->pcm_count);
	result->norm_gain   = sound_norm_gain(result->pcm, result->pcm_count, ch, channels == sound_channels_ambisonic1 ? 0 : -1);

	result->header.state = asset_state_loaded;
	return result;
}

///////////////////////////////////////////

void sound_write_samples(sound_t sound, const float *samples, uint64_t sample_count) {
	if (sound->data_type != sound_data_ring) { log_err("Sound read/write is only supported for streaming type sounds!"); return; }

	// Counts are interleaved samples, the ring works in frames.
	int32_t  ch     = sound_channel_count(sound->channels);
	uint64_t frames = sample_count / ch;
	uint64_t cap    = sound->ring_capacity;

	// Writes larger than the whole ring only keep the freshest frames that fit.
	if (frames > cap) {
		samples += (frames - cap) * ch;
		frames   = cap;
	}

	ft_mutex_lock(sound->ring_lock);
	uint64_t write_at = sound->ring_write_at;
	uint64_t at       = write_at % cap;
	uint64_t first    = mini(frames, cap - at);
	memcpy(sound->ring_data + at * ch, samples, (size_t)(first * ch) * sizeof(float));
	if (first < frames)
		memcpy(sound->ring_data, samples + first * ch, (size_t)((frames - first) * ch) * sizeof(float));

	// The release publishes the data to the audio thread's lock-free reads.
	// A full ring overwrites the oldest frames, new data always lands.
	atomic_store_u64_rel(&sound->ring_write_at, write_at + frames);
	if (write_at + frames - sound->ring_read_at > cap)
		sound->ring_read_at = write_at + frames - cap;
	ft_mutex_unlock(sound->ring_lock);
}

///////////////////////////////////////////

uint64_t sound_read_samples(sound_t sound, float *out_samples, uint64_t sample_count) {
	if (sound->data_type != sound_data_ring) { log_err("Sound read/write is only supported for streaming type sounds!"); return 0; }

	int32_t ch = sound_channel_count(sound->channels);

	// This is the consuming read for capture-style use (mic pipelines);
	// playback voices broadcast-read from their own cursors instead and
	// never touch this cursor.
	ft_mutex_lock(sound->ring_lock);
	uint64_t cap    = sound->ring_capacity;
	uint64_t frames = mini(sample_count / ch, sound->ring_write_at - sound->ring_read_at);
	uint64_t at     = sound->ring_read_at % cap;
	uint64_t first  = mini(frames, cap - at);
	memcpy(out_samples, sound->ring_data + at * ch, (size_t)(first * ch) * sizeof(float));
	if (first < frames)
		memcpy(out_samples + first * ch, sound->ring_data, (size_t)((frames - first) * ch) * sizeof(float));
	sound->ring_read_at += frames;
	ft_mutex_unlock(sound->ring_lock);

	return frames * ch;
}

///////////////////////////////////////////

// Audio thread. Lock-free: acquire the write head, copy, and never touch
// the consume cursor. A cursor lagging near a full ring snaps forward past
// the writer's margin rather than reading frames mid-overwrite.
uint64_t sound_ring_read_at(sound_t sound, uint64_t* cursor, float* dest, uint64_t frames) {
	int32_t  ch       = sound_channel_count(sound->channels);
	uint64_t cap      = sound->ring_capacity;
	uint64_t write_at = atomic_load_u64_acq(&sound->ring_write_at);
	uint64_t margin   = mini(cap / 4, (uint64_t)4800);
	uint64_t oldest   = write_at > cap - margin ? write_at - (cap - margin) : 0;
	if (*cursor < oldest) *cursor = oldest;

	uint64_t read  = mini(frames, write_at - *cursor);
	uint64_t at    = *cursor % cap;
	uint64_t first = mini(read, cap - at);
	memcpy(dest, sound->ring_data + at * ch, (size_t)(first * ch) * sizeof(float));
	if (first < read)
		memcpy(dest + first * ch, sound->ring_data, (size_t)((read - first) * ch) * sizeof(float));
	*cursor += read;
	return read;
}

///////////////////////////////////////////

uint64_t sound_unread_samples(sound_t sound) {
	if (sound->data_type != sound_data_ring) return 0;

	ft_mutex_lock(sound->ring_lock);
	uint64_t frames = sound->ring_write_at - sound->ring_read_at;
	ft_mutex_unlock(sound->ring_lock);
	return frames * sound_channel_count(sound->channels);
}

///////////////////////////////////////////

float sound_get_decibels(sound_t sound) {
	return atomic_load_f32(&sound->decibels);
}

///////////////////////////////////////////

void sound_set_decibels(sound_t sound, float decibels) {
	// Atomic because the mixer reads this per block while playing.
	atomic_store_f32(&sound->decibels, decibels);
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

// Frees a reserved voice that never made it to a submit, releasing the
// pending ref. A previously stolen slot keeps its displaced resources in
// the audio-owned fields, recovered on the next activation/shutdown.
static void sound_voice_abandon(au_voice_t* voice) {
	sound_t sound = voice->pending_sound;
	voice->wait_load     = false;
	voice->pending_sound = nullptr;
	sound_release(sound);
	atomic_store_i32_rel(&voice->state, au_voice_free);
}

// Creates a file-streaming voice's decoder and prefetch ring, seeked to
// `cursor` and prefilled so onset doesn't begin with underrun silence.
// The fill mirrors audio_voice_prefetch: FuMa converts, and loops rewind.
//
// The prefill decodes synchronously on the caller by design, trading main
// thread time for a guaranteed clean onset. It's cheaper than it looks:
// miniaudio builds -O2 even in Debug, and 250ms of stereo mp3 measures
// ~0.25ms on desktop x64. Underruns after onset pad with silence, so if
// this ever needs amortizing, a partial fill here plus a per-frame cap in
// audio_voice_prefetch degrades gracefully.
static bool sound_voice_stream_attach(au_voice_t* voice, sound_t sound, uint64_t cursor, bool loop) {
	int32_t           ch      = sound_channel_count(sound->channels);
	ma_decoder*       decoder = sk_malloc_t(ma_decoder, 1);
	ma_decoder_config config  = ma_decoder_config_init(AU_SAMPLE_FORMAT, (ma_uint32)ch, AU_SAMPLE_RATE);
	if (ma_decoder_init_memory(sound->file_data, sound->file_size, &config, decoder) != MA_SUCCESS) {
		log_errf("Failed to make a play decoder for '%s'.", sound->header.id_text);
		sk_free(decoder);
		return false;
	}
	if (cursor > 0) ma_decoder_seek_to_pcm_frame(decoder, cursor);
	ma_pcm_rb* rb  = sk_malloc_t(ma_pcm_rb, 1);
	float*     buf = sk_malloc_t(float, AU_STREAM_PREFETCH * ch);
	ma_pcm_rb_init(AU_SAMPLE_FORMAT, (ma_uint32)ch, AU_STREAM_PREFETCH, buf, nullptr, rb);

	while (ma_pcm_rb_available_write(rb) > 0) {
		ma_uint32 request = ma_pcm_rb_available_write(rb);
		void*     into    = nullptr;
		if (ma_pcm_rb_acquire_write(rb, &request, &into) != MA_SUCCESS || request == 0) break;
		ma_uint64 decoded = 0;
		ma_result res     = ma_decoder_read_pcm_frames(decoder, into, request, &decoded);
		if (sound->fuma)
			sound_fuma_to_ambix((float*)into, decoded);
		ma_pcm_rb_commit_write(rb, (ma_uint32)decoded);
		if (res != MA_SUCCESS || decoded < request) {
			if (loop && ma_decoder_seek_to_pcm_frame(decoder, 0) == MA_SUCCESS)
				continue;
			atomic_store_i32(&voice->stream_eof, 1);
			break;
		}
	}

	voice->pending_decoder   = decoder;
	voice->pending_ring      = rb;
	voice->pending_ring_data = buf;
	return true;
}

// Attaches the voice's per-play resources at `cursor` and submits the play
// command. The sound must be loaded. On failure the voice frees itself.
static bool sound_voice_submit_ready(au_voice_t* voice, int16_t slot, uint64_t cursor) {
	sound_t sound = voice->pending_sound;
	bool    loop  = (atomic_load_i32(&voice->params.flags) & sound_flags_loop) != 0;

	voice->pending_cursor = cursor;
	if (sound->data_type == sound_data_ring) {
		// Broadcast streams start at the current read front, in absolute
		// stream frames: buffered data plays, and voices never consume.
		ft_mutex_lock(sound->ring_lock);
		voice->pending_cursor = sound->ring_read_at;
		ft_mutex_unlock(sound->ring_lock);
	}
	if (sound->data_type == sound_data_stream_file &&
	    !sound_voice_stream_attach(voice, sound, cursor, loop)) {
		sound_voice_abandon(voice);
		return false;
	}

	if (!audio_voice_submit(slot)) {
		log_err("Audio command ring overflow, refusing to play sound!");
		if (voice->pending_decoder != nullptr) {
			ma_decoder_uninit(voice->pending_decoder);
			sk_free(voice->pending_decoder);
			ma_pcm_rb_uninit(voice->pending_ring);
			sk_free(voice->pending_ring);
			sk_free(voice->pending_ring_data);
			voice->pending_decoder   = nullptr;
			voice->pending_ring      = nullptr;
			voice->pending_ring_data = nullptr;
		}
		sound_voice_abandon(voice);
		return false;
	}
	return true;
}

static sound_inst_t sound_play_settings(sound_t sound, vec3 at, float volume_trim, const sound_play_t* settings) {
	sound_inst_t result;
	result._id   = 0;
	result._slot = -1;

	asset_state_ state = (asset_state_)atomic_load_i32_acq((int32_t*)&sound->header.state);
	if (state < asset_state_none) {
		log_diagf("sound_play: '%s' failed to load.", sound->header.id_text ? sound->header.id_text : "(unnamed)");
		return result;
	}

	// Multi-channel sounds don't spatialize, but the API always takes a
	// position - a zero position is the idiomatic "none", only a real one
	// suggests the caller expected placement to work.
	if (state >= asset_state_loaded &&
	    sound->channels != sound_channels_mono && (at.x != 0 || at.y != 0 || at.z != 0))
		log_diagf("sound_play: '%s' is multi-channel, its position is ignored.", sound->header.id_text ? sound->header.id_text : "(unnamed)");

	// A full pool refusing the least audible sound is normal operation,
	// the caller sees it in the returned handle.
	int16_t slot = audio_voice_reserve(sound, at, volume_trim);
	if (slot < 0)
		return result;
	au_voice_t* voice = &au_voices[slot];

	// Shapes evaluate immediately so the play command carries complete
	// initial state, and propagation delay measures from the real emit
	// point rather than the nominal position.
	vec3 emit_at         = at;
	voice->shape_count   = 0;
	voice->base_spread   = settings->spread;
	voice->smooth_init   = false;
	if (settings->shape_points != nullptr && settings->shape_point_count > 0) {
		vec3 listener = audio_listener_get().position;
		audio_voice_shape_set(voice, settings->shape_points, settings->shape_point_count, settings->shape_radius, listener);
		emit_at = listener + voice->smooth_offset;
	}

	uint64_t delay_frames = (uint64_t)(fmaxf(0, settings->delay) * AU_SAMPLE_RATE + 0.5f);
	if (settings->flags & sound_flags_propagation_delay) {
		float dist = vec3_magnitude(audio_listener_get().position - emit_at);
		delay_frames += (uint64_t)((dist / 343.0f) * AU_SAMPLE_RATE + 0.5f);
	}

	sound_addref(sound);
	voice->pending_sound     = sound;
	voice->pending_cursor    = 0;
	voice->pending_delay     = delay_frames;
	voice->pending_bus       = settings->bus;
	voice->pending_fade      = 0;
	voice->pending_decoder   = nullptr;
	voice->pending_ring      = nullptr;
	voice->pending_ring_data = nullptr;
	atomic_store_i32(&voice->stream_eof, 0);

	// Shaped voices already wrote their evaluated position and spread.
	if (voice->shape_count == 0) {
		atomic_store_f32(&voice->params.pos_x,  at.x);
		atomic_store_f32(&voice->params.pos_y,  at.y);
		atomic_store_f32(&voice->params.pos_z,  at.z);
		atomic_store_f32(&voice->params.spread, settings->spread);
	}
	atomic_store_f32(&voice->params.volume, volume_trim);
	atomic_store_f32(&voice->params.pitch,  settings->pitch);
	atomic_store_f32(&voice->params.cutoff, settings->cutoff);
	atomic_store_i32(&voice->params.flags,  settings->flags);
	atomic_store_i32(&voice->params.paused,       0);
	atomic_store_i32(&voice->params.stop_request, 0);
	atomic_store_u64(&voice->params.seek_request, AU_SEEK_NONE);

	// A sound still decoding holds its reserved slot and submits from
	// sound_play_pending_step once it lands. Playback catches up to real
	// time, as if it had started on schedule. The handle is live right
	// away, and anything set through it applies at activation.
	if (state < asset_state_loaded) {
		voice->wait_load    = true;
		voice->wait_started = au_main_clock;
	} else {
		voice->wait_load = false;
		if (!sound_voice_submit_ready(voice, slot, 0))
			return result;
	}

	result._id   = voice->id;
	result._slot = slot;
	return result;
}

///////////////////////////////////////////

// Main thread, once per frame: resolve plays that were waiting on their
// sound's async decode. The elapsed wait burns the onset delay first, then
// becomes the start cursor: loops wrap, expired one-shots never play, and
// loads inside the grace window start from the top like nothing happened.
void sound_play_pending_step() {
	for (int16_t i = 0; i < AU_VOICE_COUNT; i++) {
		au_voice_t* voice = &au_voices[i];
		if (!voice->wait_load || atomic_load_i32(&voice->state) != au_voice_reserved)
			continue;

		// Stop first: a play nobody wants anymore shouldn't hold its slot
		// hostage until the decode it was waiting on finishes.
		if (atomic_load_i32(&voice->params.stop_request) != 0) {
			sound_voice_abandon(voice);
			continue;
		}

		sound_t      sound = voice->pending_sound;
		asset_state_ state = (asset_state_)atomic_load_i32_acq((int32_t*)&sound->header.state);
		if (state < asset_state_none) {
			// The loader already logged the failure, the handle just dies.
			sound_voice_abandon(voice);
			continue;
		}
		if (state < asset_state_loaded)
			continue;

		voice->wait_load = false;
		uint64_t elapsed = (uint64_t)((au_main_clock - voice->wait_started) * AU_SAMPLE_RATE);
		uint64_t cursor  = 0;
		if (elapsed <= voice->pending_delay) {
			voice->pending_delay -= elapsed;
		} else {
			uint64_t over = elapsed - voice->pending_delay;
			voice->pending_delay = 0;
			if (over > (uint64_t)(AU_PLAY_GRACE * AU_SAMPLE_RATE)) {
				float pitch = atomic_load_f32(&voice->params.pitch);
				pitch  = pitch <= 0 ? 1 : fminf(AU_PITCH_MAX, fmaxf(AU_PITCH_MIN, pitch));
				cursor = (uint64_t)((double)over * pitch);

				uint64_t total = sound->data_type == sound_data_pcm ? sound->pcm_count : sound->file_frames;
				bool     loop  = (atomic_load_i32(&voice->params.flags) & sound_flags_loop) != 0;
				if (loop) {
					if (total > 0) cursor %= total;
				} else if (total > 0 && cursor >= total) {
					// It would already be over, it never plays at all.
					sound_voice_abandon(voice);
					continue;
				}
				// Unknown-length streams can't expire or reliably seek.
				if (total == 0) cursor = 0;
				// A mid-waveform start is a step, ramp the onset in.
				voice->pending_fade = cursor > 0 ? 1 : 0;
			}
		}
		sound_voice_submit_ready(voice, i, cursor);
	}
}

///////////////////////////////////////////

sound_inst_t sound_play(sound_t sound, vec3 at, const sound_play_t *opt_settings) {
	static const sound_play_t defaults = {};
	if (opt_settings == nullptr) opt_settings = &defaults;

	// A zeroed volume means "default", which is full trim. Negatives would
	// flip phase and confuse the audibility ranking, so they clamp silent.
	float trim = opt_settings->volume == 0 ? 1 : fmaxf(0, opt_settings->volume);
	return sound_play_settings(sound, at, trim, opt_settings);
}

///////////////////////////////////////////

static uint64_t sound_total_frames(sound_t sound) {
	switch (sound_data_type(sound)) {
	case sound_data_pcm:         return sound->pcm_count;
	case sound_data_stream_file: return sound->file_frames;
	// Everything the stream has ever seen. Against a voice's absolute
	// cursor, the difference is how much audio is queued ahead of playback.
	case sound_data_ring:        return atomic_load_u64(&sound->ring_write_at);
	default:                     return 0;
	}
}

uint64_t sound_total_samples(sound_t sound) {
	return sound_total_frames(sound) * sound_channel_count(sound->channels);
}

///////////////////////////////////////////

uint64_t sound_cursor_samples(sound_t sound) {
	// With per-voice playback state, an asset level cursor only means
	// something for live streams: how far the consuming reads have gotten.
	// Playing voices track their own positions, see sound_inst_get_cursor.
	if (sound_data_type(sound) != sound_data_ring) return 0;

	ft_mutex_lock(sound->ring_lock);
	uint64_t frames = sound->ring_read_at;
	ft_mutex_unlock(sound->ring_lock);
	return frames * sound_channel_count(sound->channels);
}

///////////////////////////////////////////

float sound_duration(sound_t sound) {
	return (float)sound_total_frames(sound) / (float)sound->sample_rate;
}

///////////////////////////////////////////

sound_channels_ sound_get_channels(sound_t sound) {
	return sound->channels;
}

///////////////////////////////////////////

asset_state_ sound_asset_state(const sound_t sound) {
	return (asset_state_)atomic_load_i32_acq((int32_t*)&sound->header.state);
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
	sk_free(sound->pcm);
	sk_free(sound->file_data);
	if (sound->data_type == sound_data_ring) {
		sk_free          (sound->ring_data);
		ft_mutex_destroy(&sound->ring_lock);
	}
	memset(sound, 0, sizeof(_sound_t));
}

///////////////////////////////////////////

// Voice handles stay valid through reserved and playing, a generation
// mismatch or terminal state means the handle is dead. A stop request kills
// the handle here too: the mixer needs a block to act on it, but the caller
// shouldn't have to wait on the audio thread to observe their own stop.
static au_voice_t* sound_inst_voice(sound_inst_t sound_inst) {
	if (sound_inst._slot < 0 || sound_inst._slot >= AU_VOICE_COUNT) return nullptr;
	au_voice_t* voice = &au_voices[sound_inst._slot];
	if (voice->id != sound_inst._id) return nullptr;
	int32_t state = atomic_load_i32(&voice->state);
	if (state != au_voice_reserved && state != au_voice_playing) return nullptr;
	if (atomic_load_i32(&voice->params.stop_request) != 0) return nullptr;
	return voice;
}

///////////////////////////////////////////

// The mixer's cursor and seeks count frames, the public API counts
// interleaved samples. This resolves the voice's channel count for the
// conversion, reading whichever of the live or pending sound is set.
static int32_t sound_inst_channels(const au_voice_t* voice) {
	_sound_t* snd = (_sound_t*)atomic_load_ptr(&voice->sound);
	if (snd == nullptr) snd = (_sound_t*)atomic_load_ptr(&voice->pending_sound);
	return snd ? sound_channel_count(snd->channels) : 1;
}

///////////////////////////////////////////

void sound_inst_stop(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	// The mixer checks this at the top of every block, and a steal of this
	// slot supersedes it, so a stop can never be lost or overflow anything.
	atomic_store_i32(&voice->params.stop_request, 1);
}

///////////////////////////////////////////

bool32_t sound_inst_is_playing(sound_inst_t sound_inst) {
	return sound_inst_voice(sound_inst) != nullptr;
}

///////////////////////////////////////////

void sound_inst_set_pos(sound_inst_t sound_inst, vec3 pos) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	// An explicit position turns a shaped emitter back into a point.
	voice->shape_count = 0;
	atomic_store_f32(&voice->params.pos_x, pos.x);
	atomic_store_f32(&voice->params.pos_y, pos.y);
	atomic_store_f32(&voice->params.pos_z, pos.z);
}

///////////////////////////////////////////

vec3 sound_inst_get_pos(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return vec3_zero;

	return vec3 {
		atomic_load_f32(&voice->params.pos_x),
		atomic_load_f32(&voice->params.pos_y),
		atomic_load_f32(&voice->params.pos_z) };
}

///////////////////////////////////////////

void sound_inst_set_volume(sound_inst_t sound_inst, float volume_pct) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	// No upper clamp: trims above 1 amplify, which has legitimate uses.
	atomic_store_f32(&voice->params.volume, fmaxf(0, volume_pct));
}

///////////////////////////////////////////

float sound_inst_get_volume(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return 0;

	return atomic_load_f32(&voice->params.volume);
}

///////////////////////////////////////////

float sound_inst_get_intensity(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return 0;

	return voice->intensity_frame;
}

///////////////////////////////////////////

void sound_inst_set_pitch(sound_inst_t sound_inst, float pitch_mult) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	atomic_store_f32(&voice->params.pitch, pitch_mult);
}

///////////////////////////////////////////

float sound_inst_get_pitch(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return 0;

	// Report the rate the mixer will actually use.
	float pitch = atomic_load_f32(&voice->params.pitch);
	return pitch <= 0 ? 1 : fminf(AU_PITCH_MAX, fmaxf(AU_PITCH_MIN, pitch));
}

///////////////////////////////////////////

void sound_inst_set_spread(sound_inst_t sound_inst, float spread_pct) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	// For shaped voices this sets the floor the shape's own width sits on.
	voice->base_spread = spread_pct;
	atomic_store_f32(&voice->params.spread, spread_pct);
}

///////////////////////////////////////////

float sound_inst_get_spread(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return 0;

	return fmaxf(0, fminf(1, atomic_load_f32(&voice->params.spread)));
}

///////////////////////////////////////////

void sound_inst_set_cutoff(sound_inst_t sound_inst, float cutoff_hz) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	atomic_store_f32(&voice->params.cutoff, cutoff_hz);
}

///////////////////////////////////////////

void sound_inst_set_paused(sound_inst_t sound_inst, bool32_t paused) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	atomic_store_i32(&voice->params.paused, paused ? 1 : 0);
}

///////////////////////////////////////////

bool32_t sound_inst_get_paused(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return false;

	return atomic_load_i32(&voice->params.paused) != 0;
}

///////////////////////////////////////////

void sound_inst_seek(sound_inst_t sound_inst, uint64_t sample) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;

	// Streams only read forward, the mixer ignores this for them. The
	// public sample position converts to the mixer's frame cursor.
	atomic_store_u64(&voice->params.seek_request, sample / sound_inst_channels(voice));
}

///////////////////////////////////////////

uint64_t sound_inst_get_cursor(sound_inst_t sound_inst) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return 0;

	// A voice the mixer hasn't activated yet (including plays waiting on
	// their sound's decode) reports its start position; the audio-owned
	// cursor is still the previous play's. Frames convert back out to the
	// public interleaved-sample position.
	uint64_t frames = atomic_load_i32(&voice->state) == au_voice_reserved
		? voice->pending_cursor
		: atomic_load_u64(&voice->cursor);
	return frames * sound_inst_channels(voice);
}

///////////////////////////////////////////

void sound_inst_set_shape(sound_inst_t sound_inst, const vec3 *in_arr_points, int32_t point_count, float radius) {
	au_voice_t* voice = sound_inst_voice(sound_inst);
	if (voice == nullptr) return;
	if (in_arr_points == nullptr || point_count <= 0) { voice->shape_count = 0; return; }

	audio_voice_shape_set(voice, in_arr_points, point_count, radius, audio_listener_get().position);
}

}
