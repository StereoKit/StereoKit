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

ID3D11VertexShader *app_vshader;
ID3D11PixelShader  *app_pshader;
ID3D11InputLayout  *app_shader_layout;
ID3D11Buffer       *app_constant_buffer;
ID3D11Buffer       *app_vertex_buffer;
ID3D11Buffer       *app_index_buffer;

shader_t     app_shader;
shaderargs_t app_shader_transforms;
mesh_t       app_cube;

HWND app_hwnd = nullptr;
bool app_run = true;

void app_init  ();
void app_draw  ();
void app_update();
void app_update_predicted();

///////////////////////////////////////////

void                 d3d_init             ();
void                 d3d_shutdown         ();
void                 d3d_render_layer     ();
ID3DBlob            *d3d_compile_shader   (const char* hlsl, const char* entrypoint, const char* target);

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
	output.pos = mul(float4(input.pos.xyz, 1), world);
	output.pos = mul(output.pos, viewproj);

	float3 normal = normalize(mul(float4(input.norm, 0), world).xyz);

	output.color = saturate(dot(normal, float3(0,1,0))).xxx;
	return output;
}
float4 ps(psIn input) : SV_TARGET {
	return float4(input.color, 1);
})_";


vert_t app_verts_type[] = {
	{ { -1,-1,-1 }, { -1,-1,-1 }, { 0, 0 }, { 255,255,255,255 } }, // Bottom verts
	{ {  1,-1,-1 }, {  1,-1,-1 }, { 0, 0 }, { 255,255,255,255 } },
	{ {  1, 1,-1 }, {  1, 1,-1 }, { 0, 0 }, { 255,255,255,255 } },
	{ { -1, 1,-1 }, { -1, 1,-1 }, { 0, 0 }, { 255,255,255,255 } },
	{ { -1,-1, 1 }, { -1,-1, 1 }, { 0, 0 }, { 255,255,255,255 } }, // Top verts
	{ {  1,-1, 1 }, {  1,-1, 1 }, { 0, 0 }, { 255,255,255,255 } },
	{ {  1, 1, 1 }, {  1, 1, 1 }, { 0, 0 }, { 255,255,255,255 } },
	{ { -1, 1, 1 }, { -1, 1, 1 }, { 0, 0 }, { 255,255,255,255 } }, };
float app_verts[] = {
	-1,-1,-1, -1,-1,-1, // Bottom verts
	 1,-1,-1,  1,-1,-1,
	 1, 1,-1,  1, 1,-1,
	-1, 1,-1, -1, 1,-1,
	-1,-1, 1, -1,-1, 1, // Top verts
	 1,-1, 1,  1,-1, 1,
	 1, 1, 1,  1, 1, 1,
	-1, 1, 1, -1, 1, 1, };

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

	d3d_init();
	app_init();

	while (app_run) {
		if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage (&msg);
			continue;
		}

		//app_update();
		d3d_render_layer();
		this_thread::sleep_for(chrono::milliseconds(1));
	}

	d3d_shutdown();
	return 0;
}


///////////////////////////////////////////
// DirectX code                          //
///////////////////////////////////////////

void d3d_init() {
	DXGI_SWAP_CHAIN_DESC sd = { 0 };
	sd.BufferCount       = 1;
	sd.BufferDesc.Width  = 640;
	sd.BufferDesc.Height = 480;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator   = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow     = app_hwnd;
	sd.SampleDesc.Count = 1;
	sd.Windowed         = true;

	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creation_flags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &sd, &d3d_swapchain, &d3d_device, nullptr, &d3d_context)))
		printf("\tFailed to init d3d!\n");

	ID3D11Texture2D *backbuffer;
	d3d_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID *)& backbuffer);
	d3d_device   ->CreateRenderTargetView( backbuffer, NULL, &d3d_rendertarget );
	backbuffer->Release();

}

///////////////////////////////////////////

void d3d_shutdown() {
	if (d3d_rendertarget) { d3d_rendertarget->Release(); d3d_rendertarget = nullptr; }
	if (d3d_context) { d3d_context->Release(); d3d_context = nullptr; }
	if (d3d_device ) { d3d_device->Release();  d3d_device  = nullptr; }
}

///////////////////////////////////////////

void d3d_render_layer() {
	// Set up where on the render target we want to draw, the view has a 
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, 640.f, 480.f);
	d3d_context->RSSetViewports(1, &viewport);

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	float clear[] = { 0, 0, 0, 1 };
	d3d_context->ClearRenderTargetView(d3d_rendertarget, clear);
	//d3d_context->ClearDepthStencilView(surface.depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	d3d_context->OMSetRenderTargets(1, &d3d_rendertarget, nullptr);

	// And now that we're set up, pass on the rest of our rendering to the application
	app_draw();

	d3d_swapchain->Present( 1, 0 );
}


///////////////////////////////////////////

ID3DBlob *d3d_compile_shader(const char* hlsl, const char* entrypoint, const char* target) {
	DWORD flags = D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_WARNINGS_ARE_ERRORS;
#ifdef _DEBUG
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG;
#else
	flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob *compiled, *errors;
	if (FAILED(D3DCompile(hlsl, strlen(hlsl), nullptr, nullptr, nullptr, entrypoint, target, flags, 0, &compiled, &errors)))
		printf("Error: D3DCompile failed %s", (char*)errors->GetBufferPointer());
	if (errors) errors->Release();

	return compiled;
}

///////////////////////////////////////////
// App                                   //
///////////////////////////////////////////

void app_init() {
	mesh_create   (app_cube);
	mesh_set_verts(app_cube, app_verts_type, _countof(app_verts_type));
	mesh_set_inds (app_cube, app_inds,       _countof(app_inds));

	shader_create    (app_shader, app_shader_code);
	shaderargs_create(app_shader_transforms, sizeof(app_transform_buffer_t), 0);

	// Compile our shader code, and turn it into a shader resource!
	ID3DBlob *vert_shader_blob  = d3d_compile_shader(app_shader_code, "vs", "vs_5_0");
	ID3DBlob *pixel_shader_blob = d3d_compile_shader(app_shader_code, "ps", "ps_5_0");
	d3d_device->CreateVertexShader(vert_shader_blob->GetBufferPointer(), vert_shader_blob ->GetBufferSize(), nullptr, &app_vshader);
	d3d_device->CreatePixelShader(pixel_shader_blob->GetBufferPointer(), pixel_shader_blob->GetBufferSize(), nullptr, &app_pshader);

	// Describe how our mesh is laid out in memory
	D3D11_INPUT_ELEMENT_DESC vert_desc[] = {
		{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}, };
	d3d_device->CreateInputLayout(vert_desc, (UINT)_countof(vert_desc), vert_shader_blob->GetBufferPointer(), vert_shader_blob->GetBufferSize(), &app_shader_layout);

	// Create GPU resources for our mesh's vertices and indices! Constant buffers are for passing transform
	// matrices into the shaders, so make a buffer for them too!
	D3D11_SUBRESOURCE_DATA vert_buff_data = { app_verts };
	D3D11_SUBRESOURCE_DATA ind_buff_data  = { app_inds };
	CD3D11_BUFFER_DESC     vert_buff_desc (sizeof(app_verts),              D3D11_BIND_VERTEX_BUFFER);
	CD3D11_BUFFER_DESC     ind_buff_desc  (sizeof(app_inds),               D3D11_BIND_INDEX_BUFFER);
	CD3D11_BUFFER_DESC     const_buff_desc(sizeof(app_transform_buffer_t), D3D11_BIND_CONSTANT_BUFFER);
	d3d_device->CreateBuffer(&vert_buff_desc, &vert_buff_data, &app_vertex_buffer);
	d3d_device->CreateBuffer(&ind_buff_desc,  &ind_buff_data,  &app_index_buffer);
	d3d_device->CreateBuffer(&const_buff_desc, nullptr,        &app_constant_buffer);
}

///////////////////////////////////////////

void app_draw() {
	// Set up camera matrices based on OpenXR's predicted viewpoint information
	XMMATRIX mat_projection = XMMatrixPerspectiveFovLH(1, 640.f/480.f, 0.1, 50);
	XMMATRIX mat_view       = XMMatrixInverse(nullptr, XMMatrixAffineTransformation(
		DirectX::g_XMOne, DirectX::g_XMZero,
		XMLoadFloat4((XMFLOAT4*)&DirectX::g_XMIdentityR3),
		XMLoadFloat3((XMFLOAT3*)&DirectX::g_XMOne)));

	// Set the active shaders and constant buffers.
	d3d_context->VSSetConstantBuffers(0, 1, &app_constant_buffer);
	d3d_context->VSSetShader(app_vshader, nullptr, 0);
	d3d_context->PSSetShader(app_pshader, nullptr, 0);

	// Set up the cube mesh's information
	UINT strides[] = { sizeof(float) * 6 };
	UINT offsets[] = { 0 };
	d3d_context->IASetVertexBuffers    (0, 1, &app_vertex_buffer, strides, offsets);
	d3d_context->IASetIndexBuffer      (app_index_buffer, DXGI_FORMAT_R16_UINT, 0);
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3d_context->IASetInputLayout      (app_shader_layout);

	// Put camera matrices into the shader's constant buffer
	app_transform_buffer_t transform_buffer;
	XMStoreFloat4x4(&transform_buffer.viewproj, XMMatrixTranspose(mat_view * mat_projection));

	// Draw all the cubes we have in our list!
	/*for (size_t i = 0; i < app_cubes.size(); i++) {
		// Create a translate, rotate, scale matrix for the cube's world location
		XMMATRIX mat_model = XMMatrixAffineTransformation(
			DirectX::g_XMOne * 0.05f, DirectX::g_XMZero,
			XMLoadFloat4((XMFLOAT4*)&app_cubes[i].orientation),
			XMLoadFloat3((XMFLOAT3*)&app_cubes[i].position));

		// Update the shader's constant buffer with the transform matrix info, and then draw the mesh!
		XMStoreFloat4x4(&transform_buffer.world, XMMatrixTranspose(mat_model));
		d3d_context->UpdateSubresource(app_constant_buffer, 0, nullptr, &transform_buffer, 0, 0);
		d3d_context->DrawIndexed((UINT)_countof(app_inds), 0, 0);
	}*/
}

///////////////////////////////////////////

void app_update() {
	// If the user presses the select action, lets add a cube at that location!
	for (uint32_t i = 0; i < 2; i++) {
		//if (xr_input.handSelect[i])
		//	app_cubes.push_back(xr_input.handPose[i]);
	}
}

///////////////////////////////////////////

void app_update_predicted() {
	// Update the location of the hand cubes. This is done after the inputs have been updated to 
	// use the predicted location, but during the render code, so we have the most up-to-date location.
	/*if (app_cubes.size() < 2)
		app_cubes.resize(2, xr_pose_identity);
	for (uint32_t i = 0; i < 2; i++) {
		app_cubes[i] = xr_input.renderHand[i] ? xr_input.handPose[i] : xr_pose_identity;
	}*/
}