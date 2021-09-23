#pragma once

#include <stdint.h>
#include "libraries/array.h"

///////////////////////////////////////////

typedef struct recti_t {
	int32_t x, y, w, h;
} recti_t;

typedef struct rect_atlas_t {
	array_t<recti_t> free_space;
	array_t<recti_t> packed;
	int32_t          w, h;
	int32_t          used_area;
} rect_atlas_t;

///////////////////////////////////////////

rect_atlas_t rect_atlas_create (int32_t width, int32_t height);
void         rect_atlas_destroy(rect_atlas_t *atlas);
int32_t      rect_atlas_add    (rect_atlas_t *atlas, int32_t width, int32_t height);
void         rect_atlas_remove (rect_atlas_t *atlas, int32_t idx);