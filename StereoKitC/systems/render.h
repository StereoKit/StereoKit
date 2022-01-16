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

matrix        render_get_projection         ();
matrix        render_get_cam_final          ();
matrix        render_get_cam_final_inv      ();
color128      render_get_clear_color        ();
vec2          render_get_clip               ();
void          render_clear                  ();
vec3          render_unproject_pt           (vec3 normalized_screen_pt);
void          render_update_projection      ();
const char   *render_fmt_name               (tex_format_ format);
skg_tex_fmt_  render_preferred_depth_fmt    ();
void          render_blit_to_bound          (material_t material);
void          render_all                    ();
void          render_viewpoints             ();

bool          render_init                   ();
void          render_update                 ();
void          render_shutdown               ();

typedef struct render_module_t {

} render_module_t;

typedef struct render_res_t {

} render_res_t;

enum render_res_type_ {
	render_res_type_display_surface,
	render_res_type_intermediate_surface,
	render_res_type_temporary_surface
};

enum render_resolution_ {
	render_resolution_absolute,
	render_resolution_percentage
};

enum render_rw_ {
	render_rw_read       = 1,
	render_rw_write      = 2,
	render_rw_read_write = 1|2,
};

void             render_res_create          (const char *name, render_resolution_ resolution_type, render_res_t *root_resource, float width, float height, tex_format_ format);
void             render_res_create_swapchain(const char *name, skg_swapchain_t *swapchain);
render_res_t    *render_res_temp            (render_resolution_ resolution_type, render_res_t *root_resource, float width, float height, tex_format_ format);
tex_t            render_res_get_read        (const char *name);
render_res_t    *render_res_get_write       (const char *name);
void             render_res_bind            (render_res_t *res);

void             rendermodule_dependency    (render_module_t *mod, const char *module_name);
void             rendermodule_uses_res      (render_module_t *mod, const char *name, render_rw_ read_write);
render_module_t *rendermodule_get           (const char *name);

void             rendergraph_clear          ();
render_module_t *rendergraph_add_module     (const char *name, void (*module_init)(render_module_t *mod), void (*module_step)(render_module_t *mod, render_list_t render_list));
void             rendergraph_execute        ();

} // namespace sk