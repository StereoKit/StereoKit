#include "stereokit.h"
#include "_stereokit.h"

#include "systems/render.h"
#include "systems/d3d.h"
#include "systems/input.h"
#include "systems/physics.h"
#include "systems/system.h"
#include "systems/text.h"
#include "systems/sprite_drawer.h"
#include "systems/line_drawer.h"
#include "systems/defaults.h"
#include "systems/platform/platform.h"

#include <thread> // sleep_for
using namespace std;

#include <chrono>
using namespace std::chrono;

namespace sk {

///////////////////////////////////////////

const char   *sk_app_name;
void        (*sk_app_update_func)(void);
runtime_      sk_runtime = runtime_flatscreen;
bool          sk_runtime_fallback = false;
settings_t    sk_settings = {};
system_info_t sk_info     = {};
bool32_t      sk_focused  = true;
bool32_t      sk_run      = true;

bool sk_initialized = false;

float   sk_timevf = 0;
double  sk_timev  = 0;
double  sk_time_start     = 0;
double  sk_timev_elapsed  = 0;
float   sk_timev_elapsedf = 0;
int64_t sk_timev_raw      = 0;

///////////////////////////////////////////

void sk_set_settings(settings_t &settings) {
	if (sk_initialized) {
		log_err("Settings need set before initialization! Please call this -before- sk_init.");
		return;
	}
	sk_settings = settings;

	// Set some default values
	if (sk_settings.flatscreen_width  == 0)
		sk_settings.flatscreen_width  = 800;
	if (sk_settings.flatscreen_height == 0)
		sk_settings.flatscreen_height = 480;
}

///////////////////////////////////////////

const system_info_t& sk_get_info() {
	return sk_info;
}

///////////////////////////////////////////

void sk_app_update() {
	if (sk_app_update_func != nullptr)
		sk_app_update_func();
}

///////////////////////////////////////////

bool32_t sk_init(const char *app_name, runtime_ runtime_preference, bool32_t fallback) {
	sk_runtime          = runtime_preference;
	sk_runtime_fallback = fallback;
	sk_app_name         = app_name;

	// Make sure settings get their default values
	sk_set_settings(sk_settings);

	sk_update_timer();

	systems_add("Graphics", nullptr, 0, nullptr, 0, d3d_init, d3d_update, d3d_shutdown);

	const char *default_deps[] = {"Graphics"};
	systems_add("Defaults", default_deps, _countof(default_deps), nullptr, 0, defaults_init, nullptr, defaults_shutdown);

	const char *platform_deps[] = {"Graphics", "Defaults"};
	systems_add("Platform", platform_deps, _countof(platform_deps), nullptr, 0, platform_init, nullptr, platform_shutdown);

	const char *physics_deps[] = {"Defaults"};
	const char *physics_update_deps[] = {"Input", "FrameBegin"};
	systems_add("Physics",  
		physics_deps,        _countof(physics_deps), 
		physics_update_deps, _countof(physics_update_deps), 
		physics_init, physics_update, physics_shutdown);

	const char *renderer_deps[] = {"Graphics", "Defaults"};
	const char *renderer_update_deps[] = {"Physics", "FrameBegin"};
	systems_add("Renderer",  
		renderer_deps,        _countof(renderer_deps), 
		renderer_update_deps, _countof(renderer_update_deps),
		render_initialize, render_update, render_shutdown);

	const char *input_deps[] = {"Platform", "Defaults"};
	const char *input_update_deps[] = {"FrameBegin"};
	systems_add("Input",  
		input_deps,        _countof(input_deps), 
		input_update_deps, _countof(input_update_deps), 
		input_init, input_update, input_shutdown);

	const char *text_deps[] = {"Defaults"};
	const char *text_update_deps[] = {"App"};
	systems_add("Text",  
		text_deps,        _countof(text_deps), 
		text_update_deps, _countof(text_update_deps), 
		nullptr, text_update, text_shutdown);

	const char *sprite_deps[] = {"Defaults"};
	const char *sprite_update_deps[] = {"App"};
	systems_add("Sprites",  
		sprite_deps,        _countof(sprite_deps), 
		sprite_update_deps, _countof(sprite_update_deps), 
		sprite_drawer_init, sprite_drawer_update, sprite_drawer_shutdown);

	const char *line_deps[] = {"Defaults"};
	const char *line_update_deps[] = {"App"};
	systems_add("Lines",  
		line_deps,        _countof(line_deps), 
		line_update_deps, _countof(line_update_deps), 
		line_drawer_init, line_drawer_update, line_drawer_shutdown);

	const char *app_deps[] = {"Input", "Defaults", "FrameBegin", "Graphics", "Physics", "Renderer"};
	systems_add("App", nullptr, 0, app_deps, _countof(app_deps), nullptr, sk_app_update, nullptr);

	systems_add("FrameBegin", nullptr, 0, nullptr, 0, nullptr, platform_begin_frame, nullptr);
	const char *platform_end_deps[] = {"App", "Text", "Sprites", "Lines"};
	systems_add("FrameRender",   nullptr, 0, platform_end_deps, _countof(platform_end_deps), nullptr, platform_end_frame,   nullptr);
	const char *platform_present_deps[] = {"FrameRender"};
	systems_add("FramePresent", nullptr, 0, platform_present_deps, _countof(platform_present_deps), nullptr, platform_present,   nullptr);

	sk_initialized = systems_initialize();
	return sk_initialized;
}

///////////////////////////////////////////

void sk_shutdown() {
	systems_shutdown();
	sk_initialized = false;
}

///////////////////////////////////////////

bool32_t sk_step(void (*app_update)(void)) {
	sk_app_update_func = app_update;
	sk_update_timer();

	systems_update();

	if (!sk_focused)
		this_thread::sleep_for(milliseconds(sk_focused ? 1 : 250));
	return sk_run;
}

///////////////////////////////////////////

void sk_update_timer() {
	time_point<high_resolution_clock> now = high_resolution_clock::now();
	sk_timev_raw = duration_cast<nanoseconds>(now.time_since_epoch()).count();
	double time_curr = sk_timev_raw / 1000000000.0;

	if (sk_time_start == 0)
		sk_time_start = time_curr;
	double new_time = time_curr - sk_time_start;
	sk_timev_elapsed  = new_time - sk_timev;
	sk_timev          = new_time;
	sk_timev_elapsedf = (float)sk_timev_elapsed;
	sk_timevf         = (float)sk_timev;
}

///////////////////////////////////////////

runtime_ sk_active_runtime() { return sk_runtime; }

///////////////////////////////////////////

float  time_getf    (){ return sk_timevf; };
double time_get     (){ return sk_timev; };
float  time_elapsedf(){ return sk_timev_elapsedf; };
double time_elapsed (){ return sk_timev_elapsed; };


} // namespace sk