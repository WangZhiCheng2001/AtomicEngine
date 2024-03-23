#ifndef _STRUCTURE_GLOBAL_GEOMETRY_
#define _STRUCTURE_GLOBAL_GEOMETRY_

struct VertexAttribute {
  vec3 position;
  vec3 normal;
  vec3 color;
  vec2 texCoord;
};

struct FaceAttribute {
  vec3 normal;
  uint materialIndex;
};

struct MaterialAttribute {
  /* basic */
  // shuffle variables to keep memory order compact
  vec3 ambient; // Ka
  uint ambient_map_index;
  vec3 diffuse; // Kd
  uint diffuse_map_index;
  vec3 specular; // Ks
  uint specular_map_index;
  vec3 transmittance; // Kt
  float dissolve;     // d
  vec3 emission;      // Ke
  int illum;          // illum
  float shininess;    // Ns
  float ior;          // Ni
  uint normal_map_index;
  uint displacement_map_index;
  uint reflection_map_index;

  /* PBR's */
  float roughness;           // Pr
  float metallic;            // Pm
  float sheen;               // Ps
  float clearcoat_thickness; // Pc
  float clearcoat_roughness; // Pcr
  float anisotropy;          // aniso
  float anisotropy_rotation; // anisor
  uint basic_pbr_map_index;
  uint extra_pbr_map_index;
};

#endif // _STRUCTURE_GLOBAL_GEOMETRY_