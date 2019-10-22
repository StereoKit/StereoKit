#include "../stereokit.h"
#include "../_stereokit.h"
#include "../libraries/stref.h"
#include "../systems/d3d.h"
#include "shader.h"
#include "assets.h"

#include <stdio.h>
#include <assert.h>
#include <direct.h>

#include <vector>
using namespace std;

namespace sk {

///////////////////////////////////////////

struct shader_blob_t {
	void *data;
	size_t size;
};

const size_t shaderarg_size[] = { 
	sizeof(float),    // shaderarg_type_float
	sizeof(float)*4,  // shaderarg_type_color
	sizeof(float)*4,  // shaderarg_type_vector
	sizeof(float)*16, // shaderarg_type_matrix
};
const size_t shader_register_size = sizeof(float) * 4;

///////////////////////////////////////////

#ifndef SK_NO_RUNTIME_SHADER_COMPILE
#include <d3dcompiler.h>
#pragma comment(lib,"D3dcompiler.lib")

ID3DBlob *compile_shader(const char *filename, const char *hlsl, const char *entrypoint, const char *target) {
	DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob *compiled, *errors;
	if (FAILED(D3DCompile(hlsl, strlen(hlsl), filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint, target, flags, 0, &compiled, &errors)))
		log_err((char*)errors->GetBufferPointer());
	if (errors) errors->Release();

	return compiled;
}

#endif

///////////////////////////////////////////

shader_blob_t load_shader(const char* filename, const char* hlsl, const char* entrypoint) {
	uint64_t hash = string_hash(hlsl);
	char folder_name[128];
	sprintf_s(folder_name, "%s/cache", sk_settings.shader_cache);
	char cache_name[128];
	sprintf_s(cache_name, "%s/cache/%lld.%s.blob", sk_settings.shader_cache, hash, entrypoint);
	char target[16];
	sprintf_s(target, "%s_5_0", entrypoint);

	shader_blob_t result = {};
	FILE         *fp     = nullptr;
	if (fopen_s(&fp, cache_name, "rb") == 0 && fp != nullptr) {
		fseek(fp, 0, SEEK_END);
		long length = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		result.data = malloc(length);
		result.size = length;
		fread(result.data, result.size, 1, fp);
		fclose(fp);
	} else {
#ifdef SK_NO_RUNTIME_SHADER_COMPILE
		log_errf("Runtime shader compile is disabled: couldn't find precompiled shader (%s) for %s!", cache_name, filename);
#else
		ID3DBlob *blob = compile_shader(filename, hlsl, entrypoint, target);
		if (blob == nullptr)
			return result;
		result.size = blob->GetBufferSize();
		result.data = malloc(result.size);
		memcpy(result.data, blob->GetBufferPointer(), result.size);
		blob->Release();

		// Ensure cache folder is present
		struct stat st = { 0 };
		if (stat(folder_name, &st) == -1) {
			int err = _mkdir(folder_name);
		}

		// Write the blob to file for future use.
		if (fopen_s(&fp, cache_name, "wb") == 0 && fp != nullptr) {
			fwrite(result.data, result.size, 1, fp);
			fclose(fp);
		}
#endif
	}
	return result;
}

///////////////////////////////////////////

void shader_parse_file(shader_t shader, const char *hlsl) {
	stref_t file = stref_make(hlsl);
	stref_t line = {};

	vector<shaderargs_desc_item_t > buffer_items;
	vector<shader_tex_slots_item_t> tex_items;

	size_t buffer_size = 0;
	while (stref_nextline(file, line)) {
		stref_t curr = line;
		stref_t word = {};
		stref_trim(curr);
		
		// Make sure it's not an empty line, and it begins with a comment tag
		if (!stref_nextword(curr, word, ' ', '[', ']') || !stref_equals(word, "//"))
			continue;
		if (!stref_nextword(curr, word, ' ', '[', ']'))
			continue;
		// Ensure this is a tag we'll want to parse (starts with '[')
		if (*word.start != '[')
			continue;
		stref_t stripped_word = stref_stripcapture(word, '[', ']');
		if (stref_equals(stripped_word, "param")) {
			if (!stref_nextword(curr, word))
				continue;

			shaderargs_desc_item_t item = {};
			if      (stref_equals(word, "float" )) item.type = material_param_float;
			else if (stref_equals(word, "color" )) item.type = material_param_vector;
			else if (stref_equals(word, "vector")) item.type = material_param_vector;
			else if (stref_equals(word, "matrix")) item.type = material_param_matrix;
			else {
				char name[64];
				stref_copy_to(word, name, 64);
				log_warnf("Unrecognized shader param type: %s", name);
				free(name);
			}
			item.size   = shaderarg_size[item.type];

			// make sure parameters are padded so they don't overflow into the next register
			if (buffer_size % shader_register_size != 0 && // if it's evenly aligned, it isn't a problem, but if it's not, we need to check!
				buffer_size/shader_register_size != (buffer_size+item.size)/shader_register_size && // if we've crossed into the next register
				(buffer_size+item.size) % shader_register_size != 0) // and if crossing over means crossing allll the way over, instead of just up-to
			{
				buffer_size += shader_register_size - (buffer_size % shader_register_size);
			}
			item.offset = buffer_size;
			buffer_size += item.size;

			item.id = 0;
			if (stref_nextword(curr, word)) {
				item.name = stref_copy(word);
				item.id   = stref_hash(word);
			}

			while (stref_nextword(curr, word, ' ', '{','}')) {
				if (stref_equals(word, "tags")) {
					if (stref_nextword(curr, word, ' ', '{','}'))
						item.tags = stref_copy(stref_stripcapture(word, '{','}'));
				} else {
					stref_t value = stref_stripcapture(word, '{', '}');
					if (item.type == material_param_float) {
						item.default_value = malloc(sizeof(float));
						*(float *)item.default_value = stref_to_f(value);

					} else if (item.type == material_param_vector) {
						vec4    result    = {};
						stref_t component = {};
						if (stref_nextword(value, component, ',')) result.x = stref_to_f(component);
						if (stref_nextword(value, component, ',')) result.y = stref_to_f(component);
						if (stref_nextword(value, component, ',')) result.z = stref_to_f(component);
						if (stref_nextword(value, component, ',')) result.w = stref_to_f(component);

						item.default_value = malloc(sizeof(vec4));
						*(vec4 *)item.default_value = result;
					}
				}
			}

			buffer_items.emplace_back(item);
		} if (stref_equals(stripped_word, "texture")) {

			shader_tex_slots_item_t item;
			item.slot = (int)tex_items.size();
			if (stref_nextword(curr, word)) {
				item.id = stref_hash(word);
			}

			// Find a default texture for this slot, in case it isn't used!
			if (stref_nextword(curr, word)) {
				if      (stref_equals(word, "white")) item.default_tex = tex2d_find("default/tex2d");
				else if (stref_equals(word, "black")) item.default_tex = tex2d_find("default/tex2d_black");
				else if (stref_equals(word, "gray" )) item.default_tex = tex2d_find("default/tex2d_gray");
				else if (stref_equals(word, "flat" )) item.default_tex = tex2d_find("default/tex2d_flat");
				else if (stref_equals(word, "rough")) item.default_tex = tex2d_find("default/tex2d_rough");
				else                                  item.default_tex = tex2d_find("default/tex2d");
			} else {
				item.default_tex = tex2d_find("default/tex2d");
			}

			tex_items.emplace_back(item);
		} if (stref_equals(stripped_word, "name")) {
			if (!stref_nextword(curr, word))
				continue;
			shader->name = stref_copy(word);
		}
	}

	// And ensure our final buffer is a multiple of register size!
	if (buffer_size % shader_register_size != 0) {
		buffer_size += shader_register_size - (buffer_size % shader_register_size);
	}

	shader->args_desc = {};
	shader->args_desc.buffer_size = (int)buffer_size;
	size_t data_size = sizeof(shaderargs_desc_item_t) * buffer_items.size();
	if (data_size > 0) {
		shader->args_desc.item_count = (int)buffer_items.size();
		shader->args_desc.item       = (shaderargs_desc_item_t *)malloc(data_size);
		memcpy(shader->args_desc.item, &buffer_items[0], data_size);
	}

	shader->tex_slots = {};
	data_size = sizeof(shader_tex_slots_item_t) * tex_items.size();
	if (data_size > 0) {
		shader->tex_slots.tex_count = (int)tex_items.size();
		shader->tex_slots.tex       = (shader_tex_slots_item_t*)malloc(data_size);
		memcpy(shader->tex_slots.tex, &tex_items[0], data_size);
	}
}

///////////////////////////////////////////

shader_t shader_find(const char *id) {
	shader_t result = (shader_t)assets_find(id);
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

shader_t shader_create_file(const char *filename) {
	shader_t result = shader_find(filename);
	if (result != nullptr)
		return result;
	result = (shader_t)assets_allocate(asset_type_shader);
	shader_set_id      (result, filename);
	shader_set_codefile(result, filename);

	return result;
}

///////////////////////////////////////////

shader_t shader_create(const char *hlsl) {
	shader_t result = (shader_t)assets_allocate(asset_type_shader);

	shader_set_code(result, hlsl, nullptr);
	
	return result;
}

///////////////////////////////////////////

bool32_t shader_set_code(shader_t shader, const char *hlsl, const char *filename) {
	// Compile the shader code
	shader_blob_t vert_shader_blob  = load_shader(filename, hlsl, "vs");
	shader_blob_t pixel_shader_blob = load_shader(filename, hlsl, "ps");
	if (vert_shader_blob.data == nullptr || pixel_shader_blob.data == nullptr)
		return false;

	// If the shader compiled fine, set up and send it to the GPU
	if (shader->vshader != nullptr) shader->vshader->Release();
	if (shader->pshader != nullptr) shader->pshader->Release();
	if (FAILED(d3d_device->CreateVertexShader(vert_shader_blob.data, vert_shader_blob.size, nullptr, &shader->vshader)))
		log_warnf("Issue creating vertex shader for %s", filename);
	if (FAILED(d3d_device->CreatePixelShader (pixel_shader_blob.data, pixel_shader_blob.size, nullptr, &shader->pshader)))
		log_warnf("Issue creating pixel shader for %s", filename);
	DX11ResType(shader->vshader, "vertex_shader");
	DX11ResType(shader->pshader, "pixel_shader");

	// Check the parameters on this shader
	shader_destroy_parsedata(shader);
	shader_parse_file(shader, hlsl);
	shaderargs_create(shader->args, shader->args_desc.buffer_size, 2);

	// Describe how our mesh is laid out in memory
	if (shader->vert_layout != nullptr) shader->vert_layout->Release();
	D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
		{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR" ,      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},};
	if (FAILED(d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), vert_shader_blob.data, vert_shader_blob.size, &shader->vert_layout)))
		log_warnf("Issue creating vertex layout for %s", filename);

	free(vert_shader_blob .data);
	free(pixel_shader_blob.data);

	return true;
}

///////////////////////////////////////////

bool32_t shader_set_codefile(shader_t shader, const char *filename) {
	// Open file
	FILE *fp;
	if (fopen_s(&fp, filename, "rb") != 0 || fp == nullptr)
		return false;

	// Get length of file
	fseek(fp, 0L, SEEK_END);
	size_t length = ftell(fp);
	rewind(fp);

	// Read the data
	unsigned char *data = (unsigned char *)malloc(sizeof(unsigned char) *length+1);
	if (data == nullptr) { fclose(fp); return false; }
	fread(data, 1, length, fp);
	data[length] = '\0';
	fclose(fp);

	// Compile the shader
	shader_set_code(shader, (const char *)data, filename);
	free(data);

	return true;
}

///////////////////////////////////////////

void shader_release(shader_t shader) {
	if (shader == nullptr)
		return;
	assets_releaseref(shader->header);
}

///////////////////////////////////////////

void shader_destroy_parsedata(shader_t shader) {
	for (size_t i = 0; i < shader->args_desc.item_count; i++) {
		free(shader->args_desc.item[i].name);
		free(shader->args_desc.item[i].tags);
		free(shader->args_desc.item[i].default_value);
	}

	shaderargs_destroy(shader->args);
	free(shader->args_desc.item);
	free(shader->name);
}

///////////////////////////////////////////

void shader_destroy(shader_t shader) {
	shader_destroy_parsedata(shader);

	for (size_t i = 0; i < shader->tex_slots.tex_count; i++) {
		tex2d_release(shader->tex_slots.tex[i].default_tex);
	}
	free(shader->tex_slots.tex );
	
	if (shader->pshader     != nullptr) shader->pshader    ->Release();
	if (shader->vshader     != nullptr) shader->vshader    ->Release();
	if (shader->vert_layout != nullptr) shader->vert_layout->Release();
	
	*shader = {};
}

///////////////////////////////////////////

void shaderargs_create(shaderargs_t &args, size_t buffer_size, int buffer_slot) {
	CD3D11_BUFFER_DESC const_buff_desc((UINT)buffer_size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	d3d_device->CreateBuffer(&const_buff_desc, nullptr, &args.const_buffer);
	args.buffer_size = (int)buffer_size;
	args.buffer_slot = buffer_slot;

	DX11ResType(args.const_buffer, "shader_args");
}

///////////////////////////////////////////

void shaderargs_set_data(shaderargs_t &args, void *data, size_t buffer_size) {
	if (args.const_buffer == nullptr)
		return;
	assert(buffer_size <= args.buffer_size);
	if (buffer_size == 0)
		buffer_size = args.buffer_size;

	D3D11_MAPPED_SUBRESOURCE res;
	d3d_context->Map(args.const_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, data, buffer_size);
	d3d_context->Unmap( args.const_buffer, 0 );
}

///////////////////////////////////////////

void shaderargs_set_active(shaderargs_t &args, bool include_ps) {
	d3d_context->VSSetConstantBuffers(args.buffer_slot, 1, &args.const_buffer);
	if (include_ps)
		d3d_context->PSSetConstantBuffers(args.buffer_slot, 1, &args.const_buffer);
}

///////////////////////////////////////////

void shaderargs_destroy(shaderargs_t &args) {
	if (args.const_buffer != nullptr) args.const_buffer->Release();
	args = {};
}

} // namespace sk