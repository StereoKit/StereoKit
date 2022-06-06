#include "demo_envmap.h"

#include <stereokit.h>
#include <stereokit_ui.h>

using namespace sk;

///////////////////////////////////////////

tex_t envmap_tex = {};

tex_t                 envmap_oldtex   = {};
spherical_harmonics_t envmap_oldlight = {};

///////////////////////////////////////////

void demo_envmap_init() {
	envmap_oldlight = render_get_skylight();
	envmap_oldtex   = render_get_skytex();

	log_info("load");
	envmap_tex = tex_create_cubemap_file("old_depot.hdr");
	tex_on_load(envmap_tex, [](tex_t t, void*) {
		log_info("callback");
		render_set_skylight(tex_get_cubemap_lighting(t));
		render_set_skytex  (t);
	}, nullptr);
}

///////////////////////////////////////////

void demo_envmap_update() {
}

///////////////////////////////////////////

void demo_envmap_shutdown() {
	render_set_skylight(envmap_oldlight);
	render_set_skytex  (envmap_oldtex);

	tex_release(envmap_tex);
	tex_release(envmap_oldtex);
}