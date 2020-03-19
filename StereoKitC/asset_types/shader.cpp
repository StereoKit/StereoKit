#include "../stereokit.h"
#include "../_stereokit.h"
#include "../libraries/stref.h"
#include "../systems/d3d.h"
#include "../systems/platform/platform_utils.h"
#include "shader.h"
#include "shader_file.h"
#include "assets.h"

#include <stdio.h>
#include <assert.h>
#include <direct.h>   // for _mkdir
#include <sys/stat.h> // for stat

namespace sk {

///////////////////////////////////////////

ID3DBlob *compile_shader(const char *filename, const char *hlsl, const char *entrypoint, const char *target);
bool32_t _shader_set_code(shader_t shader, char *name, const shaderargs_desc_t &desc, const shader_tex_slots_t &tex_slots, const shader_blob_t &vs, const shader_blob_t &ps);

const char *shader_cache_name(uint64_t hash);
const char *shader_cache_folder();
void        shader_cache (uint64_t hlsl_hash, void *data, size_t size);
bool32_t    shader_cached(uint64_t hlsl_hash, void *&out_data, size_t &out_size);

///////////////////////////////////////////

#ifndef SK_NO_RUNTIME_SHADER_COMPILE
#include <d3dcompiler.h>
#pragma comment(lib,"D3dcompiler.lib")
#endif

ID3DBlob *compile_shader(const char *filename, const char *hlsl, const char *entrypoint, const char *target) {
#ifndef SK_NO_RUNTIME_SHADER_COMPILE
	DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob *compiled, *errors;
	if (FAILED(D3DCompile(hlsl, strlen(hlsl), filename, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entrypoint, target, flags, 0, &compiled, &errors)))
		log_err((char*)errors->GetBufferPointer());
	if (errors) errors->Release();

	return compiled;
#else
	log_err("Shader compile not supported in this build!");
	return nullptr;
#endif
}

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

	if (shader->vshader)
		DX11ResName(shader->vshader, "shader_vtx", id);
	if (shader->pshader)
		DX11ResName(shader->pshader, "shader_pix", id);
	if (shader->args.const_buffer)
		DX11ResName(shader->args.const_buffer, "shader_args", id);
}

///////////////////////////////////////////

const char *shader_get_name(shader_t shader) {
	return shader->name;
}

///////////////////////////////////////////

bool32_t shader_compile(const char *hlsl, void *&out_data, size_t &out_size) {
	char *name;
	shaderargs_desc_t desc;
	shader_tex_slots_t tex_slots;
	shader_file_parse(hlsl, &name, desc, tex_slots);

	// Compile the shader into machine code!
	shader_blob_t ps, vs;
	ID3DBlob *vs_blob = compile_shader(nullptr, hlsl, "vs", "vs_5_0");
	if (vs_blob == nullptr) return false;
	ID3DBlob *ps_blob = compile_shader(nullptr, hlsl, "ps", "ps_5_0");
	if (ps_blob == nullptr) { vs_blob->Release(); return false; };

	// Create a binary shader file, and put it in our out arguments
	bool result = shader_file_write_mem(name, desc, tex_slots,
		{ vs_blob->GetBufferPointer(), vs_blob->GetBufferSize() },
		{ ps_blob->GetBufferPointer(), ps_blob->GetBufferSize() },
		out_data, out_size);

	vs_blob->Release();
	ps_blob->Release();

	return result;
}

///////////////////////////////////////////

shader_t shader_create_mem(void *data, size_t data_size) {
	char              *name;
	shaderargs_desc_t  desc;
	shader_tex_slots_t tex_slots;
	shader_blob_t      vs, ps;
	if (!shader_file_parse_mem(data, data_size, &name, desc, tex_slots, vs, ps))
		return nullptr;

	shader_t result = (shader_t)assets_allocate(asset_type_shader);
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
		if      (shader_cached(hash, compiled_data, compiled_size)) {
		}
		// If not, try compiling it and caching it
		else if (shader_compile((char *)data, compiled_data, compiled_size)) {
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

	result = shader_create_mem(data, size);
	if (result != nullptr)
		shader_set_id(result, filename);

	return result;
}

///////////////////////////////////////////

bool32_t _shader_set_code(shader_t shader, char *name, const shaderargs_desc_t &desc, const shader_tex_slots_t &tex_slots, const shader_blob_t &vs, const shader_blob_t &ps) {
	shader_destroy_parsedata(shader);
	shader->name      = name;
	shader->args_desc = desc;
	shader->tex_slots = tex_slots;
	shaderargs_create(shader->args, shader->args_desc.buffer_size, 2);

	if (shader->vshader != nullptr) shader->vshader->Release();
	if (shader->pshader != nullptr) shader->pshader->Release();
	if (FAILED(d3d_device->CreateVertexShader(vs.data, vs.size, nullptr, &shader->vshader)))
		log_warnf("Issue creating vertex shader for %s", shader->name);
	if (FAILED(d3d_device->CreatePixelShader (ps.data, ps.size, nullptr, &shader->pshader)))
		log_warnf("Issue creating pixel shader for %s", shader->name);
	DX11ResType(shader->vshader, "vertex_shader");
	DX11ResType(shader->pshader, "pixel_shader");

	// Describe how our mesh is laid out in memory
	if (shader->vert_layout != nullptr) shader->vert_layout->Release();
	D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
		{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR" ,      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SV_RenderTargetArrayIndex" ,  0, DXGI_FORMAT_R32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0} };
	if (FAILED(d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), vs.data, vs.size, &shader->vert_layout)))
		log_warnf("Issue creating vertex layout for %s", shader->name);

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