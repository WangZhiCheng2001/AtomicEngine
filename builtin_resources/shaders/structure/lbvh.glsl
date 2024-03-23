#ifndef _STRUCTURE_LBVH_
#define _STRUCTURE_LBVH_

// only used on the GPU side during construction; it is necessary to allocate
// the (empty) buffer
struct MortonCodeAttribute {
  uint mortonCode; // key for sorting
  uint elementIdx; // pointer into element buffer
};

// input for the builder (normally a triangle or some other kind of primitive);
// it is necessary to allocate and fill the buffer
// however it can be filled on GPU by input vertex & index buffer
struct Element {
  vec3 aabbMin; // aabb of the primitive
  vec3 aabbMax;
};

#endif // _STRUCTURE_LBVH_