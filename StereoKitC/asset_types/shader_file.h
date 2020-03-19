#pragma once

#include "../stereokit.h"
#include "shader.h"

namespace sk {

struct shader_blob_t {
	void  *data;
	size_t size;
};

void shader_file_parse    (const char *hlsl, char **out_name, shaderargs_desc_t &out_desc, shader_tex_slots_t &out_slots);
bool shader_file_parse_mem(void *data, size_t data_size, char **out_name, shaderargs_desc_t &out_desc, shader_tex_slots_t &out_slots, shader_blob_t &out_vs, shader_blob_t &out_ps);
bool shader_file_write_mem(const char *name, const shaderargs_desc_t &desc, const shader_tex_slots_t &slots, const shader_blob_t &vs, const shader_blob_t &ps, void *&out_data, size_t &out_size);
bool shader_file_is_mem   (void *data, size_t data_size);

} // namespace sk