/* SPDX-License-Identifier: MIT */
/* The authors below grant copyright rights under the MIT license:
 * Copyright (c) 2025 Nick Klingensmith
 * Copyright (c) 2025 Qualcomm Technologies, Inc.
 */

#pragma once

namespace sk {

typedef enum xr_thread_type_ {
	xr_thread_type_app_main,
	xr_thread_type_app_work,
	xr_thread_type_render_main,
	xr_thread_type_render_work,
} xr_thread_type_;

void xr_ext_android_thread_register();
void xr_ext_android_thread_set_type(xr_thread_type_ type);

}