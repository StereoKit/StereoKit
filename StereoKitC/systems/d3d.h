#pragma once

#include <d3d11.h>

#if _DEBUG
#pragma comment(lib, "dxguid.lib") 
#include <stdio.h>
#define DX11ResType(res, name) res->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(name)-1, name);
#define DX11ResName(res, type, name) { char tmp[256]; sprintf_s(tmp, 256, "%s|%s", type, name); res->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(tmp), tmp); }
#else
#define DX11ResType(res, name)
#define DX11ResName(res, type, name)
#endif

extern ID3D11Device           *d3d_device;
extern ID3D11DeviceContext    *d3d_context;
extern ID3D11RasterizerState  *d3d_rasterstate;

extern int                     d3d_screen_width;
extern int                     d3d_screen_height;

bool d3d_init    ();
void d3d_update  ();
void d3d_shutdown();