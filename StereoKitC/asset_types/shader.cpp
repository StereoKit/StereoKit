#include "../stereokit.h"
#include "../_stereokit.h"
#include "../libraries/stref.h"
#include "../systems/platform/platform_utils.h"
#include "shader.h"
#include "assets.h"

#include <stdio.h>
#include <assert.h>
#include <direct.h>   // for _mkdir
#include <sys/stat.h> // for stat

namespace sk {

///////////////////////////////////////////

shader_t shader_find(const char *id) {
	shader_t result = (shader_t)assets_find(id, asset_type_shader);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

void shader_set_id(shader_t shader, const char *id) {
	assets_set_id(shader->header, id);
}

///////////////////////////////////////////

const char *shader_get_name(shader_t shader) {
	return shader->shader.meta->name;
}

///////////////////////////////////////////

bool32_t shader_compile(const char *hlsl, const char *opt_filename, void *&out_data, size_t &out_size) {
	out_data = nullptr;
	out_size = 0;
	log_err("sk_gpu doesn't implement shader compile yet!");

	return false;
}

///////////////////////////////////////////

shader_t shader_create_hlsl(const char *hlsl) {
	log_err("sk_gpu doesn't implement shader compile yet!");

	return nullptr;
}

///////////////////////////////////////////

shader_t shader_create_mem(void *data, size_t data_size) {
	char name[256];
	if (!skr_shader_file_verify(data, data_size, nullptr, name, sizeof(name)))
		return nullptr;

	shader_t result = shader_find(name);
	if (result != nullptr)
		return result;

	skr_shader_t shader = skr_shader_create_mem(data, data_size);
	if (!skr_shader_is_valid(&shader)) {
		skr_shader_destroy(&shader);
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

	// Load from file
	void  *data;
	size_t size;
	if (!platform_read_file(assets_file(filename), data, size))
		return nullptr;

	return shader_create_mem(data, size);
}

///////////////////////////////////////////

void shader_release(shader_t shader) {
	if (shader == nullptr)
		return;
	assets_releaseref(shader->header);
}

///////////////////////////////////////////

void shader_destroy(shader_t shader) {
	skr_shader_destroy(&shader->shader);
	*shader = {};
}

} // namespace sk