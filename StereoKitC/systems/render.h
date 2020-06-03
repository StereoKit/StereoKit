#pragma once

#include "../stereokit.h"
#include "../libraries/array.h"
#include <directxmath.h>

namespace sk {

struct render_stats_t {
	int swaps_mesh;
	int swaps_shader;
	int swaps_texture;
	int swaps_material;
	int draw_calls;
	int draw_instances;
};

struct render_item_t {
	DirectX::XMMATRIX transform;
	color128    color;
	mesh_t      mesh;
	material_t  material;
	uint64_t    sort_id;
};

enum render_list_state_ {
	render_list_state_empty = 0,
	render_list_state_used,
	render_list_state_rendered,
};

struct _render_list_t {
	array_t<render_item_t> queue;
	render_stats_t         stats;
	render_list_state_     state;
};

typedef _render_list_t* render_list_t;

matrix render_get_projection();
color32 render_get_clear_color();
vec2 render_get_clip();
void render_draw_matrix (const matrix *views, const matrix *projs, int32_t view_count);
void render_clear       ();
vec3 render_unproject_pt(vec3 normalized_screen_pt);
void render_update_projection();

bool render_initialize();
void render_update();
void render_shutdown();

void render_set_material(material_t material);
void render_set_shader  (shader_t   shader);
void render_set_mesh    (mesh_t     mesh);
void render_draw_item   (int count);

render_list_t render_list_create ();
void          render_list_free   (render_list_t list);
void          render_list_push   (render_list_t list);
void          render_list_pop    ();
void          render_list_execute(render_list_t list, const matrix *views, const matrix *projs, int32_t view_count);
void          render_list_clear  (render_list_t list);

} // namespace sk