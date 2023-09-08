// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2019-2023 Nick Klingensmith
// Copyright (c) 2023 Qualcomm Technologies, Inc.

#include "xr.h"
#include "openxr.h"

namespace sk {

///////////////////////////////////////////

bool xr_init() {
#if defined(SK_XR_OPENXR)
	return openxr_init();
#else
	return false;
#endif
}

///////////////////////////////////////////

void xr_step_begin() {
#if defined(SK_XR_OPENXR)
	openxr_step_begin();
#else
#endif
}

///////////////////////////////////////////

void xr_step_end() {
#if defined(SK_XR_OPENXR)
	openxr_step_end();
#else
#endif
}

///////////////////////////////////////////

void xr_shutdown() {
#if defined(SK_XR_OPENXR)
	openxr_shutdown();
#else
#endif
}

}