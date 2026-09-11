#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"

namespace sk {

// Ear clipping for a set of closed rings, producing triangle indices into
// 'points'. Rings must not cross themselves or each other. Which rings are
// holes comes from the fill rule, so nested rings behave like SVG fills.
void triangulate_rings(const vec2* points, const int32_t* ring_starts, const int32_t* ring_counts, int32_t ring_count, bool even_odd, array_t<uint32_t>* ref_indices);

}
