#include "stereokit.h"
#include "shader.h"

#include "d3d.h"
#include "assets.h"
#include "stref.h"

#include <d3dcompiler.h>
#include <stdio.h>

#include <vector>
using namespace std;

const size_t shaderarg_size[] = { 
	sizeof(float),    // shaderarg_type_float
	sizeof(float)*4,  // shaderarg_type_color
	sizeof(float)*4,  // shaderarg_type_vector
	sizeof(float)*16, // shaderarg_type_matrix
};
const size_t shader_register_size = sizeof(float) * 4;

ID3DBlob *compile_shader(const char *hlsl, const char *entrypoint, const char *target) {
	DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob *compiled, *errors;
	if (FAILED(D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr, nullptr, entrypoint, target, flags, 0, &compiled, &errors)))
		MessageBox(0, (char*)errors->GetBufferPointer(), "Error", 0);
	if (errors) errors->Release();

	return compiled;
}

void shader_parse_file(shader_t shader, const char *hlsl) {
	stref_t file = stref_make(hlsl);
	stref_t line = {};

	vector<shaderargs_desc_item_t > buffer_items;
	vector<shader_tex_slots_item_t> tex_items;

	int buffer_size = 0;
	while (stref_nextline(file, line)) {
		stref_t curr = line;
		stref_t word = {};
		stref_trim(curr);
		
		if (!stref_nextword(curr, word) || !stref_equals(word, "//"))
			continue;
		if (!stref_nextword(curr, word))
			continue;
		if (stref_equals(word, "[param]")) {
			if (!stref_nextword(curr, word))
				continue;

			shaderargs_desc_item_t item;
			if      (stref_equals(word, "float" )) item.type = shaderarg_type_float;
			else if (stref_equals(word, "color" )) item.type = shaderarg_type_color;
			else if (stref_equals(word, "vector")) item.type = shaderarg_type_vector;
			else if (stref_equals(word, "matrix")) item.type = shaderarg_type_matrix;
			item.size   = shaderarg_size[item.type];
		
			// make sure parameters are padded so they don't overflow into the next register
			if (buffer_size % shader_register_size != 0 && // if it's evenly aligned, it isn't a problem, but if it's not, we need to check!
				buffer_size/shader_register_size != buffer_size+item.size/shader_register_size && // if we've crossed into the next register
				(buffer_size+item.size) % shader_register_size != 0) // and if crossing over means crossing allll the way over, instead of just up-to
			{
				buffer_size += shader_register_size - (buffer_size % shader_register_size);
			}
			item.offset = buffer_size;
			buffer_size += item.size;

			item.id = 0;
			if (stref_nextword(curr, word)) {
				item.id = stref_hash(word);
			}

			buffer_items.emplace_back(item);
		} else if (stref_equals(word, "[texture]")) {

			shader_tex_slots_item_t item;
			item.slot = tex_items.size();
			if (stref_nextword(curr, word)) {
				item.id = stref_hash(word);
			}

			tex_items.emplace_back(item);
		}
	}

	shader->args_desc = {};
	shader->args_desc.buffer_size = buffer_size;
	size_t data_size = sizeof(shaderargs_desc_item_t) * buffer_items.size();
	if (data_size > 0) {
		shader->args_desc.item_count = buffer_items.size();
		shader->args_desc.item       = (shaderargs_desc_item_t *)malloc(data_size);
		memcpy(shader->args_desc.item, &buffer_items[0], data_size);
	}

	shader->tex_slots = {};
	data_size = sizeof(shader_tex_slots_item_t) * tex_items.size();
	if (data_size > 0) {
		shader->tex_slots.tex_count = tex_items.size();
		shader->tex_slots.tex       = (shader_tex_slots_item_t*)malloc(data_size);
		memcpy(shader->tex_slots.tex, &tex_items[0], data_size);
	}
}

shader_t shader_create_file(const char *filename) {
	shader_t result = (shader_t)assets_find(filename);
	if (result != nullptr) {
		assets_addref(result->header);
		return result;
	}

	// Open file
	FILE *fp;
	if (fopen_s(&fp, filename, "rb") != 0 || fp == nullptr)
		return nullptr;

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
	result = shader_create(filename, (const char *)data);
	free(data);

	return result;
}

shader_t shader_create(const char *name, const char *hlsl) {
	shader_t  result            = (shader_t)assets_allocate(asset_type_shader, name);
	shader_parse_file(result, hlsl);
	ID3DBlob *vert_shader_blob  = compile_shader(hlsl, "vs", "vs_5_0");
	ID3DBlob *pixel_shader_blob = compile_shader(hlsl, "ps", "ps_5_0");
	d3d_device->CreateVertexShader(vert_shader_blob ->GetBufferPointer(), vert_shader_blob ->GetBufferSize(), nullptr, &result->vshader);
	d3d_device->CreatePixelShader (pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &result->pshader);

	shaderargs_create(result->args, result->args_desc.buffer_size, 1);

	// Describe how our mesh is laid out in memory
	D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
		{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR" ,      0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},};
	d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), vert_shader_blob->GetBufferPointer(), vert_shader_blob->GetBufferSize(), &result->vert_layout);

	return result;
}

void shader_set_active(shader_t shader) {
	d3d_context->VSSetShader(shader->vshader, nullptr, 0);
	d3d_context->PSSetShader(shader->pshader, nullptr, 0);
	d3d_context->IASetInputLayout(shader->vert_layout);
}

void shader_release(shader_t shader) {
	assets_releaseref(shader->header);
}
void shader_destroy(shader_t shader) {
	shaderargs_destroy(shader->args);
	if (shader->pshader     != nullptr) shader->pshader    ->Release();
	if (shader->vshader     != nullptr) shader->vshader    ->Release();
	if (shader->vert_layout != nullptr) shader->vert_layout->Release();
	if (shader->args_desc.item != nullptr) free(shader->args_desc.item);
	if (shader->tex_slots.tex  != nullptr) free(shader->tex_slots.tex );
	*shader = {};
}

void shaderargs_create(shaderargs_t &args, size_t buffer_size, int buffer_slot) {
	CD3D11_BUFFER_DESC const_buff_desc(buffer_size, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
	d3d_device->CreateBuffer(&const_buff_desc, nullptr, &args.const_buffer);
	args.buffer_size = buffer_size;
	args.buffer_slot = buffer_slot;
}
void shaderargs_set_data(shaderargs_t &args, void *data) {
	if (args.const_buffer == nullptr)
		return;
	D3D11_MAPPED_SUBRESOURCE res;
	d3d_context->Map(args.const_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, data, args.buffer_size);
	d3d_context->Unmap( args.const_buffer, 0 );
}
void shaderargs_set_active(shaderargs_t &args) {
	d3d_context->VSSetConstantBuffers(args.buffer_slot, 1, &args.const_buffer);
}
void shaderargs_destroy(shaderargs_t &args) {
	if (args.const_buffer != nullptr) args.const_buffer->Release();
	args = {};
}