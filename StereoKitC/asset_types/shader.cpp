#include "../stereokit.h"
#include "../sk_memory.h"
#include "../platforms/platform_utils.h"
#include "../libraries/stref.h"
#include "shader.h"
#include "assets.h"

#include <malloc.h>

namespace sk {

///////////////////////////////////////////

shader_t shader_find(const char *id) {
	shader_t result = (shader_t)assets_find(id, asset_type_shader);
	if (result != nullptr) {
		shader_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void shader_set_id(shader_t shader, const char *id) {
	assets_set_id(&shader->header, id);
}

///////////////////////////////////////////

const char* shader_get_id(const shader_t shader) {
	return shader->header.id_text;
}

///////////////////////////////////////////

const char *shader_get_name(shader_t shader) {
	return shader->shader.meta->name;
}

///////////////////////////////////////////

shader_t shader_create_mem(void *data, size_t data_size) {
	char name[256];
	if (!skg_shader_file_verify(data, data_size, nullptr, name, sizeof(name)))
		return nullptr;

	shader_t result = shader_find(name);
	if (result != nullptr)
		return result;

	skg_shader_t shader = skg_shader_create_memory(data, data_size);
	if (!skg_shader_is_valid(&shader)) {
		skg_shader_destroy(&shader);
		return nullptr;
	}

	result = (shader_t)assets_allocate(asset_type_shader);
	result->shader = shader;
	shader_set_id(result, name);

	return result;
}

///////////////////////////////////////////

shader_t shader_create_file(const char *filename) {
	shader_t result = shader_find(filename);
	if (result != nullptr)
		return result;

	// If it doesn't end with .sks, auto-add the .sks extension. We can't 
	// load .hlsl files directly at runtime, but it can be nice to just refer
	// to them by that name.
	const char *final_file = filename;
	char       *with_ext   = nullptr;
	if (!string_endswith(filename, ".sks", false)) {
		with_ext   = string_append(nullptr, 2, filename, ".sks");
		final_file = with_ext;
	}

	// Load from file
	void  *data;
	size_t size;
	char*  asset_filename = assets_file(final_file);
	bool   loaded         = platform_read_file(asset_filename, &data, &size);
	if (!loaded) log_warnf("Shader file failed to load: %s", filename);
	sk_free(asset_filename);
	sk_free(with_ext);

	return loaded 
		? shader_create_mem(data, size)
		: nullptr;
}

///////////////////////////////////////////

void shader_addref(shader_t shader) {
	assets_addref(&shader->header);
}

///////////////////////////////////////////

void shader_release(shader_t shader) {
	if (shader == nullptr)
		return;
	assets_releaseref(&shader->header);
}

///////////////////////////////////////////

void shader_destroy(shader_t shader) {
	skg_shader_destroy(&shader->shader);
	*shader = {};
}

} // namespace sk