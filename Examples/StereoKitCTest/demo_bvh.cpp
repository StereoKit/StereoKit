#include "demo_bvh.h"

#include <cstdio>   // For sprintf()
#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;
#include <vector>
using namespace std;

///////////////////////////////////////////

model_t model_to_intersect;
float   model_scale = 1.0f;
pose_t  model_pose;

mesh_t  mesh_from, mesh_to;
pose_t  from_pose, to_pose;
ray_t   world_ray, model_ray;
ray_t   intersection;

material_t model_material;
material_t ray_material;
mesh_t  isec_sphere;

bool32_t use_bvh = true;
bool have_intersection;

///////////////////////////////////////////

void demo_bvh_init() {

    mesh_from = mesh_gen_sphere(0.04f, 16);
    mesh_to = mesh_gen_cone(0.04f, 0.1f, vec3{0.0f,0.0f,-1.0f}, 16);

    ray_material = material_copy(material_find(default_id_material));
    material_set_color(ray_material, "color", color128{1.0f,0.5f,0.0f,1.0f});

    isec_sphere = mesh_gen_sphere(0.01f, 8);

	// Load model_to_intersect 
	model_to_intersect = model_create_file("Radio.glb");
    //model_to_intersect = model_create_file("suzanne.obj");

    printf("%d subsets\n", model_subset_count(model_to_intersect));
    for (int i = 0; i < model_subset_count(model_to_intersect); i++)        
    {
        mesh_t mesh = model_get_mesh(model_to_intersect, i);
        printf("mesh[%d] %d vertices, %d triangles\n", i, mesh_get_vert_count(mesh), mesh_get_ind_count(mesh)/3);
    }

    model_material = material_find(default_id_material);

    // Scale large models to unit size
    bounds_t b = model_get_bounds(model_to_intersect);
    float m = vec3_magnitude(b.dimensions);
    if (m > 1.0f)
        model_scale = 1.0f / vec3_magnitude(b.dimensions);

    // Initial ray endpoints
    from_pose = {(b.center - 0.7f*b.dimensions)*model_scale, quat_identity};
    to_pose = {(b.center + 0.7f*b.dimensions)*model_scale, quat_identity};
}

///////////////////////////////////////////

void demo_bvh_update() {

    // XXX add toggle model scale?
    // XXX add showing start indices (but not exposed for model_...() call)?
    // XXX could add switch to disable isec testing (or one-shot), for large models

    // Model being intersected

    matrix model_scale_matrix = matrix_s(vec3{model_scale,model_scale,model_scale});

    ui_handle_begin("model", model_pose, model_get_bounds(model_to_intersect), false);
	model_draw     (model_to_intersect, model_scale_matrix);
    ui_handle_end  ();

    // World-space ray defined by two endpoint handles

    ui_handle_begin("from", from_pose, mesh_get_bounds(mesh_from), false, ui_move_pos_only);
    mesh_draw      (mesh_from, ray_material, matrix_identity);
    ui_handle_end  ();

    to_pose.orientation = quat_lookat(from_pose.position, to_pose.position);

    ui_handle_begin("to", to_pose, mesh_get_bounds(mesh_to), false, ui_move_pos_only);
    mesh_draw      (mesh_to, ray_material, matrix_identity);
    ui_handle_end  ();

    // Compute model-space ray

    world_ray = {from_pose.position, to_pose.position-from_pose.position};
    
    matrix model_to_world_matrix = model_scale_matrix * pose_matrix(model_pose);
    matrix world_to_model_matrix = matrix_invert(model_to_world_matrix);
    model_ray = matrix_transform_ray(world_to_model_matrix, world_ray);

    // Intersection test

    const double t0 = time_get_raw();

    mesh_t      isec_mesh = nullptr;
    matrix      isec_matrix;
    uint32_t    isec_start_inds;

    if (use_bvh)
        have_intersection = model_ray_intersect_bvh_detailed(model_to_intersect, model_ray, &intersection, &isec_mesh, &isec_matrix, &isec_start_inds);
    else
        have_intersection = model_ray_intersect(model_to_intersect, model_ray, &intersection);

    const double t1 = time_get_raw();    
        
    // Intersection result

    if (have_intersection)
    {
        // Green: intersection found, show point, including normal
        line_add(from_pose.position, to_pose.position, color32{0,255,0,255}, color32{0,255,0,255}, 0.005f);

        vec3 world_isec_point = matrix_transform_pt(model_to_world_matrix, intersection.pos);
        // Ignore scale for normal
        vec3 world_isec_normal = matrix_transform_dir(matrix_invert(pose_matrix(model_pose)), intersection.dir);

        mesh_draw(isec_sphere, ray_material, matrix_t(world_isec_point));

        line_add(world_isec_point, world_isec_point+world_isec_normal*0.1f,
            color32{0,0,255,255}, color32{0,0,255,255}, 0.005f);

        if (isec_mesh != nullptr)
        {
            //printf("%d\n", isec_start_inds);
            // Highlight intersected triangle
            vert_t *verts;
            vind_t *inds;
            int32_t vcount, icount;

            mesh_get_verts(isec_mesh, verts, vcount, memory_reference);
            mesh_get_inds(isec_mesh, inds, icount, memory_reference);

            vec3 p = matrix_transform_pt(model_to_world_matrix,
                matrix_transform_pt(isec_matrix, verts[inds[isec_start_inds]].pos));
            vec3 q = matrix_transform_pt(model_to_world_matrix,
                matrix_transform_pt(isec_matrix, verts[inds[isec_start_inds+1]].pos));
            vec3 r = matrix_transform_pt(model_to_world_matrix,
                matrix_transform_pt(isec_matrix, verts[inds[isec_start_inds+2]].pos));

            line_add(p, q, color32{0,255,0,255}, color32{0,255,0,255}, 0.005f);
            line_add(q, r, color32{0,255,0,255}, color32{0,255,0,255}, 0.005f);
            line_add(r, p, color32{0,255,0,255}, color32{0,255,0,255}, 0.005f);
        }
    }
    else
    {
        // Red: no intersection
        line_add(from_pose.position, to_pose.position, color32{255,0,0,255}, color32{255,0,0,255}, 0.005f);
    }

    static pose_t window_pose = pose_t{ {0.6f,0,-0.25f}, quat_lookat({0.25f,0.0f,0.0f}, {0,0,0}) };
    ui_window_begin("Options", window_pose, vec2{ 24 }*cm2m);

    ui_toggle("Use BVH", use_bvh);
    float time_ms = 1000*(t1-t0);
    if (time_ms < 0.1f)
        ui_text("< 0.1ms");
    else
    {
        char s[64];
        snprintf(s, 64, "%.3f ms", time_ms);
        ui_text(s);
    }
    if (ui_button("Reset pose"))
        model_pose = pose_t{vec3{}, quat_identity};

    ui_window_end();        
}

///////////////////////////////////////////

void demo_bvh_shutdown() {
	// Release everything
	model_release(model_to_intersect);
}
