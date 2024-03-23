#pragma once

#include <glm/glm.hpp>

// this is not the real material system we want...just use this to complete the assignment
// desired material system: states/data combination of each stage shaders

/* texture maps */
/* basic */
// Texture diffuseMap{};  // RGB->diffuse, A->alpha(need rearrange)
// Texture specularMap{}; // RGB->specular color, A->shiness(need rearrange)
// Texture normalMap{};   // need convert bump map to normal map
// Texture displacementMap{};
// Texture reflectionMap{};
// Texture ambientMap{};
// /* PBR's */
// Texture pbrBasicMap{}; // R->metallic, G->roughness, B->AO(unused now), A->emissive(need rearrange)
// Texture pbrExtraMap{}; // only sheen now

struct alignas(16) Material
{
    /* basic */
    // shuffle variables to keep memory order compact
    glm::vec3 ambient = {0, 0, 0}; // Ka
    uint32_t ambient_map_index = {~0U};
    glm::vec3 diffuse = {0, 0, 0}; // Kd
    uint32_t diffuse_map_index = {~0U};
    glm::vec3 specular = {0, 0, 0}; // Ks
    uint32_t specular_map_index = {~0U};
    glm::vec3 transmittance = {0, 0, 0}; // Kt
    float dissolve = 1.0f;               // d
    glm::vec3 emission = {0, 0, 0};      // Ke
    int illum = 0;                       // illum
    float shininess = 1.0f;              // Ns
    float ior = 1.0f;                    // Ni
    uint32_t normal_map_index = {~0U};
    uint32_t displacement_map_index = {~0U};
    uint32_t reflection_map_index = {~0U};

    /* PBR's */
    float roughness = 0.0f;           // Pr
    float metallic = 0.0f;            // Pm
    float sheen = 0.0f;               // Ps
    float clearcoat_thickness = 0.0f; // Pc
    float clearcoat_roughness = 0.0f; // Pcr
    float anisotropy = 0.0f;          // aniso
    float anisotropy_rotation = 0.0f; // anisor
    uint32_t basic_pbr_map_index = {~0U};
    uint32_t extra_pbr_map_index = {~0U};
};