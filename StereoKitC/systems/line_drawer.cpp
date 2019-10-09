#include "line_drawer.h"
#include "../stereokit.h"
#include "../shaders_builtin/shader_builtin.h"

#include <stdlib.h>

namespace sk {

///////////////////////////////////////////

mesh_t     line_mesh;
material_t line_material;
vert_t  *line_verts    = nullptr;
uint32_t line_vert_ct  = 0;
uint32_t line_vert_cap = 0;
vind_t  *line_inds     = nullptr;
uint32_t line_ind_ct   = 0;
uint32_t line_ind_cap  = 0;

///////////////////////////////////////////

bool line_drawer_init() {
	line_mesh = mesh_create();
	mesh_set_id(line_mesh, "render/line_mesh");

	shader_t line_shader = shader_create(sk_shader_builtin_lines);
	shader_set_id(line_shader, "render/line_shader");
	line_material = material_create(line_shader);
	material_set_id(line_material, "render/line_material");
	shader_release(line_shader);
	return true;
}

///////////////////////////////////////////

void line_drawer_update() {
	if (line_ind_ct <= 0)
		return;

	mesh_set_verts    (line_mesh, line_verts, line_vert_cap);
	mesh_set_inds     (line_mesh, line_inds,  line_ind_cap);
	mesh_set_draw_inds(line_mesh, line_ind_ct);
	render_add_mesh(line_mesh, line_material, matrix_identity);

	line_ind_ct  = 0;
	line_vert_ct = 0;
}

///////////////////////////////////////////

void line_drawer_shutdown() {
	mesh_release    (line_mesh);
	material_release(line_material);
}

///////////////////////////////////////////

void line_ensure_cap(int32_t verts, int32_t inds) {
	if (line_vert_ct + verts >= line_vert_cap) {
		line_vert_cap = fmaxf(line_vert_ct + verts, line_vert_cap * 2);
		line_verts    = (vert_t*)realloc(line_verts, line_vert_cap * sizeof(vert_t));
	}

	if (line_ind_ct + inds >= line_ind_cap) {
		line_ind_cap = fmaxf(line_ind_ct + inds, line_ind_cap * 2);
		line_inds    = (vind_t*)realloc(line_inds, line_ind_cap * sizeof(vind_t));
	}
}

///////////////////////////////////////////

void line_add(vec3 start, vec3 end, color32 color, float thickness) {
	line_addv({start, thickness, color}, {end, thickness, color});
}

///////////////////////////////////////////

void line_addv(line_point_t start, line_point_t end) {
	line_ensure_cap(4, 6);
	vec3 dir = vec3_normalize(end.pt - start.pt);
	line_verts[line_vert_ct+0] = vert_t{ start.pt, dir* start.thickness, {0,0}, start.color };
	line_verts[line_vert_ct+1] = vert_t{ start.pt, dir*-start.thickness, {0,1}, start.color };
	line_verts[line_vert_ct+2] = vert_t{ end  .pt, dir* end.thickness,   {1,0}, end  .color };
	line_verts[line_vert_ct+3] = vert_t{ end  .pt, dir*-end.thickness,   {1,1}, end  .color };

	line_inds[line_ind_ct++] = line_vert_ct + 0;
	line_inds[line_ind_ct++] = line_vert_ct + 2;
	line_inds[line_ind_ct++] = line_vert_ct + 3;
	line_inds[line_ind_ct++] = line_vert_ct + 0;
	line_inds[line_ind_ct++] = line_vert_ct + 3;
	line_inds[line_ind_ct++] = line_vert_ct + 1;

	line_vert_ct += 4;
}

///////////////////////////////////////////

void line_add_list(const vec3 *points, int32_t count, color32 color, float thickness) {
}

///////////////////////////////////////////

void line_add_listv(const line_point_t *points, int32_t count) {

}

} // namespace sk