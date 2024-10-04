#ifndef _STRUCTURE_RAY_
#define _STRUCTURE_RAY_

struct Ray {
    vec3  origin;
    float t;
    vec3  direction;
    float currentIOR;
    vec3  contribution;
    bool  inside;
};

struct IntersectionRecord {
    vec2 barycentricCoord;
    uint primitiveIndex;
};

struct SampleRecord {
    vec3  directionOut;
    float pdf;
    vec3  contribution;
    bool  validFlag;
};

const SampleRecord g_invalidSample = SampleRecord(vec3(.0f), .0f, vec3(.0f), false);

Ray generateCameraRay(in vec2 screenUV, in mat4 matrixView, in mat4 matrixProj)
{
    Ray res;

    mat4 invView = inverse(matrixView);
    res.origin   = (invView * vec4(0, 0, 0, 1)).xyz;

    vec4 target   = inverse(matrixProj) * vec4(screenUV * 2 - 1, 1, 1);
    res.direction = normalize(invView * vec4(normalize(target.xyz), 0)).xyz;

    res.t            = 1.f / .0f;
    res.currentIOR   = 1.f;
    res.contribution = vec3(1.f);
    res.inside       = false;

    return res;
}

Ray generateShadowRay(in vec3 hitPos, in vec3 directionLight)
{
    Ray res;

    res.origin    = hitPos + 1e-5 * directionLight;
    res.direction = directionLight;
    res.t         = 1.f / .0f;
    res.inside    = false;

    return res;
}

void nextRay(inout Ray ray, in vec3 direction)
{
    ray.origin    += ray.t * ray.direction + 1e-5 * direction;
    ray.direction  = direction;
    ray.t          = 1.f / .0f;
}

bool hasHit(in Ray ray) { return ray.t < 1.f / .0f; }

IntersectionRecord createIntersectionRecord()
{
    IntersectionRecord res;

    res.barycentricCoord = vec2(.0f);
    res.primitiveIndex   = ~0U;

    return res;
}

#endif // _STRUCTURE_RAY_