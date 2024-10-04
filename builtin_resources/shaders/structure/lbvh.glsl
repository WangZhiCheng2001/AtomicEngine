#ifndef _STRUCTURE_LBVH
#define _STRUCTURE_LBVH

struct LBVHNode
{
    vec3 aabbMin;
    uint next;
    vec3 aabbMax;
    uint primitiveIndex;
};

#endif // _STRUCTURE_LBVH