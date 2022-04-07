#pragma once

#include "../stereokit.h"
#include "../libraries/sk_gpu.h"
#include "../libraries/array.h"

#ifndef _MSC_VER
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#endif
#include <DirectXMath.h>
#ifndef _MSC_VER
#pragma clang diagnostic pop
#endif

namespace sk {

struct render_stats_t {
	int swaps_mesh;
	int swaps_texture;
	int swaps_material;
	int draw_calls;
	int draw_instances;
};

struct render_item_t {
	DirectX::XMMATRIX transform;
	color128    color;
	skg_mesh_t *mesh;
	material_t  material;
	uint64_t    sort_id;
	int32_t     mesh_inds;
	uint16_t    layer;
};

enum render_list_state_ {
	render_list_state_destroyed = -1,
	render_list_state_empty = 0,
	render_list_state_used,
	render_list_state_rendered,
	render_list_state_rendering,
};

struct _render_list_t {
	array_t<render_item_t> queue;
	render_stats_t         stats;
	render_list_state_     state;
	bool                   prepped;
};

typedef int64_t render_list_t;

matrix        render_get_projection_matrix();
float         render_get_ortho_view_height();
matrix        render_get_cam_final        ();
matrix        render_get_cam_final_inv    ();
color128      render_get_clear_color_ln   ();
vec2          render_get_clip             ();
void          render_draw_matrix          (const matrix *views, const matrix *projs, int32_t view_count, render_layer_ render_filter);
void          render_clear                ();
vec3          render_unproject_pt         (vec3 normalized_screen_pt);
void          render_update_projection    ();
const char   *render_fmt_name             (tex_format_ format);
skg_tex_fmt_  render_preferred_depth_fmt  ();
void          render_blit_to_bound        (material_t material);

bool          render_init                 ();
void          render_update               ();
void          render_shutdown             ();

render_list_t render_list_create          ();
void          render_list_release         (render_list_t list);
void          render_list_push            (render_list_t list);
void          render_list_pop             ();
void          render_list_add             (const render_item_t *item);
void          render_list_add_to          (render_list_t list, const render_item_t *item);
void          render_list_execute         (render_list_t list, render_layer_ filter,  uint32_t view_count);
void          render_list_execute_material(render_list_t list, render_layer_ filter, uint32_t view_count, material_t override_material);
void          render_list_clear           (render_list_t list);

} // namespace sk