#pragma once
#include "../stereokit.h"
#include "../libraries/sk_gpu.h"

namespace sk {

// Platform lifecycle:
//
//// Initialization
// platform_init()
// Mixed Reality:
//    platform_start_pre_xr()
//    openxr_init()
//    platform_start_post_xr()
// Flatscreen:
//    platform_start_flat()
//    simulator_init();
//
//// Main loop
// Mixed Reality:
//    platform_step_begin_xr()
// Flatscreen:
//    platform_step_begin_flat()
//    simulator_step_begin();
//    simulator_step_end();
//    platform_step_end_flat()
//
//// Shutdown
// Mixed Reality:
//    openxr_shutdown()
// Flatscreen:
//    simulator_shutdown();
//    platform_stop_flat()
// platform_shutdown()

bool platform_init          ();
void platform_shutdown      ();
void platform_set_window    (void *window);
void platform_set_window_xam(void *window);
bool platform_set_mode      (display_type_ mode);
void platform_step_begin    ();
void platform_step_end      ();
void platform_stop_mode     ();

///////////////////////////////////////////

enum platform_win_type_ {
	platform_win_type_none,
	platform_win_type_creatable,
	platform_win_type_existing,
};

enum platform_evt_ {
	platform_evt_none,
	platform_evt_app_focus,
	platform_evt_key_press,
	platform_evt_key_release,
	platform_evt_mouse_press,
	platform_evt_mouse_release,
	platform_evt_character,
	platform_evt_scroll,
	platform_evt_close,
	platform_evt_resize,
};

enum platform_surface_ {
	platform_surface_none,
	platform_surface_swapchain
};

union platform_evt_data_t {
	app_focus_ app_focus;
	key_       press_release;
	char32_t   character;
	float      scroll;
	struct { int32_t width, height; } resize;
};

typedef struct recti_t {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
} recti_t;

typedef int32_t platform_win_t;

///////////////////////////////////////////

platform_win_type_ platform_win_type         ();
platform_win_t     platform_win_get_existing (platform_surface_ surface_type);
platform_win_t     platform_win_make         (const char *title, recti_t win_rect, platform_surface_ surface_type);
skg_swapchain_t*   platform_win_get_swapchain(platform_win_t window);
void               platform_win_destroy      (platform_win_t window);
bool               platform_win_next_event   (platform_win_t window, platform_evt_* out_event, platform_evt_data_t* out_event_data);
recti_t            platform_win_rect         (platform_win_t window);

bool platform_key_save_bytes(const char* key, void* data,       int32_t data_size);
bool platform_key_load_bytes(const char* key, void* ref_buffer, int32_t buffer_size);

} // namespace sk