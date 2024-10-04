#ifndef _STRUCTURE_BXDF_MATERIAL
#define _STRUCTURE_BXDF_MATERIAL

struct PhysicalBxDFMaterial {
    vec3  diffuse;
    float transparency;
    vec3  normal;
    float roughness;
    vec3  specularColor;
    float ior;
    vec3  emission;
    float _padding;
};

#endif // _STRUCTURE_BXDF_MATERIAL