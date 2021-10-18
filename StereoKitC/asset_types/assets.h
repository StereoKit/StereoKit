#pragma once

#include "../systems/platform/platform_utils.h" // SK_DEBUG
#include <stdint.h>

namespace sk {

enum asset_type_ {
	asset_type_mesh = 0,
	asset_type_texture,
	asset_type_shader,
	asset_type_material,
	asset_type_model,
	asset_type_font,
	asset_type_sprite,
	asset_type_sound,
};

struct asset_header_t {
	asset_type_ type;
	uint64_t    id;
	int32_t     refs;
	uint64_t    index;
#if defined(SK_DEBUG)
	char       *id_text;
#endif
};

void       *assets_find          (const char *id, asset_type_ type);
void       *assets_find          (uint64_t    id, asset_type_ type);
void       *assets_allocate      (asset_type_ type);
void        assets_set_id        (asset_header_t &header, const char *id);
void        assets_set_id        (asset_header_t &header, uint64_t    id);
void        assets_unique_name   (asset_type_ type, const char *root_name, char *dest, int dest_size);
void        assets_addref        (asset_header_t &asset);
void        assets_releaseref    (asset_header_t &asset);
void        assets_safeswap_ref  (asset_header_t **asset_link, asset_header_t *asset);
void        assets_shutdown_check();
const char *assets_file          (const char *file_name);
bool        assets_init          ();
void        assets_update        ();
void        assets_shutdown      ();

} // namespace sk