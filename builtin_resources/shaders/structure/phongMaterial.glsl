#ifndef _STRUCTURE_PHONG_MATERIAL
#define _STRUCTURE_PHONG_MATERIAL

struct PhongMaterial {
    vec3  diffuse;
    float transparency;
    vec3  normal;
    float ior;
    vec3  specularColor;
    float shininess;
    vec3  emission;
    float _padding;
};

#endif // _STRUCTURE_PHONG_MATERIAL