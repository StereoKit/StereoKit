#pragma once

#include <stdint.h>

enum asset_type_ {
	asset_type_mesh = 0,
	asset_type_texture,
	asset_type_shader,
	asset_type_material,
	asset_type_model,
};

struct asset_header_t {
	asset_type_ type;
	uint64_t    id;
	int32_t     refs;
	uint64_t    index;
};

void *assets_find       (const char *id);
void *assets_allocate   (asset_type_ type, const char *id);
void  assets_unique_name(const char *root_name, char *dest, int dest_size);
void  assets_addref     (asset_header_t &asset);
void  assets_releaseref (asset_header_t &asset);
void  assets_shutdown_check();