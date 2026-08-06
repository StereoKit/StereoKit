#include "demo_envmap.h"

#include <stereokit.h>
#include <stereokit_ui.h>

using namespace sk;

///////////////////////////////////////////

tex_t envmap_tex = {};

tex_t                 envmap_oldtex        = {};
tex_t                 envmap_oldreflection = {};
spherical_harmonics_t envmap_oldlight      = {};
model_t               model                = {};

///////////////////////////////////////////

void demo_envmap_init() {
	envmap_oldlight      = lighting_get_ambient();
	envmap_oldtex        = render_get_skytex();
	envmap_oldreflection = lighting_get_reflection();

	envmap_tex = tex_create_cubemap_file("old_depot.hdr");
	tex_on_load(envmap_tex, [](tex_t t, void*) {
		lighting_set_environment(t);
	}, nullptr);

	model = model_create_file("DamagedHelmet.gltf");
}

///////////////////////////////////////////

void demo_envmap_update() {
	model_draw(model, matrix_trs({0,0,-0.5f}, quat_lookat(vec3_zero, -vec3_forward), {0.1f, 0.1f,0.1f}));
}

///////////////////////////////////////////

void demo_envmap_shutdown() {
	render_set_skytex      (envmap_oldtex);
	lighting_set_reflection(envmap_oldreflection);
	lighting_set_ambient   (envmap_oldlight);

	tex_release(envmap_tex);
	tex_release(envmap_oldtex);
	tex_release(envmap_oldreflection);
}
