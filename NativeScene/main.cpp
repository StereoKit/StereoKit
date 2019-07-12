#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"D3dcompiler.lib") // for shader compile
#include "d3d.h"

#include "mesh.h"
#include "shader.h"

#include <windows.h>

#include <directxmath.h> // Matrix math functions and objects
#include <d3dcompiler.h> // For compiling shaders! D3DCompile

#include <thread> // sleep_for
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

HWND app_hwnd = nullptr;
bool app_run  = true;

void app_init    ();
void app_draw    ();
void app_update  ();
void app_shutdown();

///////////////////////////////////////////

constexpr char app_shader_code[] = R"_(
cbuffer TransformBuffer : register(b0) {
	float4x4 world;
	float4x4 viewproj;
};
struct vsIn {
	float4 pos  : SV_POSITION;
	float3 norm : NORMAL;
};
struct psIn {
	float4 pos   : SV_POSITION;
	float3 color : COLOR0;
};

psIn vs(vsIn input) {
	psIn output;
	output.pos = input.pos;
	output.pos = mul(float4(input.pos.xyz, 1), world);
	output.pos = mul(output.pos, viewproj);

	float3 normal = normalize(mul(float4(input.norm, 0), world).xyz);

	output.color = lerp(float3(0,0,0.1), float3(1,1,1), saturate(dot(normal, float3(0,1,0))));
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	return float4(input.color, 1);
})_";

vert_t app_verts[] = {
	{ { -1,-1,-1 }, { -1,-1,-1 }, { 0, 0 }, { 255,255,255,255 } }, // Bottom verts
	{ {  1,-1,-1 }, {  1,-1,-1 }, { 0, 0 }, { 255,255,255,255 } },
	{ {  1, 1,-1 }, {  1, 1,-1 }, { 0, 0 }, { 255,255,255,255 } },
	{ { -1, 1,-1 }, { -1, 1,-1 }, { 0, 0 }, { 255,255,255,255 } },
	{ { -1,-1, 1 }, { -1,-1, 1 }, { 0, 0 }, { 255,255,255,255 } }, // Top verts
	{ {  1,-1, 1 }, {  1,-1, 1 }, { 0, 0 }, { 255,255,255,255 } },
	{ {  1, 1, 1 }, {  1, 1, 1 }, { 0, 0 }, { 255,255,255,255 } },
	{ { -1, 1, 1 }, { -1, 1, 1 }, { 0, 0 }, { 255,255,255,255 } }, };

uint16_t app_inds[] = {
	1,2,0, 2,3,0, 4,6,5, 7,6,4,
	6,2,1, 5,6,1, 3,7,4, 0,3,4,
	4,5,1, 0,4,1, 2,7,3, 2,6,7, };

///////////////////////////////////////////
// Main                                  //
///////////////////////////////////////////


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message) {
	case WM_CLOSE: app_run = false; PostQuitMessage(0); break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow) {
	MSG      msg     = {0};
	WNDCLASS wc      = {0}; 
	wc.lpfnWndProc   = WndProc;
	wc.hInstance     = GetModuleHandle(NULL);//hInst;
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

		app_update();
		d3d_render_begin();
		app_draw();
		d3d_render_end();
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	app_shutdown();
	d3d_shutdown();
	return 0;
}

///////////////////////////////////////////
// App                                   //
///////////////////////////////////////////

void app_init() {
	mesh_create   (app_cube);
	mesh_set_verts(app_cube, app_verts, _countof(app_verts));
	mesh_set_inds (app_cube, app_inds,       _countof(app_inds));

	shader_create    (app_shader, app_shader_code);
	shaderargs_create(app_shader_transforms, sizeof(app_transform_buffer_t), 0);
}

///////////////////////////////////////////

void app_shutdown() {
	mesh_destroy(app_cube);
	shader_destroy(app_shader);
	shaderargs_destroy(app_shader_transforms);
}

///////////////////////////////////////////

float app_time = 0;
void app_draw() {
	app_time += 0.016f;

	// Set up camera matrices based on OpenXR's predicted viewpoint information
	XMMATRIX mat_projection = XMMatrixPerspectiveFovLH(1, 640.f/480.f, 0.1f, 50);
	XMMATRIX mat_view       = XMMatrixLookAtLH(XMVectorSet(cosf(app_time)*4, 4, sinf(app_time)*4, 0), DirectX::g_XMZero, XMVectorSet(0, 1, 0, 0));

	shader_set_active(app_shader);
	mesh_set_active(app_cube);

	app_transform_buffer_t transform_buffer;
	XMStoreFloat4x4(&transform_buffer.viewproj, XMMatrixTranspose(mat_view * mat_projection));
	XMMATRIX mat_model = XMMatrixAffineTransformation(
		DirectX::g_XMOne * 0.5f, DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4*)&DirectX::g_XMIdentityR3),
		XMLoadFloat3((XMFLOAT3*)&DirectX::g_XMZero));
	XMStoreFloat4x4(&transform_buffer.world, mat_model);

	shaderargs_set_data(app_shader_transforms, &transform_buffer);
	shaderargs_set_active(app_shader_transforms);
	
	mesh_draw(app_cube);
}

///////////////////////////////////////////

void app_update() {
}
