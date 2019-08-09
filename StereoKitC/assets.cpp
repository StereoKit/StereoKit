#include "assets.h"

#include "mesh.h"
#include "texture.h"
#include "shader.h"
#include "material.h"
#include "model.h"
#include "stref.h"

#include <vector>
using namespace std;

vector<asset_header_t *> assets;

void *assets_find(const char *id) {
	uint64_t hash  = string_hash(id);
	size_t   count = assets.size();
	for (size_t i = 0; i < count; i++) {
		if (assets[i]->id == hash)
			return assets[i];
	}
	return nullptr;
}

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

void *assets_allocate(asset_type_ type, const char *id) {
#if _DEBUG
	assert(assets_find(id) == nullptr);
#endif

	size_t size = sizeof(asset_header_t);
	switch(type) {
	case asset_type_mesh:     size = sizeof(_mesh_t );    break;
	case asset_type_texture:  size = sizeof(_tex2d_t);    break;
	case asset_type_shader:   size = sizeof(_shader_t);   break;
	case asset_type_material: size = sizeof(_material_t); break;
	case asset_type_model:    size = sizeof(_model_t);    break;
	default: throw "Unimplemented asset type!";
	}

	asset_header_t *header = (asset_header_t *)malloc(size);
	memset(header, 0, size);
	header->type  = type;
	header->refs += 1;
	header->id    = string_hash(id);
	header->index = assets.size();
	assets.push_back(header);
	return header;
}
void  assets_addref(asset_header_t &asset) {
	asset.refs += 1;
}
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
	case asset_type_texture:  tex2d_destroy   ((tex2d_t   )&asset); break;
	case asset_type_shader:   shader_destroy  ((shader_t  )&asset); break;
	case asset_type_material: material_destroy((material_t)&asset); break;
	case asset_type_model:    model_destroy   ((model_t   )&asset); break;
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
	free(&asset);
}

void  assets_shutdown_check() {
	if (assets.size() > 0) {
		log_writef(log_error, "%d unreleased assets still found in the asset manager!", (int)assets.size());
	}
}