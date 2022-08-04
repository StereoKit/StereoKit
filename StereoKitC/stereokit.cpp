#include "stereokit.h"
#include "_stereokit.h"
#include "_stereokit_ui.h"
#include "log.h"

#include "libraries/sokol_time.h"
#include "libraries/tinycthread.h"

#include "systems/render.h"
#include "systems/input.h"
#include "systems/physics.h"
#include "systems/system.h"
#include "systems/text.h"
#include "systems/audio.h"
#include "systems/sprite_drawer.h"
#include "systems/line_drawer.h"
#include "systems/world.h"
#include "systems/defaults.h"
#include "asset_types/animation.h"
#include "platforms/win32.h"
#include "platforms/uwp.h"
#include "platforms/android.h"
#include "platforms/web.h"
#include "platforms/platform.h"
#include "platforms/platform_utils.h"
#include "xr_backends/openxr.h"

namespace sk {

///////////////////////////////////////////

const char   *sk_app_name;
void        (*sk_app_update_func)(void);
display_mode_ sk_display_mode           = display_mode_none;
bool          sk_no_flatscreen_fallback = false;
sk_settings_t sk_settings    = {};
system_info_t sk_info        = {};
app_focus_    sk_focus       = app_focus_active;
bool32_t      sk_running     = false;
bool32_t      sk_initialized = false;
bool32_t      sk_first_step  = false;
thrd_id_t     sk_init_thread = {};

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
	return SK_VERSION " "
#if defined(SK_OS_WEB)
		"Web"
#elif defined(SK_OS_ANDROID)
		"Android"
#elif defined(SK_OS_LINUX)
		"Linux"
#elif defined(SK_OS_WINDOWS)
		"Win32"
#elif defined(SK_OS_WINDOWS_UWP)
		"UWP"
#else
		"MysteryPlatform"
#endif
		
		" "
		
#if defined(__x86_64__) || defined(_M_X64)
		"x64"
#elif defined(__aarch64__) || defined(_M_ARM64)
		"ARM64"
#elif defined(_M_ARM)
		"ARM"
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(_M_IX86)
		"x86"
#else
		"MysteryArchitecture"
#endif
	;
}

///////////////////////////////////////////

uint64_t sk_version_id() {
	return SK_VERSION_ID;
}

///////////////////////////////////////////

app_focus_ sk_app_focus() {
	return sk_focus;
}

///////////////////////////////////////////

void sk_app_update() {
	if (sk_app_update_func != nullptr)
		sk_app_update_func();
}

///////////////////////////////////////////

bool32_t sk_init(sk_settings_t settings) {
	sk_settings               = settings;
	sk_no_flatscreen_fallback = sk_settings.no_flatscreen_fallback;
	sk_app_name               = sk_settings.app_name == nullptr ? "StereoKit App" : sk_settings.app_name;
	sk_init_thread            = thrd_id_current();
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

	const char *frame_render_update_deps [] = {"App", "Text", "Sprites", "Lines", "World", "UILate", "Animation"};
	sys_platform_render .update_dependencies     = frame_render_update_deps;
	sys_platform_render .update_dependency_count = _countof(frame_render_update_deps);

	systems_add(&sys_platform);
	systems_add(&sys_platform_begin);
	systems_add(&sys_platform_render);

	// Rest of the systems
	system_t sys_defaults = { "Defaults" };
	const char *default_deps[] = { "Platform", "Assets" };
	sys_defaults.init_dependencies     = default_deps;
	sys_defaults.init_dependency_count = _countof(default_deps);
	sys_defaults.func_initialize       = defaults_init;
	sys_defaults.func_shutdown         = defaults_shutdown;
	systems_add(&sys_defaults);

	system_t sys_ui = { "UI" };
	const char *ui_deps       [] = {"Defaults"};
	const char *ui_update_deps[] = {"Input", "FrameBegin"};
	sys_ui.init_dependencies       = ui_deps;
	sys_ui.init_dependency_count   = _countof(ui_deps);
	sys_ui.update_dependencies     = ui_update_deps;
	sys_ui.update_dependency_count = _countof(ui_update_deps);
	sys_ui.func_initialize         = ui_init;
	sys_ui.func_update             = ui_update;
	sys_ui.func_shutdown           = ui_shutdown;
	systems_add(&sys_ui);

	system_t sys_ui_late = { "UILate" };
	const char *ui_late_update_deps[] = {"App"};
	sys_ui_late.update_dependencies     = ui_late_update_deps;
	sys_ui_late.update_dependency_count = _countof(ui_late_update_deps);
	sys_ui_late.func_update             = ui_update_late;
	systems_add(&sys_ui_late);

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

	system_t sys_assets = { "Assets" };
	const char* assets_deps       [] = {"Platform"};
	const char *assets_update_deps[] = {"FrameRender"};
	sys_assets.init_dependencies       = assets_deps;
	sys_assets.init_dependency_count   = _countof(assets_deps);
	sys_assets.update_dependencies     = assets_update_deps;
	sys_assets.update_dependency_count = _countof(assets_update_deps);
	sys_assets.func_initialize         = assets_init;
	sys_assets.func_update             = assets_update;
	sys_assets.func_shutdown           = assets_shutdown;
	systems_add(&sys_assets);

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
	const char *world_update_deps[] = {"Platform", "App"};
	sys_world.init_dependencies       = world_deps;
	sys_world.init_dependency_count   = _countof(world_deps);
	sys_world.update_dependencies     = world_update_deps;
	sys_world.update_dependency_count = _countof(world_update_deps);
	sys_world.func_initialize         = world_init;
	sys_world.func_update             = world_update;
	sys_world.func_shutdown           = world_shutdown;
	systems_add(&sys_world);

	system_t sys_anim = { "Animation" };
	const char *anim_update_deps[] = {"App"};
	sys_anim.update_dependencies     = anim_update_deps;
	sys_anim.update_dependency_count = _countof(anim_update_deps);
	sys_anim.func_update             = anim_update;
	sys_anim.func_shutdown           = anim_shutdown;
	systems_add(&sys_anim);

	system_t sys_app = { "App" };
	const char *app_update_deps[] = {"Input", "Defaults", "FrameBegin", "Platform", "Physics", "Renderer", "UI"};
	sys_app.update_dependencies     = app_update_deps;
	sys_app.update_dependency_count = _countof(app_update_deps);
	sys_app.func_update             = sk_app_update;
	systems_add(&sys_app);

	sk_initialized = systems_initialize();
	if (!sk_initialized) log_show_any_fail_reason();
	else                 log_clear_any_fail_reason();

	app_system    = systems_find("App");
	app_init_time = stm_now();
	sk_running    = sk_initialized;
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
	sk_running = false;
}

///////////////////////////////////////////

bool32_t sk_step(void (*app_update)(void)) {
	if (app_system->profile_start_duration == 0)
		app_system->profile_start_duration = stm_since(app_init_time);

	// TODO: remove this in v0.4 when sk_step is formally replaced by sk_run
	sk_assert_thread_valid();
	sk_first_step = true;
	
	sk_app_update_func = app_update;
	sk_update_timer();

	systems_update();

	if (sk_display_mode == display_mode_flatscreen && sk_focus != app_focus_active && !sk_settings.disable_unfocused_sleep)
		platform_sleep(100);
	return sk_running;
}

///////////////////////////////////////////

void sk_run(void (*app_update)(void), void (*app_shutdown)(void)) {
	sk_assert_thread_valid();
	sk_first_step = true;
	
#if defined(SK_OS_WEB)
	web_start_main_loop(app_update, app_shutdown);
#else
	while (sk_step(app_update));

	if (app_shutdown != nullptr)
		app_shutdown();
	sk_shutdown();
#endif
}

///////////////////////////////////////////

void (*_sk_run_data_app_update)(void *);
void  *_sk_run_data_update_data;
void (*_sk_run_data_app_shutdown)(void *);
void  *_sk_run_data_shutdown_data;
void sk_run_data(void (*app_update)(void *update_data), void *update_data, void (*app_shutdown)(void *shutdown_data), void *shutdown_data) {
	_sk_run_data_app_update    = app_update;
	_sk_run_data_update_data   = update_data;
	_sk_run_data_app_shutdown  = app_shutdown;
	_sk_run_data_shutdown_data = shutdown_data;

	sk_assert_thread_valid();
	sk_first_step = true;

#if defined(SK_OS_WEB)
	web_start_main_loop(
		[]() { if (_sk_run_data_app_update  ) _sk_run_data_app_update  (_sk_run_data_update_data  ); },
		[]() { if (_sk_run_data_app_shutdown) _sk_run_data_app_shutdown(_sk_run_data_shutdown_data); });
#else
	while (sk_step(
		[]() { if (_sk_run_data_app_update  ) _sk_run_data_app_update  (_sk_run_data_update_data  ); }));

	if (_sk_run_data_app_shutdown)
		_sk_run_data_app_shutdown(_sk_run_data_shutdown_data);

	sk_shutdown();
#endif
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

void sk_assert_thread_valid() {
	// sk_init and sk_run/step need to happen on the same thread, but there's a
	// non-zero chance that some async code can inadvertently put execution
	// onto another thread without the dev's knowledge. This can trip up async
	// asset code and cause a blocking loop as the asset waits for the main
	// thread to step. This function is used to detect and warn of such a
	// situation.
	if (sk_initialized && sk_first_step)
		return;
	
	if (thrd_id_equal(sk_init_thread, thrd_id_current()) == false) {
		const char* err = "SK.Run and pre-Run GPU asset creation currently must be called on the same thread as SK.Initialize! Has async code accidentally bumped you to another thread?";
		log_err(err);
		platform_msgbox_err(err, "Fatal Error");
		abort();
	}
}

///////////////////////////////////////////

display_mode_ sk_active_display_mode() { return sk_display_mode; }

///////////////////////////////////////////

double time_get_raw          (){ return stm_sec(stm_now()); }
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
	physics_sim_time     = sk_timev;
}

} // namespace sk
