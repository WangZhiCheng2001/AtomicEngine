#ifndef _UTILS_AABB_
#define _UTILS_AABB_

void aabbUnion(vec3 minA, vec3 maxA, vec3 minB, vec3 maxB, out vec3 minAABB, out vec3 maxAABB)
{
    minAABB = min(minA, minB);
    maxAABB = max(maxA, maxB);
}

#endif // _UTILS_AABB_