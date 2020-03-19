#include "shader_file.h"
#include "shader.h"

#include "../libraries/stref.h"
#include "../libraries/stb_ds.h"
#include "../systems/d3d.h"
#include "../systems/defaults.h"

namespace sk {

///////////////////////////////////////////

#define SHADER_MAX_NAME 64
#define SHADER_MAX_PARAM_NAME 32
#define SHADER_MAX_PARAM_TAG 128

enum shader_lang_ {
	shader_lang_hlsl_vs  = 0,
	shader_lang_hlsl_ps  = 1,
	shader_lang_spirv_vs = 2,
	shader_lang_spirv_ps = 3,
};

struct shader_file_params_t {
	material_param_ type;
	char  name[SHADER_MAX_PARAM_NAME];
	char  tags[SHADER_MAX_PARAM_TAG];
	float data[16];
};

struct shader_file_tex_t {
	char name [SHADER_MAX_PARAM_NAME];
	char value[SHADER_MAX_PARAM_NAME];
	char tags [SHADER_MAX_PARAM_TAG];
};

struct shader_file_blob_t {
	size_t       size;
	shader_lang_ language;
};

struct shader_file_t {
	char    signature[12];
	char    name     [SHADER_MAX_NAME];

	int32_t blob_count;
	int32_t param_count;
	int32_t tex_count;
};

const size_t shaderarg_sz[] = { 
	sizeof(float),    // shaderarg_type_float
	sizeof(float)*4,  // shaderarg_type_color
	sizeof(float)*4,  // shaderarg_type_vector
	sizeof(float)*16, // shaderarg_type_matrix
};
const size_t register_sz = sizeof(float) * 4;

///////////////////////////////////////////

tex_t       shader_stref_to_tex (const stref_t &tex_name);
const char *shader_tex_to_string(tex_t tex);
void        shader_init_buffer  (shaderargs_desc_t &desc);

///////////////////////////////////////////

tex_t shader_stref_to_tex(const stref_t &tex_name) {
	if      (stref_equals(tex_name, "white")) return tex_find("default/tex");
	else if (stref_equals(tex_name, "black")) return tex_find("default/tex_black");
	else if (stref_equals(tex_name, "gray" )) return tex_find("default/tex_gray");
	else if (stref_equals(tex_name, "flat" )) return tex_find("default/tex_flat");
	else if (stref_equals(tex_name, "rough")) return tex_find("default/tex_rough");
	else                                      return tex_find("default/tex");
}

///////////////////////////////////////////

const char *shader_tex_to_string(tex_t tex) {
	if      (tex == sk_default_tex      ) return "white";
	else if (tex == sk_default_tex_black) return "black";
	else if (tex == sk_default_tex_gray ) return "gray";
	else if (tex == sk_default_tex_flat ) return "flat";
	else if (tex == sk_default_tex_rough) return "rough";
	else return "white";
}

///////////////////////////////////////////

void shader_file_parse(const char *hlsl, char **out_name, shaderargs_desc_t &out_desc, shader_tex_slots_t &out_slots) {
	stref_t file = stref_make(hlsl);
	stref_t line = {};

	shaderargs_desc_item_t  *buffer_items = nullptr;
	shader_tex_slots_item_t *tex_items    = nullptr;

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
			item.size = shaderarg_sz[item.type];

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

			arrput(buffer_items, item);
		} if (stref_equals(stripped_word, "texture")) {

			shader_tex_slots_item_t item;
			item.slot = arrlen(tex_items);
			if (stref_nextword(curr, word)) {
				item.name = stref_copy(word);
				item.id   = stref_hash(word);
			}

			// Find a default texture for this slot, in case it isn't used!
			if (stref_nextword(curr, word)) {
				item.default_tex = shader_stref_to_tex(word);
			} else {
				item.default_tex = tex_find("default/tex");
			}

			arrput(tex_items, item);
		} if (stref_equals(stripped_word, "name")) {
			if (!stref_nextword(curr, word))
				continue;
			*out_name = stref_copy(word);
		}
	}

	out_desc = {};
	out_desc.buffer_size = (int)buffer_size;
	size_t data_size = sizeof(shaderargs_desc_item_t) * arrlen(buffer_items);
	if (data_size > 0) {
		out_desc.item_count = arrlen(buffer_items);
		out_desc.item       = (shaderargs_desc_item_t *)malloc(data_size);
		memcpy(out_desc.item, buffer_items, data_size);
	}
	shader_init_buffer(out_desc);

	out_slots = {};
	data_size = sizeof(shader_tex_slots_item_t) * arrlen(tex_items);
	if (data_size > 0) {
		out_slots.tex_count = arrlen(tex_items);
		out_slots.tex       = (shader_tex_slots_item_t*)malloc(data_size);
		memcpy(out_slots.tex, tex_items, data_size);
	}

	arrfree(tex_items);
	arrfree(buffer_items);
}

///////////////////////////////////////////

bool shader_file_is_mem(void *data, size_t data_size) {
	shader_file_t *header = (shader_file_t *)data;
	return data_size >= sizeof(shader_file_t) && string_eq(header->signature, "sk_shd");
}

///////////////////////////////////////////

bool shader_file_parse_mem(void *data, size_t data_size, char **out_name, shaderargs_desc_t &out_desc, shader_tex_slots_t &out_slots, shader_blob_t &out_vs, shader_blob_t &out_ps) {
	uint8_t *buffer = (uint8_t*)data;

	// Verify this is a proper StereoKit shader file
	if (!shader_file_is_mem(data, data_size))
		return false;

	shader_file_t *header = (shader_file_t *)buffer;

	// Get file addresses for each chunk of data!
	shader_file_blob_t   *blob_info  = (shader_file_blob_t  *)(buffer              + sizeof(shader_file_t));
	shader_file_params_t *params     = (shader_file_params_t*)((uint8_t*)blob_info + sizeof(shader_file_blob_t)   * header->blob_count);
	shader_file_tex_t    *textures   = (shader_file_tex_t   *)((uint8_t*)params    + sizeof(shader_file_params_t) * header->param_count);
	uint8_t              *blob_start =                         (uint8_t*)textures  + sizeof(shader_file_tex_t)    * header->tex_count;

	// Grab the header information
	*out_name = string_copy(header->name);

	// Copy over the shader parameters
	out_desc.item_count  = header->param_count;
	out_desc.item        = (shaderargs_desc_item_t*)malloc(sizeof(shaderargs_desc_item_t) * header->param_count);
	for (int32_t i = 0; i < header->param_count; i++) {
		shaderargs_desc_item_t &item = out_desc.item[i];
		item.name = string_copy(params[i].name);
		item.id   = string_hash(params[i].name);
		item.tags = string_copy(params[i].tags);
		item.type = params[i].type;
		item.size = shaderarg_sz[params[i].type];
		item.default_value = malloc(item.size);
		memcpy(item.default_value, params[i].data, item.size);
	}
	shader_init_buffer(out_desc);

	// Copy the texture parameters
	out_slots.tex_count = header->tex_count;
	out_slots.tex       = (shader_tex_slots_item_t*)malloc(sizeof(shader_tex_slots_item_t) * header->tex_count);
	for (int32_t i = 0; i < header->tex_count; i++) {
		shader_tex_slots_item_t &tex = out_slots.tex[i];
		tex.id          = string_hash(textures[i].name);
		tex.slot        = i;
		tex.default_tex = shader_stref_to_tex(stref_make(textures[i].value));
	}

	// Now, copy the shader code blobs!
	uint8_t *blob = blob_start;
	for (size_t i = 0; i < header->blob_count; i++) {
		if (blob_info[i].language == shader_lang_hlsl_vs) {
			out_vs.data = malloc(blob_info[i].size);
			out_vs.size = blob_info[i].size;
			memcpy(out_vs.data, blob, out_vs.size);
		}
		else if (blob_info[i].language == shader_lang_hlsl_ps) {
			out_ps.data = malloc(blob_info[i].size);
			out_ps.size = blob_info[i].size;
			memcpy(out_ps.data, blob, out_ps.size);
		}
		blob += blob_info[i].size;
	}

	return true;
}

///////////////////////////////////////////

void shader_init_buffer(shaderargs_desc_t &desc) {
	desc.buffer_size = 0;
	for (int32_t i = 0; i < desc.item_count; i++) {
		// make sure parameters are padded so they don't overflow into the next register
		if (desc.buffer_size % register_sz != 0 && // if it's evenly aligned, it isn't a problem, but if it's not, we need to check!
			desc.buffer_size / register_sz != (desc.buffer_size+desc.item[i].size)/register_sz && // if we've crossed into the next register
			(desc.buffer_size+desc.item[i].size) % register_sz != 0) // and if crossing over means crossing allll the way over, instead of just up-to
		{
			desc.buffer_size += register_sz - (desc.buffer_size % register_sz);
		}
		desc.item[i].offset = desc.buffer_size;
		desc.buffer_size += desc.item[i].size;
	}
	// And ensure our final buffer is a multiple of register size!
	if (desc.buffer_size % register_sz != 0) {
		desc.buffer_size += register_sz - (desc.buffer_size % register_sz);
	}
}

///////////////////////////////////////////

bool shader_file_write_mem(const char *name, const shaderargs_desc_t &desc, const shader_tex_slots_t &slots, const shader_blob_t &vs, const shader_blob_t &ps, void *&out_data, size_t &out_size) {
	if (strlen(name) > SHADER_MAX_NAME) {
		log_errf("Shader name [%s] exceeds %d characters!", name, SHADER_MAX_NAME);
		return false;
	}

	// Allocate memory for the whole file
	out_size = 
		sizeof(shader_file_t) +
		sizeof(shader_file_blob_t)   * 2 +
		sizeof(shader_file_params_t) * desc.item_count +
		sizeof(shader_file_tex_t)    * slots.tex_count +
		vs.size + ps.size;
	out_data = malloc(out_size);
	uint8_t *buffer = (uint8_t *)out_data;

	// write the header
	shader_file_t *header = (shader_file_t *)buffer;
	*header = {};
	strcpy_s(header->signature, "sk_shd");
	strcpy_s(header->name,      name);
	header->blob_count  = 2;
	header->param_count = desc.item_count;
	header->tex_count   = slots.tex_count;

	// Get file addresses for each chunk of data!
	shader_file_blob_t   *blob_info  = (shader_file_blob_t  *)(buffer              + sizeof(shader_file_t));
	shader_file_params_t *params     = (shader_file_params_t*)((uint8_t*)blob_info + sizeof(shader_file_blob_t)   * header->blob_count);
	shader_file_tex_t    *textures   = (shader_file_tex_t   *)((uint8_t*)params    + sizeof(shader_file_params_t) * header->param_count);
	uint8_t              *blob_start =                         (uint8_t*)textures  + sizeof(shader_file_tex_t)    * header->tex_count;

	blob_info[0].language = shader_lang_hlsl_vs;
	blob_info[1].language = shader_lang_hlsl_ps;
	blob_info[0].size     = vs.size;
	blob_info[1].size     = ps.size;

	// Write the shader parameters
	for (int32_t i = 0; i < header->param_count; i++) {
		if (strlen(desc.item[i].name) > SHADER_MAX_PARAM_NAME) {
			log_errf("Shader param name [%s] exceeds %d characters!", desc.item[i].name, SHADER_MAX_PARAM_NAME);
			goto fail;
		}
		if (desc.item[i].tags != nullptr && strlen(desc.item[i].tags) > SHADER_MAX_PARAM_TAG) {
			log_errf("Shader param tag [%s] exceeds %d characters!", desc.item[i].tags, SHADER_MAX_PARAM_TAG);
			goto fail;
		}

		params[i] = {};
		params[i].type = desc.item[i].type;
		strcpy_s(params[i].name, desc.item[i].name);
		memcpy  (params[i].data, desc.item[i].default_value, desc.item[i].size);
		if (desc.item[i].tags != nullptr) strcpy_s(params[i].tags, desc.item[i].tags);
	}

	// Write the texture info
	for (int32_t i = 0; i < header->tex_count; i++) {
		if (strlen(slots.tex[i].name) > SHADER_MAX_PARAM_NAME) {
			log_errf("Shader texture name [%s] exceeds %d characters!", slots.tex[i].name, SHADER_MAX_PARAM_NAME);
			goto fail;
		}

		textures[i] = {};
		strcpy_s(textures[i].name,  slots.tex[i].name);
		strcpy_s(textures[i].value, shader_tex_to_string(slots.tex[i].default_tex));
	}

	// Now, the shader code blobs!
	memcpy(blob_start,           vs.data, vs.size);
	memcpy(blob_start + vs.size, ps.data, ps.size);

	return true;
fail:
	free(out_data); 
	out_data = nullptr; 
	out_size = 0;
	return false;
}


} // namespace sk