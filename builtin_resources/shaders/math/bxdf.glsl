#ifndef _MATH_BXDF
#define _MATH_BXDF

#include "basic.glsl"

// CAUTION: here eta = eta_in / eta_out
// return the fraction of reflection
float calDielectricFresnel(in float cosThetaI, in float eta)
{
    const float cosThetaOSquare = 1.f - eta * eta * (1.f - cosThetaI * cosThetaI);
    if (cosThetaOSquare <= CONSTANT_EPS) return 1.f;
    const float sinThetaI = sqrt(1.f - cosThetaI * cosThetaI), cosThetaO = sqrt(cosThetaOSquare),
                sinThetaO = sqrt(1.f - cosThetaOSquare);

    vec2 sqrtR = vec2((eta * cosThetaI - cosThetaO) / (eta * cosThetaI + cosThetaO),
                      (eta * cosThetaO - cosThetaI) / (eta * cosThetaO + cosThetaI));
    return .5f * dot(sqrtR, sqrtR);
}

float schlickFresnel(in float cosThetaI, in float eta)
{
    const float sqrtR0 = (eta - 1) / (eta + 1), R0 = sqrtR0 * sqrtR0;
    float       pow5  = (1.f - cosThetaI) * (1.f - cosThetaI);
    pow5             *= pow5 * (1.f - cosThetaI);
    return R0 + (1.f - R0) * pow5;
}

float calSmithG1GGX(in float roughness, in vec3 V, in vec3 H)
{
    const float cosThetaO       = dot(V, H);
    const float cosThetaOSquare = cosThetaO * cosThetaO, roughnessSquare = roughness * roughness;
    return max(.0f, 2.f * cosThetaO / (cosThetaO + sqrt(mix(cosThetaOSquare, 1.f, roughnessSquare))));
}

float calSmithSeparateG2GGX(in float roughness, in vec3 V, in vec3 H, in vec3 L)
{
    return calSmithG1GGX(roughness, V, H) * calSmithG1GGX(roughness, L, H);
}

float calSmithHeightCorrelatedG2GGX(in float roughness, in vec3 V, in vec3 H, in vec3 L)
{
    const float cosThetaI = dot(L, H), cosThetaO = dot(V, H);
    const float cosThetaISquare = cosThetaI * cosThetaI, cosThetaOSquare = cosThetaO * cosThetaO,
                roughnessSquare = roughness * roughness;
    return max(.0f,
               2.f * cosThetaI * cosThetaO
                   / ((cosThetaO + sqrt(mix(cosThetaOSquare, 1.f, roughnessSquare)))
                      * (cosThetaI + sqrt(mix(cosThetaISquare, 1.f, roughnessSquare)))));
}

#endif // _MATH_BXDF