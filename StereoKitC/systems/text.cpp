#include "text.h"

#include "../asset_types/material.h"
#include "../asset_types/assets.h"
#include "../systems/defaults.h"
#include "../hierarchy.h"
#include "../math.h"

#include <vector>
using namespace std;

#include <directxmath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

vector<_text_style_t> text_styles;
vector<text_buffer_t> text_buffers;

///////////////////////////////////////////

void text_buffer_ensure_capacity(text_buffer_t &buffer, size_t characters) {
	if (buffer.vert_count + characters * 4 <= buffer.vert_cap)
		return;

	buffer.vert_cap = buffer.vert_count + (int)characters * 4;
	buffer.verts    = (vert_t *)realloc(buffer.verts, sizeof(vert_t) * buffer.vert_cap);

	// regenerate indices
	vind_t  quads = (vind_t)(buffer.vert_cap / 4);
	vind_t *inds  = (vind_t *)malloc(quads * 6 * sizeof(vind_t));
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
	for (size_t i = 0; i < text_buffers.size(); i++) {
		if (text_buffers[i].id == id) {
			buffer = &text_buffers[i];
			index  = i;
			break;
		}
	}
	if (buffer == nullptr) {
		text_buffers.push_back({});
		index  = text_buffers.size() - 1;
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
	style.line_spacing    = font->character_height * 0.25f;
	text_styles.push_back(style);

	return (text_style_t)(text_styles.size() - 1);
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
	float curr_width = 0;
	float last_width = 0;
	const char *last_at = start;
	const char *ch = start;

	while (true) {
		char curr = *ch;
		// We prefer to line break at spaces, rather than in the middle of words
		if (isspace(curr) || curr == '\0') {
			last_width = curr_width;
			last_at = ch;
		}
		// End of line or string?
		if (curr == '\0' || curr == '\n')
			break;

		// Advance by character width
		font_char_t &char_info = step.style->font->characters[curr];
		float next_width = char_info.xadvance*step.style->size + curr_width;

		// Check if it steps out of bounds
		if (next_width > step.bounds.x) {
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
		ch++;
	}

	if (out_char_count != nullptr)
		*out_char_count = last_at - start;
	return last_width;
}

///

float text_step_height(const char *text, int32_t *out_length, const text_stepper_t &step) {
	int32_t     count  = 1;
	const char *curr   = text;
	float       height = 0;
	while (count > 0) {
		text_step_line_length(curr, &count, step);
		curr   += count;
		height += 1;
	}

	if (out_length != nullptr)
		*out_length = curr - text;
	return height * step.style->font->character_height * step.style->size;
}

///

void text_step_next_line(const char *start, text_stepper_t &step) {
	float line_size  = text_step_line_length(start, &step.line_remaining, step);
	float align_x    = 0;
	if (step.align & text_align_x_center) align_x = ((step.bounds.x - line_size) / 2.f);
	if (step.align & text_align_x_right)  align_x =  (step.bounds.x - line_size);
	step.pos.x  = step.start.x - align_x;
	step.pos.y -= step.style->size * (step.style->font->character_height+step.style->line_spacing);
}

void text_step_position(char ch, const char *start, text_stepper_t &step) {
	font_char_t &char_info = step.style->font->characters[ch];
	step.line_remaining--;
	if (step.line_remaining <= 0)
		text_step_next_line(start, step);

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

void text_add_at(const char* text, const matrix &transform, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z) {
	XMMATRIX tr;
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.back().transform, tr);
	} else {
		math_matrix_to_fast(transform, &tr);
	}

	text_style_t   style_id    = style == -1 ? 0 : style;
	_text_style_t &style_data  = text_styles [style_id];
	text_buffer_t &buffer      = text_buffers[style_data.buffer_index];
	font_t         font        = style_data.font;
	vec2           raw_size    = text_size(text, style_id);
	float          char_height = font->character_height;

	// Resize array if we need more room for this text
	size_t length = strlen(text);
	text_buffer_ensure_capacity(buffer, length);
	
	vec3    normal  = matrix_mul_direction(tr, vec3_forward);
	const char *curr = text;
	vec2    line_size = text_line_size(style_id, curr);
	float   start_x = off_x;
	float   start_y = off_y - char_height * style_data.size;
	if (position & text_align_y_center) start_y += (raw_size.y / 2.f);
	if (position & text_align_y_bottom) start_y += raw_size.y;
	if (position & text_align_x_center) start_x += raw_size.x / 2.f;
	if (position & text_align_x_right)  start_x += raw_size.x;
	float align_x = 0;
	if (align & text_align_x_center) align_x = ((raw_size.x - line_size.x) / 2.f);
	if (align & text_align_x_right)  align_x = (raw_size.x - line_size.x);
	float  x = start_x - align_x;
	float  y = start_y;

	while (*curr != '\0') {
		curr += 1;
		font_char_t &char_info = style_data.font->characters[(int)*curr];

		// Do spacing for whitespace characters
		switch (*curr) {
		case '\t': x -= style_data.font->characters[(int)' '].xadvance * 4 * style_data.size; continue;
		case ' ':  x -= char_info.xadvance * style_data.size; continue;
		case '\n': {
			line_size = text_line_size(style_id, curr);
			align_x = 0;
			if (align & text_align_x_center) align_x = ((raw_size.x - line_size.x) / 2.f);
			if (align & text_align_x_right)  align_x = (raw_size.x - line_size.x);
			x  = start_x - align_x;
			y -= style_data.size * (char_height+style_data.line_spacing);
		} continue;
		default:break;
		}
		
		// Add a character quad
		text_add_quad(x, y, off_z, char_info, style_data, buffer, tr, normal);

		x -= char_info.xadvance * style_data.size;
	}
}

///////////////////////////////////////////

void text_add_in(const char* text, const matrix& transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z) {
	XMMATRIX tr;
	if (hierarchy_enabled) {
		matrix_mul(transform, hierarchy_stack.back().transform, tr);
	} else {
		math_matrix_to_fast(transform, &tr);
	}
	vec3 normal = matrix_mul_direction(tr, vec3_forward);

	// Find the initial stepping information for this chunk of text
	text_stepper_t step;
	step.align  = align;
	step.style  = &text_styles[style == -1 ? 0 : style];
	step.bounds = size;
	step.start  = { off_x, off_y };
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

	line_add({ step.start.x, step.start.y,                 off_z }, { step.start.x - step.bounds.x, step.start.y, off_z }, { 255,0,255,255 }, { 255,0,255,255 }, 0.002f);
	line_add({ step.start.x, step.start.y - step.bounds.y, off_z }, { step.start.x - step.bounds.x, step.start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);
	line_add({ step.start.x, step.start.y,                 off_z }, { step.start.x,                 step.start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);
	line_add({ step.start.x - step.bounds.x, step.start.y, off_z }, { step.start.x - step.bounds.x, step.start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);

	// Ensure text capacity
	text_buffer_t &buffer = text_buffers[step.style->buffer_index];
	text_buffer_ensure_capacity(buffer, text_length);

	// Core loop for drawing the text
	const char *curr = text;
	text_step_position('\n', text, step);
	for (int32_t i=0; i<text_length; i++) {
		if (!isspace(text[i])) {
			font_char_t &char_info = step.style->font->characters[text[i]];
			text_add_quad(step.pos.x, step.pos.y, off_z, char_info, *step.style, buffer, tr, normal);
		}
		text_step_position(text[i], &text[i], step);
	}
}

///////////////////////////////////////////

void text_update() {
	for (size_t i = 0; i < text_buffers.size(); i++) {
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
	for (size_t i = 0; i < text_buffers.size(); i++) {
		text_buffer_t &buffer = text_buffers[i];
		mesh_release(buffer.mesh);
		font_release(buffer.font);
		material_release(buffer.material);
		free(buffer.verts);
	}
	text_buffers.clear();
}

} // namespace sk