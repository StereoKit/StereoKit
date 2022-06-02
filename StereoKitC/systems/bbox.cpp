#include "bbox.h"

// Intersect a ray (delimited by t0 and t1) with the given bounding box.
// Returns 1 (and sets t_min and t_max) when the ray intersects the bbox,
// returns 0 otherwise

namespace sk {

/*
bool
boundingbox::intersect_full(float& t_min, float& t_max, Ray r, float t0, float t1) const
{
    // This code makes handy use of IEEE floating-point behaviour, such
    // as division by zero. See
    // 1) Shirley et.al. Section 10.9.1
    // 2) Williams et.al., "An Efficient and Robust Ray-Box Intersection Algorithm"

    float   tmin, tmax;
    float   tymin, tymax, tzmin, tzmax;

    //num_bboxes_tested++;

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
// Returns 1 when the ray intersects the bbox, returns 0 otherwise

bool
boundingbox::intersect(Ray r, float t0, float t1) const
{
    // This code makes handy use of IEEE floating-point behaviour, such
    // as division by zero. See
    // 1) Shirley et.al. Section 10.9.1
    // 2) Williams et.al., "An Efficient and Robust Ray-Box Intersection Algorithm"

    //num_bboxes_tested++;

    float   tmin, tmax;
    float   tymin, tymax, tzmin, tzmax;

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

    //printf("tmin = %.6f, tmax = %.6f; t0 = %.6f, t1 = %.6f\n", tmin, tmax, t0, t1);

    return (tmin < t1 && tmax > t0);
}
*/
    
}
