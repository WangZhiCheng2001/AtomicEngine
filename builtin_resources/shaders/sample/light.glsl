#ifndef _SAMPLE_LIGHT
#define _SAMPLE_LIGHT

#include "../intersect/lbvh.glsl"
#include "../structure/light.glsl"
#include "../math/sample.glsl"
#include "bxdfMaterial.glsl"
#include "triangle.glsl"

// return light index
uint sampleOneLight(in vec2 u, out SampleRecord record)
{
    uint passIndex = uint(float(lightAlias.length()) * u.x);
    record.pdf     = ((u.y < lightAlias[passIndex].probability) ? lightAlias[passIndex].probability
                                                                : 1.f - lightAlias[passIndex].probability)
                 / lightAlias.length();
    return (u.y < lightAlias[passIndex].probability) ? passIndex : lightAlias[passIndex].failIndex;
}

// return distance from material point to sampled light point
void sampleAreaLight(in uint lightIndex, in vec3 pos, in vec2 u, inout SampleRecord record)
{
    TriangleSampleRecord geometry = getTriangleAttribute(lights[lightIndex].elementIndex, sampleTriangle(u));
    PhysicalBxDFMaterial material = extractPhysicalBxDFMaterial(geometry.materialIndex,
                                                                vec2(geometry.texCoordU, geometry.texCoordV),
                                                                geometry.dpdx,
                                                                geometry.dpdy,
                                                                geometry.normal);

    record.directionOut  = geometry.position - pos;
    float dist           = length(record.directionOut);
    record.directionOut /= dist;
    float cosTheta       = dot(-record.directionOut, material.normal);
    record.pdf          *= dist * dist * lights[lightIndex].pdf / cosTheta;
    if (cosTheta <= .0f || record.pdf < 1e-8) {
        record = g_invalidSample;
        return;
    }

    Ray                shadowRay    = generateShadowRay(pos, record.directionOut);
    IntersectionRecord intersectRec = createIntersectionRecord();
    rayIntersectBVH(shadowRay, intersectRec);
    if (shadowRay.t < dist - 1e-4) {
        record = g_invalidSample;
        return;
    }

    record.contribution = material.emission / record.pdf;
    record.validFlag    = true;
}

void sampleLights(in vec3 p, in vec4 u, out SampleRecord record)
{
    uint lightIndex = sampleOneLight(u.xy, record);
    sampleAreaLight(lightIndex, p, u.zw, record);
}

#endif // _SAMPLE_LIGHT