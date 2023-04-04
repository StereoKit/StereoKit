#pragma once

#include "../stereokit.h"
#include "../asset_types/assets.h"

namespace sk {

bool physics_init    ();
void physics_shutdown();
void physics_step    ();

void solid_destroy(solid_t solid);

extern double physics_sim_time;

struct _solid_t {
	asset_header_t header;
	void          *data;
#if defined(SK_PHYSICS_PASSTHROUGH)
	vec3 pos;
	quat rot;
#endif
};

} // namespace sk