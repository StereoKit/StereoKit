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
    
    // Long narrow triangles that are hard to split
    //model = model_create_file("Radio.glb");

    //model = model_create_file("uvsphere.glb");
    //model = model_create_file("cube.glb");

    //model = model_create_file("/home/melis/models/mesa-verde/scene.gltf");

    // No isec -10,-10,-10 dir 1,1,1?
    /*
    vec3{10,10,10}, vec3{-1,-1,-1}

    mesh[14] 65532 vertices, 104034 triangles
    BVH statistics:
    ... 104034 triangles
    ... depth 21
    ... 9591 leaf nodes, 9590 inner nodes
    ... maximum leaf size 16
    ... 0 forced leafs
    ... Split axis histogram:
    ... 0 |  4709
    ... 1 |  4620
    ... 2 |   261
    intersection at -0.663039,-0.663039,-0.663039 (normal 0.000000,-0.000000,-1.000000)!

    ???

    */
    
    model = model_create_file("/home/melis/models/red-car-wreck/scene.gltf");
    //model = model_create_file("t.glb");
    //model = model_create_file("/home/melis/models/angel/angel.obj");
    //model = model_create_file("/home/melis/models/lucy/lucy2.glb");
    
    ray_t ray{
        vec3{10,10,10}, vec3{-1,-1,-1}
        //vec3{0,-10,0}, vec3{0,1,0}
    };

    ray_t intersection = {};

    timeval t0, t1;

    printf("Performing intersection against mesh subset(s):\n");

    gettimeofday(&t0, NULL);

#if 1
    mesh_t mesh;
    printf("%d subsets\n", model_subset_count(model));
    for (int i = 0; i < model_subset_count(model); i++)        
    {
        mesh = model_get_mesh(model, i);
        printf("mesh[%d] %d vertices, %d triangles\n", i, mesh_get_vert_count(mesh), mesh_get_ind_count(mesh)/3);

        if (mesh_ray_intersect_bvh(mesh, ray, &intersection))
        {
            printf("intersection at %6f,%6f,%6f (normal %.6f,%.6f,%.6f)!\n",
                intersection.pos.x, intersection.pos.y, intersection.pos.z,
                intersection.dir.x, intersection.dir.y, intersection.dir.z);
        }
    }
#else
    // Should result in the same intersection as closest hit found above 
    // (i.e. when explicitly looping over meshes). Yet isn't always the case.
    if (model_ray_intersect_bvh(model, ray, &intersection))
    {
        printf("intersection at %6f,%6f,%6f (normal %.6f,%.6f,%.6f)!\n",
            intersection.pos.x, intersection.pos.y, intersection.pos.z,
            intersection.dir.x, intersection.dir.y, intersection.dir.z);
    }
#endif

    gettimeofday(&t1, NULL);
    double tdiff = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec)/1000000.0f;
    printf("intersection test (including BVH build) took %.6fs\n", tdiff);
    
    sk_shutdown();
    return 0;
}

