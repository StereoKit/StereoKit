// SPDX-License-Identifier: MIT
// The authors below grant copyright rights under the MIT license:
// Copyright (c) 2026 Nick Klingensmith
// Copyright (c) 2026 Qualcomm Technologies, Inc.

#pragma once

#include "../stereokit.h"
#include <sk_renderer.h>

namespace sk {

// Registry of deduplicated, refcounted vertex formats. An id's
// skr_vert_type_t address is stable, gpu meshes point directly at it.
#define VERT_FORMAT_DEFAULT 0 // the built-in vert_t format, always present

bool vert_format_sys_init    ();
void vert_format_sys_shutdown();

// Finds or creates a format matching these components, and adds a
// reference to it. Returns -1 for an invalid component list.
int32_t vert_format_ref    (const vert_component_t* components, int32_t count);
// Refcounts are atomic, but only addref ids that are currently alive.
void    vert_format_addref (int32_t id);
void    vert_format_release(int32_t id);

const skr_vert_type_t*  vert_format_get_skr       (int32_t id);
const vert_component_t* vert_format_get_components(int32_t id, int32_t* out_count);
uint32_t                vert_format_get_stride    (int32_t id);
// Byte offset of a semantic's component within a vertex, or -1 if the
// format has no such component.
int32_t                 vert_format_semantic_offset(int32_t id, vert_semantic_ semantic, int32_t semantic_slot, vert_fmt_* out_opt_fmt, int32_t* out_opt_count);

// Decode or encode a single vertex's semantic as floats. Decode writes only
// the elements the component has, encode reads only those. Both return
// false when the format has no such component.
bool vert_format_decode(int32_t id, const void* vertex, vert_semantic_ semantic, int32_t semantic_slot, vec4* ref_value);
bool vert_format_encode(int32_t id,       void* vertex, vert_semantic_ semantic, int32_t semantic_slot, vec4 value);

int32_t vert_fmt_size(vert_fmt_ format);

} // namespace sk
