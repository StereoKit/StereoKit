#include "stereokit.h"
#include "_stereokit.h"
#include "_stereokit_ui.h"
#include "log.h"

#include "libraries/sokol_time.h"

#include "systems/render.h"
#include "systems/input.h"
#ifdef SK_SYSTEMS_PHYSICS
	#include "systems/physics.h"
#endif
#include "systems/system.h"
#include "systems/text.h"
#include "systems/audio.h"
#include "systems/sprite_drawer.h"
#include "systems/line_drawer.h"
#include "systems/world.h"
#include "systems/defaults.h"
#include "systems/platform/win32.h"
#include "systems/platform/uwp.h"
#include "systems/platform/android.h"
#include "systems/platform/openxr.h"
#include "systems/platform/platform.h"
#include "systems/platform/platform_utils.h"

namespace sk {

///////////////////////////////////////////

const char   *sk_app_name;
void        (*sk_app_update_func)(void);
display_mode_ sk_display_mode           = display_mode_mixedreality;
bool          sk_no_flatscreen_fallback = false;
sk_settings_t sk_settings = {};
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
uint64_t sk_timev_raw        = 0;

uint64_t  app_init_time = 0;
system_t *app_system    = nullptr;

///////////////////////////////////////////

sk_settings_t sk_get_settings() {
	return sk_settings;
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

bool32_t sk_init(sk_settings_t settings) {
	sk_settings               = settings;
	sk_display_mode           = sk_settings.display_preference;
	sk_no_flatscreen_fallback = sk_settings.no_flatscreen_fallback;
	sk_app_name               = sk_settings.app_name == nullptr ? "StereoKit App" : sk_settings.app_name;
	if (sk_settings.log_filter != log_none)
		log_set_filter(sk_settings.log_filter);

	// Set some default values
	if (sk_settings.flatscreen_width  == 0)
		sk_settings.flatscreen_width  = 1280;
	if (sk_settings.flatscreen_height == 0)
		sk_settings.flatscreen_height = 720;

	log_diagf("Initializing StereoKit v%s...", sk_version_name());

	stm_setup();
	sk_update_timer();

	// Platform related systems
	system_t sys_platform         = { "Platform"    };
	system_t sys_platform_begin   = { "FrameBegin"  };
	system_t sys_platform_render  = { "FrameRender" };

	sys_platform        .func_initialize = platform_init;
	sys_platform        .func_shutdown   = platform_shutdown;
	sys_platform_begin  .func_update     = platform_step_begin;
	sys_platform_render .func_update     = platform_step_end;

	const char *frame_render_update_deps [] = {"App", "Text", "Sprites", "Lines"};
	sys_platform_render .update_dependencies     = frame_render_update_deps;
	sys_platform_render .update_dependency_count = _countof(frame_render_update_deps);

	systems_add(&sys_platform);
	systems_add(&sys_platform_begin);
	systems_add(&sys_platform_render);

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

	#ifdef SK_SYSTEMS_PHYSICS
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
	#endif

	system_t sys_renderer = { "Renderer" };
	const char *renderer_deps       [] = {"Platform", "Defaults"};
	const char *renderer_update_deps[] = {
#ifdef SK_SYSTEMS_PHYSICS
		"Physics",
#endif
		"FrameBegin"
	};
	sys_renderer.init_dependencies       = renderer_deps;
	sys_renderer.init_dependency_count   = _countof(renderer_deps);
	sys_renderer.update_dependencies     = renderer_update_deps;
	sys_renderer.update_dependency_count = _countof(renderer_update_deps);
	sys_renderer.func_initialize         = render_init;
	sys_renderer.func_update             = render_update;
	sys_renderer.func_shutdown           = render_shutdown;
	systems_add(&sys_renderer);

	system_t sys_audio = { "Audio" };
	const char *audio_deps       [] = {"Platform"};
	const char *audio_update_deps[] = {"Platform"};
	sys_audio.init_dependencies       = audio_deps;
	sys_audio.init_dependency_count   = _countof(audio_deps);
	sys_audio.update_dependencies     = audio_update_deps;
	sys_audio.update_dependency_count = _countof(audio_update_deps);
	sys_audio.func_initialize         = audio_init;
	sys_audio.func_update             = audio_update;
	sys_audio.func_shutdown           = audio_shutdown;
	systems_add(&sys_audio);

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

	system_t sys_world = { "World" };
	const char *world_deps       [] = {"Platform", "Defaults"};
	const char *world_update_deps[] = {"Platform"};
	sys_world.init_dependencies       = world_deps;
	sys_world.init_dependency_count   = _countof(world_deps);
	sys_world.update_dependencies     = world_update_deps;
	sys_world.update_dependency_count = _countof(world_update_deps);
	sys_world.func_initialize         = world_init;
	sys_world.func_update             = world_update;
	sys_world.func_shutdown           = world_shutdown;
	systems_add(&sys_world);

	system_t sys_app = { "App" };
	const char *app_update_deps[] = {
		"Input",
		"Defaults",
		"FrameBegin",
		"Platform",
		#ifdef SK_SYSTEMS_PHYSICS
		"Physics",
		#endif
		"Renderer",
		"UI",
		"World"
	};
	sys_app.update_dependencies     = app_update_deps;
	sys_app.update_dependency_count = _countof(app_update_deps);
	sys_app.func_update             = sk_app_update;
	systems_add(&sys_app);

	sk_initialized = systems_initialize();
	if (!sk_initialized) log_show_any_fail_reason();
	else                 log_clear_any_fail_reason();

	app_system    = systems_find("App");
	app_init_time = stm_now();
	return sk_initialized;
}

///////////////////////////////////////////

void sk_set_window(void *window) {
	platform_set_window(window);
}

///////////////////////////////////////////

void sk_set_window_xam(void *window) {
	platform_set_window_xam(window);
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
	if (app_system->profile_start_duration == 0)
		app_system->profile_start_duration = stm_since(app_init_time);

	sk_app_update_func = app_update;
	sk_update_timer();

	systems_update();

	if (!sk_focused)
		platform_sleep(sk_focused ? 1 : 100);
	return sk_run;
}

///////////////////////////////////////////

void sk_update_timer() {
	sk_timev_raw = stm_now();
	double time_curr = stm_sec(sk_timev_raw);

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

display_mode_ sk_active_display_mode() { return sk_display_mode; }

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

	sk_timev_raw  = stm_now();
	sk_time_start = stm_sec(sk_timev_raw) - total_seconds;

	sk_timev_elapsed_us  = frame_elapsed_seconds;
	sk_timev_elapsed     = frame_elapsed_seconds * sk_timev_scale;
	sk_timev_us          = total_seconds;
	sk_timev             = total_seconds;
	sk_timev_elapsedf_us = (float)sk_timev_elapsed_us;
	sk_timev_elapsedf    = (float)sk_timev_elapsed;
	sk_timevf_us         = (float)sk_timev_us;
	sk_timevf            = (float)sk_timev;
#ifdef SK_SYSTEMS_PHYSICS
	physics_sim_time     = sk_timev;
#endif
}

} // namespace sk
