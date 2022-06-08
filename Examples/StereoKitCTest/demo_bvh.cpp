#include "demo_bvh.h"

#include <stereokit.h>
#include <stereokit_ui.h>
using namespace sk;
#include <vector>
using namespace std;

///////////////////////////////////////////

pose_t  model_xform;
model_t model_to_intersect;

mesh_t  mesh_from, mesh_to;
pose_t  from_pose, to_pose;
ray_t   model_ray, world_ray;
ray_t   intersection;

material_t isec_material;
mesh_t  isec_sphere;

///////////////////////////////////////////

void demo_bvh_init() {

    mesh_from = mesh_gen_sphere(0.04f, 16);
    mesh_to = mesh_gen_cone(0.04f, 0.1f, vec3{0.0f,0.0f,-1.0f}, 16);

    isec_material = material_find(default_id_material);
    isec_sphere = mesh_gen_sphere(0.01f, 8);

	// Load model_to_intersect 
	model_to_intersect = model_create_file("Radio.glb");

    // Initial ray endpoints
    bounds_t b = model_get_bounds(model_to_intersect);
    from_pose = {b.center - 0.7f*b.dimensions, quat_identity};
    to_pose = {b.center + 0.7f*b.dimensions, quat_identity};
}

///////////////////////////////////////////

void demo_bvh_update() {

	model_draw(model_to_intersect, pose_matrix(model_xform, vec3_one));    

    ui_handle_begin("from", from_pose, mesh_get_bounds(mesh_from), false, ui_move_pos_only);
    mesh_draw      (mesh_from, isec_material, matrix_identity);
    ui_handle_end  ();

    to_pose.orientation = quat_lookat(from_pose.position, to_pose.position);

    ui_handle_begin("to", to_pose, mesh_get_bounds(mesh_to), false, ui_move_pos_only);
    mesh_draw      (mesh_to, isec_material, matrix_identity);
    ui_handle_end  ();

    world_ray = ray_t{from_pose.position, to_pose.position - from_pose.position};
    // XXX
    model_ray = world_ray;

    if (model_ray_intersect_bvh(model_to_intersect, model_ray, &intersection))
    {        
        line_add(from_pose.position, to_pose.position, color32{0,255,0,255}, color32{0,255,0,255}, 0.005f);

        mesh_draw(isec_sphere, isec_material, matrix_t(intersection.pos));

        line_add(intersection.pos, intersection.pos+intersection.dir*0.1f,
            color32{0,0,255,255}, color32{0,0,255,255}, 0.005f);
    }
    else
    {
        line_add(from_pose.position, to_pose.position, color32{255,0,0,255}, color32{255,0,0,255}, 0.005f);
    }
}

///////////////////////////////////////////

void demo_bvh_shutdown() {
	// Release everything
	model_release(model_to_intersect);
}