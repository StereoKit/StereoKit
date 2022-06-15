#pragma once

#include "../stereokit.h"

const float C_EPSILON = 1e-6f;
const float C_INFINITY = 1e6f;

namespace sk {

// Ray with some pre-computed values, for faster bbox intersection queries
struct bbox_ray_t
{    
    bbox_ray_t(ray_t ray)
    {
        origin = ray.pos;
        inv_direction = vec3{1.0f/ray.dir.x, 1.0f/ray.dir.y, 1.0f/ray.dir.z};
        sign[0] = inv_direction.x < 0;
        sign[1] = inv_direction.y < 0;
        sign[2] = inv_direction.z < 0;        
    }

    vec3 origin;
    vec3 inv_direction;
    short sign[3];
};

//
// An axis-aligned bounding box (AABB)
//

struct boundingbox
{
    vec3 bounds[2];
};

// Empty bounding box (i.e. min > max)
inline void
bbox_clear(boundingbox& bbox)
{
    bbox.bounds[0] = vec3{C_INFINITY, C_INFINITY, C_INFINITY};
    bbox.bounds[1] = vec3{-C_INFINITY, -C_INFINITY, -C_INFINITY};
}

inline void
bbox_grow(boundingbox& bbox, float f)
{
    bbox.bounds[0] -= vec3{f, f, f};
    bbox.bounds[1] += vec3{f, f, f};
}

inline vec3
bbox_min(const boundingbox& bbox)
{
    return bbox.bounds[0];
}

inline vec3
bbox_max(const boundingbox& bbox)
{
    return bbox.bounds[1];
}

inline vec3
bbox_center(const boundingbox& bbox)
{
    return 0.5f*(bbox.bounds[0]+bbox.bounds[1]);
}

inline vec3
bbox_size(const boundingbox& bbox)
{
    return bbox.bounds[1] - bbox.bounds[0];
}

// Return the volume of the bounding box
inline float
bbox_volume(const boundingbox& bbox)
{
    return (bbox.bounds[1].x - bbox.bounds[0].x) * (bbox.bounds[1].y - bbox.bounds[0].y) * (bbox.bounds[1].z - bbox.bounds[0].z);
}

inline float
bbox_surface_area(const boundingbox& bbox)
{
    const vec3 s = bbox_size(bbox);
    return 2.0f * (s.x*s.y + s.x*s.z + s.y*s.z);
}

// 0 -> min, 1 -> max
inline vec3
bbox_item(const boundingbox& bbox, short index)
{
    return bbox.bounds[index];
}

// Update the bounding box to include the point p
void bbox_update(boundingbox& bbox, vec3 p);

// Return the bounding box resulting from the combination of the
// two given bounding boxes
boundingbox bbox_combine(const boundingbox& bbox1, const boundingbox& bbox2);

// Intersect a ray with (delimited by t0 and t1) with the given bounding box.
// Returns true when the ray intersects the bbox, returns false otherwise
bool bbox_intersect(const boundingbox& bbox, bbox_ray_t r, float t0, float t1);

// Intersect a ray with (delimited by t0 and t1) with the given bounding box.
// Returns 1 (and sets t_min and t_max) when the ray intersects the bbox,
// returns 0 otherwise
bool bbox_intersect_full(const boundingbox& bbox, float& t_min, float& t_max, bbox_ray_t r, float t0, float t1);

} // namespace sk
