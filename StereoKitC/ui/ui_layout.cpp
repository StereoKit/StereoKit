#include "ui_layout.h"

#include "../libraries/array.h"

///////////////////////////////////////////

namespace sk {

array_t<ui_layout_t> skui_layouts  = {};
ui_settings_t        skui_settings = {};
bounds_t             skui_recent_layout;

///////////////////////////////////////////

inline bool ui_layout_is_auto_width (const ui_layout_t* layout) { return layout->size.x == 0; }
inline bool ui_layout_is_auto_height(const ui_layout_t* layout) { return layout->size.y == 0; }

///////////////////////////////////////////

void ui_layout_area(vec3 start, vec2 dimensions, bool32_t add_margin) {
	if (add_margin) {
		start      -= vec3{ skui_settings.margin, skui_settings.margin, 0 };
		dimensions -= vec2{ skui_settings.margin, skui_settings.margin } * 2;
	}
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset_initial   = start;
	layout->offset           = start;
	layout->size             = dimensions;
	layout->size_used        = {};
	layout->line             = {};
	layout->offset_prev      = layout->offset;
	layout->line_prev        = {};
	layout->line_pad         = 0;
}

///////////////////////////////////////////

vec2 ui_layout_remaining() {
	ui_layout_t *layout = &skui_layouts.last();
	float size_x = ui_layout_is_auto_width(layout)
		? (layout->window ? (layout->window->prev_size.x - skui_settings.margin * 2) : layout->size_used.x)
		: layout->size.x;
	float size_y = ui_layout_is_auto_height(layout)
		? (layout->window ? (layout->window->prev_size.y - skui_settings.margin * 2) : 0)
		: layout->size.y;
	return vec2 {
		fmaxf(0, size_x - (layout->offset_initial.x - layout->offset.x)),
		fmaxf(0, size_y - (layout->offset_initial.y - layout->offset.y))
	};
}

///////////////////////////////////////////

vec2 ui_area_remaining() {
	return ui_layout_remaining();
}

///////////////////////////////////////////

vec3 ui_layout_at() {
	return skui_layouts.last().offset;
}

///////////////////////////////////////////

bounds_t ui_layout_last() {
	return skui_recent_layout;
}

///////////////////////////////////////////

void ui_layout_reserve_vertical_sz(vec2 size, bool32_t add_padding, vec3* out_position, vec2* out_size) {
	if (size.x == 0) size.x = ui_line_height();
	if (size.y == 0) size.y = ui_layout_remaining().y - (add_padding ? skui_settings.padding * 2 : 0);
	ui_layout_reserve_sz(size, add_padding, out_position, out_size);
}

///////////////////////////////////////////

void ui_layout_reserve_sz(vec2 size, bool32_t add_padding, vec3 *out_position, vec2 *out_size) {
	if (size.x == 0) size.x = ui_layout_remaining().x - (add_padding ? skui_settings.padding*2 : 0);
	if (size.y == 0) size.y = ui_line_height();

	ui_layout_t *layout = &skui_layouts.last();

	vec3 final_pos  = layout->offset;
	vec2 final_size = add_padding
		? size + vec2{ skui_settings.padding, skui_settings.padding }*2
		: size;

	// If this is not the first element, and it goes outside the active window,
	// then we'll want to start this element on the next line down
	// 
	// Floating point rounding can cause this to occasionally round past the
	// end of the layout, so 0.0001f is meant to forgive a bit of rounding.
	if (final_pos.x    != layout->offset_initial.x &&
		layout->size.x != 0                        &&
		final_pos.x - final_size.x < layout->offset_initial.x - (layout->size.x + 0.0001f))
	{
		ui_nextline();
		final_pos = layout->offset;
	}

	// Track the sizes for this line, for ui_layout_remaining, as well as
	// window auto-sizing.
	layout->line.x += final_size.x;
	if (layout->line.y < final_size.y)
		layout->line.y = final_size.y;
	if (layout->size_used.x < layout->line.x)
		layout->size_used.x = layout->line.x;
	if (layout->size_used.y < (layout->offset_initial.y-layout->offset.y)+layout->line.y)
		layout->size_used.y = (layout->offset_initial.y-layout->offset.y)+layout->line.y;
	layout->line.x += skui_settings.gutter;

	// Advance the UI layout position
	layout->offset -= vec3{ final_size.x + skui_settings.gutter, 0, 0 };

	ui_nextline();

	// Store this as the most recent layout
	skui_recent_layout = {
		final_pos - vec3{final_size.x/2, final_size.y/2, 0},
		{final_size.x, final_size.y, 0}
	};

	*out_position = skui_recent_layout.center + skui_recent_layout.dimensions/2;
	*out_size     = final_size;
}

///////////////////////////////////////////

bounds_t ui_layout_reserve(vec2 size, bool32_t add_padding, float depth) {
	vec3 final_pos;
	vec2 final_size;
	ui_layout_reserve_sz(size, add_padding, &final_pos, &final_size);

	bounds_t result = skui_recent_layout;
	result.center.z -= depth/2;
	result.dimensions.z = depth;
	return result;
}

///////////////////////////////////////////

void ui_layout_push(vec3 start, vec2 dimensions, bool32_t add_margin) {
	if (add_margin) {
		start -= vec3{ skui_settings.margin, skui_settings.margin, 0 };
		if (dimensions.x != 0) dimensions.x -= skui_settings.margin * 2;
		if (dimensions.y != 0) dimensions.y -= skui_settings.margin * 2;
	}
	ui_layout_t layout = {};
	layout.offset_initial   = start;
	layout.offset_prev      = start;
	layout.offset           = start;
	layout.size             = dimensions;
	layout.size_used        = {};
	layout.line             = {};
	layout.line_prev        = {};
	skui_layouts.add(layout);
}

///////////////////////////////////////////

void ui_layout_push_cut(ui_cut_ cut_to, float size, bool32_t add_margin) {
	ui_layout_t* curr = &skui_layouts.last();
	if      (cut_to == ui_cut_bottom && ui_layout_is_auto_height(curr)) log_warn("Can't cut bottom for layout with a height of 0!");
	else if (cut_to == ui_cut_right  && ui_layout_is_auto_width (curr)) log_warn("Can't cut right for layout with an width of 0!");

	vec3 cut_start   = {};
	vec2 cut_size    = {};
	vec3 curr_offset = {};
	switch (cut_to) {
	case ui_cut_left:
		curr_offset   = {-(size+skui_settings.gutter), 0, 0 };
		cut_start     = curr->offset_initial;
		cut_size      = { size, curr->size.y };
		curr->size.x -= size+skui_settings.gutter;
		break;
	case ui_cut_right:
		curr_offset   = {0, 0, 0 };
		cut_start     = curr->offset_initial - vec3{curr->size.x-size, 0, 0};
		cut_size      = { size, curr->size.y };
		curr->size.x -= size+skui_settings.gutter;
		break;
	case ui_cut_top:
		curr_offset   = {0, -(size+skui_settings.gutter), 0 };
		cut_start     = curr->offset_initial;
		cut_size      = { curr->size.x, size };
		curr->size.y -= size+skui_settings.gutter;
		break;
	case ui_cut_bottom:
		curr_offset   = {0, 0, 0 };
		cut_start     = curr->offset_initial - vec3{0,curr->size.y-size,0};
		cut_size      = { curr->size.x, size };
		curr->size.y -= size+skui_settings.gutter;
		break;
	}
	curr->offset         += curr_offset;
	curr->offset_initial += curr_offset;
	curr->offset_prev    += curr_offset;
	ui_layout_push(cut_start, cut_size, add_margin);
	//skui_layouts.last().window = curr->window;
}

///////////////////////////////////////////

void ui_layout_push_win(ui_window_t *window, vec3 start, vec2 dimensions, bool32_t add_margin) {
	ui_layout_push(start, dimensions, add_margin);
	skui_layouts.last().window = window;
	window->layout_start = start;
	window->layout_size  = dimensions;
	
	window->curr_size = {};
	if (dimensions.x != 0) window->curr_size.x = dimensions.x;
	if (dimensions.y != 0) window->curr_size.y = dimensions.y;
}

///////////////////////////////////////////
#include <stdio.h>
void ui_layout_pop() {
	ui_layout_t* layout = &skui_layouts.last();

	// Move to next line if we're still on a previous line
	if (layout->offset.x != layout->offset_initial.x)
		ui_nextline();

	if (layout->window) {
		if (layout->window->layout_size.x == 0)
			layout->window->curr_size.x = fmaxf(
				layout->size_used.x + skui_settings.margin * 2,
				layout->window->curr_size.x);
		if (layout->window->layout_size.y == 0)
			layout->window->curr_size.y = fmaxf(
				layout->size_used.y + skui_settings.margin * 2,
				layout->window->curr_size.y);
	}

	if (layout->size_used.x != 0 || layout->size_used.y != 0) {
		float   z_offset = 0.001f;
		color32 col      = { 0,255,255,255 };
		float   weight   = 0.001f;

		vec3 start = layout->offset_initial + vec3{ skui_settings.margin, skui_settings.margin };
		vec2 size  = layout->window
			? layout->window->curr_size
			: vec2{ layout->size_used.x + skui_settings.margin * 2, layout->size_used.y + skui_settings.margin * 2 };
		line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ size.x, 0, z_offset },      col, col, weight);
		line_add(start - vec3{ 0, 0, z_offset },      start - vec3{ 0, size.y, z_offset },      col, col, weight);
		line_add(start - vec3{ size.x, 0, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);
		line_add(start - vec3{ 0, size.y, z_offset }, start - vec3{ size.x, size.y, z_offset }, col, col, weight);
		line_add_axis({ start, quat_identity }, 0.04f);

		char text[256];
		snprintf(text, 256, "%s\n%.2gx%.2g", layout->window?"win":"layout", size.x * 100, size.y * 100);
		text_add_at(text, matrix_t(start - vec3{ size.x, 0, z_offset }), 0, text_align_top_left, text_align_top_left);
	}
	skui_layouts.pop();
}

///////////////////////////////////////////

ui_window_t* ui_layout_curr_window() {
	return skui_layouts.last().window;
}

///////////////////////////////////////////

ui_layout_t* ui_layout_curr() {
	return &skui_layouts.last();
}

///////////////////////////////////////////

void ui_nextline() {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset_prev = layout->offset;
	layout->line_prev   = layout->line;

	layout->offset.x    = layout->offset_initial.x - layout->line_pad;
	layout->offset.y   -= layout->line.y + skui_settings.gutter;
	layout->line        = {};
}

///////////////////////////////////////////

void ui_sameline() {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset      = layout->offset_prev;
	layout->line        = layout->line_prev;
}

///////////////////////////////////////////

void ui_space(float space) {
	ui_layout_t *layout = &skui_layouts.last();
	if (layout->offset.x == layout->offset_initial.x)
		layout->offset.y -= space;
	else
		layout->offset.x -= space;
}


///////////////////////////////////////////

struct panel_stack_data_t {
	vec3    at;
	float   original_line_start;
	ui_pad_ padding;
};
array_t<panel_stack_data_t> skui_panel_stack = {};
void ui_panel_begin(ui_pad_ padding) {
	ui_layout_t* layout = &skui_layouts.last();
	panel_stack_data_t data;
	data.at      = layout->offset;
	data.padding = padding;
	data.original_line_start = layout->line_pad;

	skui_panel_stack.add(data);
	/*layout->offset_initial.z -= skui_settings.depth * 0.1f;
	layout->offset.z          = layout->offset_initial.z;

	if (padding == ui_pad_inside) {
		float gutter = skui_settings.gutter / 2;
		layout->offset_initial.x -= gutter;
		layout->offset_initial.y -= gutter;
		layout->offset.x -= gutter;
		layout->offset.y -= gutter;
		layout->line.x += skui_settings.gutter;
	}

	layout->line_pad = layout->offset.x + layout->offset_initial.x + skui_settings.margin;*/

	vec2 size = {
		layout->size.x != 0 ? layout->size.x - (layout->offset_initial.x - layout->offset.x) - skui_settings.margin*2 : 0,
		layout->size.y != 0 ? layout->size.y - (layout->offset_initial.y - layout->offset.y) - skui_settings.margin*2 : 0 };
	ui_layout_push(layout->offset - vec3{0,0,skui_settings.depth * 0.1f}, size, true);
}

///////////////////////////////////////////

void ui_panel_end() {
	/*ui_sameline();

	float              gutter = skui_settings.gutter / 2;
	ui_layout_t*       layout = &skui_layouts.last();
	panel_stack_data_t start  = skui_panel_stack.last();

	vec3 curr = vec3{layout->offset_initial.x + layout->size_used.x, layout->offset.y - (layout->line.y + (start.padding == ui_pad_inside?gutter:0)), layout->offset.z};

	ui_panel_at(start.at, {fabsf(curr.x-start.at.x), start.at.y-curr.y}, start.padding);

	layout->offset_initial.z = start.at.z;
	layout->offset.z         = start.at.z;
	layout->line_pad       = start.original_line_start;
	if (start.padding == ui_pad_inside) {
		layout->offset_initial.x += gutter;
		layout->offset_initial.y += gutter;
		layout->line.y += gutter * 2;
		layout->offset.y += gutter;
		layout->offset.x -= gutter;
	}
	skui_panel_stack.pop();
	ui_nextline();*/

	ui_layout_t* panel_layout = &skui_layouts.last();
	vec2         panel_size   = panel_layout->size_used + vec2{skui_settings.margin * 2, skui_settings.margin * 2};
	ui_layout_pop();
	bounds_t bounds = ui_layout_reserve(panel_size, false);

	panel_stack_data_t start = skui_panel_stack.last();
	ui_panel_at(bounds.center + bounds.dimensions / 2, { bounds.dimensions.x, bounds.dimensions.y }, start.padding);
}

}