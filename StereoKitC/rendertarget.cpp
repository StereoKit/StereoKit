#include "rendertarget.h"

#include "d3d.h"
#include "texture.h"

void rendertarget_release(rendertarget_t &target) {
	if (target.shader_view != nullptr) target.shader_view->Release();
	if (target.target_view != nullptr) target.target_view->Release();
	if (target.texture     != nullptr) target.texture    ->Release();
	if (target.depth_view  != nullptr) target.depth_view ->Release();
	target = {};
}
void rendertarget_resize(rendertarget_t &target, int width, int height) {
	if (target.width != width || target.height != height)
		rendertarget_release(target);
	else return;
}
void rendertarget_make_views(rendertarget_t &target) {
	D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
	target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	target_desc.Format        = target.format;
	d3d_device->CreateRenderTargetView(target.texture, &target_desc, &target.target_view);

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format                    = target.format;
	shaderResourceViewDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels       = 1;
	d3d_device->CreateShaderResourceView(target.texture, &shaderResourceViewDesc, &target.shader_view);
}
void rendertarget_set_surface(rendertarget_t &target, ID3D11Texture2D *surface, DXGI_FORMAT format) {
	if (target.texture == surface)
		return;
	if (target.texture     != nullptr) target.texture    ->Release();
	if (target.target_view != nullptr) target.target_view->Release();
	if (target.shader_view != nullptr) target.shader_view->Release();

	target.texture = surface;

	int old_width  = target.width;
	int old_height = target.height;
	D3D11_TEXTURE2D_DESC color_desc;
	target.texture->GetDesc(&color_desc);
	target.width  = color_desc.Width;
	target.height = color_desc.Height;
	target.format = format == DXGI_FORMAT_UNKNOWN ? color_desc.Format : format;

	rendertarget_make_views(target);
	if (target.depth_view != nullptr && (old_width != target.width || old_height != target.height)) {
		target.depth_view->Release();
		rendertarget_make_depthbuffer(target);
	}
}

void rendertarget_make_depthbuffer(rendertarget_t &target) {
	if (target.depth_view != nullptr)
		return;
	ID3D11Texture2D     *depth_texture;
	D3D11_TEXTURE2D_DESC depth_desc = {};
	depth_desc.SampleDesc.Count = 1;
	depth_desc.MipLevels        = 1;
	depth_desc.Width            = target.width;
	depth_desc.Height           = target.height;
	depth_desc.ArraySize        = 1;
	depth_desc.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depth_desc.BindFlags        = D3D11_BIND_DEPTH_STENCIL;
	d3d_device->CreateTexture2D(&depth_desc, nullptr, &depth_texture);

	// And create a view resource for the depth buffer, so we can set that up for rendering to as well!
	D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
	stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	stencil_desc.Format        = depth_desc.Format;
	d3d_device->CreateDepthStencilView(depth_texture, &stencil_desc, &target.depth_view);

	// We don't need direct access to the ID3D11Texture2D object anymore, we only need the view
	depth_texture->Release();
}

void rendertarget_clear(rendertarget_t &target, const float *color) {
	if (target.target_view != nullptr) d3d_context->ClearRenderTargetView (target.target_view, color);
	if (target.depth_view  != nullptr) d3d_context->ClearDepthStencilView (target.depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
void rendertarget_set_active(rendertarget_t &target) {
	d3d_context->OMSetRenderTargets(1, &target.target_view, target.depth_view);
}

void rendertarget_clear(tex2d_t &target, const float *color) {
	if (target->target_view != nullptr)
		d3d_context->ClearRenderTargetView (target->target_view, color);

	if (target->depth_buffer != nullptr && target->depth_buffer->depth_view != nullptr)
		d3d_context->ClearDepthStencilView (target->depth_buffer->depth_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}
void rendertarget_set_active(tex2d_t &target) {
	bool                    has_depth  = target->depth_buffer != nullptr && target->depth_buffer->depth_view != nullptr;
	ID3D11DepthStencilView *depth_view = has_depth ? target->depth_buffer->depth_view : nullptr;

	d3d_context->OMSetRenderTargets(1, &target->target_view, depth_view);
}