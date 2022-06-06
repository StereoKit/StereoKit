#if defined(_WIN32) && !defined(WINDOWS_UWP)
#define _CRT_SECURE_NO_WARNINGS

#include "demo_windows.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <d3d11.h>

#include "../../StereoKitC/stereokit.h"
#include "../../StereoKitC/stereokit_ui.h"
using namespace sk;

#include <vector>
using namespace std;

///////////////////////////////////////////

typedef BOOL(WINAPI* fn_GetDxSharedSurface)( HANDLE hHandle, HANDLE* phSurface, LUID* pAdapterLuid, ULONG* pFmtWindow, ULONG* pPresentFlags, ULONGLONG* pWin32kUpdateId );
fn_GetDxSharedSurface DwmGetDxSharedSurface;

///////////////////////////////////////////

mesh_t        mirror_mesh;
ID3D11Device *mirror_device;

struct window_t {
	HWND       window;
	tex_t      texture;
	material_t material;
	pose_t     pose;
	char       name[128];
};
vector<window_t> mirror_windows;

///////////////////////////////////////////

void demo_windows_init() {
	mirror_mesh = mesh_find (default_id_mesh_quad);

	DwmGetDxSharedSurface = (fn_GetDxSharedSurface)GetProcAddress(LoadLibrary("user32.dll"), "DwmGetDxSharedSurface");

	ID3D11DeviceContext *context;
	render_get_device((void**)&mirror_device, (void**)&context);
	
	EnumWindows([](HWND hwnd, LPARAM) {
		if (hwnd == nullptr)          return TRUE;
		if (hwnd == GetShellWindow()) return TRUE;
		if (!IsWindowVisible(hwnd))   return TRUE;
		if (GetAncestor(hwnd, GA_ROOT) != hwnd) return TRUE;

		LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (style & WS_DISABLED) return TRUE;

		//int hrTemp = DwmGetWindowAttribute(hwnd, DWMWINDOWATTRIBUTE.Cloaked, out bool cloaked, Marshal.SizeOf<bool>());
		//if (hrTemp == 0 && cloaked) return false;

		char text[256];
		GetWindowText(hwnd, text, sizeof(text));
		if (strcmp(text, "") == 0) return TRUE;

		HANDLE    surface   = nullptr;
		LUID      luid      = { 0, };
		ULONG     format    = 0;
		ULONG     flags     = 0;
		ULONGLONG update_id = 0;
		if (!DwmGetDxSharedSurface(hwnd, &surface, &luid, &format, &flags, &update_id)) {
			log_warn("Failed to get surface!");
			return TRUE;
		}

		ID3D11Texture2D *shared_tex = nullptr;
		if (FAILED(mirror_device->OpenSharedResource(surface, __uuidof(ID3D11Texture2D), (void**)(&shared_tex)))) {
			log_warn("Failed to get shared surface!");
			return TRUE;
		}

		window_t win = {};
		win.window   = hwnd;
		win.material = material_copy_id(default_id_material_unlit);
		win.texture  = tex_create();
		win.pose     = pose_t{ vec3{0,0,-0.5f} + vec3{0.04f, 0.04f, -0.04f} *mirror_windows.size(), quat_lookat(vec3_zero, {0,0,1}) };
		strncpy(win.name, text, sizeof(win.name));
		tex_set_surface(win.texture, shared_tex, tex_type_image_nomips, format, 0, 0, 1);
		tex_set_address(win.texture, tex_address_clamp);
		material_set_texture(win.material, "diffuse", win.texture);
		mirror_windows.push_back(win);

		return TRUE;
	}, 0);
}

///////////////////////////////////////////

void demo_windows_update() {
	static pose_t window_pose =
		pose_t{ {0.25,0.25,-0.25f}, quat_lookat({0.25,0.25,-0.25f}, {0,0.25,0}) };

	ui_window_begin("Main", window_pose);
	
	ui_window_end();

	for (size_t i = 0; i < mirror_windows.size(); i++) {
		ui_window_begin(mirror_windows[i].name, mirror_windows[i].pose, vec2_zero, ui_win_head);
		
		vec2 size = vec2 {
			(float)tex_get_width (mirror_windows[i].texture), 
			(float)tex_get_height(mirror_windows[i].texture) } *0.0004f;
		ui_layout_reserve(size);
		
		render_add_mesh(mirror_mesh, mirror_windows[i].material, matrix_trs(vec3{0, -size.y/2, 0}, quat_identity, vec3{ size.x, size.y, 1 }));

		ui_window_end();
	}
}

///////////////////////////////////////////

void demo_windows_shutdown() {
	mesh_release    (mirror_mesh);
}
#else
void demo_windows_init() {}
void demo_windows_update() {}
void demo_windows_shutdown() {}
#endif