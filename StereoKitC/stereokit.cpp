#include "stereokit.h"
#include "_stereokit.h"
#include "_stereokit_ui.h"
#include "log.h"

#include "systems/render.h"
#include "systems/input.h"
#include "systems/physics.h"
#include "systems/system.h"
#include "systems/text.h"
#include "systems/sprite_drawer.h"
#include "systems/line_drawer.h"
#include "systems/defaults.h"
#include "systems/platform/win32.h"
#include "systems/platform/uwp.h"
#include "systems/platform/android.h"
#include "systems/platform/openxr.h"
#include "systems/platform/platform.h"
#include "systems/platform/platform_utils.h"
#include "asset_types/sound.h"

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

double  sk_timev_scale       = 1;
float   sk_timevf            = 0;
double  sk_timev             = 0;
float   sk_timevf_us         = 0;
double  sk_timev_us          = 0;
double  sk_time_start        = 0;
double  sk_timev_elapsed     = 0;
float   sk_timev_elapsedf    = 0;
double  sk_timev_elapsed_us  = 0;
float   sk_timev_elapsedf_us = 0;
int64_t sk_timev_raw         = 0;

///////////////////////////////////////////

settings_t sk_get_settings() {
	return sk_settings;
}

///////////////////////////////////////////

void sk_set_settings(const settings_t &settings) {
	if (sk_initialized) {
		log_err("Settings need set before initialization! Please call this -before- sk_init.");
		return;
	}
	sk_settings = settings;

	// Set some default values
	if (sk_settings.flatscreen_width  == 0)
		sk_settings.flatscreen_width  = 1280;
	if (sk_settings.flatscreen_height == 0)
		sk_settings.flatscreen_height = 720;
}

///////////////////////////////////////////

system_info_t sk_system_info() {
	return sk_info;
}

///////////////////////////////////////////

const char *sk_version_name() {
	return SK_VERSION;
}

///////////////////////////////////////////

uint64_t sk_version_id() {
	return SK_VERSION_ID;
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

	log_diagf("Initializing StereoKit v%s...", sk_version_name());

	// Make sure settings get their default values
	sk_set_settings(sk_settings);

	sk_update_timer();

	// Platform related systems
	system_t sys_platform         = { "Platform"     };
	system_t sys_platform_begin   = { "FrameBegin"   };
	system_t sys_platform_render  = { "FrameRender"  };
	system_t sys_platform_present = { "FramePresent" };

	sys_platform        .func_initialize = platform_init;
	sys_platform        .func_shutdown   = platform_shutdown;
	sys_platform_begin  .func_update     = platform_step_begin;
	sys_platform_render .func_update     = platform_step_end;
	sys_platform_present.func_update     = platform_present;

	const char *frame_present_update_deps[] = {"FrameRender"};
	const char *frame_render_update_deps [] = {"App", "Text", "Sprites", "Lines"};
	sys_platform_render .update_dependencies     = frame_render_update_deps;
	sys_platform_render .update_dependency_count = _countof(frame_render_update_deps);
	sys_platform_present.update_dependencies     = frame_present_update_deps;
	sys_platform_present.update_dependency_count = _countof(frame_present_update_deps);
	
	systems_add(&sys_platform);
	systems_add(&sys_platform_begin);
	systems_add(&sys_platform_render);
	systems_add(&sys_platform_present);

	// Rest of the systems
	system_t sys_defaults = { "Defaults" };
	const char *default_deps[] = { "Platform" };
	sys_defaults.init_dependencies     = default_deps;
	sys_defaults.init_dependency_count = _countof(default_deps);
	sys_defaults.func_initialize       = defaults_init;
	sys_defaults.func_shutdown         = defaults_shutdown;
	systems_add(&sys_defaults);

	system_t sys_ui = { "UI" };
	const char *ui_deps       [] = {"Defaults"};
	const char *ui_update_deps[] = {"Input"};
	sys_ui.init_dependencies       = ui_deps;
	sys_ui.init_dependency_count   = _countof(ui_deps);
	sys_ui.update_dependencies     = ui_update_deps;
	sys_ui.update_dependency_count = _countof(ui_update_deps);
	sys_ui.func_initialize         = ui_init;
	sys_ui.func_update             = ui_update;
	sys_ui.func_shutdown           = ui_shutdown;
	systems_add(&sys_ui);

	system_t sys_physics = { "Physics" };
	const char *physics_deps       [] = {"Defaults"};
	const char *physics_update_deps[] = {"Input", "FrameBegin"};
	sys_physics.init_dependencies       = physics_deps;
	sys_physics.init_dependency_count   = _countof(physics_deps);
	sys_physics.update_dependencies     = physics_update_deps;
	sys_physics.update_dependency_count = _countof(physics_update_deps);
	sys_physics.func_initialize         = physics_init;
	sys_physics.func_update             = physics_update;
	sys_physics.func_shutdown           = physics_shutdown;
	systems_add(&sys_physics);

	system_t sys_renderer = { "Renderer" };
	const char *renderer_deps       [] = {"Platform", "Defaults"};
	const char *renderer_update_deps[] = {"Physics", "FrameBegin"};
	sys_renderer.init_dependencies       = renderer_deps;
	sys_renderer.init_dependency_count   = _countof(renderer_deps);
	sys_renderer.update_dependencies     = renderer_update_deps;
	sys_renderer.update_dependency_count = _countof(renderer_update_deps);
	sys_renderer.func_initialize         = render_init;
	sys_renderer.func_update             = render_update;
	sys_renderer.func_shutdown           = render_shutdown;
	systems_add(&sys_renderer);

	system_t sys_sound = { "Sound" };
	const char *sound_deps       [] = {"Platform"};
	const char *sound_update_deps[] = {"Platform"};
	sys_sound.init_dependencies       = sound_deps;
	sys_sound.init_dependency_count   = _countof(sound_deps);
	sys_sound.update_dependencies     = sound_update_deps;
	sys_sound.update_dependency_count = _countof(sound_update_deps);
	sys_sound.func_initialize         = sound_init;
	sys_sound.func_update             = sound_update;
	sys_sound.func_shutdown           = sound_shutdown;
	systems_add(&sys_sound);

	system_t sys_input = { "Input" };
	const char *input_deps       [] = {"Platform", "Defaults"};
	const char *input_update_deps[] = {"FrameBegin"};
	sys_input.init_dependencies       = input_deps;
	sys_input.init_dependency_count   = _countof(input_deps);
	sys_input.update_dependencies     = input_update_deps;
	sys_input.update_dependency_count = _countof(input_update_deps);
	sys_input.func_initialize         = input_init;
	sys_input.func_update             = input_update;
	sys_input.func_shutdown           = input_shutdown;
	systems_add(&sys_input);

	system_t sys_text = { "Text" };
	const char *text_deps       [] = {"Defaults"};
	const char *text_update_deps[] = {"App"};
	sys_text.init_dependencies       = text_deps;
	sys_text.init_dependency_count   = _countof(text_deps);
	sys_text.update_dependencies     = text_update_deps;
	sys_text.update_dependency_count = _countof(text_update_deps);
	sys_text.func_update             = text_update;
	sys_text.func_shutdown           = text_shutdown;
	systems_add(&sys_text);

	system_t sys_sprite = { "Sprites" };
	const char *sprite_deps       [] = {"Defaults"};
	const char *sprite_update_deps[] = {"App"};
	sys_sprite.init_dependencies       = sprite_deps;
	sys_sprite.init_dependency_count   = _countof(sprite_deps);
	sys_sprite.update_dependencies     = sprite_update_deps;
	sys_sprite.update_dependency_count = _countof(sprite_update_deps);
	sys_sprite.func_initialize         = sprite_drawer_init;
	sys_sprite.func_update             = sprite_drawer_update;
	sys_sprite.func_shutdown           = sprite_drawer_shutdown;
	systems_add(&sys_sprite);

	system_t sys_lines = { "Lines" };
	const char *line_deps       [] = {"Defaults"};
	const char *line_update_deps[] = {"App"};
	sys_lines.init_dependencies       = line_deps;
	sys_lines.init_dependency_count   = _countof(line_deps);
	sys_lines.update_dependencies     = line_update_deps;
	sys_lines.update_dependency_count = _countof(line_update_deps);
	sys_lines.func_initialize         = line_drawer_init;
	sys_lines.func_update             = line_drawer_update;
	sys_lines.func_shutdown           = line_drawer_shutdown;
	systems_add(&sys_lines);

	system_t sys_app = { "App" };
	const char *app_update_deps[] = {"Input", "Defaults", "FrameBegin", "Platform", "Physics", "Renderer", "UI"};
	sys_app.update_dependencies     = app_update_deps;
	sys_app.update_dependency_count = _countof(app_update_deps);
	sys_app.func_update             = sk_app_update;
	systems_add(&sys_app);

	sk_initialized = systems_initialize();
	if (!sk_initialized) log_show_any_fail_reason();
	else                 log_clear_any_fail_reason();
	return sk_initialized;
}

///////////////////////////////////////////

bool32_t sk_set_window(void *window, runtime_ preferred_runtime) {
	return platform_set_window(window, preferred_runtime);
}

///////////////////////////////////////////

void sk_shutdown() {
	log_show_any_fail_reason();

	systems_shutdown();
	sk_initialized = false;
}

///////////////////////////////////////////

void sk_quit() {
	sk_run = false;
}

///////////////////////////////////////////

bool32_t sk_step(void (*app_update)(void)) {
	sk_app_update_func = app_update;
	sk_update_timer();

	systems_update();

	if (!sk_focused)
		platform_sleep(sk_focused ? 1 : 100);
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
	sk_timev_elapsed_us  =  new_time - sk_timev_us;
	sk_timev_elapsed     = (new_time - sk_timev_us) * sk_timev_scale;
	sk_timev_us          = new_time;
	sk_timev            += sk_timev_elapsed;
	sk_timev_elapsedf_us = (float)sk_timev_elapsed_us;
	sk_timev_elapsedf    = (float)sk_timev_elapsed;
	sk_timevf_us         = (float)sk_timev_us;
	sk_timevf            = (float)sk_timev;
}

///////////////////////////////////////////

runtime_ sk_active_runtime() { return sk_runtime; }

///////////////////////////////////////////

float  time_getf_unscaled    (){ return sk_timevf_us; };
double time_get_unscaled     (){ return sk_timev_us; };
float  time_getf             (){ return sk_timevf; };
double time_get              (){ return sk_timev; };
float  time_elapsedf_unscaled(){ return sk_timev_elapsedf_us; };
double time_elapsed_unscaled (){ return sk_timev_elapsed_us; };
float  time_elapsedf         (){ return sk_timev_elapsedf; };
double time_elapsed          (){ return sk_timev_elapsed; };
void   time_scale(double scale) { sk_timev_scale = scale; }

///////////////////////////////////////////

void time_set_time(double total_seconds, double frame_elapsed_seconds) {
	if (frame_elapsed_seconds < 0) {
		frame_elapsed_seconds = sk_timev_elapsed_us;
		if (frame_elapsed_seconds == 0)
			frame_elapsed_seconds = 1.f / 90.f;
	}
	total_seconds = fmax(total_seconds, 0);

	time_point<high_resolution_clock> now = high_resolution_clock::now();
	sk_timev_raw  = duration_cast<nanoseconds>(now.time_since_epoch()).count();
	sk_time_start = (sk_timev_raw / 1000000000.0) - total_seconds; 

	sk_timev_elapsed_us  = frame_elapsed_seconds;
	sk_timev_elapsed     = frame_elapsed_seconds * sk_timev_scale;
	sk_timev_us          = total_seconds;
	sk_timev             = total_seconds;
	sk_timev_elapsedf_us = (float)sk_timev_elapsed_us;
	sk_timev_elapsedf    = (float)sk_timev_elapsed;
	sk_timevf_us         = (float)sk_timev_us;
	sk_timevf            = (float)sk_timev;
	physics_sim_time     = sk_timev;
}

} // namespace sk