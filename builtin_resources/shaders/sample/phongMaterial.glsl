#ifndef _SAMPLE_PHONG_MATERIAL
#define _SAMPLE_PHONG_MATERIAL

#include "../structure/phongMaterial.glsl"
#include "../structure/ray.glsl"
#include "../math/bxdf.glsl"
#include "../math/sample.glsl"

PhongMaterial extractPhongMaterial(in uint materialIndex, in vec2 texCoord, in vec2 dpdx, in vec2 dpdy, in vec3 geometryNormal)
{
    PhongMaterial res;

    vec4 diffuse  = materials[materialIndex].diffuse_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].diffuse_map_index], texCoord, dpdx, dpdy)
                        : vec4(.0f);
    vec4 specular = materials[materialIndex].specular_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].specular_map_index], texCoord, dpdx, dpdy)
                        : vec4(.0f);
    vec3 normal   = materials[materialIndex].normal_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].normal_map_index], texCoord, dpdx, dpdy).xyz * 2 - 1
                        : vec3(.0f);

    res.diffuse       = any(greaterThan(diffuse.xyz, vec3(.0f))) ? diffuse.xyz : materials[materialIndex].diffuse;
    res.transparency  = diffuse.w > .0f ? diffuse.w : materials[materialIndex].dissolve;
    res.normal        = any(greaterThan(normal, vec3(.0f))) ? normal : geometryNormal;
    res.shininess     = specular.w > .0f ? specular.w : materials[materialIndex].shininess;
    res.specularColor = any(greaterThan(specular.xyz, vec3(.0f))) ? specular.xyz : materials[materialIndex].specular;
    res.ior           = materials[materialIndex].ior;
    res.emission      = materials[materialIndex].emission;

    return res;
}

void sampleDiffuse(in PhongMaterial material, in vec2 u, out SampleRecord record)
{
    sampleHemiSphere(u, record.directionOut, record.pdf);
    if (record.pdf < CONSTANT_EPS) {
        record = g_invalidSample;
        return;
    }

    record.validFlag    = true;
    record.directionOut = directionLocalTBNToWorld(record.directionOut, material.normal);
    record.contribution = CONSTANT_1_PI * material.diffuse;
}

void evaluateDiffuse(in vec3 wo, in PhongMaterial material, inout SampleRecord record)
{
    record.pdf          = max(dot(wo, material.normal), .0f) * CONSTANT_1_PI;
    record.contribution = float(record.pdf > .0f) * CONSTANT_1_PI * material.diffuse;
}

void sampleMaterial(in Ray woRay, in PhongMaterial material, in vec4 u, out SampleRecord record)
{
    vec3 Kd = material.diffuse, Ks = material.specularColor;
    Ks /= (Kd + Ks);

    float eta;
    vec3  N;
    if (dot(-woRay.direction, material.normal) < .0f) {
        eta = !woRay.inside ? material.ior : 1.f / material.ior;
        N   = -material.normal;
    } else {
        eta = woRay.inside ? material.ior : 1.f / material.ior;
        N   = material.normal;
    }

    if (u.x <= luminance(Ks)) {
        float F = calDielectricFresnel(dot(-woRay.direction, N), eta);
        samplePhong(material.shininess, u.zw, record.directionOut, record.pdf);
        record.directionOut = directionLocalTBNToWorld(record.directionOut, -woRay.direction);
        record.contribution = material.specularColor * record.pdf * (material.shininess + 2) / (material.shininess + 1);
        if (u.y <= F) {
            record.pdf *= F;
        } else {
            vec3 H               = normalize(record.directionOut - woRay.direction);
            record.directionOut  = refract(woRay.direction, H, 1.f / eta);
            record.contribution *= eta * eta;
            record.pdf          *= (1.f - F);
        }
        record.pdf *= luminance(Ks);
    } else {
        sampleDiffuse(material, u.yz, record);
        record.pdf *= luminance(1.f - Ks);
    }
    record.validFlag = true;
}

void evaluateMaterial(in Ray woRay, in PhongMaterial material, inout SampleRecord record)
{
    vec3 Kd = material.diffuse, Ks = material.specularColor;
    Ks /= (Kd + Ks);

    float eta;
    vec3  N;
    if (dot(record.directionOut, material.normal) < .0f) {
        eta = !woRay.inside ? material.ior : 1.f / material.ior;
        N   = -material.normal;
    } else {
        eta = woRay.inside ? material.ior : 1.f / material.ior;
        N   = material.normal;
    }
    const float NdotV = dot(N, -woRay.direction);
    if (NdotV < CONSTANT_EPS) {
        record = g_invalidSample;
        return;
    }
    // always confirm that H is at same side as wi (e.g. light direction)
    const vec3  H     = (NdotV > .0f) ? normalize(-woRay.direction + record.directionOut)
                                      : -normalize(eta * record.directionOut - woRay.direction);
    const float HdotI = dot(H, record.directionOut), F = calDielectricFresnel(HdotI, eta);

    SampleRecord diffuse;
    evaluateDiffuse(-woRay.direction, material, diffuse);
    record.contribution = (1.f - Ks) * diffuse.contribution;
    record.pdf          = luminance(1.f - Ks) * diffuse.pdf;

    const float temp     = CONSTANT_1_PI * .5f * pow(abs(dot(-woRay.direction, record.directionOut)), material.shininess);
    record.contribution += Ks * (material.shininess + 2) * temp * mix(eta * eta, 1.f, F);
    record.pdf          += luminance(Ks) * (material.shininess + 1) * temp;

    record.validFlag = true;
}

#endif // _SAMPLE_PHONG_MATERIAL