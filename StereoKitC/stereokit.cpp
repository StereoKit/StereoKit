#include "stereokit.h"

#include "assets.h"
#include "render.h"
#include "d3d.h"
#include "win32.h"
#include "openxr.h"
#include "input.h"
#include "shader_builtin.h"

#include <thread> // sleep_for

using namespace std;

sk_runtime_ sk_runtime = sk_runtime_flatscreen;
bool32_t sk_focused = true;
bool32_t sk_run     = true;

bool sk_d3d_initialized = false;

float  sk_timevf = 0;
double sk_timev  = 0;
double sk_time_start    = 0;
double sk_timev_elapsed  = 0;
float  sk_timev_elapsedf = 0;
int64_t sk_timev_raw = 0;

tex2d_t    sk_default_tex;
tex2d_t    sk_default_tex_black;
tex2d_t    sk_default_tex_gray;
shader_t   sk_default_shader;
shader_t   sk_default_shader_pbr;
material_t sk_default_material;
bool sk_create_defaults();
void sk_destroy_defaults();

bool32_t sk_init(const char *app_name, sk_runtime_ runtime_preference, bool32_t fallback) {
	// Test to avoid initializing the d3d context twice
	if (!sk_d3d_initialized) {
		if (d3d_init()) {
			sk_d3d_initialized = true;
		} else {
			log_write(log_error, "Failed to init d3d!");
			return false;
		}
		if (!sk_create_defaults()) {
			log_write(log_error, "Failed to create defaults!");
			return false;
		}
	}

	// Create a runtime
	sk_runtime = runtime_preference;
	bool result = sk_runtime == sk_runtime_mixedreality ? 
		openxr_init(app_name) :
		win32_init (app_name);

	if (!result)
		log_writef(log_warning, "Couldn't create StereoKit in %s mode!", sk_runtime == sk_runtime_mixedreality ? "MixedReality" : "Flatscreen");

	// Try falling back to flatscreen, if we didn't just try it
	if (!result && fallback && runtime_preference != sk_runtime_flatscreen) {
		log_writef(log_info, "Runtime falling back to Flatscreen");
		sk_runtime = sk_runtime_flatscreen;
		result     = win32_init (app_name);
	}

	// No runtime, return failure
	if (!result) {
		log_write(log_error, "Couldn't create a StereoKit runtime!");
		return false;
	}

	render_initialize();
	input_init();
	return true;
}

void sk_shutdown() {
	input_shutdown();
	render_shutdown();
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   win32_shutdown (); break;
	case sk_runtime_mixedreality: openxr_shutdown(); break;
	}
	//assets_shutdown_check();
	sk_destroy_defaults();
	d3d_shutdown();
}

void sk_update_timer() {
	FILETIME time;
	GetSystemTimePreciseAsFileTime(&time);
	sk_timev_raw = (int64_t)time.dwLowDateTime + ((int64_t)(time.dwHighDateTime) << 32LL);
	double time_curr = sk_timev_raw / 10000000.0;

	if (sk_time_start == 0)
		sk_time_start = time_curr;
	double new_time = time_curr - sk_time_start;
	sk_timev_elapsed  = new_time - sk_timev;
	sk_timev          = new_time;
	sk_timev_elapsedf = (float)sk_timev_elapsed;
	sk_timevf         = (float)sk_timev;
}

bool32_t sk_step(void (*app_update)(void)) {
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   win32_step_begin (); break;
	case sk_runtime_mixedreality: openxr_step_begin(); break;
	}
	
	sk_update_timer();
	input_update();

	app_update();

	d3d_render_begin();
	switch (sk_runtime) {
	case sk_runtime_flatscreen:   win32_step_end (); break;
	case sk_runtime_mixedreality: openxr_step_end(); break;
	}
	d3d_render_end();

	this_thread::sleep_for(chrono::milliseconds(sk_focused?1:250));

	return sk_run;
}

float  sk_timef        (){ return sk_timevf; };
double sk_time         (){ return sk_timev; };
float  sk_time_elapsedf(){ return sk_timev_elapsedf; };
double sk_time_elapsed (){ return sk_timev_elapsed; };
sk_runtime_ sk_active_runtime() { return sk_runtime;  }

bool sk_create_defaults() {
	// Default white texture
	sk_default_tex = tex2d_create("default/tex2d");
	if (sk_default_tex == nullptr) {
		return false;
	}
	color32 tex_colors[2*2];
	memset(tex_colors, 255, sizeof(color32) * 2 * 2);
	tex2d_set_colors(sk_default_tex, 2, 2, tex_colors);

	// Default black texture, for use with shader defaults
	sk_default_tex_black = tex2d_create("default/tex2d_black");
	if (sk_default_tex_black == nullptr) {
		return false;
	}
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = { 0,0,0,255 };
	tex2d_set_colors(sk_default_tex_black, 2, 2, tex_colors);

	// Default middle gray texture, for use with shader defaults
	sk_default_tex_gray = tex2d_create("default/tex2d_gray");
	if (sk_default_tex_gray == nullptr) {
		return false;
	}
	for (size_t i = 0; i < 2 * 2; i++) 
		tex_colors[i] = { 128,128,128,255 };
	tex2d_set_colors(sk_default_tex_gray, 2, 2, tex_colors);

	sk_default_shader = shader_create("default/shader", sk_shader_builtin_default);
	if (sk_default_shader == nullptr) {
		return false;
	}
	sk_default_shader_pbr = shader_create("default/shader_pbr", sk_shader_builtin_pbr);
	if (sk_default_shader_pbr == nullptr) {
		return false;
	}

	sk_default_material = material_create("default/material", sk_default_shader);
	if (sk_default_material == nullptr) {
		return false;
	}

	material_set_texture(sk_default_material, "diffuse", sk_default_tex);

	return true;
}

void sk_destroy_defaults() {
	material_release(sk_default_material);
	shader_release  (sk_default_shader_pbr);
	shader_release  (sk_default_shader);
	tex2d_release   (sk_default_tex);
	tex2d_release   (sk_default_tex_black);
	tex2d_release   (sk_default_tex_gray);
}