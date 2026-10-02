#pragma once

#include "../stereokit.h"
#include "mesh.h"

namespace sk {

// Loader knobs that aren't part of the public API yet.
typedef struct svg_options_t {
	float    tolerance;     // how far a flattened curve may stray, as a fraction of the larger image dimension
	bool32_t reverse_order; // emit the last shapes first, the depth test then keeps SVG's painter's order
} svg_options_t;

static const svg_options_t svg_options_default = { 0.001f, true };

// Parses and triangulates SVG text into vertex data the caller owns. 'name'
// is only for log messages. Safe to call from asset threads.
bool   svg_build_mem(const void* svg_data, size_t data_size, const char* name, svg_options_t options, mesh_load_t* out_mesh, bool32_t* out_translucent);

}
