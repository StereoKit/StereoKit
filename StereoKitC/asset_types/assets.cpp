#include "assets.h"
#include "../_stereokit.h"

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "material.h"
#include "model.h"
#include "font.h"
#include "sprite.h"
#include "../libraries/stref.h"

#include <stdio.h>
#include <assert.h>
#include <vector>
using namespace std;

namespace sk {

///////////////////////////////////////////

vector<asset_header_t *> assets;

///////////////////////////////////////////

void *assets_find(const char *id) {
	return assets_find(string_hash(id));
}

///////////////////////////////////////////

void *assets_find(uint64_t id) {
	size_t count = assets.size();
	for (size_t i = 0; i < count; i++) {
		if (assets[i]->id == id)
			return assets[i];
	}
	return nullptr;
}

///////////////////////////////////////////

void assets_unique_name(const char *root_name, char *dest, int dest_size) {
	sprintf_s(dest, dest_size, "%s", root_name);
	uint64_t id    = string_hash(dest);
	int      count = 1;
	while (assets_find(dest) != nullptr) {
		sprintf_s(dest, dest_size, "%s%d", root_name, count);
		id = string_hash(dest);
		count += 1;
	}
}

///////////////////////////////////////////

void *assets_allocate(asset_type_ type) {
	size_t size = sizeof(asset_header_t);
	switch(type) {
	case asset_type_mesh:     size = sizeof(_mesh_t );    break;
	case asset_type_texture:  size = sizeof(_tex_t);      break;
	case asset_type_shader:   size = sizeof(_shader_t);   break;
	case asset_type_material: size = sizeof(_material_t); break;
	case asset_type_model:    size = sizeof(_model_t);    break;
	case asset_type_font:     size = sizeof(_font_t);     break;
	case asset_type_sprite:   size = sizeof(_sprite_t);   break;
	default: throw "Unimplemented asset type!";
	}

	char name[64];
	sprintf_s(name, "auto/asset_%d", (int)assets.size());

	asset_header_t *header = (asset_header_t *)malloc(size);
	memset(header, 0, size);
	header->type  = type;
	header->refs += 1;
	header->id    = string_hash(name);
	header->index = assets.size();
	assets.push_back(header);
	return header;
}

///////////////////////////////////////////

void assets_set_id(asset_header_t &header, const char *id) {
	assets_set_id(header, string_hash(id));
#ifdef _DEBUG
	header.id_text = string_copy(id);
#endif
}

///////////////////////////////////////////

void assets_set_id(asset_header_t &header, uint64_t id) {
#if _DEBUG
	assert(assets_find(id) == nullptr);
#endif
	header.id = id;
}

///////////////////////////////////////////

void  assets_addref(asset_header_t &asset) {
	asset.refs += 1;
}

///////////////////////////////////////////

void  assets_releaseref(asset_header_t &asset) {
	// Manage the reference count
	asset.refs -= 1;
	if (asset.refs < 0)
		throw "Released too many references to asset!";
	if (asset.refs != 0)
		return;

	// Call asset specific destroy function
	switch(asset.type) {
	case asset_type_mesh:     mesh_destroy    ((mesh_t    )&asset); break;
	case asset_type_texture:  tex_destroy     ((tex_t     )&asset); break;
	case asset_type_shader:   shader_destroy  ((shader_t  )&asset); break;
	case asset_type_material: material_destroy((material_t)&asset); break;
	case asset_type_model:    model_destroy   ((model_t   )&asset); break;
	case asset_type_font:     font_destroy    ((font_t    )&asset); break;
	case asset_type_sprite:   sprite_destroy  ((sprite_t  )&asset); break;
	default: throw "Unimplemented asset type!";
	}

	// Remove it from our list of assets
	for (size_t i = 0; i < assets.size(); i++) {
		if (assets[i] == &asset) {
			assets.erase(assets.begin() + i);
			break;
		}
	}

	// And at last, free the memory we allocated for it!
#ifdef _DEBUG
	free(asset.id_text);
#endif
	free(&asset);
}

///////////////////////////////////////////

void  assets_shutdown_check() {
	if (assets.size() > 0) {
		log_errf("%d unreleased assets still found in the asset manager!", (int)assets.size());
	}
}

///////////////////////////////////////////

char assets_file_buffer[1024];
const char *assets_file(const char *file_name) {
	if (file_name == nullptr)
		return file_name;

	const char *ch = file_name;
	while (*ch != '\0') {
		if (*ch == ':') {
			return file_name;
		}
		ch++;
	}

	sprintf_s(assets_file_buffer, "%s/%s", sk_settings.assets_folder, file_name);
	return assets_file_buffer;
}

} // namespace sk