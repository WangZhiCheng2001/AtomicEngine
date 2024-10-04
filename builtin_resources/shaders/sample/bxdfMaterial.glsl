#ifndef _SAMPLE_BXDF_MATERIAL
#define _SAMPLE_BXDF_MATERIAL

#include "../structure/bxdfMaterial.glsl"
#include "../structure/ray.glsl"
#include "../math/bxdf.glsl"
#include "../math/sample.glsl"

PhysicalBxDFMaterial extractPhysicalBxDFMaterial(in uint materialIndex,
                                                 in vec2 texCoord,
                                                 in vec2 dpdx,
                                                 in vec2 dpdy,
                                                 in vec3 geometryNormal)
{
    PhysicalBxDFMaterial res;

    vec4 diffuse  = materials[materialIndex].diffuse_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].diffuse_map_index], texCoord, dpdx, dpdy)
                        : vec4(.0f);
    vec4 specular = materials[materialIndex].specular_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].specular_map_index], texCoord, dpdx, dpdy)
                        : vec4(.0f);
    vec3 normal   = materials[materialIndex].normal_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].normal_map_index], texCoord, dpdx, dpdy).xyz * 2 - 1
                        : vec3(.0f);
    vec4 basicPBR = materials[materialIndex].basic_pbr_map_index != ~0U
                        ? textureGrad(textures[materials[materialIndex].basic_pbr_map_index], texCoord, dpdx, dpdy)
                        : vec4(.0f);

    res.diffuse       = any(greaterThan(diffuse.xyz, vec3(.0f))) ? diffuse.xyz : materials[materialIndex].diffuse;
    res.transparency  = diffuse.w > .0f ? diffuse.w : materials[materialIndex].dissolve;
    res.normal        = any(greaterThan(normal, vec3(.0f))) ? normal : geometryNormal;
    res.roughness     = (basicPBR.y > .0f) ? basicPBR.y : materials[materialIndex].roughness;
    res.roughness     = (res.roughness > .0f)
                            ? res.roughness
                            : sqrt(2.f / (2.f + ((specular.w > .0f) ? specular.w : materials[materialIndex].shininess)));
    res.specularColor = any(greaterThan(specular.xyz, vec3(.0f))) ? specular.xyz : materials[materialIndex].specular;
    res.ior           = materials[materialIndex].ior;
    res.emission      = (basicPBR.w > .0f) ? vec3(basicPBR.w) : materials[materialIndex].emission;

    return res;
}

void sampleDiffuse(in PhysicalBxDFMaterial material, in vec2 u, out SampleRecord record)
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

void evaluateDiffuse(in vec3 wo, in PhysicalBxDFMaterial material, inout SampleRecord record)
{
    record.pdf          = max(dot(wo, material.normal), .0f) * CONSTANT_1_PI;
    record.contribution = float(record.pdf > .0f) * CONSTANT_1_PI * material.diffuse;
}

// TODO: kulla-conty
void sampleBSDF(in Ray woRay, in PhysicalBxDFMaterial material, in vec3 u, out SampleRecord record)
{
    float eta;
    vec3  N;
    if (dot(-woRay.direction, material.normal) < .0f) {
        eta = !woRay.inside ? material.ior : 1.f / material.ior;
        N   = -material.normal;
    } else {
        eta = woRay.inside ? material.ior : 1.f / material.ior;
        N   = material.normal;
    }

    vec3  H;
    float D;
    sampleGGX(material.roughness, u.xy, H, D);
    H           = directionLocalTBNToWorld(H, N);
    float HdotO = dot(H, -woRay.direction);
    if (HdotO < CONSTANT_EPS) {
        record = g_invalidSample;
        return;
    }

    record.directionOut = refract(woRay.direction, H, 1.f / eta);
    float F             = schlickFresnel(HdotO, eta);
    // full internal reflect, or reflect behavior
    if (length(record.directionOut) < CONSTANT_EPS || u.z < F) {
        record.directionOut = reflect(woRay.direction, H);
        const float NdotI   = dot(-woRay.direction, N);
        // theoretically we need to recalculate F, but under reflection HdotI == HdotO
        record.pdf          = (F * D) / (4.f * HdotO);
        if (NdotI < CONSTANT_EPS || record.pdf < CONSTANT_EPS) {
            record = g_invalidSample;
            return;
        }

        const float G       = calSmithHeightCorrelatedG2GGX(material.roughness, -woRay.direction, H, record.directionOut);
        float       f       = (F * D * G) / abs(4.f * NdotI * dot(record.directionOut, N));
        record.contribution = material.specularColor * f;
    } else {
        // CAUTION: for now H is at same side as wi, but not wo
        const float NdotI = dot(record.directionOut, N), HdotI = dot(record.directionOut, H);
        if (NdotI < CONSTANT_EPS || HdotI < CONSTANT_EPS) {
            record = g_invalidSample;
            return;
        }

        F                 = schlickFresnel(HdotI, eta);
        const float denom = HdotI * eta + HdotO;
        record.pdf        = ((1.f - F) * D) * abs(HdotO / (denom * denom));
        if (record.pdf < CONSTANT_EPS) {
            record = g_invalidSample;
            return;
        }

        const float G       = calSmithHeightCorrelatedG2GGX(material.roughness, -woRay.direction, H, record.directionOut);
        float       f       = record.pdf * G * abs(HdotI / (dot(-woRay.direction, N) * dot(record.directionOut, N)));
        // CAUTION: wi's lobe is not equal to wo's lobe, so there is an additional coeff to change integrated element
        record.contribution = material.specularColor * f * (eta * eta);
    }
    record.validFlag = true;
}

// TODO: kulla-conty
void evaluateBSDF(in Ray woRay, in PhysicalBxDFMaterial material, inout SampleRecord record)
{
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
    const bool isReflect = NdotV > .0f;

    // always confirm that H is at same side as wi (e.g. light direction)
    const vec3 H =
        isReflect ? normalize(-woRay.direction + record.directionOut) : -normalize(eta * record.directionOut - woRay.direction);
    const float HdotI = dot(H, record.directionOut), HdotO = dot(H, -woRay.direction), D = calGGXPdf(material.roughness, N, H),
                F = schlickFresnel(HdotI, eta);

    if (isReflect)
        record.pdf = (F * D) / (4.f * HdotO);
    else {
        const float denom = HdotI * eta + HdotO;
        record.pdf        = ((1.f - F) * D) * abs(HdotO / (denom * denom));
    }
    if (record.pdf < CONSTANT_EPS) {
        record = g_invalidSample;
        return;
    }

    if (isReflect) {
        const float G       = calSmithHeightCorrelatedG2GGX(material.roughness, -woRay.direction, H, record.directionOut);
        float       f       = (F * D * G) / abs(4.f * dot(-woRay.direction, N) * dot(record.directionOut, N));
        record.contribution = material.specularColor * f;
    } else {
        const float G       = calSmithHeightCorrelatedG2GGX(material.roughness, -woRay.direction, H, -record.directionOut);
        float       f       = record.pdf * G * abs(HdotI / (dot(-woRay.direction, N) * dot(record.directionOut, N)));
        // CAUTION: wi's lobe is not equal to wo's lobe, so there is an additional coeff to change integrated element
        record.contribution = material.specularColor * f * (eta * eta);
    }
    record.validFlag = true;
}

void sampleMaterial(in Ray woRay, in PhysicalBxDFMaterial material, in vec4 u, out SampleRecord record)
{
    float Kd = luminance(material.diffuse), Ks = luminance(material.specularColor);
    Ks /= (Kd + Ks);
    if (u.x <= Ks) {
        sampleBSDF(woRay, material, u.yzw, record);
        record.pdf /= Ks;
    } else {
        sampleDiffuse(material, u.yz, record);
        record.pdf /= (1.f - Ks);
    }
}

void evaluateMaterial(in Ray woRay, in PhysicalBxDFMaterial material, inout SampleRecord record)
{
    float Kd = luminance(material.diffuse), Ks = luminance(material.specularColor);
    Ks /= (Kd + Ks);
    SampleRecord diffuse;
    evaluateBSDF(woRay, material, record);
    evaluateDiffuse(-woRay.direction, material, diffuse);
    record.pdf          = mix(diffuse.pdf, record.pdf, Ks);
    record.contribution = mix(diffuse.contribution, record.contribution, Ks);
}

#endif // _SAMPLE_BXDF_MATERIAL