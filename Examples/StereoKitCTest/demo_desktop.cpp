#if defined(_WIN32) && !defined(WINDOWS_UWP)
#define _CRT_SECURE_NO_WARNINGS

#include "demo_desktop.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winuser.h>
#include <d3d11.h>
#include <dxgi1_2.h>

#include "../../StereoKitC/stereokit.h"
#include "../../StereoKitC/stereokit_ui.h"
using namespace sk;

#include "Shaders/blit.hlsl.h"

///////////////////////////////////////////

IDXGIOutputDuplication *duplication_api = nullptr;
tex_t                   duplication_tex;

material_t    desktop_blit_mat;
mesh_t        desktop_mesh;
tex_t         desktop_tex;
material_t    desktop_material;

///////////////////////////////////////////

int32_t duplication_init(IDXGIOutputDuplication **duplication) {
	ID3D11Device        *d3d_device;
	ID3D11DeviceContext *d3d_context;
	render_get_device((void **)&d3d_device, (void **)&d3d_context);

	IDXGIDevice  *dxgi_device  = nullptr;
	IDXGIAdapter *dxgi_adapter = nullptr;
	IDXGIOutput  *dxgi_output  = nullptr;
	IDXGIOutput1 *dxgi_output1 = nullptr;

	HRESULT hr = d3d_device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgi_device);
	if (FAILED(hr)) return -1;

	// Get DXGI adapter
	hr = dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void **)&dxgi_adapter);
	dxgi_device->Release();
	if (FAILED(hr)) return -2;

	// Get output
	hr = dxgi_adapter->EnumOutputs(0, &dxgi_output);
	dxgi_adapter->Release();
	if (FAILED(hr)) return -3;

	// QI for Output 1
	hr = dxgi_output->QueryInterface(__uuidof(dxgi_output1), (void **)&dxgi_output1);
	dxgi_output->Release();
	if (FAILED(hr)) return -4;

	// Create desktop duplication
	hr = dxgi_output1->DuplicateOutput(d3d_device, duplication);
	DXGI_OUTPUT_DESC output_desc;
	dxgi_output1->GetDesc(&output_desc);
	dxgi_output1->Release();
	if (FAILED(hr)) return -5;

	return 1;
}

bool duplication_next_frame(IDXGIOutputDuplication *duplication, tex_t frame_tex) {
	if (!duplication) return false;

	if (tex_asset_state(frame_tex) == asset_state_loaded) {
		duplication->ReleaseFrame();
		tex_set_surface(frame_tex, nullptr, tex_type_image_nomips, 0, 0, 0, 1);
	}

	IDXGIResource          *desktop_resource = nullptr;
	ID3D11Texture2D        *desktop_image    = nullptr;
	DXGI_OUTDUPL_FRAME_INFO frame_info;
	HRESULT                 hr = duplication->AcquireNextFrame(0, &frame_info, &desktop_resource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT) { return false; }
	if (FAILED(hr)) { log_warn("Acquire failed"); return false; }

	hr = desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), (void **)&desktop_image);
	desktop_resource->Release();
	if (FAILED(hr)) { return false; }

	tex_set_surface(frame_tex, desktop_image, tex_type_image_nomips, 0, 0, 0, 1);

	return true;
}

void duplication_shutdown(IDXGIOutputDuplication *duplication) {
	duplication->ReleaseFrame();
	duplication->Release();
}

///////////////////////////////////////////

void demo_mesh_create(tex_t for_tex, float curve_radius, mesh_t mesh) {
	int32_t cols = 32;
	int32_t rows = 2;

	float aspect = tex_get_width(for_tex) / (float)tex_get_height(for_tex);
	float angle  = aspect / curve_radius;

	vert_t *verts = (vert_t*)malloc(sizeof(vert_t) * cols * rows);
	vind_t *inds  = (vind_t*)malloc(sizeof(vind_t) * (cols-1) * (rows-1) * 6);
	for (int32_t y = 0; y < rows; y++)
	{
		float yp = y / (float)(rows - 1);
		for (int32_t x = 0; x < cols; x++)
		{
			float xp   = x / (float)(cols - 1);
			float curr = (xp - 0.5f) * angle;

			verts[x + y * cols] = vert_t{ vec3{sinf(curr) * curve_radius, yp - 0.5f, cosf(curr)*curve_radius -curve_radius}, vec3{0,0,1}, vec2{1-xp,1-yp}, {255,255,255,255} };

			if (x < cols-1 && y < rows-1) {
				int32_t ind = (x+y*cols)*6;
				inds[ind  ] = (x  ) + (y  ) * cols;
				inds[ind+1] = (x+1) + (y+1) * cols;
				inds[ind+2] = (x+1) + (y  ) * cols;

				inds[ind+3] = (x  ) + (y  ) * cols;
				inds[ind+4] = (x  ) + (y+1) * cols;
				inds[ind+5] = (x+1) + (y+1) * cols;
			}
		}
	}

	mesh_set_data(mesh, verts, cols*rows, inds, (cols-1)*(rows-1)*6);
}

///////////////////////////////////////////

void demo_desktop_init() {
	duplication_init(&duplication_api);

	desktop_mesh     = mesh_gen_plane(vec2_one, { 0,0,1 }, {0,1,0});
	desktop_tex      = tex_create(tex_type_rendertarget);
	duplication_tex  = tex_create();
	tex_set_address(desktop_tex,     tex_address_clamp);
	tex_set_address(duplication_tex, tex_address_clamp);

	shader_t shader  = shader_create_mem((void *)sks_blit_hlsl, sizeof(sks_blit_hlsl));
	desktop_material = material_copy_id (default_id_material_unlit);
	desktop_blit_mat = material_create  (shader);
	material_set_texture(desktop_material, "diffuse", desktop_tex);
	material_set_texture(desktop_blit_mat, "source",  duplication_tex);
	shader_release(shader);
}

///////////////////////////////////////////

void demo_desktop_shutdown() {
	duplication_shutdown(duplication_api);
	mesh_release(desktop_mesh);
	tex_release(desktop_tex);
	tex_release(duplication_tex);
	material_release(desktop_material);
	material_release(desktop_blit_mat);
}

///////////////////////////////////////////

void demo_desktop_update() {
	// Update our desktop texture
	if (duplication_next_frame(duplication_api, duplication_tex)) {
		if (tex_get_width (desktop_tex) != tex_get_width (duplication_tex) ||
			tex_get_height(desktop_tex) != tex_get_height(duplication_tex)) {
			tex_set_colors(desktop_tex, tex_get_width(duplication_tex), tex_get_height(duplication_tex), nullptr);
			demo_mesh_create(desktop_tex, 1, desktop_mesh);
		}
		render_blit(desktop_tex, desktop_blit_mat);
	}

	// prepare a little grab handle underneath the desktop texture
	float    size   = tex_get_height(desktop_tex) * 0.0004f;
	bounds_t bounds = mesh_get_bounds(desktop_mesh);
	bounds.center     = (bounds.center + vec3{ 0, -bounds.dimensions.y / 2, bounds.dimensions.z / 2 }) * size + vec3{0,-0.02f,-0.02f};
	bounds.dimensions = vec3{0.2f, 0.02f, 0.02f};

	// UI for the grab handle
	static pose_t desktop_pose        = pose_t{ vec3{0,0,-0.5f}, quat_lookat(vec3_zero, {0,0,1}) };
	static pose_t desktop_pose_smooth = desktop_pose;
	ui_enable_far_interact(false);
	ui_handle_begin("Desktop", desktop_pose, bounds, true);
	ui_handle_end();
	ui_enable_far_interact(true);

	// Smooth out the motion extra for nicer placement
	desktop_pose_smooth = {
		vec3_lerp (desktop_pose_smooth.position,    desktop_pose.position,    4 * time_elapsedf()),
		quat_slerp(desktop_pose_smooth.orientation, desktop_pose.orientation, 4 * time_elapsedf()) };
	desktop_pose = desktop_pose_smooth;
	render_add_mesh(desktop_mesh, desktop_material, pose_matrix(desktop_pose, vec3_one * size));
}

///////////////////////////////////////////

#else
void demo_desktop_init    () {}
void demo_desktop_update  () {}
void demo_desktop_shutdown() {}
#endif