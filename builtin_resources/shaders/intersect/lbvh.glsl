#ifndef _INTERSECT_LBVH
#define _INTERSECT_LBVH

#include "../structure/lbvh.glsl"
#include "aabb.glsl"
#include "triangle.glsl"

void rayIntersectBVH(inout Ray ray, inout IntersectionRecord record)
{
    uint nodeIndex = 0U;
    while (nodeIndex != ~0U) {
        LBVHNode node = bvhNodes[nodeIndex];
        if (node.primitiveIndex != ~0U) {
            vec3 interParams = rayIntersectTriangle(ray, node.primitiveIndex);
            if (interParams.x >= _TRIANGLE_INTERSECT_EPS && interParams.x <= ray.t && interParams.y >= .0f
                && interParams.y <= 1.f && interParams.z >= .0f && interParams.y + interParams.z <= 1.f) {
                ray.t                   = interParams.x;
                record.primitiveIndex   = node.primitiveIndex;
                record.barycentricCoord = interParams.yz;
            }
        } else {
            vec2 t = rayIntersectAABB(ray, node.aabbMin, node.aabbMax);
            if (t.x <= t.y && t.x < ray.t) {
                nodeIndex++;
                continue;
            }
        }

        nodeIndex = node.next;
    }
}

#endif // _INTERSECT_LBVH