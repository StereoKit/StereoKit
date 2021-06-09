#include "line_drawer.h"
#include "../stereokit.h"
#include "../sk_math.h"
#include "../sk_memory.h"
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

bool line_drawer_init() {
	line_mesh = mesh_create();
	mesh_set_keep_data(line_mesh, false);
	mesh_set_id(line_mesh, "render/line_mesh");

	line_material = material_create(shader_find(default_id_shader_lines));
	material_set_id(line_material, "render/line_material");
	material_set_transparency(line_material, transparency_blend);
	material_set_cull(line_material, cull_none);
	return true;
}

///////////////////////////////////////////

void line_drawer_update() {
	if (line_ind_ct <= 0)
		return;

	mesh_set_verts    (line_mesh, line_verts, line_vert_cap, false);
	mesh_set_inds     (line_mesh, line_inds,  line_ind_cap);
	mesh_set_draw_inds(line_mesh, line_ind_ct);
	render_add_mesh   (line_mesh, line_material, matrix_identity, {1,1,1,1}, render_layer_vfx);

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
		line_verts    = sk_realloc_t(vert_t, line_verts, line_vert_cap);
	}

	if (line_ind_ct + inds >= line_ind_cap) {
		line_ind_cap = maxi(line_ind_ct + inds, line_ind_cap * 2);
		line_inds    = sk_realloc_t(vind_t, line_inds, line_ind_cap);
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

	line_verts[line_vert_ct+0] = vert_t{ start.pt, dir, { start.thickness,0}, start.color };
	line_verts[line_vert_ct+1] = vert_t{ start.pt, dir, {-start.thickness,1}, start.color };
	line_verts[line_vert_ct+2] = vert_t{ end  .pt, dir, { end  .thickness,0}, end  .color };
	line_verts[line_vert_ct+3] = vert_t{ end  .pt, dir, {-end  .thickness,1}, end  .color };

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
	
	vec3 curr = hierarchy_to_world_point(points[0]);
	vec3 next = hierarchy_to_world_point(points[1]);
	vec3 prev_dir = vec3_normalize(next - curr);
	vec3 curr_dir = prev_dir;
	for (int32_t i = 0; i < count; i++) {
		if (i < count-1) {
			next     = hierarchy_to_world_point(points[i+1]);
			curr_dir = vec3_normalize(next - curr);

			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 2;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 1;
		}
		vec3 tangent = curr_dir + prev_dir;
		line_verts[line_vert_ct + 0] = vert_t{ curr, tangent, { thickness,0}, color };
		line_verts[line_vert_ct + 1] = vert_t{ curr, tangent, {-thickness,1}, color };

		line_vert_ct += 2;
		curr          = next;
		prev_dir      = curr_dir;
	}
}

///////////////////////////////////////////

void line_add_listv(const line_point_t *points, int32_t count) {
	if (count < 2) return;
	line_ensure_cap(count*2, (count-1)*6);
	
	vec3 curr = hierarchy_to_world_point(points[0].pt);
	vec3 next = hierarchy_to_world_point(points[1].pt);
	vec3 prev_dir = vec3_normalize(next - curr);
	vec3 curr_dir = prev_dir;
	for (int32_t i = 0; i < count; i++) {
		if (i < count-1) {
			next     = hierarchy_to_world_point(points[i+1].pt);
			curr_dir = vec3_normalize(next - curr);

			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 2;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 0;
			line_inds[line_ind_ct++] = line_vert_ct + 3;
			line_inds[line_ind_ct++] = line_vert_ct + 1;
		}
		vec3 tangent = curr_dir + prev_dir;
		line_verts[line_vert_ct + 0] = vert_t{ curr, tangent, { points[i].thickness * 0.5f,0}, points[i].color };
		line_verts[line_vert_ct + 1] = vert_t{ curr, tangent, {-points[i].thickness * 0.5f,1}, points[i].color };

		line_vert_ct += 2;
		curr          = next;
		prev_dir      = curr_dir;
	} 
}

} // namespace sk