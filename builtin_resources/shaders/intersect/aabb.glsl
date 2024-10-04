#ifndef _INTERSECT_AABB_
#define _INTERSECT_AABB_

#include "../structure/ray.glsl"

vec2 rayIntersectAABB(in Ray ray, in vec3 aabbMin, in vec3 aabbMax)
{
    vec3 t1   = (aabbMin - ray.origin) / ray.direction;
    vec3 t2   = (aabbMax - ray.origin) / ray.direction;
    vec3 tmin = min(t1, t2);
    vec3 tmax = max(t1, t2);
    return vec2(max(.0f, max(tmin.x, max(tmin.y, tmin.z))), min(tmax.x, min(tmax.y, tmax.z)));
}

#endif // _INTERSECT_AABB_