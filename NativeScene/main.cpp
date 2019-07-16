#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"D3dcompiler.lib") // for shader compile
#include "d3d.h"

#include "mesh.h"
#include "shader.h"
#include "texture.h"

#include <windows.h>

#include <directxmath.h> // Matrix math functions and objects

#include <vector>

using namespace std;
using namespace DirectX; // Matrix math

///////////////////////////////////////////

struct app_transform_buffer_t {
	XMFLOAT4X4 world;
	XMFLOAT4X4 viewproj;
};

shader_t     app_shader;
shaderargs_t app_shader_transforms;
mesh_t       app_cube;
tex2d_t      app_tex;

HWND app_hwnd = nullptr;
bool app_run  = true;

void app_init    ();
void app_draw    ();
void app_update  ();
void app_shutdown();

///////////////////////////////////////////

vert_t app_verts[] = {
	{ { -1, 0, -1 }, { 0, 1, 0 }, { 0, 0 }, { 255,255,255,255 } }, // Bottom verts
	{ {  1, 0, -1 }, { 0, 1, 0 }, { 1, 0 }, { 255,255,255,255 } },
	{ {  1, 0,  1 }, { 0, 1, 0 }, { 1, 1 }, { 255,255,255,255 } },
	{ { -1, 0,  1 }, { 0, 1, 0 }, { 0, 1 }, { 255,255,255,255 } },};
uint16_t app_inds[] = { 2,1,0, 3,2,0, };

///////////////////////////////////////////
// Main                                  //
///////////////////////////////////////////

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
	MSG      msg     = {0};
	WNDCLASS wc      = {0}; 
	wc.lpfnWndProc = [](HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch(message) {
		case WM_CLOSE: app_run = false; PostQuitMessage(0); break;
		case WM_SIZE: if (wParam != SIZE_MINIMIZED) d3d_resize_screen((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		return (LRESULT)0;
	};
	wc.hInstance     = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
	wc.lpszClassName = "Min DX";
	if( !RegisterClass(&wc) ) return 1;
	app_hwnd = CreateWindow(wc.lpszClassName, "Min DX", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 640, 480, 0, 0, wc.hInstance, nullptr);
	if( !app_hwnd ) return 2;

	d3d_init(app_hwnd);
	app_init();

	while (app_run) {
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage (&msg);
			continue;
		}
		d3d_frame_begin();

		app_update();
		d3d_render_begin();
		app_draw();
		d3d_render_end();

		d3d_frame_end();
	}

	app_shutdown();
	d3d_shutdown();
	return 0;
}

///////////////////////////////////////////
// App                                   //
///////////////////////////////////////////

void app_init() {
	mesh_create_file  (app_cube,   "cube.obj");
	tex2d_create_file (app_tex,    "test.png");
	shader_create_file(app_shader, "shader.hlsl");
	shaderargs_create(app_shader_transforms, sizeof(app_transform_buffer_t), 0);
}

///////////////////////////////////////////

void app_shutdown() {
	tex2d_destroy(app_tex);
	mesh_destroy (app_cube);

	shader_destroy    (app_shader);
	shaderargs_destroy(app_shader_transforms);
}

///////////////////////////////////////////

void app_draw() {
	// Set up camera matrices based on OpenXR's predicted viewpoint information
	XMMATRIX mat_projection = XMMatrixPerspectiveFovLH(1, (float)d3d_screen_width/d3d_screen_height, 0.1f, 50);
	XMMATRIX mat_view       = XMMatrixLookAtLH(XMVectorSet(cosf(d3d_timef)*4, 4, sinf(d3d_timef)*4, 0), DirectX::g_XMZero, XMVectorSet(0, 1, 0, 0));

	app_transform_buffer_t transform_buffer;
	XMStoreFloat4x4(&transform_buffer.viewproj, XMMatrixTranspose(mat_view * mat_projection));
	XMMATRIX mat_model = XMMatrixAffineTransformation(
		DirectX::g_XMOne*2, DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4*)&DirectX::g_XMIdentityR3),
		XMLoadFloat3((XMFLOAT3*)&DirectX::g_XMZero));
	XMStoreFloat4x4(&transform_buffer.world, mat_model);

	shader_set_active(app_shader);
	shaderargs_set_data(app_shader_transforms, &transform_buffer);
	shaderargs_set_active(app_shader_transforms);
	tex2d_set_active(app_tex, 0);

	mesh_set_active(app_cube);
	mesh_draw(app_cube);
}

///////////////////////////////////////////

void app_update() {
}
