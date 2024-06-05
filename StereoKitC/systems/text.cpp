#include "text.h"
#include "../stereokit.h"
#include "../asset_types/font.h"
#include "../systems/defaults.h"
#include "../hierarchy.h"
#include "../sk_math_dx.h"
#include "../sk_math.h"
#include "../sk_memory.h"
#include "../libraries/array.h"
#include "../libraries/unicode.h"

#include <ctype.h>

#include <DirectXMath.h> // Matrix math functions and objects
using namespace DirectX;

namespace sk {

///////////////////////////////////////////

struct _text_style_t {
	font_t         font;
	uint32_t       buffer_index;
	color32        color;
	float          char_height;
	float          line_spacing;
};

struct text_buffer_t {
	font_t         font;
	material_t     material;
	mesh_t         mesh;
	vert_t        *verts; // TODO: potentially change this to array_t
	uint32_t       id;
	int32_t        vert_count;
	int32_t        vert_cap;
	bool32_t       dirty_inds;
};

struct text_stepper_t {
	_text_style_t *style;
	text_align_    align;
	int32_t        line_remaining;
	bool32_t       wrap;
	vec2           start;
	vec2           bounds;
	vec2           pos;
};

///////////////////////////////////////////

array_t<_text_style_t> text_styles  = {};
array_t<text_buffer_t> text_buffers = {};

/*template<typename C, bool (*char_decode_b_T)(const C*, const C**, char32_t*)>
float text_step_height(const C* text, int32_t* out_length, const text_stepper_t& step);
template<typename C, bool (*char_decode_b_T)(const C*, const C**, char32_t*)>
void text_step_next_line(const C* start, text_stepper_t& step);*/

///////////////////////////////////////////

inline bool text_is_space(char32_t c) {
	return c == ' ' || c=='\n' || c == '\r' || c== '\t';
}

//////////////////////////////////////////

inline bool text_is_breakable(char32_t c) {
	return c==' ' || c=='-' || c=='/' || c=='\\' || c=='|' || c=='\n' || c == '\r' || c== '\t';
}

//////////////////////////////////////////

void text_buffer_ensure_capacity(text_buffer_t &buffer, size_t characters) {
	if (buffer.vert_count + (int32_t)characters*4 <= buffer.vert_cap)
		return;

	buffer.vert_cap   = maxi(buffer.vert_count + (int)characters * 4, buffer.vert_cap * 2);
	buffer.verts      = sk_realloc_t(vert_t, buffer.verts, buffer.vert_cap);
	buffer.dirty_inds = true;
}

//////////////////////////////////////////

void text_buffer_check_dirty_inds(text_buffer_t& buffer) {
	if (!buffer.dirty_inds) return;
	buffer.dirty_inds = false;

	// regenerate indices
	vind_t  quads = (vind_t)(buffer.vert_cap / 4);
	vind_t *inds  = sk_malloc_t(vind_t, quads * 6);
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
	sk_free(inds);
}

///////////////////////////////////////////

text_style_t text_make_style(font_t font, float character_height, color128 color) {
	shader_t     shader   = shader_find        (default_id_shader_font);
	material_t   material = material_create    (shader);
	text_style_t result   = text_make_style_mat(font, character_height, material, color);

	// The style now references the material if creation was successful, so
	// we need to let go of things from here.
	material_release(material);
	shader_release  (shader);
	return result;
}

///////////////////////////////////////////

text_style_t text_make_style_shader(font_t font, float character_height, shader_t shader, color128 color) {
	material_t   material = material_create(shader);
	text_style_t result   = text_make_style_mat(font, character_height, material, color);

	// The style now references the material if creation was successful, so
	// we need to let go of it from here.
	material_release(material);
	return result;
}

///////////////////////////////////////////

text_style_t text_make_style_mat(font_t font, float character_height, material_t material, color128 color) {
	uint32_t       id     = (uint32_t)(font->header.id << 16 | ((asset_header_t*)material)->id);
	int32_t        index  = 0;
	text_buffer_t *buffer = nullptr;

	if (font == nullptr) {
		log_err("text_make_style was given a null font!");
	}
	
	// Find or make a buffer for this style
	for (int32_t i = 0; i < text_buffers.count; i++) {
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
		font_addref    (font);
		material_addref(material);

		tex_t font_tex = font_get_tex(font);
		material_set_texture     (material, "diffuse", font_tex);
		material_set_cull        (material, cull_none);
		material_set_transparency(material, transparency_blend);
		material_set_depth_test  (material, depth_test_less_or_eq);

		mesh_set_keep_data(buffer->mesh, false);

		tex_release(font_tex);
	}

	// Create the style
	_text_style_t style;
	style.font            = font;
	style.buffer_index    = (uint32_t)index;
	style.color           = color_to_32( color_to_linear( color ) );
	style.line_spacing    = 0.5f;
	style.char_height     = character_height;
	
	return (text_style_t)text_styles.add(style);
}

///////////////////////////////////////////

material_t text_style_get_material(text_style_t style) {
	material_t result = text_buffers[text_styles[style].buffer_index].material;
	material_addref(result);
	return result;
}

///////////////////////////////////////////

float text_style_get_char_height(text_style_t style) {
	return text_styles[style].char_height;
}

///////////////////////////////////////////

void text_style_set_char_height(text_style_t style, float height_meters) {
	text_styles[style].char_height = height_meters;
}

///////////////////////////////////////////

template<typename C, bool (*char_decode_b_T)(const C*, const C**, char32_t*)>
inline vec2 text_size_constrained_g(const C* text, text_style_t style_id, float max_width) {
	const _text_style_t* style = &text_styles[style_id];

	const C *curr           = text;
	int32_t  line_count     = 1;
	int32_t  line_remaining = 0;
	float    largest_width  = text_step_line_length<C, char_decode_b_T>(curr, &line_remaining, &curr, style, true, max_width);
	while (*curr != 0) {
		float curr_width = text_step_line_length<C, char_decode_b_T>(curr, &line_remaining, &curr, style, true, max_width);
		if (largest_width < curr_width)
			largest_width = curr_width;
		line_count += 1;
	}

	return {line_count > 1 ? max_width : largest_width, (line_count + (line_count - 1) * style->line_spacing) * style->char_height };
}

vec2 text_size_constrained   (const char     *text_utf8,  text_style_t style, float max_width) { return text_size_constrained_g<char,     utf8_decode_fast_b >(text_utf8,  style, max_width); }
vec2 text_size_constrained_16(const char16_t* text_utf16, text_style_t style, float max_width) { return text_size_constrained_g<char16_t, utf16_decode_fast_b>(text_utf16, style, max_width); }

///////////////////////////////////////////

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
inline vec2 text_size_g(const C *text, text_style_t style) {
	if (text == nullptr) return {};

	font_t      font  = text_styles[style].font;
	char32_t    curr  = 0;
	float       x     = 0;
	int         y     = 1;
	float       max_x = 0;
	while (char_decode_b_T(text, &text, &curr)) {
		const font_char_t *ch = font_get_glyph(font, curr);

		// Do spacing for whitespace characters
		if (curr == '\n') {
			if (x > max_x) max_x = x; 
			x  = 0;
			y += 1;
		} else x += ch->xadvance * text_styles[style].char_height;
	}
	if (x > max_x) max_x = x;
	return vec2{ max_x, (y + (y - 1) * text_styles[style].line_spacing) * text_styles[style].char_height };
}

vec2 text_size   (const char     *text_utf8,  text_style_t style) { return text_size_g<char,     utf8_decode_fast_b >(text_utf8,  style); }
vec2 text_size_16(const char16_t *text_utf16, text_style_t style) { return text_size_g<char16_t, utf16_decode_fast_b>(text_utf16, style); }

///////////////////////////////////////////

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
inline vec2 text_char_at_g(const C *text, text_style_t style, int32_t char_index, vec2 *opt_size, text_fit_ fit, text_align_ position, text_align_ align) {
	if (text == nullptr) return {};
	vec2 size = opt_size == nullptr
		? text_size_g<C, char_decode_b_T>(text, style)
		: *opt_size;

	// Find the initial stepping information for this chunk of text
	text_stepper_t step;
	step.line_remaining = 0;
	step.align          = align;
	step.wrap           = fit & text_fit_wrap;
	step.style          = &text_styles[style];
	step.bounds         = size;
	step.start          = {0, step.style->char_height};

	// Ensure scale is right for our fit
	if (fit & (text_fit_squeeze | text_fit_exact)) {
		vec2 txt_size = text_size_g<C, char_decode_b_T>(text, style);
		vec2 scale_xy = {
			size.x / txt_size.x,
			size.y / txt_size.y };
		float scale = fminf(scale_xy.x, scale_xy.y)*0.999f;
		if (fit & text_fit_squeeze)
			scale = fminf(1, scale);

		// Apply the scale to the transform matrix
		//XMMATRIX scale_m = XMMatrixScaling(scale, scale, 1);
		//tr = XMMatrixMultiply(scale_m, tr);
		step.bounds = step.bounds / scale;
	}

	// Calculate the strlen and text height for verical centering
	int32_t text_length = 0;
	float   text_height = text_step_height<C, char_decode_b_T>(text, &text_length, step.style, step.wrap, step.bounds.x);
	// if the size is still zero, then lets use the calculated height
	if (step.bounds.y == 0)
		step.bounds.y = text_height;

	// Align the start based on the size of the bounds
	if      (position & text_align_x_center) step.start.x += step.bounds.x / 2.f;
	else if (position & text_align_x_right)  step.start.x += step.bounds.x;
	if      (position & text_align_y_center) step.start.y += step.bounds.y / 2.f;
	else if (position & text_align_y_bottom) step.start.y += step.bounds.y;
	step.pos = step.start;

	// Figure out the vertical align of the text
	if      (align & text_align_y_center) step.pos.y -= (step.bounds.y-text_height) / 2.f;
	else if (align & text_align_y_bottom) step.pos.y -=  step.bounds.y-text_height;

	// Core loop for drawing the text
	vec2     bounds_min = step.start - step.bounds;
	char32_t c          = 0;
	int32_t  count      = 0;
	if (*text != '\0')
		text_step_next_line<C, char_decode_b_T>(text, step.style, step.align, step.wrap, step.bounds.x, step.start.x, &step.line_remaining, &step.pos);
	while(char_decode_b_T(text, &text, &c)) {
		if (count == char_index) {
			return step.pos;
		}
		const font_char_t* char_info = font_get_glyph(step.style->font, c);
		
		step.line_remaining--;
		if (step.line_remaining <= 0 && *text != '\0') {
			text_step_next_line<C, char_decode_b_T>(text, step.style, step.align, step.wrap, step.bounds.x, step.start.x, &step.line_remaining, &step.pos);
			step.pos.y -= step.style->char_height * step.style->line_spacing;
		} else if (c != '\n') {
			step.pos.x -= char_info->xadvance * step.style->char_height;
		}
		
		count += 1;
	}
	return step.pos;
}

vec2 text_char_at   (const char*     text_utf8,  text_style_t style, int32_t char_index, vec2 *opt_size, text_fit_ fit, text_align_ position, text_align_ align) { return text_char_at_g<char,     utf8_decode_fast_b >(text_utf8,  style, char_index, opt_size, fit, position, align); }
vec2 text_char_at_16(const char16_t* text_utf16, text_style_t style, int32_t char_index, vec2 *opt_size, text_fit_ fit, text_align_ position, text_align_ align) { return text_char_at_g<char16_t, utf16_decode_fast_b>(text_utf16, style, char_index, opt_size, fit, position, align); }

///////////////////////////////////////////

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
float text_step_line_length(const C *start, int32_t *out_char_count, const C **out_next_start, const _text_style_t *style, bool wrap, float max_width) {
	// If we're not wrapping, this is really simple
	if (wrap == false) {
		const C* curr  = start;
		char32_t ch    = 0;
		float    width = 0;
		int32_t  count = 0;
		while (char_decode_b_T(curr, &curr, &ch) && ch != '\n') {
			width += font_get_glyph(style->font, ch)->xadvance;
			count++;
		}
		*out_char_count = ch == '\n' ? count + 1 : count;
		*out_next_start = curr;
		return width * style->char_height;
	}

	// If we _are_ wrapping, we gotta do it the tricky way
	float    curr_width = 0;
	float    last_width = 0;
	int32_t  last_count = 0;
	const C *last_at    = start;
	const C *ch         = start;
	char32_t curr       = 0;
	bool     was_break  = false;
	int32_t  count      = 0;

	while (true) {
		const C *next_char = start;
		char_decode_b_T(ch, &next_char, &curr);
		bool is_break = text_is_breakable(curr);

		// We prefer to line break at spaces and other breakable characters, 
		// rather than in the middle of words
		if (is_break || curr == '\0') {
			if (!was_break)
				last_width = curr_width;
			last_at    = curr != '\0' ? next_char : ch;
			last_count = curr != '\0' ? count + 1 : count;
		}
		// End of line or string?
		if (curr == '\0' || curr == '\n')
			break;

		// Advance by character width
		const font_char_t *char_info  = font_get_glyph(style->font, curr);
		float              next_width = char_info->xadvance*style->char_height + curr_width;

		// Check if it steps out of bounds
		if (!is_break && next_width > max_width) {
			// If there were no breaks in this line, set to the previous character
			if (last_width == 0) {
				last_width = curr_width;
				last_at    = ch;
				last_count = count;
			}
			// Exit the line
			break;
		}

		// Next character!
		curr_width = next_width;
		was_break  = is_break;
		ch         = next_char;
		count     += 1;
	}

	*out_char_count = last_count;
	*out_next_start = last_at;
	return last_width;
}

///////////////////////////////////////////

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
float text_step_height(const C *text, int32_t *out_length, const _text_style_t *style, bool wrap, float max_width) {
	int32_t  count  = 1;
	int32_t  length = 0;
	const C *curr   = text;
	float    height = 0;
	while (count > 0) {
		text_step_line_length<C, char_decode_b_T>(curr, &count, &curr, style, wrap, max_width);
		length += count;
		if (count > 0)
			height += 1;
	}

	*out_length = length;
	return (height + (height-1)*style->line_spacing) * style->char_height;
}

///

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
void text_step_next_line(const C *start, const _text_style_t* style, text_align_ align, bool wrap, float max_width, float start_x, int32_t *out_line_remaining, vec2 *ref_pos) {
	const C *next;
	float line_size  = text_step_line_length<C, char_decode_b_T>(start, out_line_remaining, &next, style, wrap, max_width);
	float align_x    = 0;
	if (align & text_align_x_center) align_x = ((max_width - line_size) / 2.f);
	if (align & text_align_x_right)  align_x =  (max_width - line_size);
	ref_pos->x  = start_x - align_x;
	ref_pos->y -= style->char_height;
}

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
void text_step_position(char32_t ch, const font_char_t *char_info, const C *next, const _text_style_t* style, text_align_ align, bool wrap, float max_width, float start_x, int32_t* ref_line_remaining, vec2* ref_pos) {
	*ref_line_remaining = *ref_line_remaining - 1;
	if (*ref_line_remaining <= 0) {
		text_step_next_line<C, char_decode_b_T>(next, style, align, wrap, max_width, start_x, ref_line_remaining, ref_pos);
		ref_pos->y -= style->char_height * style->line_spacing;
		return;
	}

	if (ch != '\n'){
		ref_pos->x -= char_info->xadvance*style->char_height;
	}
}

///

void text_add_quad(float x, float y, float off_z, const font_char_t *char_info, _text_style_t &style_data, color32 color, text_buffer_t &buffer, vec3 base, vec3 normal, vec3 up, vec3 right) {
	base -= normal * off_z;
	vec3 x_min = base + (x - char_info->x0 * style_data.char_height) * right;
	vec3 x_max = base + (x - char_info->x1 * style_data.char_height) * right;
	vec3 y_min =        (y + char_info->y0 * style_data.char_height) * up;
	vec3 y_max =        (y + char_info->y1 * style_data.char_height) * up;

	buffer.verts[buffer.vert_count++] = { x_min + y_min, normal, vec2{ char_info->u0, char_info->v0 }, color };
	buffer.verts[buffer.vert_count++] = { x_max + y_min, normal, vec2{ char_info->u1, char_info->v0 }, color };
	buffer.verts[buffer.vert_count++] = { x_max + y_max, normal, vec2{ char_info->u1, char_info->v1 }, color };
	buffer.verts[buffer.vert_count++] = { x_min + y_max, normal, vec2{ char_info->u0, char_info->v1 }, color };
}

///

void text_add_quad_clipped(float x, float y, float off_z, vec2 bounds_min, vec2 bounds_max, const font_char_t *char_info, _text_style_t &style_data, color32 color, text_buffer_t &buffer, const XMMATRIX &tr, vec3 normal, vec3 up, vec3 right) {
	float x_max = x - char_info->x0 * style_data.char_height;
	float x_min = x - char_info->x1 * style_data.char_height;
	float y_max = y + char_info->y0 * style_data.char_height;
	float y_min = y + char_info->y1 * style_data.char_height;

	// Ditch out if it's completely out of bounds
	if (x_min > bounds_max.x ||
		x_max < bounds_min.x ||
		y_min > bounds_max.y ||
		y_max < bounds_min.y)
		return;

	float u_max = char_info->u0;
	float u_min = char_info->u1;
	float v_max = char_info->v0;
	float v_min = char_info->v1;

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

	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_max, y_max, off_z }), normal, vec2{ u_max, v_max }, color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_min, y_max, off_z }), normal, vec2{ u_min, v_max }, color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_min, y_min, off_z }), normal, vec2{ u_min, v_min }, color };
	buffer.verts[buffer.vert_count++] = { matrix_mul_point(tr, vec3{ x_max, y_min, off_z }), normal, vec2{ u_max, v_min }, color };
}

///////////////////////////////////////////

void text_add_at(const char* text, const matrix &transform, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear) {
	text_add_in(text, transform, text_size(text, style), text_fit_exact, style, position, align, off_x, off_y, off_z, vertex_tint_linear);
}

///////////////////////////////////////////

void text_add_at_16(const char16_t* text, const matrix &transform, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear) {
	text_add_in_16(text, transform, text_size_16(text, style), text_fit_exact, style, position, align, off_x, off_y, off_z, vertex_tint_linear);
}

///////////////////////////////////////////

template<typename C, bool (*char_decode_b_T)(const C *, const C **, char32_t *)>
float text_add_in_g(const C* text, const matrix& transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear) {
	if (text == nullptr) return 0;
	if (size.x <= 0) return 0; // Zero width text isn't visible, and causes issues when trying to determine text height.

	XMMATRIX tr;
	if (hierarchy_use_top()) {
		matrix_mul(transform, hierarchy_top(), tr);
	} else {
		math_matrix_to_fast(transform, &tr);
	}
	vec3 normal = matrix_mul_direction(tr, vec3_forward);

	// Find the initial stepping information for this chunk of text
	text_stepper_t step;
	step.line_remaining = 0;
	step.align  = align;
	step.wrap   = fit & text_fit_wrap;
	step.style  = &text_styles[style];
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
	line_add({ dbg_start.x - step.bounds.x, dbg_start.y, off_z }, { dbg_start.x - step.bounds.x, dbg_start.y - step.bounds.y, off_z }, { 255,255,255,255 }, { 255,255,255,255 }, 0.002f);*/

	// Ensure scale is right for our fit
	if (fit & (text_fit_squeeze | text_fit_exact)) {
		vec2 txt_size = text_size_g<C, char_decode_b_T>(text, style);
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

	vec3 up    = matrix_mul_direction(tr, vec3_up);
	vec3 right = matrix_mul_direction(tr, vec3_right);
	vec3 base  = matrix_mul_point    (tr, vec3_zero);

	// Calculate the strlen and text height for verical centering
	int32_t text_length = 0;
	float   text_height = text_step_height<C, char_decode_b_T>(text, &text_length, step.style, step.wrap, step.bounds.x);
	// if the size is still zero, then lets use the calculated height
	if (step.bounds.y == 0)
		step.bounds.y = text_height;

	// Align the start based on the size of the bounds
	if      (position & text_align_x_center) step.start.x += step.bounds.x / 2.f;
	else if (position & text_align_x_right)  step.start.x += step.bounds.x;
	if      (position & text_align_y_center) step.start.y += step.bounds.y / 2.f;
	else if (position & text_align_y_bottom) step.start.y += step.bounds.y;
	step.pos = step.start;

	// Figure out the vertical align of the text
	if      (align & text_align_y_center) step.pos.y -= (step.bounds.y-text_height) / 2.f;
	else if (align & text_align_y_bottom) step.pos.y -=  step.bounds.y-text_height;

	// Ensure text capacity
	text_buffer_t &buffer = text_buffers[step.style->buffer_index];
	text_buffer_ensure_capacity(buffer, text_length);

	// Get the final color
	color32 color = color_to_32( color32_to_128(step.style->color) * vertex_tint_linear );

	// Core loop for drawing the text
	vec2     bounds_min = step.start - step.bounds;
	bool     clip       = fit & text_fit_clip;
	char32_t c          = 0;
	text_step_next_line<C, char_decode_b_T>(text, step.style, step.align, step.wrap, step.bounds.x, step.start.x, &step.line_remaining, &step.pos);
	if (clip) {
		while(char_decode_b_T(text, &text, &c)) {
			const font_char_t *char_info = font_get_glyph(step.style->font, c);
			if (!text_is_space(c)) {
				text_add_quad_clipped(step.pos.x, step.pos.y, off_z, bounds_min, step.start, char_info, *step.style, color, buffer, tr, normal, up, right);
			}
			text_step_position<C, char_decode_b_T>(c, char_info, text, step.style, step.align, step.wrap, step.bounds.x, step.start.x, &step.line_remaining, &step.pos);
		}
	} else {
		while (char_decode_b_T(text, &text, &c)) {
			const font_char_t* char_info = font_get_glyph(step.style->font, c);
			if (!text_is_space(c)) {
				text_add_quad(step.pos.x, step.pos.y, off_z, char_info, *step.style, color, buffer, base, normal, up, right);
			}
			text_step_position<C, char_decode_b_T>(c, char_info, text, step.style, step.align, step.wrap, step.bounds.x, step.start.x, &step.line_remaining, &step.pos);
		}
	}
	return (step.start.y - step.pos.y) - step.style->char_height;
}


float text_add_in(const char *text, const matrix &transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear) {
	return text_add_in_g<char, utf8_decode_fast_b>(text, transform, size, fit, style, position, align, off_x, off_y, off_z, vertex_tint_linear);
}
float text_add_in_16(const char16_t *text, const matrix &transform, vec2 size, text_fit_ fit, text_style_t style, text_align_ position, text_align_ align, float off_x, float off_y, float off_z, color128 vertex_tint_linear) {
	return text_add_in_g<char16_t, utf16_decode_fast_b>(text, transform, size, fit, style, position, align, off_x, off_y, off_z, vertex_tint_linear);
}

///////////////////////////////////////////

void text_step() {
	font_update_fonts();

	for (int32_t i = 0; i < text_buffers.count; i++) {
		text_buffer_t &buffer = text_buffers[i];
		if (buffer.vert_count <= 0)
			continue;

		text_buffer_check_dirty_inds(buffer);

		mesh_set_verts    (buffer.mesh, buffer.verts, buffer.vert_count, false);
		mesh_set_draw_inds(buffer.mesh, (buffer.vert_count / 4) * 6);

		render_add_mesh(buffer.mesh, buffer.material, matrix_identity);
		buffer.vert_count = 0;
	}
}

///////////////////////////////////////////

void text_shutdown() {
	for (int32_t i = 0; i < text_buffers.count; i++) {
		text_buffer_t &buffer = text_buffers[i];
		mesh_release(buffer.mesh);
		font_release(buffer.font);
		material_release(buffer.material);
		sk_free(buffer.verts);
	}

	text_styles .free();
	text_buffers.free();
}

} // namespace sk