#include "log_window.h"

#include <stereokit.h>
#include <stereokit_ui.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace sk;

///////////////////////////////////////////

struct log_item_t {
	log_  level;
	char* text;
	int   count;
	float height; // Cached layout height (m), incl. padding.
};
// One entry per log line that overlaps the viewport. Rebuilt only when the
// scroll, content, or width changes - not every frame.
struct visible_line_t {
	int   index; // Index into 'items'.
	float y;     // Entry top, viewport-relative (0 = viewport top, down is -).
};

// A small ring-buffer line graph for per-frame timing values.
struct diag_graph_t {
	float        buffer[100];
	line_point_t points[100];
	int          curr;
	float        min;
	float        max;
	float        avg;
};

static void diag_graph_init(diag_graph_t* graph, float min, float max);
static void diag_graph_add (diag_graph_t* graph, float value);
static void diag_graph_draw(diag_graph_t* graph, text_style_t style, const char* label, vec3 at, vec2 size);

///////////////////////////////////////////

// One spacing value for both the gaps between entries and the lines within a
// wrapped entry, so the two look identical.
static const float line_spacing = 1.4f; // Fraction of font height.
// Sizes in lines of log text, scaled to meters in ensure_styles so they track
// the font size.
static const float view_lines  = 16;   // Viewport height, in lines.
static const float badge_lines = 1.5f; // Badge width, in lines.

static bool styles_ready = false; // Styles are built lazily, after StereoKit is up.

static text_style_t style_diag;
static text_style_t style_info;
static text_style_t style_warn;
static text_style_t style_err;
static text_style_t style_graph;

static float row_pad     = 0; // Per-entry padding (m); derived from line_spacing.
static float view_height = 0; // Viewport height (m); view_lines scaled in init.
static float badge_size  = 0; // Badge box width (m); badge_lines scaled in init.

static log_item_t* items       = nullptr;
static int         items_count = 0;
static int         items_cap   = 0;

static visible_line_t* visible       = nullptr;
static int             visible_count = 0;
static int             visible_cap   = 0;

static float scroll         = 0;    // Scroll offset (m) from the top (oldest) of the content.
static float scroll_max     = 0;    // Largest valid scroll, so the newest line sits at the bottom.
static float content_height = 0;    // Total height (m) of every log item.
static bool  stick_bottom   = true; // Keep the view pinned to the newest line.
static int   measured_count = 0;    // How many items have a valid cached height.
static float measure_width  = 0;    // Width the cached heights were measured against.
static float cached_scroll  = -1;   // Scroll value the visible list was built for.
static bool  view_dirty     = true;

static diag_graph_t fps_graph;
static diag_graph_t cpu_perf_graph;
static diag_graph_t gpu_perf_graph;

///////////////////////////////////////////

static float clampf(float v, float min, float max) {
	return v < min ? min : (v > max ? max : v);
}

// Grows a heap array to fit one more element, doubling capacity. Returns the
// (possibly relocated) data pointer.
static void* arr_ensure(void* data, int count, int* cap, size_t stride) {
	if (count < *cap) return data;
	*cap = *cap < 16 ? 16 : *cap * 2;
	return realloc(data, (size_t)*cap * stride);
}

static char* str_copy(const char* src) {
	size_t size = strlen(src) + 1;
	char*  dst  = (char*)malloc(size);
	memcpy(dst, src, size);
	return dst;
}

static text_style_t style_for(log_ level) {
	switch (level) {
	case log_error:      return style_err;
	case log_warning:    return style_warn;
	case log_inform:     return style_info;
	case log_diagnostic: return style_diag;
	default:             return style_info;
	}
}

///////////////////////////////////////////

static void on_log_window(void*, log_ level, const char* log_c_str) {
	// Collapse a repeat of the previous line into a count on that entry.
	if (items_count > 0 && strcmp(items[items_count-1].text, log_c_str) == 0) {
		items[items_count-1].count++;
		return;
	}
	// Append at the bottom so existing scroll offsets stay valid; draw_logs
	// measures the new height and follows it when pinned.
	items = (log_item_t*)arr_ensure(items, items_count, &items_cap, sizeof(log_item_t));
	items[items_count++] = log_item_t{ level, str_copy(log_c_str), 1, 0 };
}

///////////////////////////////////////////

void log_window_init() {
	log_subscribe(on_log_window);
}

///////////////////////////////////////////

// Build the text styles and derived metrics the first time we draw. Kept out of
// log_window_init so that init can run before sk_init to catch startup logs.
static void ensure_styles() {
	if (styles_ready) return;
	styles_ready = true;

	font_t log_fnt = font_create_family("monospace");
	style_diag  = text_make_style(log_fnt, 0.008f, color_hsv(1,    0,    0.7f, 1));
	style_info  = text_make_style(log_fnt, 0.008f, color_hsv(1,    0,    1,    1));
	style_warn  = text_make_style(log_fnt, 0.008f, color_hsv(0.17f,0.7f, 1,    1));
	style_err   = text_make_style(log_fnt, 0.008f, color_hsv(1,    0.7f, 1,    1));
	style_graph = text_make_style(log_fnt, 1,      color_hsv(1,    0,    1,    1));
	font_release(log_fnt);

	text_style_set_render_layer(style_diag,  render_layer_ui);
	text_style_set_render_layer(style_info,  render_layer_ui);
	text_style_set_render_layer(style_warn,  render_layer_ui);
	text_style_set_render_layer(style_err,   render_layer_ui);
	text_style_set_render_layer(style_graph, render_layer_ui);

	text_style_set_line_height_pct(style_diag, line_spacing);
	text_style_set_line_height_pct(style_info, line_spacing);
	text_style_set_line_height_pct(style_warn, line_spacing);
	text_style_set_line_height_pct(style_err,  line_spacing);

	// row_pad tops a single entry's layout height up to a full line, matching the
	// baseline-to-baseline spacing of wrapped lines.
	float line  = text_style_get_total_height(style_info) * line_spacing;
	row_pad     = line - text_style_get_layout_height(style_info);
	view_height = line * view_lines;
	badge_size  = line * badge_lines;

	diag_graph_init(&fps_graph,      0, 0);
	diag_graph_init(&cpu_perf_graph, 0, 0);
	diag_graph_init(&gpu_perf_graph, 0, 0);
}

///////////////////////////////////////////

void log_window_shutdown() {
	log_unsubscribe(on_log_window);

	for (int i = 0; i < items_count; i++)
		free(items[i].text);
	free(items);
	free(visible);

	items          = nullptr; items_count   = 0; items_cap   = 0;
	visible        = nullptr; visible_count = 0; visible_cap = 0;
	measured_count = 0;
	content_height = 0;
}

///////////////////////////////////////////

// Fills 'visible' with every item that overlaps the viewport, walking top
// (oldest) to bottom. Edge clipping happens at draw time, so partial lines are
// kept.
static void rebuild_visible() {
	visible_count = 0;
	float o = 0; // Content offset (m) of the current item's top.
	for (int i = 0; i < items_count; i++) {
		float h     = items[i].height;
		float top_y = scroll - o; // Item top, viewport-relative (0 = top, down is -).
		o += h;

		if (top_y >= h)            continue; // Entirely above the viewport.
		if (top_y <= -view_height) break;    // Entirely below the viewport.

		visible = (visible_line_t*)arr_ensure(visible, visible_count, &visible_cap, sizeof(visible_line_t));
		visible[visible_count++] = visible_line_t{ i, top_y };
	}
}

///////////////////////////////////////////

static void draw_logs() {
	float width      = ui_layout_remaining().x;
	float slider_w   = ui_line_height()*0.6f;
	float text_width = width - slider_w; // Stop short of the slider.

	// Measure & cache layout height for any new items (or all of them when the
	// width changed, since wrapping depends on it). Cheap when nothing changed.
	if (text_width != measure_width) { measured_count = 0; measure_width = text_width; content_height = 0; }
	if (measured_count != items_count) {
		for (int i = measured_count; i < items_count; i++) {
			items[i].height = text_size_layout_constrained(items[i].text, style_for(items[i].level), text_width).y + row_pad;
			content_height += items[i].height;
		}
		measured_count = items_count;
		scroll_max     = fmaxf(0, content_height - view_height);
		view_dirty     = true;
	}

	if (stick_bottom) scroll = scroll_max;

	ui_layout_push_cut(ui_cut_top,   view_height);
	ui_layout_push_cut(ui_cut_right, slider_w);
	// The slider handle tracks the content directly: bottom of the track is the
	// newest line, top is the oldest.
	if (ui_vslider("scroll", scroll, 0, scroll_max))
		stick_bottom = scroll >= scroll_max - 0.0001f;
	ui_layout_pop();

	scroll = clampf(scroll, 0, scroll_max);
	if (scroll != cached_scroll || view_dirty) {
		rebuild_visible();
		cached_scroll = scroll;
		view_dirty    = false;
	}

	vec3 start = ui_layout_at();
	ui_layout_reserve(vec2{ width, view_height });

	// text_add_in's 'size' doubles as the clip box and is taken at the pivot
	// before off_x/off_y, so off_y slides each line within a fixed top-anchored
	// clip - clipping top and bottom in one pass.
	float  z         = start.z - 0.004f;
	matrix text_tr   = matrix_t(vec3{ start.x,              start.y, z });
	matrix badge_tr  = matrix_t(vec3{ start.x - text_width, start.y, z });
	vec2   text_box  = vec2{ text_width, view_height };
	vec2   badge_box = vec2{ badge_size, view_height };
	for (int i = 0; i < visible_count; i++) {
		const visible_line_t& line = visible[i];
		const log_item_t&     item = items[line.index];
		text_add_in(item.text, text_tr, text_box, (text_fit_)(text_fit_clip | text_fit_wrap), style_for(item.level), pivot_top_left, align_top_left, 0, line.y);

		// Right-aligned duplicate-count badge, just inside the slider.
		if (item.count > 1) {
			char count_str[16];
			snprintf(count_str, sizeof(count_str), "%d", item.count);
			text_add_in(count_str, badge_tr, badge_box, text_fit_clip, style_info, pivot_top_right, align_top_right, 0, line.y);
		}
	}
	ui_layout_pop();
}

///////////////////////////////////////////

static void draw_graphs() {
	float frame_ms = 1000.0f / device_display_get_refresh_rate();
	fps_graph     .min = frame_ms * 0.5f;
	fps_graph     .max = frame_ms * 1.5f;
	cpu_perf_graph.max = frame_ms * 1.5f;
	gpu_perf_graph.max = frame_ms * 1.5f;

	float third_width = (ui_layout_remaining().x - ui_get_settings().gutter*2) / 3.0f;
	vec2  graph_size  = vec2{ third_width, ui_line_height() };

	auto graph = [&](diag_graph_t &g, const char* label, float ms) {
		diag_graph_add (&g, ms);
		diag_graph_draw(&g, style_graph, label, ui_layout_at() + vec3{0,0,-0.004f}, graph_size);
		ui_layout_reserve(graph_size);
		ui_sameline();
	};
	graph(fps_graph,      "Frame (ms)", time_stepf()*1000);
	graph(cpu_perf_graph, "CPU (ms)",   time_perf_cpu_us() / 1000.0f);
	graph(gpu_perf_graph, "GPU (ms)",   time_perf_gpu_us() / 1000.0f);
}

///////////////////////////////////////////

void log_window_update(pose_t* pose) {
	ensure_styles();

	ui_window_begin("Log", pose, vec2{ 40*cm2m, 0 });

	draw_logs();
	ui_hseparator();
	draw_graphs();

	ui_window_end();
}

///////////////////////////////////////////

static void diag_graph_init(diag_graph_t* graph, float min, float max) {
	graph->min  = min;
	graph->max  = max;
	graph->curr = 0;
	graph->avg  = 0;
	for (int i = 0; i < (int)(sizeof(graph->points)/sizeof(graph->points[0])); i++) {
		graph->buffer[i]           = 0;
		graph->points[i].color     = color32{255,255,255,100};
		graph->points[i].thickness = 0.001f;
	}
}

///////////////////////////////////////////

static void diag_graph_add(diag_graph_t* graph, float value) {
	int count = (int)(sizeof(graph->buffer)/sizeof(graph->buffer[0]));
	graph->buffer[graph->curr] = value;
	graph->curr = (graph->curr + 1) % count;
	graph->avg  = graph->avg + (value - graph->avg) * 0.1f;
}

///////////////////////////////////////////

static void diag_graph_draw(diag_graph_t* graph, text_style_t style, const char* label, vec3 at, vec2 size) {
	const float label_width = 0.01f;
	float       text_scale  = size.y * 0.15f;
	int         count       = (int)(sizeof(graph->points)/sizeof(graph->points[0]));
	size.x -= label_width;

	float step = 1.0f/(count-1);
	for (int i = 0; i < count; i++) {
		float pct = i * step;
		int   idx = (graph->curr + i) % count;
		float val = clampf((graph->buffer[idx] - graph->min) / (graph->max - graph->min), 0.0f, 1.0f);
		graph->points[i].pt = vec3{ at.x - pct*size.x, at.y-size.y + val*size.y, at.z };
	}
	line_add_listv(graph->points, count);

	char display[128];
	char num[16];
	snprintf(display, sizeof(display), "%s %.3f", label, graph->avg);
	snprintf(num,     sizeof(num),     "%d", (int)graph->max);
	text_add_in(num,     matrix_ts(vec3{at.x-size.x, at.y,        at.z}, vec3{text_scale,text_scale,text_scale}), vec2{label_width*text_scale, 1}, text_fit_overflow, style, pivot_top_left,    align_top_left);
	snprintf(num,     sizeof(num),     "%d", (int)graph->min);
	text_add_in(num,     matrix_ts(vec3{at.x-size.x, at.y-size.y, at.z}, vec3{text_scale,text_scale,text_scale}), vec2{label_width*text_scale, 1}, text_fit_overflow, style, pivot_bottom_left, align_bottom_left);
	text_add_at(display, matrix_ts(vec3{at.x,        at.y,        at.z}, vec3{text_scale,text_scale,text_scale}),                                                    style, pivot_top_left,    align_top_left);

	float display_w = text_size_layout(display, style).x;
	line_add(at + vec3{-display_w*text_scale - 0.002f, 0,       0}, at + vec3{-size.x, 0,       0}, color32{255,255,255,20}, color32{255,255,255,20}, 0.001f);
	line_add(at + vec3{0,                              -size.y, 0}, at + vec3{-size.x, -size.y, 0}, color32{255,255,255,20}, color32{255,255,255,20}, 0.001f);
}
