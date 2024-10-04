#ifndef _STRUCTURE_GLOBAL_GEOMETRY_
#define _STRUCTURE_GLOBAL_GEOMETRY_

struct VertexAttribute {
    vec3  position;
    float _padding0;
    vec3  normal;
    float _padding1;
    vec3  color;
    float _padding2;
    vec2  texCoord;
    vec2  _padding3;
};

struct FaceAttribute {
    vec3 normal;
    uint materialIndex;
};

struct MaterialAttribute {
    /* basic */
    // shuffle variables to keep memory order compact
    vec3  ambient;       // Ka
    uint  ambient_map_index;
    vec3  diffuse;       // Kd
    uint  diffuse_map_index;
    vec3  specular;      // Ks
    uint  specular_map_index;
    vec3  transmittance; // Kt
    float dissolve;      // d
    vec3  emission;      // Ke
    int   illum;         // illum
    float shininess;     // Ns
    float ior;           // Ni
    uint  normal_map_index;
    uint  displacement_map_index;
    uint  reflection_map_index;

    /* PBR's */
    float roughness;           // Pr
    float metallic;            // Pm
    float sheen;               // Ps
    float clearcoat_thickness; // Pc
    float clearcoat_roughness; // Pcr
    float anisotropy;          // aniso
    float anisotropy_rotation; // anisor
    uint  basic_pbr_map_index;
    uint  extra_pbr_map_index;

    vec2 _padding;
};

#endif // _STRUCTURE_GLOBAL_GEOMETRY_