#pragma once

#include "../stereokit.h"

namespace sk {

struct mesh_collision_t;
struct bvh_node_t;

struct bvh_stats_t
{    
    short depth;
    uint32_t num_leafs, num_inner_nodes;
    uint32_t max_leaf_size;
    uint32_t num_forced_leafs;
};

struct mesh_bvh_t
{
    mesh_t              the_mesh;
    const mesh_collision_t *collision_data;

    bvh_node_t          *nodes;
    uint32_t            *sorted_triangles;    
};

mesh_bvh_t* mesh_bvh_create(const mesh_t mesh, int acc_leaf_size=16, bool show_stats=true);
void        mesh_bvh_destroy(mesh_bvh_t* bvh);
bool        mesh_bvh_intersect(const mesh_bvh_t *bvh, ray_t model_space_ray, ray_t *out_pt, uint32_t* out_start_inds, cull_ cull_mode);
void        mesh_bvh_statistics(const mesh_bvh_t *bvh, bvh_stats_t *stats, int acc_leaf_size=16);

} // namespace sk
