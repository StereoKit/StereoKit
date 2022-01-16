#include "win32.h"

#if defined(SK_OS_WINDOWS)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../../stereokit.h"
#include "../../_stereokit.h"
#include "../../asset_types/texture.h"
#include "../../libraries/sokol_time.h"
#include "../defaults.h"
#include "../system.h"
#include "../render.h"
#include "../input.h"
#include "../input_keyboard.h"
#include "../hand/input_hand.h"
#include "flatscreen_input.h"

namespace sk {

///////////////////////////////////////////

HWND            win32_window              = nullptr;
skg_swapchain_t win32_swapchain           = {};
tex_t           win32_target              = {};
float           win32_scroll              = 0;
LONG_PTR        win32_openxr_base_winproc = 0;
system_t       *win32_render_sys          = nullptr;

// For managing window resizing
bool win32_check_resize = true;
UINT win32_resize_x     = 0;
UINT win32_resize_y     = 0;

#if defined(SKG_OPENGL)
const int32_t win32_multisample = 1;
#else
const int32_t win32_multisample = 8;
#endif
const int32_t win32_supersample = 2;

///////////////////////////////////////////

void win32_resize(int width, int height) {
	if (width == sk_info.display_width && height == sk_info.display_height)
		return;
	sk_info.display_width  = width;
	sk_info.display_height = height;
	log_diagf("Resized to: %d<~BLK>x<~clr>%d", width, height);
	
	skg_swapchain_resize(&win32_swapchain, sk_info.display_width,                   sk_info.display_height);
	tex_set_color_arr   (win32_target,     sk_info.display_width*win32_supersample, sk_info.display_height*win32_supersample, nullptr, 1, nullptr, 1);
	render_update_projection();
}

///////////////////////////////////////////

void win32_physical_key_interact() {
	// On desktop, we want to hide soft keyboards on physical presses
	input_last_physical_keypress = time_getf();
	platform_keyboard_show(false, text_context_text);
}

///////////////////////////////////////////

bool win32_window_message_common(UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
	case WM_LBUTTONDOWN: if (sk_focused) input_keyboard_inject_press  (key_mouse_left);   return true;
	case WM_LBUTTONUP:   if (sk_focused) input_keyboard_inject_release(key_mouse_left);   return true;
	case WM_RBUTTONDOWN: if (sk_focused) input_keyboard_inject_press  (key_mouse_right);  return true;
	case WM_RBUTTONUP:   if (sk_focused) input_keyboard_inject_release(key_mouse_right);  return true;
	case WM_MBUTTONDOWN: if (sk_focused) input_keyboard_inject_press  (key_mouse_center); return true;
	case WM_MBUTTONUP:   if (sk_focused) input_keyboard_inject_release(key_mouse_center); return true;
	case WM_XBUTTONDOWN: input_keyboard_inject_press  (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? key_mouse_back : key_mouse_forward); return true;
	case WM_XBUTTONUP:   input_keyboard_inject_release(GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? key_mouse_back : key_mouse_forward); return true;
	case WM_KEYDOWN:     input_keyboard_inject_press  ((key_)wParam); win32_physical_key_interact(); return true;
	case WM_KEYUP:       input_keyboard_inject_release((key_)wParam); win32_physical_key_interact(); return true;
	case WM_SYSKEYDOWN:  input_keyboard_inject_press  ((key_)wParam); win32_physical_key_interact(); return true;
	case WM_SYSKEYUP:    input_keyboard_inject_release((key_)wParam); win32_physical_key_interact(); return true;
	case WM_CHAR:        input_text_inject_char   ((uint32_t)wParam); return true;
	case WM_MOUSEWHEEL:  if (sk_focused) win32_scroll += (short)HIWORD(wParam); return true;
	default: return false;
	}
}

///////////////////////////////////////////

LRESULT win32_openxr_winproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (!win32_window_message_common(message, wParam, lParam)) {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

///////////////////////////////////////////

bool win32_start_pre_xr() {
	return true;
}

///////////////////////////////////////////

bool win32_start_post_xr() {
	wchar_t app_name_w[256];
	mbstowcs(app_name_w, sk_app_name, _countof(app_name_w));

	// Create a window just to grab input
	WNDCLASSW wc = {0}; 
	wc.lpfnWndProc   = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (!win32_window_message_common(message, wParam, lParam)) {
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = app_name_w;
	if( !RegisterClassW(&wc) ) return false;

	win32_window = CreateWindowW(
		app_name_w, 
		app_name_w, 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		0, 0, 0, 0,
		0, 0, 
		wc.hInstance, 
		nullptr);

	if (!win32_window) {
		return false;
	}
	return true;
}

///////////////////////////////////////////

bool win32_init() {
	win32_render_sys = systems_find("FrameRender");
	return true;
}

///////////////////////////////////////////

void win32_shutdown() {
}

///////////////////////////////////////////

bool win32_start_flat() {
	sk_info.display_width  = sk_settings.flatscreen_width;
	sk_info.display_height = sk_settings.flatscreen_height;
	sk_info.display_type   = display_opaque;

	wchar_t app_name_w[256];
	mbstowcs(app_name_w, sk_app_name, _countof(app_name_w));

	WNDCLASSW wc = {0}; 
	wc.lpfnWndProc   = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		if (!win32_window_message_common(message, wParam, lParam)) {
			switch(message) {
			case WM_CLOSE:      sk_running = false; PostQuitMessage(0); break;
			case WM_SETFOCUS:   sk_focused = true;  break;
			case WM_KILLFOCUS:  sk_focused = false; break;
			case WM_MOUSEWHEEL: if (sk_focused) win32_scroll += (short)HIWORD(wParam); break;
			case WM_SYSCOMMAND: {
				// Has the user pressed the restore/'un-maximize' button?
				// WM_SIZE happens -after- this event, and contains the new size.
				if (GET_SC_WPARAM(wParam) == SC_RESTORE)
					win32_check_resize = true;
			
				// Disable alt menu
				if (GET_SC_WPARAM(wParam) == SC_KEYMENU) 
					return (LRESULT)0; 
			} return DefWindowProcW(hWnd, message, wParam, lParam); 
			case WM_SIZE: {
				win32_resize_x = (UINT)LOWORD(lParam);
				win32_resize_y = (UINT)HIWORD(lParam);

				// Don't check every time the size changes, this can lead to ugly memory alloc.
				// If a restore event, a maximize, or something else says we should resize, check it!
				if (win32_check_resize || wParam == SIZE_MAXIMIZED) { 
					win32_check_resize = false; 
					win32_resize(win32_resize_x, win32_resize_y); 
				}
			} return DefWindowProcW(hWnd, message, wParam, lParam);
			case WM_EXITSIZEMOVE: {
				// If the user was dragging the window around, WM_SIZE is called -before- this 
				// event, so we can go ahead and resize now!
				win32_resize(win32_resize_x, win32_resize_y);
			} return DefWindowProcW(hWnd, message, wParam, lParam);
			default: return DefWindowProcW(hWnd, message, wParam, lParam);
			}
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandleW(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = app_name_w;
	if( !RegisterClassW(&wc) ) return false;

	RECT r;
	r.left   = sk_settings.flatscreen_pos_x;
	r.right  = sk_settings.flatscreen_pos_x + sk_info.display_width;
	r.top    = sk_settings.flatscreen_pos_y;
	r.bottom = sk_settings.flatscreen_pos_y + sk_info.display_height;
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW | WS_VISIBLE, false);
	win32_window = CreateWindowW(
		app_name_w, 
		app_name_w, 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
		max(0,r.left), 
		max(0,r.top), 
		r.right  - r.left, 
		r.bottom - r.top, 
		0, 0, 
		wc.hInstance, 
		nullptr);
	if( !win32_window ) return false;

	RECT bounds;
	GetClientRect(win32_window, &bounds);
	int32_t width  = bounds.right  - bounds.left;
	int32_t height = bounds.bottom - bounds.top;

	skg_tex_fmt_ color_fmt = skg_tex_fmt_rgba32_linear;
	skg_tex_fmt_ depth_fmt = render_preferred_depth_fmt();
#if defined(SKG_OPENGL)
	depth_fmt = skg_tex_fmt_depthstencil;
#endif
	win32_swapchain = skg_swapchain_create(win32_window, color_fmt, skg_tex_fmt_none, width, height);
	sk_info.display_width  = win32_swapchain.width;
	sk_info.display_height = win32_swapchain.height;
	win32_target = tex_create(tex_type_rendertarget, tex_format_rgba32);
	tex_set_color_arr(win32_target, sk_info.display_width*win32_supersample, sk_info.display_height*win32_supersample, nullptr, 1, nullptr, 1);
	tex_add_zbuffer  (win32_target, (tex_format_)depth_fmt);

	log_diagf("Created swapchain: %dx%d color:%s depth:%s", win32_swapchain.width, win32_swapchain.height, render_fmt_name((tex_format_)color_fmt), render_fmt_name((tex_format_)depth_fmt));

	flatscreen_input_init();

	//////////////////

	/*render_res_create_swapchain("Render", &win32_swapchain);

	rendergraph_add_module("Shadow", [](render_module_t *self) {
		render_res_create    ("Shadow", render_resolution_absolute, nullptr, 512, 512, tex_format_r32);
		rendermodule_uses_res(self, "Shadow", render_rw_write);

		render_module_t *mod = rendermodule_get("Render");
		rendermodule_dependency(mod, "Shadow");
	},
	[](render_module_t *self, render_list_t render_list) {
		render_global_texture(12, nullptr);
		render_res_t *target = render_res_get_write("Shadow");
		render_res_bind(target);

		static material_t shadow_material = nullptr;
		if (shadow_material == nullptr) {
			shadow_material = material_copy_id(default_id_material);
		}

		vec3   center = input_head()->position;
		vec3   from   = center + vec3{ 50, 50, 0 };
		matrix view   = matrix_invert(matrix_trs(from, quat_lookat(from, center)));
		matrix proj   = matrix_orthographic(20, 20, 0.01, 100);
		render_list_draw_material(render_list, &view, &proj, 1, render_get_filter(), shadow_material);

		render_res_bind(nullptr);
		render_global_texture(12, render_res_get_read("Shadow"));
	});

	//////////////////

	rendergraph_add_module("Render",
	[](render_module_t *self) {
		rendermodule_uses_res(self, "Render", render_rw_write);
	},
	[](render_module_t *self, render_list_t render_list) {
		render_res_t *target = render_res_get_write("Render");
		render_res_bind(target);

		matrix view = matrix_invert(render_get_cam_final());
		matrix proj = render_get_projection();
		render_list_draw(render_list, &view, &proj, 1, render_get_filter());
	});

	//////////////////

	rendergraph_add_module("Downsample", [](render_module_t *self) {
		rendermodule_uses_res  (self, "Render", render_rw_read_write);
		rendermodule_dependency(self, "Render");
	},
	[](render_module_t *self) {
		render_res_t *target = render_res_get_write("Render");
		render_res_bind(target);

		static material_t downsample = nullptr;
		if (downsample == nullptr) {
			downsample = material_create(shader_create_mem((void*)sks_shader_builtin_fxaa_hlsl, sizeof(sks_shader_builtin_fxaa_hlsl)));
		}
		
		tex_t source = render_res_get_read("Render");
		material_set_texture(downsample, "source", source);
		render_blit_to_bound(downsample);
	});*/
	
	//////////////////

	return true;
}

///////////////////////////////////////////

void win32_stop_flat() {
	flatscreen_input_shutdown();
	tex_release          (win32_target);
	skg_swapchain_destroy(&win32_swapchain);
}

///////////////////////////////////////////

void win32_step_begin_xr() {
	MSG msg = {0};
	while (PeekMessage(&msg, win32_window, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
}

void win32_step_begin_flat() {
	MSG msg = {0};
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage (&msg);
	}
	flatscreen_input_update();
}

///////////////////////////////////////////

void win32_step_end_flat() {
	input_update_poses(true);

	// Set up our primary render list for drawing
	render_list_t          list     = render_get_primary_list();
	matrix                 view     = render_get_cam_final ();
	matrix                 proj     = render_get_projection();
	render_pass_settings_t settings = {};
	settings.clear_color_linear = render_get_clear_color();
	settings.layer_filter       = render_get_filter();
	render_list_draw_to(list, win32_target, &view, &proj, 1, settings);
	
	// Draw all outstanding items
	skg_draw_begin();
	render_all();

	render_list_clear  (list);
	render_list_release(list);

	// Downsample the supersampled surface
	skg_swapchain_bind(&win32_swapchain);
	material_set_texture(sk_default_material_blit_linear, "source", win32_target);
	render_blit_to_bound(sk_default_material_blit_linear); 

	// win32_swapchain needs removed before presentation
	skg_tex_target_bind(nullptr);
	// win32_target needs cleared from the list of bound textures
	for (uint16_t i = 0; i < 16; i++) {
		skg_tex_clear({ i, skg_stage_pixel | skg_stage_vertex, skg_register_resource });
	}

	win32_render_sys->profile_frame_duration = stm_since(win32_render_sys->profile_frame_start);
	skg_swapchain_present(&win32_swapchain);
}

///////////////////////////////////////////

void *win32_hwnd() {
	return win32_window;
}

} // namespace sk

#endif // defined(SK_OS_WINDOWS)