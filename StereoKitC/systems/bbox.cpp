#include "bbox.h"

namespace sk {

// Update the bounding box to include the point p
void
bbox_update(boundingbox& bbox, vec3 p)
{
    vec3 *bounds = bbox.bounds;

    if (p.x < bounds[0].x) bounds[0].x = p.x;
    if (p.y < bounds[0].y) bounds[0].y = p.y;
    if (p.z < bounds[0].z) bounds[0].z = p.z;

    if (p.x > bounds[1].x) bounds[1].x = p.x;
    if (p.y > bounds[1].y) bounds[1].y = p.y;
    if (p.z > bounds[1].z) bounds[1].z = p.z;
}

// Return the bounding box resulting from the combination of the
// two given bounding boxes
boundingbox
bbox_combine(const boundingbox& bbox1, const boundingbox& bbox2)
{
    vec3 min, max;

    min = bbox_min(bbox1);
    max = bbox_max(bbox1);

    const vec3 *bounds2 = bbox2.bounds;

    if (bounds2[0].x < min.x) min.x = bounds2[0].x;
    if (bounds2[0].y < min.y) min.y = bounds2[0].y;
    if (bounds2[0].z < min.z) min.z = bounds2[0].z;

    if (bounds2[1].x > max.x) max.x = bounds2[1].x;
    if (bounds2[1].y > max.y) max.y = bounds2[1].y;
    if (bounds2[1].z > max.z) max.z = bounds2[1].z;

    return boundingbox{min, max};
}


// Intersect a ray (delimited by t0 and t1) with the given bounding box.
// Returns true (and sets t_min and t_max) when the ray intersects the bbox,
// returns false otherwise

bool
bbox_intersect_full(const boundingbox& bbox, float& t_min, float& t_max, bbox_ray_t r, float t0, float t1)
{
    // This code makes handy use of IEEE floating-point behaviour, such
    // as division by zero. See
    // 1) Shirley et.al. Section 10.9.1
    // 2) Williams et.al., "An Efficient and Robust Ray-Box Intersection Algorithm"

    float tmin, tmax;
    float tymin, tymax, tzmin, tzmax;
    const vec3* bounds = bbox.bounds;

    tmin = (bounds[r.sign[0]].x - r.origin.x) * r.inv_direction.x;
    tmax = (bounds[1-r.sign[0]].x - r.origin.x) * r.inv_direction.x;

    tymin = (bounds[r.sign[1]].y - r.origin.y) * r.inv_direction.y;
    tymax = (bounds[1-r.sign[1]].y - r.origin.y) * r.inv_direction.y;

    if (tmin > tymax || tymin > tmax)
        return false;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[r.sign[2]].z - r.origin.z) * r.inv_direction.z;
    tzmax = (bounds[1-r.sign[2]].z - r.origin.z) * r.inv_direction.z;

    if (tmin > tzmax || tzmin > tmax)
        return false;

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    t_min = tmin;
    t_max = tmax;

    return (tmin < t1 && tmax > t0);
}

// Intersect a ray with the given bounding box.
// Returns true when the ray intersects the bbox, returns false otherwise

bool
bbox_intersect(const boundingbox& bbox, bbox_ray_t r, float t0, float t1)
{
    // This code makes handy use of IEEE floating-point behaviour, such
    // as division by zero. See
    // 1) Shirley et.al. Section 10.9.1
    // 2) Williams et.al., "An Efficient and Robust Ray-Box Intersection Algorithm"

    float tmin, tmax;
    float tymin, tymax, tzmin, tzmax;
    const vec3* bounds = bbox.bounds;

    tmin = (bounds[r.sign[0]].x - r.origin.x) * r.inv_direction.x;
    tmax = (bounds[1-r.sign[0]].x - r.origin.x) * r.inv_direction.x;

    tymin = (bounds[r.sign[1]].y - r.origin.y) * r.inv_direction.y;
    tymax = (bounds[1-r.sign[1]].y - r.origin.y) * r.inv_direction.y;

    if (tmin > tymax || tymin > tmax)
        return false;

    if (tymin > tmin)
        tmin = tymin;
    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[r.sign[2]].z - r.origin.z) * r.inv_direction.z;
    tzmax = (bounds[1-r.sign[2]].z - r.origin.z) * r.inv_direction.z;

    if (tmin > tzmax || tzmin > tmax)
        return false;

    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;

    return (tmin < t1 && tmax > t0);
}
    
} // namespace sk
