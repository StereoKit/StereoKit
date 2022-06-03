#pragma once

#include "../stereokit.h"

namespace sk {

struct mesh_collision_t;
struct bvh_node_t;
struct boundingbox;

struct mesh_bvh_t
{
    mesh_bvh_t(int acc_leaf_size=16);
    ~mesh_bvh_t();

    // Build a BVH for the triangles in the mesh
    void build(const mesh_t mesh);
    void build_recursive(int depth, int node_index, 
        const vec3* triangle_vertices, const vec3* triangle_centroids, 
        const mesh_collision_t* collision_data);

    bool intersect(ray_t model_space_ray, ray_t *out_pt, uint32_t* out_start_inds) const;

    struct statistics_t
    {
        // Statistics counters        
        uint32_t num_leafs, num_inner_nodes;
        uint32_t max_leaf_size;
        uint32_t num_forced_leafs;

        int depth;
        int split_axis_histogram[3];

        statistics_t()
        {
            depth = 0;
            num_leafs = 0;
            num_inner_nodes = 0;
            max_leaf_size = 0;
            num_forced_leafs = 0;
            for (int i = 0; i < 3; i++)
                split_axis_histogram[i] = 0;
        }

        void print();
    };

    int  acceptable_leaf_size;

    mesh_t the_mesh;   // XXX debugging only
    const mesh_collision_t *collision_data;

    bvh_node_t      *nodes;
    uint32_t        next_node_index;

    uint32_t        *sorted_triangles;

    statistics_t    *statistics;
};



} // namespace sk
