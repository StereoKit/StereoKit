#include "../stereokit.h"
#include "../asset_types/assets.h"
#include "sound.h"

#define DR_WAV_IMPLEMENTATION
#include "../libraries/dr_wav.h"   /* Enables WAV decoding. */

#define MINIAUDIO_IMPLEMENTATION
#include "../libraries/miniaudio.h"

namespace sk {

uint32_t     au_active_count = 0;
sound_inst_t au_active_sounds[8];
float        au_mix_temp[4096];

ma_decoder_config au_decoder_config = {};
ma_device_config  au_config = {};
ma_device         au_device = {};

#define SAMPLE_FORMAT   ma_format_f32
#define CHANNEL_COUNT   1
#define SAMPLE_RATE     48000

///////////////////////////////////////////

sound_t sound_find(const char *id) {
    sound_t result = (sound_t)assets_find(id, asset_type_sound);
    if (result != nullptr) {
        assets_addref(result->header);
        return result;
    }
    return nullptr;
}

///////////////////////////////////////////

void sound_set_id(sound_t sound, const char *id) {
    assets_set_id(sound->header, id);
}

///////////////////////////////////////////

sound_t sound_create(const char *filename) {
    sound_t result = sound_find(filename);
    if (result != nullptr)
        return result;
    result = (_sound_t*)assets_allocate(asset_type_sound);
    sound_set_id(result, filename);

    const char *sound_file = assets_file(filename);

    au_decoder_config = ma_decoder_config_init(SAMPLE_FORMAT, CHANNEL_COUNT, SAMPLE_RATE);
    if (ma_decoder_init_file(sound_file, &au_decoder_config, &result->decoder) != MA_SUCCESS) {
        log_errf("Failed to load sound '%s'.", sound_file);
        return nullptr;
    }
    return result;
}

///////////////////////////////////////////

sound_t sound_generate(float (*function)(float), float duration) {
    sound_t result = (_sound_t*)assets_allocate(asset_type_sound);

    au_decoder_config = ma_decoder_config_init(SAMPLE_FORMAT, CHANNEL_COUNT, SAMPLE_RATE);
    result->sound_data = malloc(sizeof(float) * duration * SAMPLE_RATE);
    float *data = (float*)result->sound_data;
    for (uint32_t i = 0, s = duration * SAMPLE_RATE; i < s; i += 1) {
        data[i] = function((float)i / SAMPLE_RATE);
    }
    
    au_decoder_config = ma_decoder_config_init(SAMPLE_FORMAT, CHANNEL_COUNT, SAMPLE_RATE);
    if (ma_decoder_init_memory_raw(result->sound_data, sizeof(float) * duration * SAMPLE_RATE, &au_decoder_config, &au_decoder_config, &result->decoder) != MA_SUCCESS) {
        log_err("Failed to generate sound!");
        free(result->sound_data);
        return nullptr;
    }
    return result;
}

///////////////////////////////////////////

void sound_play(sound_t sound, vec3 at, float volume) {
    int32_t found = -1;
    for (size_t i = 0; i < au_active_count; i++) {
        if (au_active_sounds[i].sound == sound) {
            found = i;
            break;
        }
    }
    if (found == -1 && au_active_count < _countof(au_active_sounds)) {
        found = au_active_count;
        au_active_count += 1;
    }

    if (found != -1) {
        ma_decoder_seek_to_pcm_frame(&sound->decoder, 0);
        au_active_sounds[found].sound    = sound;
        au_active_sounds[found].position = at;
        au_active_sounds[found].volume   = volume;
    }
}

///////////////////////////////////////////

void sound_release(sound_t sound) {
    if (sound == nullptr)
        return;

    assets_releaseref(sound->header);
}

///////////////////////////////////////////

void sound_destroy(sound_t sound) {
    ma_decoder_uninit(&sound->decoder);
    free(sound->sound_data);
    memset(sound, 0, sizeof(_sound_t));
}

///////////////////////////////////////////

ma_uint32 read_and_mix_pcm_frames_f32(sound_inst_t &inst, float* pOutputF32, ma_uint32 frame_count) {
    // The way mixing works is that we just read into a temporary buffer, then take the contents of that buffer and mix it with the
    // contents of the output buffer by simply adding the samples together. You could also clip the samples to -1..+1, but I'm not
    // doing that in this example.

    vec3 head_pos = input_head().position;

    ma_uint32 tempCapInFrames = _countof(au_mix_temp) / CHANNEL_COUNT;
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
        float volume = (1.f / dist) * inst.volume;

        // Mix the frames together.
        for (ma_uint32 sample = 0; sample < frames_read*CHANNEL_COUNT; ++sample) {
            pOutputF32[total_frames_read*CHANNEL_COUNT + sample] += au_mix_temp[sample] * volume;
        }

        total_frames_read += frames_read;
        if (frames_read < frames_to_read) {
            break;  // Reached EOF.
        }
    }

    return total_frames_read;
}

///////////////////////////////////////////

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    float* pOutputF32 = (float*)pOutput;
    ma_uint32 iDecoder;

    ma_assert(pDevice->playback.format == SAMPLE_FORMAT);   // <-- Important for this example.

    for (uint32_t i = 0; i < au_active_count; i++) {
        ma_uint32 framesRead = read_and_mix_pcm_frames_f32(au_active_sounds[i], pOutputF32, frameCount);
        if (framesRead < frameCount) {
            ma_decoder_seek_to_pcm_frame(&au_active_sounds[i].sound->decoder, 0);
            memmove(&au_active_sounds[i], &au_active_sounds[i + 1], sizeof(ma_decoder *) * (au_active_count - (i + 1)));
            au_active_count--;
            i--;
        }
    }

    (void)pInput;
}

///////////////////////////////////////////

bool sound_init() {
    
    au_config = ma_device_config_init(ma_device_type_playback);
    au_config.playback.format   = SAMPLE_FORMAT;
    au_config.playback.channels = CHANNEL_COUNT;
    au_config.sampleRate        = SAMPLE_RATE;
    au_config.dataCallback      = data_callback;
    au_config.pUserData         = nullptr;

    if (ma_device_init(NULL, &au_config, &au_device) != MA_SUCCESS) {
        log_err("Failed to open playback device.");
        return false;
    }

    if (ma_device_start(&au_device) != MA_SUCCESS) {
        log_err("Failed to start playback device.\n");
        ma_device_uninit(&au_device);
        return false;
    }
    return true;
}

///////////////////////////////////////////

void sound_update() {
}

///////////////////////////////////////////

void sound_shutdown() {
    ma_device_uninit (&au_device);
}

}