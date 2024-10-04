#ifndef _INTERSECT_TRIANGLE_
#define _INTERSECT_TRIANGLE_

#include "../structure/ray.glsl"

#define _TRIANGLE_INTERSECT_EPS 1e-5

vec3 rayIntersectTriangle(inout Ray ray, in uint triangleIndex)
{
    vec3 vert0 = vertices[indices[triangleIndex * 3 + 0]].position, vert1 = vertices[indices[triangleIndex * 3 + 1]].position,
         vert2 = vertices[indices[triangleIndex * 3 + 2]].position;

    vec3  e1 = vert1 - vert0, e2 = vert2 - vert0;
    vec3  p   = cross(ray.direction, e2);
    float det = dot(e1, p);

    vec3  t      = ray.origin - vert0;
    float invDet = 1.f / det;

    float u  = dot(t, p) * invDet;
    vec3  q  = cross(t, e1);
    float v  = dot(ray.direction, q) * invDet;
    float t_ = dot(e2, q) * invDet;

    ray.inside = det <= -_TRIANGLE_INTERSECT_EPS;
    return vec3(abs(det) < _TRIANGLE_INTERSECT_EPS ? -1.f : t_, u, v);
}

#endif // _INTERSECT_TRIANGLE_