#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"Dxgi.lib") // CreateSwapChainForHwnd

#include "../stereokit.h"
#include "d3d.h"

namespace sk {

///////////////////////////////////////////

ID3D11Device             *d3d_device        = nullptr;
ID3D11DeviceContext      *d3d_context       = nullptr;
ID3D11InfoQueue          *d3d_info          = nullptr;
ID3D11DepthStencilState  *d3d_depthstate    = nullptr;
ID3D11RasterizerState    *d3d_rasterstate   = nullptr;
int                       d3d_screen_width  = 640;
int                       d3d_screen_height = 480;

///////////////////////////////////////////

bool d3d_init() {
	UINT creation_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creation_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };
	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creation_flags, featureLevels, _countof(featureLevels), D3D11_SDK_VERSION, &d3d_device, nullptr, &d3d_context)))
		return false;

	// Hook into debug information
	ID3D11Debug *d3dDebug = nullptr;
	if (SUCCEEDED(d3d_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
		d3d_info = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3d_info))) {
			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS, (D3D11_MESSAGE_ID)351
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3d_info->ClearStorageFilter();
			d3d_info->AddStorageFilterEntries(&filter);
		}
		d3dDebug->Release();
	}

	D3D11_DEPTH_STENCIL_DESC desc_depthstate = {};
	desc_depthstate.DepthEnable    = true;
	desc_depthstate.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	desc_depthstate.DepthFunc      = D3D11_COMPARISON_LESS;
	desc_depthstate.StencilEnable    = true;
	desc_depthstate.StencilReadMask  = 0xFF;
	desc_depthstate.StencilWriteMask = 0xFF;
	desc_depthstate.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	desc_depthstate.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	desc_depthstate.BackFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	desc_depthstate.BackFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	desc_depthstate.BackFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	d3d_device->CreateDepthStencilState(&desc_depthstate, &d3d_depthstate);
	d3d_context->OMSetDepthStencilState(d3d_depthstate, 1);
	
	D3D11_RASTERIZER_DESC desc_rasterizer = {};
	desc_rasterizer.FillMode = D3D11_FILL_SOLID;
	desc_rasterizer.CullMode = D3D11_CULL_BACK;
	desc_rasterizer.FrontCounterClockwise = true;
	d3d_device->CreateRasterizerState(&desc_rasterizer, &d3d_rasterstate);
	d3d_context->RSSetState(d3d_rasterstate);
	d3d_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

///////////////////////////////////////////

void d3d_shutdown() {
	if (d3d_context) { d3d_context->Release(); d3d_context = nullptr; }
	if (d3d_device ) { d3d_device->Release();  d3d_device  = nullptr; }
}

///////////////////////////////////////////

void d3d_update() {
	#ifdef _DEBUG
	if (d3d_info == nullptr)
		return;
	for(size_t i = 0; i < d3d_info->GetNumStoredMessages(); i++) {
		SIZE_T size = 0;
		d3d_info->GetMessageA(0, nullptr, &size);
		D3D11_MESSAGE * pMessage = (D3D11_MESSAGE*)malloc(size);
		if (SUCCEEDED(d3d_info->GetMessageA(i, pMessage, &size)) && pMessage->Severity <= D3D11_MESSAGE_SEVERITY_WARNING) {
			const char *cat = "None";
			switch(pMessage->Category) {
			case D3D11_MESSAGE_CATEGORY_APPLICATION_DEFINED:  cat = "App";          break;
			case D3D11_MESSAGE_CATEGORY_MISCELLANEOUS:        cat = "Misc";         break;
			case D3D11_MESSAGE_CATEGORY_INITIALIZATION:       cat = "Init";         break;
			case D3D11_MESSAGE_CATEGORY_CLEANUP:              cat = "Cleanup";      break;
			case D3D11_MESSAGE_CATEGORY_COMPILATION:          cat = "Compile";      break;
			case D3D11_MESSAGE_CATEGORY_STATE_CREATION:       cat = "State Create"; break;
			case D3D11_MESSAGE_CATEGORY_STATE_SETTING:        cat = "State Set";    break;
			case D3D11_MESSAGE_CATEGORY_STATE_GETTING:        cat = "State Get";    break;
			case D3D11_MESSAGE_CATEGORY_RESOURCE_MANIPULATION:cat = "Resource";   break;
			case D3D11_MESSAGE_CATEGORY_EXECUTION:            cat = "Exec";         break;
			case D3D11_MESSAGE_CATEGORY_SHADER:               cat = "Shader";       break;
			}
			log_ level;
			switch (pMessage->Severity) {
			case D3D11_MESSAGE_SEVERITY_MESSAGE:
			case D3D11_MESSAGE_SEVERITY_INFO:       level = log_inform;  break;
			case D3D11_MESSAGE_SEVERITY_WARNING:    level = log_warning; break;
			case D3D11_MESSAGE_SEVERITY_CORRUPTION:
			case D3D11_MESSAGE_SEVERITY_ERROR:      level = log_error;   break;
			default: level = log_inform;
			}
			log_writef(level, "%s: [%d] %s", cat, pMessage->ID, pMessage->pDescription);
		}
		free(pMessage);
	}
	d3d_info->ClearStoredMessages();
	#endif
}

} // namespace sk