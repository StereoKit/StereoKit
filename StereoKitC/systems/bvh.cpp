/*
Bounding Volume Hierarchy for accelerated ray-triangle intersection testing.

Heavily inspired by Jacco Bikker's excellent series starting with
https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/

Possibile optimizations:
- Use surface area heuristic (and binning)
- Use a custom float3 value to get rid of vec3 usage in boundingbox, 
  so vec3_field() isn't needed anymore

Paul Melis, 2022
*/
#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <cassert>
#include <math.h>
#include "bvh.h"
#include "bbox.h"
#include "../stereokit.h"
#include "../sk_memory.h"
#include "../asset_types/mesh.h"

//#define VERBOSE

namespace sk {

// One node in the Bounding Volume Hierarchy

struct bvh_node_t
{
    // The bounding box for this node
    boundingbox bbox;
    
    // If num_triangles == 0 then this is an inner node, 
    // otherwise a leaf node of num_triangles triangles.
    uint32_t    num_triangles;

    // If an inner node this is the index of the left child (with the right child
    // at leaf_first+1). If a leaf node this is the index in mesh_bvh_t::sorted_triangles 
    // of the first triangle.
    uint32_t    leaf_first;

    bool is_leaf() const { return num_triangles > 0; }
};

// BVH class

mesh_bvh_t::mesh_bvh_t(int acc_leaf_size)
{
    this->acceptable_leaf_size = acc_leaf_size;
    statistics = new statistics_t;
}

mesh_bvh_t::~mesh_bvh_t()
{
    // XXX anything else to delete?
    free(nodes);
    free(sorted_triangles);
    if (statistics)
        delete statistics;
}

// Determine the bounding box that encloses the given triangles,
// which are specified as a sub-sequence of sorted_triangles, 
// i.e. sorted_triangles[first..first+count-1]
static void
bound_triangles(boundingbox& bbox, const uint32_t *sorted_triangles, const vec3 *triangle_vertices, int first, int count)
{
    bbox.clear();

    for (int t = first; t < first+count; t++)
    {
        const vec3 *p = &triangle_vertices[3*sorted_triangles[t]];
        bbox.update(p[0]);
        bbox.update(p[1]);
        bbox.update(p[2]);
    }

    // Safety margin
    bbox.grow(C_EPSILON);
}

// Convenience method for indexing a vec3 by coordinate index
inline float
vec3_field(vec3 v, int index)
{
    switch (index)
    {
    case 0:
        return v.x;
    case 1:
        return v.y;
    case 2:
        return v.z;
    default:
        return 0.0f;
    }
}

// Recursively subdivide the triangles in the current leaf node into two groups, 
// by determining a split plane based on the current bound, 
// and sorting the triangles into "left-of" and "right-of".
void
mesh_bvh_t::build_recursive(int depth, int current_node_index, 
    const vec3* triangle_vertices, const vec3* triangle_centroids, 
    const mesh_collision_t *collision_data)
{
    vec3        bbox_size, bbox_center;
    int         i, j, t;
    short       sorted_dimensions[3];
    short       split_axis;
    float       split_value;
    int         num_triangles_left, num_triangles_right;    

    bvh_node_t& node = nodes[current_node_index];

#ifdef VERBOSE
    printf("build_recursive(): depth = %d, current_node_index = %d\n", depth, current_node_index);
    printf("... leaf_first = %d, num_triangles = %d\n", node.leaf_first, node.num_triangles);
    for (i = node.leaf_first; i < node.leaf_first+node.num_triangles; i++)
        printf("[%d] %d  ", i, sorted_triangles[i]);
    printf("\n");
#endif

    if (node.num_triangles <= acceptable_leaf_size)
    {
#ifdef VERBOSE
        printf("[%d] Creating a leaf node of %d triangles\n", depth, node.num_triangles);
#endif
        if (statistics)
        {
            statistics->num_leafs++;
            statistics->max_leaf_size = std::max(statistics->max_leaf_size, node.num_triangles);
        }

        return;
    }

    //
    // Split the triangles into two groups, using a split plane based
    // on largest bbox side
    //
    
    boundingbox& bbox = node.bbox;

#ifdef VERBOSE
    printf("... bounding box:\n");
    printf("... (min) %.6f, %.6f, %.6f\n", bbox.min().x, bbox.min().y, bbox.min().z);
    printf("... (max) %.6f, %.6f, %.6f\n", bbox.max().x, bbox.max().y, bbox.max().z);    
#endif

    bbox_size = bbox[1] - bbox[0];
    bbox_center = 0.5f * (bbox[0] + bbox[1]);

    // Sort bbox sides by size in descending order

    sorted_dimensions[0] = 0;
    sorted_dimensions[1] = 1;
    sorted_dimensions[2] = 2;

    // Good old bubble sort :)
    for (i = 0; i < 2; i++)
    {
        for (j = i+1; j < 3; j++)
        {
            if (vec3_field(bbox_size, sorted_dimensions[i])
                <
                vec3_field(bbox_size, sorted_dimensions[j]))
            {
                t = sorted_dimensions[i];
                sorted_dimensions[i] = sorted_dimensions[j];
                sorted_dimensions[j] = t;
            }
        }
    }

    // Iterate over the three split dimensions in descending order of
    // bbox size on that dimension. When the triangles are unsplitable
    // (or we can't split in a favorable way) continue to the next dimension.
    // Create a leaf with all triangles in case none of dimensions is a good
    // split candidate.

    uint32_t l, r;
    boundingbox left_bbox, right_bbox;

    for (int d = 0; d < 3; d++)
    {
        // Partition the triangles on the chosen split axis, with the
        // split plane at the center of the bbox

        split_axis = sorted_dimensions[d];
        split_value = vec3_field(bbox_center, split_axis);

#ifdef VERBOSE
        printf("[%d] Splitting on axis %d, value %.6f\n", depth, split_axis, split_value);

        for (l = node.leaf_first; l < node.leaf_first+node.num_triangles; l++)
        {
            const vec3&p = triangle_centroids[sorted_triangles[l]];
            printf("[%d] tri %d, %6f, %6f, %6f\n", l, sorted_triangles[l], p.x, p.y, p.z);
        }
#endif

        l = node.leaf_first;
        r = l + node.num_triangles - 1;
        while (l <= r)
        {            
            if (vec3_field(triangle_centroids[sorted_triangles[l]], split_axis) < split_value)
                l++;
            else
                std::swap(sorted_triangles[l], sorted_triangles[r--]);
        }        

        num_triangles_left = l - node.leaf_first;
        num_triangles_right = node.num_triangles - num_triangles_left;

#ifdef VERBOSE
        printf("[%d] %d left, %d right\n", depth, num_triangles_left, num_triangles_right);
#endif

        if (num_triangles_left == 0 || num_triangles_right == 0)
        {
            // All triangles in one of the half, discontinue this attempt
#ifdef VERBOSE
            printf("[%d] skipping dimension %d, all triangles on one side\n", depth);
#endif
            continue;
        }

        // Triangles are now split into two groups, determine bboxes for each.    

        bound_triangles(left_bbox, sorted_triangles, triangle_vertices, node.leaf_first, num_triangles_left);
        bound_triangles(right_bbox, sorted_triangles, triangle_vertices, l, num_triangles_right);

#ifdef VERBOSE
        printf("[%d] left bbox:  %.6f, %.6f, %.6f .. %.6f, %.6f, %.6f\n",
            depth, left_bbox[0].x, left_bbox[0].y, left_bbox[0].z,
            left_bbox[1].x, left_bbox[1].y, left_bbox[1].z);
        printf("[%d] right bbox: %.6f, %.6f, %.6f .. %.6f, %.6f, %.6f\n",
            depth, right_bbox[0].x, right_bbox[0].y, right_bbox[0].z,
            right_bbox[1].x, right_bbox[1].y, right_bbox[1].z);
#endif

        // Create two child nodes and recurse
        
        const uint32_t left_child_index = next_node_index++;
        const uint32_t right_child_index = next_node_index++;    

        bvh_node_t& left_node = nodes[left_child_index];
        left_node.bbox = left_bbox;
        left_node.leaf_first = node.leaf_first;
        left_node.num_triangles = num_triangles_left;

        bvh_node_t& right_node = nodes[right_child_index];
        right_node.bbox = right_bbox;
        right_node.leaf_first = l;
        right_node.num_triangles = num_triangles_right;

        // Turn original leaf node into an inner node
        node.leaf_first = left_child_index;
        node.num_triangles = 0;
        
        // XXX Could check for leaf size here and only recurse when needed, instead of
        // doing the check in build_recursive()
        build_recursive(depth+1, left_child_index, triangle_vertices, triangle_centroids, collision_data);
        build_recursive(depth+1, right_child_index, triangle_vertices, triangle_centroids, collision_data);

        if (statistics)
        {
            statistics->split_axis_histogram[split_axis]++;
            statistics->num_inner_nodes++;
        }

        return;
    }

    // Split dimensions exhausted, forced to create a leaf

#ifdef VERBOSE
    // XXX warn
    printf("[%d] Split options exhausted, creating a leaf of %d triangles\n", depth, node.num_triangles);
#endif

#if 0
    char s[256];
    static int next_forced_leaf = 0;
    sprintf(s, "forced_leaf%04d.obj", next_forced_leaf++);
    printf("-> Writing %s\n", s);
    FILE *f = fopen(s, "wt");
    // XXX yuck, saves ALL scene vertices, not just the ones used by the triangles
    for (i = 0; i < the_mesh->vert_count; i++)
        fprintf(f, "v %g %g %g\n", the_mesh->verts[i].pos.x, the_mesh->verts[i].pos.y, the_mesh->verts[i].pos.z);
    // Note: 0-based indices converted to 1-based
    for (t = node.leaf_first; t < node.leaf_first+node.num_triangles; t++)
    {
        const uint32_t &tri = sorted_triangles[t];
        fprintf(f, "f %d %d %d\n", 1+the_mesh->inds[3*tri+0], 1+the_mesh->inds[3*tri+1], 1+the_mesh->inds[3*tri+2]);
    }
    fclose(f);
#endif

    if (statistics)
    {
        statistics->num_leafs++;
        statistics->num_forced_leafs++;
        statistics->max_leaf_size = std::max(statistics->max_leaf_size, node.num_triangles);
    }
}

// Build a BVH for the triangles in the mesh.
// If the statistics member is non-NULL it will contain BVH statistics 
// after construction is done.
void
mesh_bvh_t::build(const mesh_t mesh)
{    
    // XXX refuse if num triangles is zero

    const mesh_collision_t *collision_data = mesh_get_collision_data(mesh);
    if (collision_data == nullptr)
    {
        log_err("mesh_bvh_t::build(): no mesh collision data available");
        return;
    }

    // Compute triangle centroids, used during construction to partition
    // triangles in two groups

    const vert_t *vertices = mesh->verts;
    const vind_t *indices = mesh->inds;
    const uint32_t num_triangles = mesh->ind_count / 3;

    const vec3* triangle_vertices = collision_data->pts;
    vec3* triangle_centroids = sk_malloc_t(vec3, num_triangles);
    
    for (uint32_t t = 0; t < num_triangles; t++) {
        triangle_centroids[t] = 0.33333f * (
            triangle_vertices[3*t+0] + triangle_vertices[3*t+1] + triangle_vertices[3*t+2]
        );
    }

#ifdef VERBOSE
    printf("%d triangles:\n", num_triangles);
    for (int t = 0; t < num_triangles; t++)
    {
        printf("[tri %d] %d %d %d\n", t, indices[3*t+0], indices[3*t+1], indices[3*t+2]);
        for (int i = 0; i < 3; i++)
            printf("... %.6f %.6f %.6f\n", vertices[indices[3*t+i]].pos.x, vertices[indices[3*t+i]].pos.y, vertices[indices[3*t+i]].pos.z);
        printf("... centroid %.6f, %.6f, %.6f\n", triangle_centroids[t].x, triangle_centroids[t].y, triangle_centroids[t].z);
    }
#endif


    // List of triangle indices, will get reordered during construction
    sorted_triangles = sk_malloc_t(uint32_t, num_triangles);
    for (int i = 0; i < num_triangles; i++)
        sorted_triangles[i] = i;    

    // A restriction during BVH construction is that we don't want to touch the
    // underlying vertex and index arrays in the mesh. So we need to keep some local
    // array of triangle indices and sort that during BVH construction.
    // This array needs to be kept around after construction, as the BVH leaf nodes
    // will point into it to list the triangles in each node.
    //
    // Also, we need, either explicitly or implicitly, a bounding box
    // for each triangle during BVH construction to partition on a split plane. 
    // We could pre-compute those triangle bboxes and store them in a temporary array. 
    // Each bbox would take up 6 floats, i.e. 24 bytes. So for a 1M triangle model 
    // the temporary bbox array uses 24MB. 
    // For typical (smallish) SK meshes this overhead should not be a problem, but for 
    // large meshes on memory-constrained devices it could be. Plus it would come
    // on top of the array mentioned above.
    //
    // The alternative is to duplicate the original mesh->inds array here
    // and sort that during BVH construction, at the expense of having
    // to consult mesh->verts[i/j/k].pos each time we need a triangle extent.
    // Also, we will be manipulating 12 bytes per triangle (assuming 32-bit indices), 
    // instead of a triangle index of 4 bytes that would look up into the temporary bbox array. 
    // Using a local index array would use num_triangles*3*4 bytes, 
    // so 12MB for a 1M triangle mesh. Plus it can serve as the array the BVH leaf
    // node reference. 

    boundingbox mesh_bbox;
    bound_triangles(mesh_bbox, sorted_triangles, triangle_vertices, 0, num_triangles-1);

#ifdef VERBOSE
    printf("bvh_build():\n");
    printf("... mesh of %d triangles\n", num_triangles);
    printf("... bounding box:\n");
    printf("... (min) %.6f, %.6f, %.6f\n", mesh_bbox.min().x, mesh_bbox.min().y, mesh_bbox.min().z);
    printf("... (max) %.6f, %.6f, %.6f\n", mesh_bbox.max().x, mesh_bbox.max().y, mesh_bbox.max().z);
#endif

    mesh_bbox.grow(C_EPSILON);

    if (statistics)
    {
        statistics->num_leafs = 0;
        statistics->num_inner_nodes = 0;
        statistics->max_leaf_size = 0;
        statistics->num_forced_leafs = 0;
        for (int i = 0; i < 3; i++)
            statistics->split_axis_histogram[i] = 0;
    }

    // A balanced BVH built fully to depth D will have
    //     2^(D-1)-1    inner nodes
    // and
    //     2^(D-1)      leaf nodes
    //
    // Divide the total number of triangles to store by acceptable_leaf_size
    // to get an estimate of the number of leaf nodes needed. Then, set
    // the maximum depth to a value that will allow that number of
    // leafs to be reached.
    //
    // As the tree in reality will almost always not be nicely balanced, 
    // the above does not guarantee that there won't be instances in which the
    // maximum depth is reached and a leaf needs to be created. So we
    // add more levels just to be sure :)
    
    int estimated_num_leafs = (int)(1.0f * num_triangles / acceptable_leaf_size + 0.5f);

#ifdef VERBOSE    
    printf("... Estimated number of leaf nodes needed = %d\n", estimated_num_leafs);
#endif   

    //acceptable_leaf_size = 2;

    the_mesh = mesh;    // XXX debugging only

    // Allocate some necessary arrays 
    
    // XXX need to trim this number    
    nodes = sk_malloc_t(bvh_node_t, num_triangles*2);

    // Build the BVH

    bvh_node_t& root_node = nodes[0];
    root_node.leaf_first = 0;
    root_node.num_triangles = num_triangles;
    root_node.bbox = mesh_bbox;
    next_node_index = 1;

    build_recursive(1, 0, triangle_vertices, triangle_centroids, collision_data);

    if (statistics)
    {
        printf("BVH statistics:\n");
        statistics->print();
    }    

    // Clean up

    free(triangle_centroids);
}

void
mesh_bvh_t::statistics_t::print()
{    
    printf("... %d leaf nodes, %d inner nodes\n",
        num_leafs, num_inner_nodes);
    printf("... maximum leaf size %d\n", max_leaf_size);
    printf("... %d forced leafs\n", num_forced_leafs);
    printf("... Split axis histogram:\n");
    for (int i = 0; i < 3; i++)
        printf("... %d | %5d\n", i, split_axis_histogram[i]);
}

}
