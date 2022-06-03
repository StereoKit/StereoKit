#include <stereokit.h>
#include <stereokit_ui.h>
#include <sys/time.h>
#include <vector>
#include <cstdio>
using namespace sk;

text_style_t text_style;

int main()
{
    sk_settings_t settings = {};
    settings.app_name           = "BVH test";
    settings.assets_folder      = "Assets";    

    if (!sk_init(settings))
        return 1;

    printf("Loading model\n");

    model_t model;
    
    //model = model_create_file("DamagedHelmet.gltf");
    //model = model_create_file("/home/melis/models/mesa-verde/scene.gltf");
    model = model_create_file("/home/melis/models/red-car-wreck/scene.gltf");
    //model = model_create_file("t.glb");
    //model = model_create_file("/home/melis/models/angel/angel.obj");
    //model = model_create_file("/home/melis/models/lucy/lucy2.glb");

    mesh_t mesh;
    
    printf("%d subsets\n", model_subset_count(model));
    for (int i = 0; i < model_subset_count(model); i++)        
    {
        mesh = model_get_mesh(model, i);
        printf("mesh[%d] %d vertices, %d triangles\n", i, mesh_get_vert_count(mesh), mesh_get_ind_count(mesh)/3);
    }

    printf("Building BVH for subset 0 only!\n");
    mesh = model_get_mesh(model, 0);
    
    ray_t ray{
        vec3{-10,-10-10}, vec3{1,1,1}
    };

    ray_t intersection = {};    

    timeval t0, t1;

    printf("Doing intersection (and BVH build)\n");

    gettimeofday(&t0, NULL);

    if (mesh_ray_intersect_bvh(mesh, ray, &intersection))
        printf("intersection!\n");

    gettimeofday(&t1, NULL);
    double tdiff = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec)/1000000.0f;
    printf("%.6f\n", tdiff);
    
    sk_shutdown();
    return 0;
}

