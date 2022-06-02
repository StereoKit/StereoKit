#pragma once

#include "../stereokit.h"

namespace sk {

struct bvh_node_t;
struct boundingbox;

struct mesh_bvh_t
{
    mesh_bvh_t(int max_depth=16, int acc_leaf_size=16);
    ~mesh_bvh_t();

    // Build a BVH for the triangles in the mesh
    void build(const mesh_t mesh);
    void build_recursive(int depth, int node_index, const boundingbox* triangle_bboxes);

    struct statistics_t
    {
        // Statistics counters
        uint32_t num_leafs, num_inner_nodes;
        uint32_t max_leaf_size;
        uint32_t num_forced_leafs;

        int split_axis_histogram[3];

        void print();
    };

    int  max_depth;
    int  acceptable_leaf_size;

    mesh_t          the_mesh;

    bvh_node_t      *nodes;
    uint32_t        next_node_index;

    uint32_t        *triangle_indices;

    statistics_t    *statistics;
};



} // namespace sk
