#ifndef _MATH_SAMPLE
#define _MATH_SAMPLE

#include "basic.glsl"

void sampleHemiSphere(in vec2 u, out vec3 dir, out float pdf)
{
    float cosTheta = sqrt(u.x), sinTheta = sqrt(1 - cosTheta * cosTheta);
    float phi = CONSTANT_TWO_PI * u.y, cosPhi = cos(phi), sinPhi = sin(phi);
    dir = vec3(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
    pdf = CONSTANT_1_PI * cosTheta;
}

float calHemiSpherePdf(in vec3 dir) { return CONSTANT_1_PI * dir.y; }

void samplePhong(in float shininess, in vec2 u, out vec3 dir, out float pdf)
{
    float cosAlpha = pow(u.x, 1.f / (shininess + 1.f)), sinAlpha = sqrt(1.f - cosAlpha * cosAlpha);
    float phi = CONSTANT_TWO_PI * u.y, cosPhi = cos(phi), sinPhi = sin(phi);
    dir = vec3(cosPhi * sinAlpha, sinPhi * sinAlpha, cosAlpha);
    pdf = (shininess + 1.f) * CONSTANT_1_PI * .5f * pow(cosAlpha, shininess);
}

float calPhongPdf(in float shininess, in vec3 wi, in vec3 wo)
{
    return (shininess + 1.f) * CONSTANT_1_PI * .5f * pow(dot(wi, wo), shininess);
}

// return a barycentric coordinate
vec2 sampleTriangle(in vec2 u)
{
    float temp = sqrt(u.x);
    return vec2(1.f - temp, u.y * temp);
}

void sampleGGX(in float roughness, in vec2 u, out vec3 dir, out float pdf)
{
    const float roughnessSquare = roughness * roughness;
    const float cosTheta        = sqrt((1.f - u.x) / (u.x * (roughnessSquare - 1.f) + 1.f)),
                sinTheta        = sqrt(1.f - cosTheta * cosTheta);
    const float phi = CONSTANT_TWO_PI * u.y, cosPhi = cos(phi), sinPhi = sin(phi);
    dir               = vec3(cosPhi * sinTheta, sinPhi * sinTheta, cosTheta);
    const float denom = (roughnessSquare - 1.f) * cosTheta * cosTheta + 1.f;
    pdf               = roughnessSquare * cosTheta / (CONSTANT_PI * denom * denom);
}

float calGGXPdf(in float roughness, in vec3 N, in vec3 H)
{
    const float cosTheta = dot(N, H), roughnessSquare = roughness * roughness,
                denom = (roughnessSquare - 1.f) * cosTheta * cosTheta + 1.f;
    return roughnessSquare * cosTheta / (CONSTANT_PI * denom * denom);
}

float powerWeightHeuristic(in float pdf1, in float pdf2)
{
    pdf1 *= pdf1;
    pdf2 *= pdf2;
    return pdf1 / (pdf1 + pdf2);
}

#endif // _MATH_SAMPLE