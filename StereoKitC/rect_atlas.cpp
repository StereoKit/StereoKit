#pragma once

#include "rect_atlas.h"

///////////////////////////////////////////

rect_atlas_t rect_atlas_create(int32_t width, int32_t height) {
	rect_atlas_t result = {};
	result.free_space.add({0,0,width,height});
	result.w = width;
	result.h = height;
	return result;
}

///////////////////////////////////////////

void rect_atlas_destroy(rect_atlas_t *atlas) {
	atlas->free_space.free();
	atlas->packed.free();
	*atlas = {};
}

///////////////////////////////////////////

int32_t _rect_atlas_fit(recti_t src, const recti_t &r) {
	if (r.w < src.w || r.h < src.h) return 0x7FFFFFFF;
	int16_t w_diff = r.w - src.w;
	int16_t h_diff = r.h - src.h;
	return w_diff << 16 | h_diff;
}

///////////////////////////////////////////

int32_t rect_atlas_add(rect_atlas_t *atlas, recti_t rect) {
	int32_t idx = atlas->free_space.index_best_small_with(rect, _rect_atlas_fit);
	if (idx == -1)
		return -1;

	const recti_t parent = atlas->free_space[idx];

	if (parent.h == rect.h && parent.w == rect.w) { 
		// Perfect fit!
		atlas->free_space.remove(idx);
	} else if (parent.h == rect.h) {
		// Perfect vertical fit, just slide the bounds over to the right
		atlas->free_space[idx].x += rect.w;
		atlas->free_space[idx].w -= rect.w;
	} else if (parent.w == rect.w) {
		// Perfect horizontal fit, just slide the bounds down
		atlas->free_space[idx].y += rect.h;
		atlas->free_space[idx].h -= rect.h;
	} else {
		// Split along the shortest edge, try and preserve large areas
		recti_t new_free = {};
		if (parent.h - rect.h < parent.w - rect.w) {
			// Vertical axis is shorter, split horizontally
			atlas->free_space[idx].y += rect.h;
			atlas->free_space[idx].h -= rect.h;
			atlas->free_space[idx].w  = rect.w;
			new_free.x = parent.x + rect.w;
			new_free.y = parent.y;
			new_free.w = parent.w - rect.w;
			new_free.h = parent.h;
		} else {
			// Horizontal axis is shorter, split vertically
			atlas->free_space[idx].x += rect.w;
			atlas->free_space[idx].w -= rect.w;
			atlas->free_space[idx].h  = rect.h;
			new_free.x = parent.x;
			new_free.y = parent.y + rect.h;
			new_free.w = parent.w;
			new_free.h = parent.h - rect.h;
		}
		atlas->free_space.add(new_free);
	}

	recti_t new_rect = {};
	new_rect.x = parent.x;
	new_rect.y = parent.y;
	new_rect.w = rect.w;
	new_rect.h = rect.h;
	atlas->used_area += rect.w * rect.h;
	return atlas->packed.add(new_rect);
}

///////////////////////////////////////////

int32_t _rect_atlas_add_free_space(rect_atlas_t *atlas, recti_t rect, int32_t idx) {
	int32_t rect_r = rect.x + rect.w;
	int32_t rect_b = rect.y + rect.h;
	int32_t result = -1;

	for (size_t i = 0; i < atlas->free_space.count; i++) {
		if (i == idx) continue;
		recti_t cell = atlas->free_space[i];

		if (cell.h == rect.h && cell.y == rect.y) {
			int32_t cell_r = cell.x + cell.w;
			if (cell.x == rect_r) { // left side of this cell matches up
				atlas->free_space[i].x  = rect.x;
				atlas->free_space[i].w += rect.w;
				result = i;
				break;
			} else if (cell_r == rect.x) { // right side of this cell matches up
				atlas->free_space[i].w += rect.w;
				result = i;
				break;
			}
		} else if (cell.w == rect.w && cell.x == rect.x) {
			int32_t cell_b = cell.y + cell.h;
			if (cell.y == rect_b) { // top side of this cell matches up
				atlas->free_space[i].y  = rect.y;
				atlas->free_space[i].h += rect.h;
				result = i;
				break;
			} else if (cell_b == rect.y) { // bottom side of this cell matches up
				atlas->free_space[i].h += rect.h;
				result = i;
				break;
			}
		}
	}

	if (idx != -1 && result != -1) {
		// If this was already free space we knew about, but we merged it
		// with some existing stuff, then remove the old free space.
		atlas->free_space.remove(idx);
		if (result > idx)
			result -= 1;
	} else if (idx == -1 && result == -1) {
		// If the source was not free space, and we didn't find free space to
		// merge this into, make a new one!
		atlas->free_space.add(rect);
	}

	return result;
}

///////////////////////////////////////////

void rect_atlas_remove(rect_atlas_t *atlas, int32_t idx) {
	recti_t free_space = atlas->packed[idx];
	atlas->packed.remove(idx);
	atlas->used_area -= free_space.w * free_space.h;

	int32_t new_rect = _rect_atlas_add_free_space(atlas, free_space, -1);
	while (new_rect != -1) {
		new_rect = _rect_atlas_add_free_space(atlas, atlas->free_space[new_rect], new_rect);
	}
}
