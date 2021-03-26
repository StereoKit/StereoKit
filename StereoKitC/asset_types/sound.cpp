#include "../stereokit.h"
#include "../sk_memory.h"
#include "../asset_types/assets.h"
#include "../systems/platform/platform_utils.h"
#include "../systems/audio.h"
#include "sound.h"

#include <string.h>

namespace sk {

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

	const char *sound_file = assets_file(filename);
	void  *data;
	size_t length;
	if (!platform_read_file(sound_file, &data, &length))
		return nullptr;

	result = (_sound_t*)assets_allocate(asset_type_sound);
	sound_set_id(result, filename);

	ma_decoder_config config = ma_decoder_config_init(AU_SAMPLE_FORMAT, AU_CHANNEL_COUNT, AU_SAMPLE_RATE);
	if (ma_decoder_init_memory(data, length, &config, &result->decoder) != MA_SUCCESS) {
		log_errf("Failed to parse sound '%s'.", sound_file);
		return nullptr;
	}
	return result;
}

///////////////////////////////////////////

sound_t sound_generate(float (*function)(float), float duration) {
	sound_t result = (_sound_t*)assets_allocate(asset_type_sound);

	result->sound_data = sk_malloc_t<float>(duration * AU_SAMPLE_RATE);
	float *data = (float*)result->sound_data;
	for (uint32_t i = 0, s = (size_t)(duration * AU_SAMPLE_RATE); i < s; i += 1) {
		data[i] = function((float)i / (float)AU_SAMPLE_RATE);
	}

	ma_decoder_config config = ma_decoder_config_init(AU_SAMPLE_FORMAT, AU_CHANNEL_COUNT, AU_SAMPLE_RATE);
	if (ma_decoder_init_memory_raw(result->sound_data, sizeof(float) * (duration * AU_SAMPLE_RATE), &config, &config, &result->decoder) != MA_SUCCESS) {
		log_err("Failed to generate sound!");
		free(result->sound_data);
		return nullptr;
	}
	return result;
}

///////////////////////////////////////////

void sound_play(sound_t sound, vec3 at, float volume) {
	ma_decoder_seek_to_pcm_frame(&sound->decoder, 0);

	for (size_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == sound) {
			au_active_sounds[i] = {sound, at, volume};
			return;
		}
	}

	for (size_t i = 0; i < _countof(au_active_sounds); i++) {
		if (au_active_sounds[i].sound == nullptr) {
			assets_addref(sound->header);
			au_active_sounds[i] = { sound, at, volume };
			return;
		}
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

}