#pragma once

#include <stdint.h>

enum asset_type_ {
	asset_type_mesh = 0,
	asset_type_texture,
	asset_type_shader,
	asset_type_material,
};

struct asset_header_t {
	asset_type_ type;
	int32_t     refs;
};

void *assets_allocate  (asset_type_ type);
void  assets_addref    (asset_header_t &asset);
void  assets_releaseref(asset_header_t &asset);
void  assets_shutdown_check();