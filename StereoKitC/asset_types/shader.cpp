#include "../stereokit.h"
#include "../_stereokit.h"
#include "../libraries/stref.h"
#include "../systems/platform/platform_utils.h"
#include "../shaders_builtin/shader_builtin_include.h"
#include "shader.h"
#include "shader_file.h"
#include "assets.h"

#include <stdio.h>
#include <assert.h>
#include <direct.h>   // for _mkdir
#include <sys/stat.h> // for stat

namespace sk {

///////////////////////////////////////////

bool32_t _shader_set_code(shader_t shader, char *name, const shaderargs_desc_t &desc, const shader_tex_slots_t &tex_slots, const shader_blob_t &vs, const shader_blob_t &ps);

const char *shader_cache_name(uint64_t hash);
const char *shader_cache_folder();
void        shader_cache (uint64_t hlsl_hash, void *data, size_t size);
bool32_t    shader_cached(uint64_t hlsl_hash, void *&out_data, size_t &out_size);

///////////////////////////////////////////

char cache_folder[512];
const char *shader_cache_name(uint64_t hash) {
	char temp[512];
	GetTempPathA(512, temp);
	sprintf_s(cache_folder, "%s\\cache\\%I64u.sks", temp, hash);
	return cache_folder;
}

///////////////////////////////////////////

const char *shader_cache_folder() {
	char temp[512];
	GetTempPathA(512, temp);
	sprintf_s(cache_folder, "%s\\cache", temp);
	return cache_folder;
}

///////////////////////////////////////////

bool32_t shader_cached(uint64_t hlsl_hash, void *&out_data, size_t &out_size) {
	FILE *fp = nullptr;
	if (fopen_s(&fp, shader_cache_name(hlsl_hash), "rb") != 0 || fp == nullptr) {
		return false;
	}

	fseek(fp, 0, SEEK_END);
	out_size = ftell(fp);
	out_data = malloc(out_size);
	fseek(fp, 0, SEEK_SET);
	fread(out_data, out_size, 1, fp);
	fclose(fp);

	return true;
}

///////////////////////////////////////////

void shader_cache(uint64_t hlsl_hash, void *data, size_t size) {
	// Ensure cache folder is present
	struct stat st = { 0 };
	const char *folder_name = shader_cache_folder();
	if (stat(folder_name, &st) == -1) {
		if (_mkdir(folder_name) == -1) {
			log_warnf("Couldn't create the cache folder!");
		}
	}

	// Write the blob to file
	FILE *fp = nullptr;
	if (fopen_s(&fp, shader_cache_name(hlsl_hash), "wb") == 0 && fp != nullptr) {
		fwrite(data, size, 1, fp);
		fclose(fp);
	} else {
		log_warn("Couldn't write shader cache file!");
	}
}

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
	return shader->name;
}

///////////////////////////////////////////

bool32_t shader_compile(const char *hlsl, const char *opt_filename, void *&out_data, size_t &out_size) {
	char *name;
	shaderargs_desc_t desc;
	shader_tex_slots_t tex_slots;
	shader_file_parse(hlsl, &name, desc, tex_slots);

	log_err("sk_gpu doesn't implement shader compile yet!");

	return false;
}

///////////////////////////////////////////

shader_t shader_create_mem(void *data, size_t data_size) {
	char              *name;
	shaderargs_desc_t  desc;
	shader_tex_slots_t tex_slots;
	shader_blob_t      vs, ps;
	if (!shader_file_parse_mem(data, data_size, &name, desc, tex_slots, vs, ps))
		return nullptr;

	shader_t result = shader_find(name);
	if (result != nullptr)
		return result;

	result = (shader_t)assets_allocate(asset_type_shader);
	if (!_shader_set_code(result, name, desc, tex_slots, vs, ps)) {
		shader_destroy(result);
		return nullptr;
	}
	shader_set_id(result, result->name);

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

	// If it's not a compiled binary shader file...
	if (!shader_file_is_mem(data, size)) {
		void    *compiled_data;
		size_t   compiled_size;
		uint64_t hash = string_hash((char*)data);

		// Check if it's cached first
		if      (shader_cached(hash, compiled_data, compiled_size)) { }
		// If not, try compiling it and caching it
		else if (shader_compile((char *)data, filename, compiled_data, compiled_size)) {
			shader_cache(hash, compiled_data, compiled_size);
		} 
		// If still no luck, fail out!
		else {
			free(data);
			return nullptr;
		}

		// Got our shader, lets move on!
		free(data);
		data = compiled_data;
		size = compiled_size;
	}

	return shader_create_mem(data, size);
}

///////////////////////////////////////////

shader_t shader_create_hlsl(const char *hlsl) {
	void    *data;
	size_t   size;
	uint64_t hash = string_hash(hlsl);

	// Check if it's cached first
	if      (shader_cached (hash, data, size)) { }
	// If not, try compiling it and caching it
	else if (shader_compile(hlsl, nullptr, data, size)) { shader_cache(hash, data, size); } 
	// If still no luck, fail out!
	else { return nullptr; }

	return shader_create_mem(data, size);
}

///////////////////////////////////////////

bool32_t _shader_set_code(shader_t shader, char *name, const shaderargs_desc_t &desc, const shader_tex_slots_t &tex_slots, const shader_blob_t &vs, const shader_blob_t &ps) {
	shader_destroy_parsedata(shader);
	shader->name      = name;
	shader->args_desc = desc;
	shader->tex_slots = tex_slots;
	shaderargs_create(shader->args, shader->args_desc.buffer_size, 2);

	shader->vshader = skr_shader_stage_create((uint8_t*)vs.data, vs.size, skr_shader_vertex);
	shader->pshader = skr_shader_stage_create((uint8_t*)ps.data, ps.size, skr_shader_pixel );
	shader->shader  = skr_shader_create      (&shader->vshader, &shader->pshader);

	free(vs.data);
	free(ps.data);

	return true;
}

///////////////////////////////////////////

bool32_t shader_set_code(shader_t shader, void *data, size_t data_size) {
	char              *name;
	shaderargs_desc_t  desc;
	shader_tex_slots_t tex_slots;
	shader_blob_t      vs, ps;
	if (!shader_file_parse_mem(data, data_size, &name, desc, tex_slots, vs, ps))
		return false;

	return _shader_set_code(shader, name, desc, tex_slots, vs, ps);
}

///////////////////////////////////////////

bool32_t shader_set_codefile(shader_t shader, const char *filename) {
	void  *data;
	size_t size;
	if (!platform_read_file(filename, data, size))
		return false;

	bool32_t result = shader_set_code(shader, data, size);
	free(data);
	return result;
}

///////////////////////////////////////////

void shader_release(shader_t shader) {
	if (shader == nullptr)
		return;
	assets_releaseref(shader->header);
}

///////////////////////////////////////////

void shader_destroy_parsedata(shader_t shader) {
	for (int32_t i = 0; i < shader->args_desc.item_count; i++) {
		free(shader->args_desc.item[i].name);
		free(shader->args_desc.item[i].tags);
		free(shader->args_desc.item[i].default_value);
	}

	for (size_t i = 0; i < shader->tex_slots.tex_count; i++) {
		free(shader->tex_slots.tex[i].name);
		free(shader->tex_slots.tex[i].default_name);
		tex_release(shader->tex_slots.tex[i].default_tex);
	}
	
	shaderargs_destroy(shader->args);
	free(shader->args_desc.item);
	free(shader->tex_slots.tex);
	free(shader->name);
}

///////////////////////////////////////////

void shader_destroy(shader_t shader) {
	shader_destroy_parsedata(shader);

	skr_shader_destroy      (&shader->shader );
	skr_shader_stage_destroy(&shader->pshader);
	skr_shader_stage_destroy(&shader->vshader);

	*shader = {};
}

///////////////////////////////////////////

void shaderargs_create(shaderargs_t &args, size_t buffer_size, int buffer_slot) {
	args.const_buffer = skr_buffer_create(nullptr, buffer_size, skr_buffer_type_constant, skr_use_dynamic);
	args.buffer_size  = (int)buffer_size;
	args.buffer_slot  = buffer_slot;
}

///////////////////////////////////////////

void shaderargs_set_data(shaderargs_t &args, void *data, size_t buffer_size) {
	if (!skr_buffer_is_valid( &args.const_buffer ))
		return;
	assert(buffer_size <= args.buffer_size);
	if (buffer_size == 0)
		buffer_size = args.buffer_size;

	skr_buffer_update(&args.const_buffer, data, buffer_size);
}

///////////////////////////////////////////

void shaderargs_set_active(shaderargs_t &args) {
	skr_buffer_set(&args.const_buffer, 1, args.buffer_size, 0);
}

///////////////////////////////////////////

void shaderargs_destroy(shaderargs_t &args) {
	skr_buffer_destroy(&args.const_buffer);
	args = {};
}

} // namespace sk