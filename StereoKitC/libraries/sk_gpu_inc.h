#pragma once

#ifdef WINDOWS_UWP
#define SKR_DIRECT3D11
#else
#define SKR_OPENGL
#endif
#include "sk_gpu.h"