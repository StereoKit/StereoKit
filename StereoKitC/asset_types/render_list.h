#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"
#include "../systems/render.h"
#include "assets.h"

namespace sk {

enum render_list_state_ {
	render_list_state_destroyed = -1,
	render_list_state_empty = 0,
	render_list_state_used
};

struct _render_list_t {
	asset_header_t         header;
	array_t<render_item_t> queue;
	render_stats_t         stats;
	render_list_state_     state;
	bool                   sorted;
};

void          render_list_destroy         (render_list_t list);
void          render_list_sort            (render_list_t list);
void          render_list_add_to          (render_list_t list, const render_item_t *item);
void          render_list_execute         (render_list_t list, render_layer_ filter, uint32_t view_count);
void          render_list_execute_material(render_list_t list, render_layer_ filter, uint32_t view_count, material_t override_material);

void          render_list_draw            (render_list_t list, matrix *views, matrix *projections, uint32_t view_count, render_layer_ filter);
void          render_list_draw_material   (render_list_t list, matrix *views, matrix *projections, uint32_t view_count, render_layer_ filter, material_t override_material);


} // namespace sk