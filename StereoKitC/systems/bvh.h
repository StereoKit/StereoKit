#pragma once

#include "../stereokit.h"

namespace sk {

struct mesh_collision_t;
struct bvh_node_t;
struct boundingbox;

struct bvh_stats_t
{
    // Statistics counters        
    uint32_t num_leafs, num_inner_nodes;
    uint32_t max_leaf_size;
    uint32_t num_forced_leafs;

    int depth;
    int split_axis_histogram[3];
};

struct mesh_bvh_t
{
    int                 acceptable_leaf_size;

    mesh_t              the_mesh;
    const mesh_collision_t *collision_data;

    bvh_node_t          *nodes;
    uint32_t            next_node_index;
    uint32_t            *sorted_triangles;
    bvh_stats_t         statistics;
};

mesh_bvh_t* mesh_bvh_create(const mesh_t mesh, int acc_leaf_size=16);
void        mesh_bvh_destroy(mesh_bvh_t* bvh);
bool        mesh_bvh_intersect(const mesh_bvh_t *bvh, ray_t model_space_ray, ray_t *out_pt, uint32_t* out_start_inds);

} // namespace sk
