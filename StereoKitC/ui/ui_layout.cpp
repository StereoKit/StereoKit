#include "ui_layout.h"

#include "../libraries/array.h"

///////////////////////////////////////////

namespace sk {

array_t<ui_layout_t> skui_layouts  = {};
ui_settings_t        skui_settings = {};
bounds_t             skui_recent_layout;

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
	layout->max_x            = 0;
	layout->line_height      = 0;
	layout->prev_offset      = layout->offset;
	layout->prev_line_height = layout->line_height;
}

///////////////////////////////////////////

vec2 ui_layout_remaining() {
	ui_layout_t *layout = &skui_layouts.last();
	float size_x = layout->size.x != 0
		? layout->size.x
		: (layout->window ? (layout->window->prev_size.x-skui_settings.margin*2) : 0);
	float size_y = layout->size.y != 0
		? layout->size.y
		: (layout->window ? (layout->window->prev_size.y-skui_settings.margin*2) : 0);
	float max_x = size_x == 0 ? -layout->max_x : -size_x;
	return vec2{
		fmaxf(max_x, size_x - (layout->offset_initial.x - layout->offset.x)),
		fmaxf(0,     size_y - (layout->offset_initial.y - layout->offset.y))
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
	if (layout->max_x > layout->offset.x - final_size.x)
		layout->max_x = layout->offset.x - final_size.x;
	if (layout->line_height < final_size.y)
		layout->line_height = final_size.y;

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
		start      -= vec3{ skui_settings.margin, skui_settings.margin, 0 };
		if (dimensions.x != 0) dimensions.x = dimensions.x - skui_settings.margin * 2;
		if (dimensions.y != 0) dimensions.y = dimensions.y - skui_settings.margin * 2;
	}
	ui_layout_t layout = {};
	layout.offset_initial   = start;
	layout.prev_offset      = start;
	layout.offset           = start;
	layout.size             = dimensions;
	layout.line_height      = 0;
	layout.max_x            = 0;
	layout.prev_line_height = 0;
	skui_layouts.add(layout);
}

///////////////////////////////////////////

void ui_layout_push_cut(ui_cut_ cut_to, float size, bool32_t add_margin) {
	ui_layout_t* curr = &skui_layouts.last();
	if      (cut_to == ui_cut_bottom && curr->size.y == 0) log_warn("Can't cut bottom for layout with a height of 0!");
	else if (cut_to == ui_cut_right  && curr->size.x == 0) log_warn("Can't cut right for layout with an width of 0!");

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
	curr->prev_offset    += curr_offset;
	ui_layout_push(cut_start, cut_size, add_margin);
	skui_layouts.last().window = curr->window;
}

///////////////////////////////////////////

void ui_layout_push_win(ui_window_t *window, vec3 start, vec2 dimensions, bool32_t add_margin) {
	ui_layout_push(start, dimensions, add_margin);
	skui_layouts.last().window = window;
	window->layout_start = start;
	window->layout_size  = dimensions;
	
	window->curr_size = {};
	if (window->layout_size.x != 0) window->curr_size.x = dimensions.x;
	if (window->layout_size.y != 0) window->curr_size.y = dimensions.y;
}

///////////////////////////////////////////

void ui_layout_pop() {
	ui_layout_t* layout = &skui_layouts.last();

	// Move to next line if we're still on a previous line
	if (layout->offset.x != layout->offset_initial.x)
		ui_nextline();

	if (layout->window) {
		vec3 start = layout->window->layout_start;// layout->offset_initial + vec3{0,0,skui_settings.depth};
		vec3 end   = { layout->max_x, layout->offset.y - (layout->line_height-skui_settings.gutter),  layout->offset_initial.z};
		vec3 size  = start - end;
		size = { fmaxf(size.x+skui_settings.margin, layout->size.x), fmaxf(size.y+skui_settings.margin, layout->size.y), size.z };
		if (layout->window->layout_size.x == 0)
			layout->window->curr_size.x = fmaxf(size.x, layout->window->curr_size.x);
		if (layout->window->layout_size.y == 0)
			layout->window->curr_size.y = fmaxf(size.y, layout->window->curr_size.y);
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
	layout->prev_offset      = layout->offset;
	layout->prev_line_height = layout->line_height;

	layout->offset.x    = layout->offset_initial.x;
	layout->offset.y   -= layout->line_height + skui_settings.gutter;
	layout->line_height = 0;
}

///////////////////////////////////////////

void ui_sameline() {
	ui_layout_t *layout = &skui_layouts.last();
	layout->offset      = layout->prev_offset;
	layout->line_height = layout->prev_line_height;
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
	float   max_x;
	ui_pad_ padding;
};
array_t<panel_stack_data_t> skui_panel_stack = {};
void ui_panel_begin(ui_pad_ padding) {
	ui_layout_t*       layout = &skui_layouts.last();
	panel_stack_data_t data;
	data.at      = layout->offset;
	data.max_x   = layout->max_x;
	data.padding = padding;

	skui_panel_stack.add( data );
	layout->max_x = layout->offset_initial.x;

	if (padding == ui_pad_inside) {
		float gutter = skui_settings.gutter / 2;
		layout->offset_initial.x -= gutter;
		layout->offset_initial.y -= gutter;
		layout->offset.x -= gutter;
		layout->offset.y -= gutter;
	}
}

///////////////////////////////////////////

void ui_panel_end() {
	ui_sameline();

	float              gutter = skui_settings.gutter / 2;
	ui_layout_t*       layout = &skui_layouts.last();
	panel_stack_data_t start  = skui_panel_stack.last();
	if (start.padding == ui_pad_inside) layout->max_x -= gutter;

	vec3 curr = vec3{layout->max_x, layout->offset.y - (layout->line_height + (start.padding == ui_pad_inside?gutter:0)), layout->offset.z};

	ui_panel_at(start.at, {fabsf(curr.x-start.at.x), start.at.y-curr.y}, start.padding);

	layout->max_x = fminf(layout->max_x, start.max_x);
	if (start.padding == ui_pad_inside) {
		layout->offset_initial.x += gutter;
		layout->offset_initial.y += gutter;
		layout->line_height += gutter * 2;
		layout->offset.y += gutter;
		layout->offset.x -= gutter;
	}
	skui_panel_stack.pop();
	ui_nextline();
}

}