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
material_t isec_material;
mesh_t  isec_sphere;

bool32_t use_bvh = true;
bool have_intersection;

///////////////////////////////////////////

void demo_bvh_init() {

    mesh_from = mesh_gen_sphere(0.04f, 16);
    mesh_to = mesh_gen_cone(0.04f, 0.1f, vec3{0.0f,0.0f,-1.0f}, 16);

    isec_material = material_copy(material_find(default_id_material));
    material_set_color(isec_material, "color", color128{1.0f,0.5f,0.0f,1.0f});
    isec_sphere = mesh_gen_sphere(0.01f, 8);

	// Load model_to_intersect 
	//model_to_intersect = model_create_file("Radio.glb");
    model_to_intersect = model_create_file("suzanne.obj");
    //model_to_intersect = model_create_file("/home/melis/models/mesa-verde/scene.gltf");

    model_material = material_find(default_id_material);

    // Scale model to unit size
    bounds_t b = model_get_bounds(model_to_intersect);
    model_scale = 1.0f / vec3_magnitude(b.dimensions);

    // Initial ray endpoints
    from_pose = {(b.center - 0.7f*b.dimensions)*model_scale, quat_identity};
    to_pose = {(b.center + 0.7f*b.dimensions)*model_scale, quat_identity};
}

///////////////////////////////////////////

void demo_bvh_update() {

    // XXX add reset model pose option
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
    mesh_draw      (mesh_from, isec_material, matrix_identity);
    ui_handle_end  ();

    to_pose.orientation = quat_lookat(from_pose.position, to_pose.position);

    ui_handle_begin("to", to_pose, mesh_get_bounds(mesh_to), false, ui_move_pos_only);
    mesh_draw      (mesh_to, isec_material, matrix_identity);
    ui_handle_end  ();

    // Compute model-space ray

    world_ray = {from_pose.position, to_pose.position-from_pose.position};
    
    matrix model_to_world_matrix = model_scale_matrix * pose_matrix(model_pose);
    matrix world_to_model_matrix = matrix_invert(model_to_world_matrix);
    model_ray = matrix_transform_ray(world_to_model_matrix, world_ray);

    // Intersection test

    const double t0 = time_get_raw();

    if (use_bvh)
        have_intersection = model_ray_intersect_bvh(model_to_intersect, model_ray, &intersection);
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

        mesh_draw(isec_sphere, isec_material, matrix_t(world_isec_point));

        line_add(world_isec_point, world_isec_point+world_isec_normal*0.1f,
            color32{0,0,255,255}, color32{0,0,255,255}, 0.005f);
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
        sprintf(s, "%.3f ms", time_ms);
        ui_text(s);
    }

    ui_window_end();        
}

///////////////////////////////////////////

void demo_bvh_shutdown() {
	// Release everything
	model_release(model_to_intersect);
}
