#include "line_drawer.h"
#include "../stereokit.h"
#include "../math.h"
#include "../hierarchy.h"

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

bool line_drawer_init(void *) {
	line_mesh = mesh_create();
	mesh_set_id(line_mesh, "render/line_mesh");

	line_material = material_create(shader_find(default_id_shader_lines));
	material_set_id(line_material, "render/line_material");
	material_set_transparency(line_material, transparency_blend);
	return true;
}

///////////////////////////////////////////

void line_drawer_update() {
	if (line_ind_ct <= 0)
		return;

	mesh_set_verts    (line_mesh, line_verts, line_vert_cap, false);
	mesh_set_inds     (line_mesh, line_inds,  line_ind_cap);
	mesh_set_draw_inds(line_mesh, line_ind_ct);
	render_add_mesh   (line_mesh, line_material, matrix_identity);

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
		line_vert_cap = maxi(line_vert_ct + verts, line_vert_cap * 2);
		line_verts    = (vert_t*)realloc(line_verts, line_vert_cap * sizeof(vert_t));
	}

	if (line_ind_ct + inds >= line_ind_cap) {
		line_ind_cap = maxi(line_ind_ct + inds, line_ind_cap * 2);
		line_inds    = (vind_t*)realloc(line_inds, line_ind_cap * sizeof(vind_t));
	}
}

///////////////////////////////////////////

void line_add(vec3 start, vec3 end, color32 color_start, color32 color_end, float thickness) {
	line_addv({start, thickness, color_start}, {end, thickness, color_end});
}

///////////////////////////////////////////

void line_addv(line_point_t start, line_point_t end) {
	line_ensure_cap(4, 6);
	start.thickness *= 0.5f;
	end  .thickness *= 0.5f;

	if (hierarchy_enabled) {
		matrix &transform = hierarchy_stack.last().transform;
		start.pt = matrix_mul_point(transform, start.pt);
		end  .pt = matrix_mul_point(transform, end.pt);
	}

	vind_t start_vert = (vind_t)line_vert_ct;
	vec3   dir        = vec3_normalize(end.pt - start.pt);

	line_verts[line_vert_ct+0] = vert_t{ start.pt, dir* start.thickness, {0,0}, start.color };
	line_verts[line_vert_ct+1] = vert_t{ start.pt, dir*-start.thickness, {0,1}, start.color };
	line_verts[line_vert_ct+2] = vert_t{ end  .pt, dir* end.thickness,   {1,0}, end  .color };
	line_verts[line_vert_ct+3] = vert_t{ end  .pt, dir*-end.thickness,   {1,1}, end  .color };

	line_inds[line_ind_ct++] = start_vert + 0;
	line_inds[line_ind_ct++] = start_vert + 2;
	line_inds[line_ind_ct++] = start_vert + 3;
	line_inds[line_ind_ct++] = start_vert + 0;
	line_inds[line_ind_ct++] = start_vert + 3;
	line_inds[line_ind_ct++] = start_vert + 1;

	line_vert_ct += 4;
}

///////////////////////////////////////////

void line_add_list(const vec3 *points, int32_t count, color32 color, float thickness) {
	if (count < 2) return;
	line_ensure_cap(count*2, (count-1)*6);
	thickness *= 0.5f;
	
	vec3 prev = hierarchy_to_world_point(points[0] - (points[1]-points[0]));
	for (int32_t i = 0; i < count; i++) {
		vec3 curr = hierarchy_to_world_point(points[i]);
		vec3 dir  = vec3_normalize(curr - prev);
		line_verts[line_vert_ct + 0] = vert_t{ curr, dir *  thickness, {0,0}, color };
		line_verts[line_vert_ct + 1] = vert_t{ curr, dir * -thickness, {0,1}, color };

		if (i < count - 1) {
			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 2;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 1;

			dir = vec3_normalize(curr - prev);
		}

		line_vert_ct += 2;
		prev          = curr;
	}
}

///////////////////////////////////////////

void line_add_listv(const line_point_t *points, int32_t count) {
	if (count < 2) return;
	line_ensure_cap(count*2, (count-1)*6);
	
	vec3 prev = hierarchy_to_world_point(points[0].pt - (points[1].pt-points[0].pt));
	for (int32_t i = 0; i < count; i++) {
		vec3 curr = hierarchy_to_world_point(points[i].pt);
		vec3 dir  = vec3_normalize(curr - prev);
		line_verts[line_vert_ct + 0] = vert_t{ curr, dir *  points[i].thickness * 0.5f, {0,0}, points[i].color };
		line_verts[line_vert_ct + 1] = vert_t{ curr, dir * -points[i].thickness * 0.5f, {0,1}, points[i].color };

		if (i < count - 1) {
			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 2;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 1;
		}

		line_vert_ct += 2;
		prev          = curr;
	}
}

} // namespace sk