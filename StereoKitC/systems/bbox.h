#pragma once

#include "../stereokit.h"
//#include "ray.h"

const float C_EPSILON = 1e-6f;
const float C_INFINITY = 1e6f;

namespace sk {

//
// An axis-aligned bounding box (AABB)
//

class boundingbox
{
public:
    // Create a bounding box (initialized to be empty, i.e. min > max)
    boundingbox()
    {
        bounds[0] = vec3{C_INFINITY, C_INFINITY, C_INFINITY};
        bounds[1] = vec3{-C_INFINITY, -C_INFINITY, -C_INFINITY};
    }

    boundingbox(const boundingbox& bbox)
    {
        bounds[0] = bbox.bounds[0];
        bounds[1] = bbox.bounds[1];
    }

    boundingbox(const vec3& minval, const vec3& maxval)
    {
        bounds[0] = minval;
        bounds[1] = maxval;
    }

    boundingbox(float *v)
    {
        bounds[0] = vec3{v[0], v[1], v[2]};
        bounds[1] = vec3{v[3], v[4], v[5]};
    }

    void grow(float f)
    {
        bounds[0] -= vec3{f, f, f};
        bounds[1] += vec3{f, f, f};
    }

    void clear()
    {
        bounds[0] = vec3{C_INFINITY, C_INFINITY, C_INFINITY};
        bounds[1] = vec3{-C_INFINITY, -C_INFINITY, -C_INFINITY};
    }

    vec3    min() const         { return bounds[0]; }
    vec3    max() const         { return bounds[1]; }
    vec3&   min()               { return bounds[0]; }
    vec3&   max()               { return bounds[1]; }

    vec3    center() const      { return 0.5f*(bounds[0]+bounds[1]); }

    // 0 -> min, 1 -> max
    vec3&       operator[](int i)       { return bounds[i]; }
    const vec3  operator[](int i) const { return bounds[i]; }

    // Return the bounding box resulting from the combination of the
    // two given bounding boxes
    static boundingbox combine(const boundingbox& bbox1, const boundingbox& bbox2)
    {
        vec3    min, max;

        min = bbox1.min();
        max = bbox1.max();

        if (bbox2.min().x < min.x) min.x = bbox2.min().x;
        if (bbox2.min().y < min.y) min.y = bbox2.min().y;
        if (bbox2.min().z < min.z) min.z = bbox2.min().z;

        if (bbox2.max().x > max.x) max.x = bbox2.max().x;
        if (bbox2.max().y > max.y) max.y = bbox2.max().y;
        if (bbox2.max().z > max.z) max.z = bbox2.max().z;

        return boundingbox(min, max);
    }

    // Update the bounding box to include the point p
    void update(vec3 p)
    {
        if (p.x < bounds[0].x) bounds[0].x = p.x;
        if (p.y < bounds[0].y) bounds[0].y = p.y;
        if (p.z < bounds[0].z) bounds[0].z = p.z;

        if (p.x > bounds[1].x) bounds[1].x = p.x;
        if (p.y > bounds[1].y) bounds[1].y = p.y;
        if (p.z > bounds[1].z) bounds[1].z = p.z;
    }

    // Update the bounding box to enclose the bounding box b
    void update(const boundingbox& b)
    {
        update(b.min());
        update(b.max());
    }

    // Return the volume of the bounding box
    float volume() const
    {
        return (bounds[1].x - bounds[0].x) * (bounds[1].y - bounds[0].y) * (bounds[1].z - bounds[0].z);
    }

    vec3 size() const
    {
        return bounds[1] - bounds[0];
    }

    float surface_area() const
    {
        const vec3 s = size();
        return 2.0 * (s.x*s.y + s.x*s.z + s.y*s.z);
    }

/*
    // Intersect a ray with (delimited by t0 and t1) with the given bounding box.
    // Returns true when the ray intersects the bbox, returns false otherwise
    bool    intersect(Ray r, float t0, float t1) const;

    // Intersect a ray with (delimited by t0 and t1) with the given bounding box.
    // Returns 1 (and sets t_min and t_max) when the ray intersects the bbox,
    // returns 0 otherwise
    bool    intersect_full(float& t_min, float& t_max, Ray r, float t0, float t1) const;
*/

    void print() const
    {
        printf("<bbox %.6f %.6f %.6f -> %.6f %.6f %.6f>", bounds[0].x, bounds[0].y, bounds[0].z, bounds[1].x, bounds[1].y, bounds[1].z);
    }


protected:
    vec3    bounds[2];
};

} // namespace sk
