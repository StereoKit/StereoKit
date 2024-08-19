#include "sdf.h"
#include "../sk_math.h"
#include "../sk_memory.h"

#include <stdio.h>

namespace sk {

///////////////////////////////////////////

float sdf_box_round(vec2 pt, vec2 size, float radius) {
	vec2 q = vec2{fabsf(pt.x)-size.x+radius, fabsf(pt.y)-size.y+radius};
	return fminf(fmaxf(q.x, q.y), 0.0) + vec2_magnitude({ fmaxf(q.x,0), fmaxf(q.y,0)}) - radius;
}

///////////////////////////////////////////

float sdf_box(vec2 pt, vec2 size) {
	vec2 d = vec2{fabsf(pt.x)-size.x, fabsf(pt.y)-size.y};
	return fminf(fmaxf(d.x, d.y), 0.0) + vec2_magnitude({ fmaxf(d.x,0), fmaxf(d.y,0)});
}

///////////////////////////////////////////

// The MIT License
// Copyright (c) 2018 Inigo Quilez
float sdf_triangle(vec2 pt, vec2 size) {
	pt.x = fabsf(pt.x);
	vec2  base_pt     = pt - size * math_clamp(vec2_dot(pt, size) / vec2_dot(size, size), 0.f, 1.f);
	vec2  ascend_pt   = pt - size * vec2{ math_clamp(pt.x / size.x, 0.f, 1.f), 1.f };

	float orientation = size.y > 0 ? -1.0f : 1.0f;
	vec2  closest     = vec2_min(
		vec2{ vec2_dot(base_pt,   base_pt  ), orientation * (pt.x * size.y - pt.y * size.x) },
		vec2{ vec2_dot(ascend_pt, ascend_pt), orientation * (pt.y - size.y) } );
	return sqrtf(closest.x) * (closest.y > 0 ? -1 : 1);
}

///////////////////////////////////////////

// The MIT License
// Copyright (c) 2017 Inigo Quilez
float sdf_diamond(vec2 pt, vec2  size) {
	pt = vec2_abs(pt);

	vec2  dot  = size - 2.0f * pt;
	float ndot = dot.x*size.x - dot.y*size.y;

	float proportions = math_clamp(ndot / vec2_dot(size, size), -1.f, 1.f);
	float dist        = vec2_magnitude(pt - 0.5f * size * vec2{ 1.f - proportions, 1.f + proportions });

	return dist * ((pt.x*size.y + pt.y*size.x - size.x*size.y) > 0 ? 1 : -1);
}

///////////////////////////////////////////

// The MIT License
// Copyright (c) 2019 Inigo Quilez
float sdf_rounded_x(vec2 pt, float size, float radius) {
	pt = vec2_abs(pt);
	return vec2_magnitude(pt - fminf(pt.x + pt.y, size) * 0.5f) - radius;
}

///////////////////////////////////////////

tex_t sdf_create_tex(int32_t width, int32_t height, float (*sdf)(vec2 pt), float scale) {
	color32 *data     = sk_malloc_t(color32, width * height);
	float    center_x = width  / 2.0f - 0.5f;
	float    center_y = height / 2.0f - 0.5f;
	for (int32_t y = 0; y < height; y++) {
		int32_t yoff = y * width;
		for (int32_t x = 0; x < width; x++) {
			float dist = sdf({ x - center_x, y - center_y });
			float lerp = 1-math_saturate(dist * scale);

			data[x + yoff] = {255,255,255,(uint8_t)(lerp * 255)};
		}
	}
	return tex_create_color32(data, width, height, false);
}

///////////////////////////////////////////

sprite_t sdf_create_sprite(const char* name, int32_t width, int32_t height, float (*sdf)(vec2 pt), float scale) {
	tex_t tex = sdf_create_tex(width, height, sdf, scale);
	tex_set_address(tex, tex_address_clamp);

	char id[128];
	snprintf(id, sizeof(id), "%s/tex", name);
	tex_set_id(tex, id);

	sprite_t spr = sprite_create(tex, sprite_type_single);
	sprite_set_id(spr, name);

	tex_release(tex);
	return spr;
}

}