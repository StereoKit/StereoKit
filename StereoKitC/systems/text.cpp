#include "text.h"

#include "../asset_types/material.h"
#include "../asset_types/assets.h"
#include "../systems/defaults.h"
#include "../hierarchy.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../libraries/array.h"

#include <ctype.h>

#include <DirectXMath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

array_t<_text_style_t> text_styles  = {};
array_t<text_buffer_t> text_buffers = {};

///////////////////////////////////////////

void text_buffer_ensure_capacity(text_buffer_t &buffer, size_t characters) {
	if (buffer.vert_count + characters * 4 <= buffer.vert_cap)
		return;

	buffer.vert_cap = buffer.vert_count + (int)characters * 4;
	buffer.verts    = sk_realloc_t<vert_t>(buffer.verts, buffer.vert_cap);

	// regenerate indices
	vind_t  quads = (vind_t)(buffer.vert_cap / 4);
	vind_t *inds  = sk_malloc_t<vind_t>(quads * 6);
	for (vind_t i = 0; i < quads; i++) {
		vind_t q = i * 4;
		vind_t c = i * 6;
		inds[c+0] = q+2;
		inds[c+1] = q+1;
		inds[c+2] = q;

		inds[c+3] = q+3;
		inds[c+4] = q+2;
		inds[c+5] = q;
	}
	mesh_set_inds(buffer.mesh, inds, quads * 6);
	free(inds);
}

///////////////////////////////////////////

text_style_t text_make_style(font_t font, float character_height, material_t material, color32 color) {
	uint32_t       id     = (uint32_t)(font->header.id << 16 | material->header.id);
	size_t         index  = 0;
	text_buffer_t *buffer = nullptr;
	
	// Find or make a buffer for this style
	for (size_t i = 0; i < text_buffers.count; i++) {
		if (text_buffers[i].id == id) {
			buffer = &text_buffers[i];
			index  = i;
			break;
		}
	}
	if (buffer == nullptr) {
		index  = text_buffers.add({});
		buffer = &text_buffers[index];

		buffer->mesh     = mesh_create();
		buffer->id       = id;
		buffer->font     = font;
		buffer->material = material;
		assets_addref(font->header);
		assets_addref(material->header);

		material_set_texture     (material, "diffuse", font_get_tex(font));
		material_set_cull        (material, cull_none);
		material_set_transparency(material, transparency_blend);
	}

	// Create the style
	_text_style_t style;
	style.font            = font;
	style.buffer_index    = (uint32_t)index;
	style.color           = color;
	style.size            = character_height/font->character_height;
	style.line_spacing    = font->character_height * 0.5f;
	
	return (text_style_t)text_styles.add(style);
}

///////////////////////////////////////////

vec2 text_line_size(text_style_t style, const char *text) {
	font_t      font = text_styles[style].font;
	const char *curr = text;
	float       x    = 0;
	while (*curr != '\0' && *curr != '\n') {
		char         currch = *curr;
		font_char_t &ch     = font->characters[(int)currch];

		// Do spacing for whitespace characters
		switch (currch) {
		case '\t': x += font->characters[(int)' '].xadvance * 4; break;
		default:   x += ch.xadvance; break;
		}
		curr += 1;
	}
	return vec2{ x, font->character_height } * text_styles[style].size;
}

///////////////////////////////////////////

vec2 text_size(const char *text, text_style_t style) {
	if (text == nullptr) return {};

	font_t font = text_styles[style == -1 ? 0 : style].font;
	const char *curr = text;
	float x = 0;
	int   y = 1;
	float max_x = 0;
	while (*curr != '\0') {
		char currch = *curr;
		curr += 1;
		font_char_t &ch = font->characters[(int)currch];

		// Do spacing for whitespace characters
		switch (currch) {
		case '\t': x += font->characters[(int)' '].xadvance * 4; break;
		case '\n': if (x > max_x) max_x = x; x = 0; y += 1; break;
		default  : x += ch.xadvance; break;
		}
	}
	if (x > max_x) max_x = x;
	return vec2{ max_x, y * font->character_height + (y-1)*text_styles[style].line_spacing} * text_styles[style].size;
}

///////////////////////////////////////////

float text_step_line_length(const char *start, int32_t *out_char_count, const text_stepper_t &step) {
	// If we're not wrapping, this is really simple
	if (!step.wrap) {
		const char *curr = start;
		float width = 0;
		while (*curr != '\n' && *curr != '\0') {
			width += step.style->font->characters[(int)*curr].xadvance;
			curr++;
		}
		if (out_char_count != nullptr)
			*out_char_count = (int32_t)((*curr == '\n' ? curr+1 : curr) - start);
		return width * step.style->size;
	}

	// Otherwise, we gotta do it the tricky way
	float curr_width = 0;
	float last_width = 0;
	const char *last_at = start;
	const char *ch = start;
	bool was_space = false;

	while (true) {
		char curr = *ch;
		bool is_space = isspace(curr);

		// We prefer to line break at spaces, rather than in the middle of words
		if (is_space || curr == '\0') {
			if (!was_space)
				last_width = curr_width;
			last_at = curr != '\0' ? ch+1 : ch;
		}
		// End of line or string?
		if (curr == '\0' || curr == '\n')
			break;

		// Advance by character width
		font_char_t &char_info = step.style->font->characters[(int)curr];
		float next_width = char_info.xadvance*step.style->size + curr_width;

		// Check if it steps out of bounds
		if (!is_space && next_width > step.bounds.x) {
			// If there were no spaces in this line, set to the previous character
			if (last_width == 0) {
				last_width = curr_width;
				last_at = ch;
			}
			// Exit the line
			break;
		}
		
		// Next character!
		curr_width = next_width;
		was_space = is_space;
		ch++;
	}

	if (out_char_count != nullptr)
		*out_char_count = (int32_t)(last_at - start);
	return last_width;
}

///

float text_step_height(const char *text, int32_t *out_length, const text_stepper_t &step) {
	int32_t     count  = 1;
	const char *curr   = text;
	float       height = 0;
	while (count > 0) {
		text_step_line_length(curr, &count, step);
		curr += count;
		if (count > 0)
			height += 1;
	}

	if (out_length != nullptr)
		*out_length = (int32_t)(curr - text);
	return (height * step.style->font->character_height + (height-1)*step.style->line_spacing) * step.style->size;
}

///

void text_step_next_line(const char *start, text_stepper_t &step) {
	float line_size  = text_step_line_length(start, &step.line_remaining, step);
	float align_x    = 0;
	if (step.align & text_align_x_center) align_x = ((step.bounds.x - line_size) / 2.f);
	if (step.align & text_align_x_right)  align_x =  (step.bounds.x - line_size);
	step.pos.x  = step.start.x - align_x;
	step.pos.y -= step.style->size * step.style->font->character_height;
}

void text_step_position(char ch, const char *start, text_stepper_t &step) {
	font_char_t &char_info = step.style->font->characters[(int)ch];
	step.line_remaining--;
	if (step.line_remaining <= 0) {
		text_step_next_line(start+1, step);
		step.pos.y -= step.style->size * step.style->line_spacing;
		return;
	}

	switch (ch) {
	case '\t': step.pos.x -= step.style->font->characters[(int)' '].xadvance * 4 * step.style->size; break;
	case '\n': {
	} break;
	default : step.pos.x -= char_info.xadvance*step.style->size; break;
	}
}

///

void text_add_quad(float x, float y, float off_z, font_char_t &char_info, _text_style_t &style_data,  text_buffer_t &buffer, XMMATRIX &tr, const vec3 &normal) {
	float x_min = x - char_info.x0 * style_data.size;
	float x_max = x - char_info.x1 * style_data.size;
	float y_min = y + char_info.y0 * style_data.size;
	float y_max = y + char_info.y1 * style_data.size;
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_min, y_min, off_z }), normal, vec2{ char_info.u0, char_info.v0 }, style_data.color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_max, y_min, off_z }), normal, vec2{ char_info.u1, char_info.v0 }, style_data.color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_max, y_max, off_z }), normal, vec2{ char_info.u1, char_info.v1 }, style_data.color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_min, y_max, off_z }), normal, vec2{ char_info.u0, char_info.v1 }, style_data.color };
}

///

void text_add_quad_clipped(float x, float y, float off_z, vec2 bounds_min, vec2 bounds_max, font_char_t &char_info, _text_style_t &style_data,  text_buffer_t &buffer, XMMATRIX &tr, const vec3 &normal) {
	float x_max = x - char_info.x0 * style_data.size;
	float x_min = x - char_info.x1 * style_data.size;
	float y_max = y + char_info.y0 * style_data.size;
	float y_min = y + char_info.y1 * style_data.size;

	// Ditch out if it's completely out of bounds
	if (x_min > bounds_max.x ||
		x_max < bounds_min.x ||
		y_min > bounds_max.y ||
		y_max < bounds_min.y)
		return;

	float u_max = char_info.u0;
	float u_min = char_info.u1;
	float v_max = char_info.v0;
	float v_min = char_info.v1;

	// Clip it if it's partially out of bounds
	if (x_min < bounds_min.x) {
		u_min = u_min + (u_max-u_min) * ((bounds_min.x-x_min) / (x_max-x_min));
		x_min = bounds_min.x;
	}
	if (x_max > bounds_max.x) {
		u_max = u_min + (u_max-u_min) * ((bounds_max.x-x_min) / (x_max-x_min));
		x_max = bounds_max.x;
	}
	if (y_min < bounds_min.y) {
		v_min = v_min + (v_max-v_min) * ((bounds_min.y-y_min) / (y_max-y_min));
		y_min = bounds_min.y;
	}
	if (y_max > bounds_max.y) {
		v_max = v_min + (v_max-v_min) * ((bounds_max.y-y_min) / (y_max-y_min));
		y_max = bounds_max.y;
	}

	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_max, y_max, off_z }), normal, vec2{ u_max, v_max }, style_data.color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_min, y_max, off_z }), normal, vec2{ u_min, v_max }, style_data.color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_min, y_min, off_z }), normal, vec2{ u_min, v_min }, style_data.color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_max, y_min, off_z }), normal, vec2{ u_max, v_min }, style_data.color };
}

///

void text_add_at(const char* text, const matrix &transform, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z) {
	text_add_in(text, transform, text_size(text, style == -1 ? 0 : style), text_fit_exact, style, position, align, off_x, off_y, off_z);
}

///////////////////////////////////////////

void text_add_in(const char* text, const matrix& transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z) {
	if (text == nullptr) return;

	XMMATRIX tr;
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.last().transform, tr);
	} else {
		math_matrix_to_fast(transform, &tr);
	}
	vec3 normal = matrix_mul_direction(tr, vec3_forward);

	// Find the initial stepping information for this chunk of text
	text_stepper_t step;
	step.line_remaining = 0;
	step.align  = align;
	step.wrap   = fit & text_fit_wrap;
	step.style  = &text_styles[style == -1 ? 0 : style];
	step.bounds = size;
	step.start  = { off_x, off_y };

	// Debug draw bounds
	/*vec2 dbg_start = step.start;
	if      (position & text_align_x_center) dbg_start.x += step.bounds.x / 2.f;
	else if (position & text_align_x_right)  dbg_start.x += step.bounds.x;
	if      (position & text_align_y_center) dbg_start.y += step.bounds.y / 2.f;
	else if (position & text_align_y_bottom) dbg_start.y += step.bounds.y;
	line_add({ dbg_start.x, dbg_start.y,                 off_z }, { dbg_start.x - step.bounds.x, dbg_start.y, off_z }, { 255,0,255,255 }, { 255,0,255,255 }, 0.002f);
	line_add({ dbg_start.x, dbg_start.y - step.bounds.y, off_z }, { dbg_start.x - step.bounds.x, dbg_start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);
	line_add({ dbg_start.x, dbg_start.y,                 off_z }, { dbg_start.x,                 dbg_start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);
	line_add({ dbg_start.x - step.bounds.x, dbg_start.y, off_z }, { dbg_start.x - step.bounds.x, dbg_start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);
	*/

	// Ensure scale is right for our fit
	if (fit & (text_fit_squeeze | text_fit_exact)) {
		vec2 txt_size = text_size(text, style == -1 ? 0 : style);
		vec2 scale_xy = {
			size.x / txt_size.x,
			size.y / txt_size.y };
		float scale = fminf(scale_xy.x, scale_xy.y)*0.999f;
		if (fit & text_fit_squeeze)
			scale = fminf(1, scale);

		// Apply the scale to the transform matrix
		XMMATRIX scale_m = XMMatrixTranslation(-off_x, -off_y, -off_z) * XMMatrixScaling(scale, scale, 1) * XMMatrixTranslation(off_x, off_y, off_z);
		tr = XMMatrixMultiply(scale_m, tr);
		step.bounds = step.bounds / scale;
	}

	// Align the start based on the size of the bounds
	if      (position & text_align_x_center) step.start.x += step.bounds.x / 2.f;
	else if (position & text_align_x_right)  step.start.x += step.bounds.x;
	if      (position & text_align_y_center) step.start.y += step.bounds.y / 2.f;
	else if (position & text_align_y_bottom) step.start.y += step.bounds.y;
	step.pos = step.start;

	// Figure out the vertical align of the text
	int32_t text_length = 0;
	float   text_height = text_step_height(text, &text_length, step);
	if      (align & text_align_y_center) step.pos.y -= (step.bounds.y-text_height) / 2.f;
	else if (align & text_align_y_bottom) step.pos.y -=  step.bounds.y-text_height;

	// Ensure text capacity
	text_buffer_t &buffer = text_buffers[step.style->buffer_index];
	text_buffer_ensure_capacity(buffer, text_length);

	// Core loop for drawing the text
	vec2 bounds_min = step.start - step.bounds;
	bool clip = fit & text_fit_clip;
	text_step_next_line(text, step);
	for (int32_t i=0; i<text_length; i++) {
		if (!isspace(text[i])) {
			font_char_t &char_info = step.style->font->characters[(int)text[i]];
			if (clip)
				text_add_quad_clipped(step.pos.x, step.pos.y, off_z, bounds_min, step.start, char_info, *step.style, buffer, tr, normal);
			else
				text_add_quad(step.pos.x, step.pos.y, off_z, char_info, *step.style, buffer, tr, normal);
		}
		text_step_position(text[i], &text[i], step);
	}
}

///////////////////////////////////////////

void text_update() {
	for (size_t i = 0; i < text_buffers.count; i++) {
		text_buffer_t &buffer = text_buffers[i];
		if (buffer.vert_count <= 0)
			continue;

		mesh_set_verts(buffer.mesh, buffer.verts, buffer.vert_count, false);
		mesh_set_draw_inds(buffer.mesh, (buffer.vert_count / 4) * 6);

		render_add_mesh(buffer.mesh, buffer.material, matrix_identity);
		buffer.vert_count = 0;
	}
}

///////////////////////////////////////////

void text_shutdown() {
	for (size_t i = 0; i < text_buffers.count; i++) {
		text_buffer_t &buffer = text_buffers[i];
		mesh_release(buffer.mesh);
		font_release(buffer.font);
		material_release(buffer.material);
		free(buffer.verts);
	}

	text_styles .free();
	text_buffers.free();
}

} // namespace sk