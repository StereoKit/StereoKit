/*
Bounding Volume Hierarchy for accelerated ray-triangle intersection testing.

Based on my existing ray tracing code, but also heavily inspired by 
Jacco Bikker's excellent BVH series starting with
https://jacco.ompf2.com/2022/04/13/how-to-build-a-bvh-part-1-basics/

Possible optimizations:
- Use surface area heuristic for determining when to stop
  partitioning, instead of the fixed max leaf size currently used
- Add multiple split plane candidates, particularly binned SAH
- Use a custom float3 value to get rid of vec3 usage in boundingbox, 
  so vec3_field() isn't needed anymore
- SIMD operations for certain computations
- Multi-threaded construction
- Currently, the split step during recursive construction simply gives up 
  when it encounters a bunch of triangles for which none of the 3 split
  axes provide any way to split the triangles into two groups (i.e. all 
  triangles found to be on the left and/or all on the right). In those cases
  currently a leaf node holding all those triangles is created. But an 
  option could be to simply divide the triangles into two groups randomly
  and recurse into the two groups. This will still provide a speedup, as
  bboxes can be tested for each group and discarded early, instead of always
  having to test each triangle separately.

June, 2022
Paul Melis, SURF (paul.melis@surf.nl)
*/
#define NOMINMAX
#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <cassert>
#include <math.h>
#include "bvh.h"
#include "bbox.h"
#include "../stereokit.h"
#include "../sk_memory.h"
#include "../sk_math.h"
#include "../asset_types/mesh.h"
#include "../libraries/sokol_time.h"

//#define VERBOSE_BUILD
//#define VERBOSE_INTERSECTION
//#define VERBOSE_STATS

namespace sk {

const int TRAVERSAL_STACK_SIZE = 128;

// One node in the Bounding Volume Hierarchy

struct bvh_node_t
{
    // The bounding box for this node
    boundingbox bbox;
    
    // If num_triangles == 0 then this is an inner node, 
    // otherwise a leaf node of num_triangles triangles.
    uint32_t    num_triangles;

    // If an inner node this is the index of the left child (with the right child
    // at leaf_first+1). If a leaf node this is the index in sorted_triangles[]
    // of the first triangle.
    uint32_t    leaf_first;

    bool is_leaf() const { return num_triangles > 0; }
};

// Statistics on a built BVH

static void
bvh_stats_clear(bvh_stats_t *stats)
{
    stats->depth = 0;
    stats->num_leafs = 0;
    stats->num_inner_nodes = 0;
    stats->max_leaf_size = 0;
    stats->num_forced_leafs = 0;
}

static void
gather_stats(const mesh_bvh_t *bvh, short depth, int current_node_index, bvh_stats_t *stats, int acc_leaf_size)
{
    const bvh_node_t& node = bvh->nodes[current_node_index];
    stats->depth = (short)maxi(stats->depth, depth);

    if (node.is_leaf())
    {
        stats->num_leafs++;
        stats->max_leaf_size = maxi(stats->max_leaf_size, node.num_triangles);
        if (node.num_triangles > (uint32_t)acc_leaf_size)
            stats->num_forced_leafs++;
    }
    else
    {
        stats->num_inner_nodes++;
        gather_stats(bvh, depth+1, node.leaf_first, stats, acc_leaf_size);
        gather_stats(bvh, depth+1, node.leaf_first+1, stats, acc_leaf_size);
    }
}

void        
mesh_bvh_statistics(const mesh_bvh_t *bvh, bvh_stats_t *stats, int acc_leaf_size)
{
    bvh_stats_clear(stats);
    gather_stats(bvh, 1, 0, stats, acc_leaf_size);
}

// Determine the bounding box that encloses the given triangles,
// which are specified as a sub-sequence of sorted_triangles, 
// i.e. sorted_triangles[first..first+count-1]
static void
bound_triangles(boundingbox& bbox, const uint32_t *sorted_triangles, const vec3 *triangle_vertices, int first, int count)
{
    bbox_clear(bbox);

    for (int t = first; t < first+count; t++)
    {
        const vec3 *p = &triangle_vertices[3*sorted_triangles[t]];
        bbox_update(bbox, p[0]);
        bbox_update(bbox, p[1]);
        bbox_update(bbox, p[2]);
    }

    // Safety margin
    bbox_grow(bbox, C_EPSILON);
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
static void
mesh_bvh_build_recursive(int current_node_index, 
    bvh_node_t *nodes, uint32_t *next_node_index,
    uint32_t *sorted_triangles,
    int acceptable_leaf_size, 
    const vec3* triangle_vertices, const vec3* triangle_centroids, 
    const mesh_collision_t *collision_data)
{
    vec3        bbox_size, bbox_center;
    int         i, j, t;
    short       sorted_dimensions[3];
    short       split_axis;
    float       split_value;

    bvh_node_t& node = nodes[current_node_index];

#ifdef VERBOSE_BUILD
    printf("build_recursive(): current_node_index = %d\n", current_node_index);
    printf("... leaf_first = %d, num_triangles = %d\n", node.leaf_first, node.num_triangles);
    for (i = node.leaf_first; i < node.leaf_first+node.num_triangles; i++)
        printf("[%d] %d  ", i, sorted_triangles[i]);
    printf("\n");
#endif

    if (node.num_triangles <= (uint32_t)acceptable_leaf_size)
    {
#ifdef VERBOSE_BUILD
        printf("Creating a leaf node of %d triangles\n", node.num_triangles);
#endif

        return;
    }

    //
    // Split the triangles into two groups, using a split plane based
    // on largest bbox side
    //
    
    boundingbox& bbox = node.bbox;

#ifdef VERBOSE_BUILD
    printf("... bounding box:\n");
    printf("... (min) %.6f, %.6f, %.6f\n", bbox_min(bbox).x, bbox_min(bbox).y, bbox_min(bbox).z);
    printf("... (max) %.6f, %.6f, %.6f\n", bbox_max(bbox).x, bbox_max(bbox).y, bbox_max(bbox).z);
#endif

    bbox_size = bbox.bounds[1] - bbox.bounds[0];
    bbox_center = 0.5f * (bbox.bounds[0] + bbox.bounds[1]);

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
                sorted_dimensions[j] = (short)t;
            }
        }
    }

    // Iterate over the three split dimensions in descending order of
    // bbox size on that dimension. When the triangles are unsplitable
    // (or we can't split in a favorable way) continue to the next dimension.
    // Create a leaf with all triangles in case none of the dimensions is a 
    // good split candidate.

    uint32_t l, r;
    boundingbox left_bbox, right_bbox;

    for (int d = 0; d < 3; d++)
    {
        // Partition the triangles on the chosen split axis, with the
        // split plane at the center of the bbox

        split_axis = sorted_dimensions[d];
        split_value = vec3_field(bbox_center, split_axis);

#ifdef VERBOSE_BUILD
        printf("Splitting on axis %d, value %.6f\n", split_axis, split_value);

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
            {
                uint32_t temp = sorted_triangles[l];
                sorted_triangles[l] = sorted_triangles[r];
                sorted_triangles[r--] = temp;
            }
        }        

        const int num_triangles_left = l - node.leaf_first;
        const int num_triangles_right = node.num_triangles - num_triangles_left;

#ifdef VERBOSE_BUILD
        printf("%d left, %d right\n", num_triangles_left, num_triangles_right);
#endif

        if (num_triangles_left == 0 || num_triangles_right == 0)
        {
            // All triangles in one of the half, discontinue this attempt
#ifdef VERBOSE_BUILD
            printf("Skipping dimension %d, all triangles on one side\n");
#endif
            continue;
        }

        // Triangles are now split into two groups, determine bboxes for each.    

        bound_triangles(left_bbox, sorted_triangles, triangle_vertices, node.leaf_first, num_triangles_left);
        bound_triangles(right_bbox, sorted_triangles, triangle_vertices, l, num_triangles_right);

#ifdef VERBOSE_BUILD
        printf("left bbox:  %.6f, %.6f, %.6f .. %.6f, %.6f, %.6f\n",
            left_bbox.bounds[0].x, left_bbox.bounds[0].y, left_bbox.bounds[0].z,
            left_bbox.bounds[1].x, left_bbox.bounds[1].y, left_bbox.bounds[1].z);
        printf("right bbox: %.6f, %.6f, %.6f .. %.6f, %.6f, %.6f\n",
            right_bbox.bounds[0].x, right_bbox.bounds[0].y, right_bbox.bounds[0].z,
            right_bbox.bounds[1].x, right_bbox.bounds[1].y, right_bbox.bounds[1].z);
#endif

        // Create two child nodes and recurse
        
        const uint32_t left_child_index = *next_node_index;
        const uint32_t right_child_index = left_child_index + 1;
        *next_node_index += 2;

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
        mesh_bvh_build_recursive(left_child_index, nodes, next_node_index, sorted_triangles, acceptable_leaf_size, 
            triangle_vertices, triangle_centroids, collision_data);
        mesh_bvh_build_recursive(right_child_index, nodes, next_node_index, sorted_triangles, acceptable_leaf_size, 
            triangle_vertices, triangle_centroids, collision_data);

        return;
    }

    // Split dimensions exhausted, forced to create a leaf

#ifdef VERBOSE_BUILD
    // XXX warn
    printf("Split options exhausted, creating a leaf of %d triangles\n", node.num_triangles);
#endif

#if 0
    // Save forced leafs to .obj files, for debugging
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
}

// Build a BVH over the triangles in the given mesh.
mesh_bvh_t*
mesh_bvh_create(const mesh_t mesh, int acc_leaf_size, bool show_stats)
{
    // XXX refuse if num triangles is zero?
#if defined(VERBOSE_STATS)
    const double t0 = time_get_raw();
#endif

    mesh_bvh_t *bvh = sk_calloc_t(mesh_bvh_t, 1);

    // A restriction during BVH construction is that we don't want to touch the
    // underlying vertex and index arrays in the passed mesh. So we need to keep some local
    // array of triangle indices and sort that during BVH construction.
    // This array needs to be kept around after construction, as the BVH leaf nodes
    // will point into it to list the triangles in each node.
    //
    // Also, we need, either explicitly or implicitly, a bounding box
    // for each triangle during BVH construction. We could pre-compute those triangle 
    // bboxes and store them in a temporary array. But each bbox would take up 6 floats, 
    // i.e. 24 bytes. So for a 1M triangle model  the temporary bbox array uses 24MB. 
    // For typical (smallish) SK meshes this overhead should not be a problem, but for 
    // large meshes on memory-constrained devices it could be. Plus it would come
    // on top of the array mentioned above.
    //
    // Instead, we leverage the existing mesh collision data, which provides
    // an array of triangle vertices, to precompute triangle centroids, which
    // are then used during BVH construction. Whenever a bounding box of a 
    // group of triangles is needed this is computed on-the-fly.

    bvh->collision_data = mesh_get_collision_data(mesh);
    if (bvh->collision_data == nullptr)
    {
        log_err("mesh_bvh_t::build(): no mesh collision data available");
        return nullptr;
    }

    // Compute triangle centroids, used during construction to partition
    // triangles in two groups

    const uint32_t num_triangles = mesh->ind_count / 3;

    const vec3* triangle_vertices = bvh->collision_data->pts;
    vec3* triangle_centroids = sk_malloc_t(vec3, num_triangles);
    
    for (uint32_t t = 0; t < num_triangles; t++) {
        triangle_centroids[t] = 0.33333f * (
            triangle_vertices[3*t+0] + triangle_vertices[3*t+1] + triangle_vertices[3*t+2]
        );
    }

#ifdef VERBOSE_BUILD
    const vert_t* vertices = mesh->verts;
    const vind_t* indices  = mesh->inds;

    printf("%d triangles:\n", num_triangles);
    for (int t = 0; t < num_triangles; t++)
    {
        printf("[tri %d] %d %d %d\n", t, indices[3*t+0], indices[3*t+1], indices[3*t+2]);
        for (int i = 0; i < 3; i++)
            printf("... %.6f %.6f %.6f\n", vertices[indices[3*t+i]].pos.x, vertices[indices[3*t+i]].pos.y, vertices[indices[3*t+i]].pos.z);
        printf("... centroid %.6f, %.6f, %.6f\n", triangle_centroids[t].x, triangle_centroids[t].y, triangle_centroids[t].z);
    }
#endif

    // List of triangle indices, which will get reordered during construction
    uint32_t *sorted_triangles = bvh->sorted_triangles = sk_malloc_t(uint32_t, num_triangles);
    for (uint32_t i = 0; i < num_triangles; i++)
        sorted_triangles[i] = i;

    // Compute mesh bounding box (could reuse what's in mesh_t, but not sure it's accurate)

    boundingbox mesh_bbox;
    bound_triangles(mesh_bbox, sorted_triangles, triangle_vertices, 0, num_triangles-1);

#ifdef VERBOSE_BUILD
    printf("bvh_build():\n");
    printf("... mesh of %d triangles\n", num_triangles);
    printf("... bounding box:\n");
    printf("... (min) %.6f, %.6f, %.6f\n", bbox_min(mesh_bbox).x, bbox_min(mesh_bbox).y, bbox_min(mesh_bbox).z);
    printf("... (max) %.6f, %.6f, %.6f\n", bbox_max(mesh_bbox).x, bbox_max(mesh_bbox).y, bbox_max(mesh_bbox).z);
    printf("... acceptable leaf size %d\n", acc_leaf_size);
#endif

    bbox_grow(mesh_bbox, C_EPSILON);
    bvh->the_mesh = mesh;

    // We pre-allocate an array of BVH nodes, enough to always fit. 
    // XXX After construction is done, and the actual
    // number of nodes needed is known we should trim
    // the array.

    bvh_node_t *nodes = bvh->nodes = sk_malloc_t(bvh_node_t, num_triangles*2);

    // Bootstrap with a single leaf node holding all triangles
    
    bvh_node_t& root_node = nodes[0];
    root_node.leaf_first = 0;
    root_node.num_triangles = num_triangles;
    root_node.bbox = mesh_bbox;

    // XXX could return value from build function, instead of using a variable
    uint32_t next_node_index = 1;

    // Build the BVH

    mesh_bvh_build_recursive(0, nodes, &next_node_index, sorted_triangles,
        acc_leaf_size, triangle_vertices, triangle_centroids, bvh->collision_data);

#if defined(VERBOSE_STATS)
    const double t1 = time_get_raw();

    // XXX use log function
    printf("BVH construction done in %.1fms\n", 1000*(t1-t0));

    // Done!

    if (show_stats)
    {
        printf("BVH statistics:\n");
        printf("... %d triangles\n", bvh->the_mesh->ind_count/3);
        bvh_stats_t stats;
        mesh_bvh_statistics(bvh, &stats, acc_leaf_size);
        printf("... depth %d\n", stats.depth);
        printf("... %d leaf nodes, %d inner nodes\n",
            stats.num_leafs, stats.num_inner_nodes);
        printf("... maximum leaf size %d\n", stats.max_leaf_size);
        printf("... %d forced leafs (%.1f%%)\n", stats.num_forced_leafs, 100.0f * stats.num_forced_leafs / stats.num_leafs);
    }
#endif

    // Clean up

    free(triangle_centroids);

    return bvh;
}

void
mesh_bvh_destroy(mesh_bvh_t *bvh)
{
    free(bvh->nodes);
    free(bvh->sorted_triangles);
    *bvh = {};
}

// Find closest triangle intersection for the given model-space ray
bool
mesh_bvh_intersect(const mesh_bvh_t *bvh, ray_t model_space_ray, ray_t *out_pt, uint32_t *out_start_inds, cull_ cull_mode)
{
    const bvh_node_t *nodes = bvh->nodes;
    const uint32_t *sorted_triangles = bvh->sorted_triangles;
    const mesh_collision_t *collision_data = bvh->collision_data;

    // Use local stack to avoid having to recurse
    uint32_t traversal_node_stack[TRAVERSAL_STACK_SIZE];
    float traversal_tmin_stack[TRAVERSAL_STACK_SIZE];

    uint32_t left_child;
    bool traverse_left_child, traverse_right_child;
    
    float t_left_min, t_right_min;
    float t_left_max, t_right_max;
    float t_hit, t_nearest_hit = FLT_MAX;
    float nearest_dist = FLT_MAX;
    vec3  pt = {};

    // Stack is initially empty
    short stack_top = -1;

    // Start at root node
    uint32_t current_node_index = 0;

    // Pre-compute a ray value that's quicker to test against the bboxes
    bbox_ray_t bbox_ray(model_space_ray);

    while (true)
    {        
#ifdef VERBOSE_INTERSECTION
        printf("t_nearest_hit = %.6f\n", t_nearest_hit);
        printf("current node = %d\n", current_node_index);
        printf("stack_top = %d\n", stack_top);

        printf("stack now:\n");
        for (int i = 0; i <= stack_top; i++)
            printf("%d (%.6f)\n", traversal_node_stack[i], traversal_tmin_stack[i]);
#endif

        const bvh_node_t& node = nodes[current_node_index];

        if (!node.is_leaf())
        {
            // Check for traversal down to the child nodes of this inner node
            left_child = node.leaf_first;

            traverse_left_child = bbox_intersect_full(nodes[left_child].bbox, t_left_min, t_left_max, bbox_ray, 0.0f, t_nearest_hit);
            traverse_right_child = bbox_intersect_full(nodes[left_child+1].bbox, t_right_min, t_right_max, bbox_ray, 0.0f, t_nearest_hit);

#ifdef VERBOSE_INTERSECTION
            if (traverse_left_child)
                printf("traverse left, node %d, t=%.6f..%.6f\n", left_child, t_left_min, t_left_max);
            if (traverse_right_child)
                printf("traverse right, node %d, t=%.6f..%.6f\n", left_child+1, t_right_min, t_right_max);
#endif

            if (traverse_left_child)
            {
                if (traverse_right_child)
                {
                    // Traverse both children, but traverse into closest one first

                    if (t_left_min < t_right_min)
                    {
                        // Left, possibly right

                        // Push right
                        stack_top++;
                        traversal_node_stack[stack_top] = left_child+1;
                        traversal_tmin_stack[stack_top] = t_right_min;

                        // Traverse to left
#ifdef VERBOSE_INTERSECTION
                        printf("traversing left, then right\n");
#endif
                        current_node_index = left_child;
                    }
                    else
                    {
                        // Right, possibly left

                        // Push left
                        stack_top++;
                        traversal_node_stack[stack_top] = left_child;
                        traversal_tmin_stack[stack_top] = t_left_min;

                        // Traverse to right
#ifdef VERBOSE_INTERSECTION
                        printf("traversing right, then left\n");
#endif
                        current_node_index = left_child+1;
                    }
                }
                else
                {
                    // Traverse left only
#ifdef VERBOSE_INTERSECTION
                    printf("traversing left only\n");
#endif                    
                    current_node_index = left_child;
                }

                continue;
            }
            else if (traverse_right_child)
            {
                // Traverse right only
#ifdef VERBOSE_INTERSECTION
                printf("traversing right only\n");
#endif
                current_node_index = left_child+1;
                continue;
            }
        }
        else
        {
            // Intersect ray with triangles in this leaf node

#ifdef VERBOSE_INTERSECTION
            printf("Checking %d triangles\n", node.num_triangles);
#endif
            for (uint32_t t = node.leaf_first; t < node.leaf_first+node.num_triangles; t++)
            {
                uint32_t triangle = sorted_triangles[t];
                const plane_t& plane = collision_data->planes[triangle];

                // Inline version of plane_ray_intersect(), as we need the t value
                // XXX use cull_mode value based on dot denom
                float denom = vec3_dot(model_space_ray.dir, plane.normal);

                if (fabsf(denom) < 1e-6f)
                {
                    // Ray direction (almost) perpendicular to plane, no intersection
                    continue;
                }

                if ((cull_mode == cull_front && denom < 0) || (cull_mode == cull_back && denom > 0))
                {
                    // Front/back-face culling
                    // XXX is there a smaller test?
                    continue;
                }

                t_hit = -(vec3_dot(model_space_ray.pos, plane.normal) + plane.d) / denom;

                if (t_hit >= t_nearest_hit)
                {
                    // Hit will not be closer than best one found so far, no need to check further
                    continue;
                }

                pt = model_space_ray.pos + model_space_ray.dir * t_hit;

                // point in triangle, implementation based on:
                // https://blackpawn.com/texts/pointinpoly/default.html

                // Compute vectors
                vec3 v0 = collision_data->pts[3*triangle+1] - collision_data->pts[3*triangle+0];
                vec3 v1 = collision_data->pts[3*triangle+2] - collision_data->pts[3*triangle+0];
                vec3 v2 = pt - collision_data->pts[3*triangle+0];

                // Compute dot products
                float dot00 = vec3_dot(v0, v0);
                float dot01 = vec3_dot(v0, v1);
                float dot02 = vec3_dot(v0, v2);
                float dot11 = vec3_dot(v1, v1);
                float dot12 = vec3_dot(v1, v2);

                // Compute barycentric coordinates
                float inv_denom = 1.0f / (dot00 * dot11 - dot01 * dot01);
                float u = (dot11 * dot02 - dot01 * dot12) * inv_denom;
                float v = (dot00 * dot12 - dot01 * dot02) * inv_denom;

                // Check if point is in triangle
                if ((u >= 0) && (v >= 0) && (u + v < 1)) {
                    float dist = vec3_magnitude_sq(pt - model_space_ray.pos);
                    if (t_hit > 0 && t_hit < t_nearest_hit) {
#ifdef VERBOSE_INTERSECTION
                        printf("Found new hit at t = %.6f, distance^2 = %.6f\n", t_hit, dist);
#endif                        
                        t_nearest_hit = t_hit;
                        nearest_dist = dist;
                        if (out_start_inds != nullptr) {
                            *out_start_inds = 3*triangle;
                        }
                        *out_pt = {pt, collision_data->planes[triangle].normal};
                    }
                }
            }
        }

        // Pop from stack

        if (stack_top == -1)
        {
            // Empty stack, done!
            return nearest_dist != FLT_MAX;
        }

        // Keep popping next node to visit until we find a node whose bbox
        // is intersected closer to the ray origin than the best hit
        // found so far (or the stack becomes empty)

#ifdef VERBOSE_INTERSECTION
        printf("popping from stack:\n");
        for (int i = 0; i <= stack_top; i++)
            printf("%d (%.6f)\n", traversal_node_stack[i], traversal_tmin_stack[i]);
#endif

        while (stack_top >= 0)
        {
            current_node_index = traversal_node_stack[stack_top];
            if (traversal_tmin_stack[stack_top--] < t_nearest_hit)
            {
                // Traverse to node
                break;
            }

            if (stack_top == -1)
            {
                // Empty stack, done!
                return nearest_dist != FLT_MAX;
            }
        }
    }
}

} // namespace sk

