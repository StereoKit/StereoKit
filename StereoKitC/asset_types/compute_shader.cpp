#include "../stereokit.h"
#include "../sk_memory.h"
#include "../platforms/platform.h"
#include "../libraries/stref.h"
#include <sk_gpu.h>
#include "compute_shader.h"
#include "assets.h"

#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

compute_shader_t compute_shader_find(const char *id) {
	compute_shader_t result = (compute_shader_t)assets_find(id, asset_type_compute_shader);
	if (result != nullptr) {
		compute_shader_addref(result);
		return result;
	}
	return nullptr;
}

///////////////////////////////////////////

compute_shader_t compute_shader_from(shader_t shader) {
	compute_shader_t result = (compute_shader_t)assets_allocate(asset_type_compute_shader);
	shader_addref(shader);
	result->shader = shader;

	return result;
}

///////////////////////////////////////////

void compute_shader_set_id(compute_shader_t shader, const char *id) {
	assets_set_id(&shader->header, id);
}

///////////////////////////////////////////

const char* compute_shader_get_id(const compute_shader_t shader) {
	return shader->header.id_text;
}

///////////////////////////////////////////

void compute_shader_addref(compute_shader_t shader) {
	assets_addref(&shader->header);
}

///////////////////////////////////////////

void compute_shader_release(compute_shader_t shader){
	if (shader == nullptr)
		return;
	assets_releaseref(&shader->header);
}

///////////////////////////////////////////

void compute_shader_destroy(compute_shader_t shader) {
	*shader = {};
}

///////////////////////////////////////////

bool32_t compute_shader_set_texture(compute_shader_t shader, const char *name, tex_t value) {
}

///////////////////////////////////////////

bool32_t compute_shader_set_buffer(compute_shader_t shader, const char *name, material_buffer_t value) {
}

///////////////////////////////////////////

void compute_shader_execute(compute_shader_t shader, uint32_t thread_count_x, uint32_t thread_count_y, uint32_t thread_count_z) {
	skg_shader_compute_bind(&shader->shader);
	skg_compute(thread_count_x, thread_count_y, thread_count_z);
}

}