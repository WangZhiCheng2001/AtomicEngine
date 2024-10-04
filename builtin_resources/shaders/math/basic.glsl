#ifndef _MATH_BASIC
#define _MATH_BASIC

#define CONSTANT_PI     3.14159265358979323846f
#define CONSTANT_TWO_PI 6.28318530717958647692f
#define CONSTANT_PI_2   1.57079632679489661923f
#define CONSTANT_PI_3   1.047197551196597746153f
#define CONSTANT_PI_4   0.785398163397448309616f
#define CONSTANT_1_PI   0.318309886183790671538f
#define CONSTANT_2_PI   0.636619772367581343076f
#define CONSTANT_EPS    1e-5

// CAUTION: always use right hand coord system
mat3 getTBNMatrix(vec3 normal)
{
    vec3 tangent  = (1.f - abs(normal.y) >= CONSTANT_EPS) ? vec3(0, 1, 0) : vec3(0, 0, 1);
    vec3 biNormal = normalize(cross(normal, tangent));
    tangent       = cross(biNormal, normal);
    return mat3(tangent, biNormal, normal);
}

vec3 directionWorldToLocalTBN(in vec3 dir, in vec3 normal)
{
    mat3 tbn = getTBNMatrix(normal);
    return vec3(dot(dir, tbn[0]), dot(dir, tbn[1]), dot(dir, tbn[2]));
}

vec3 directionLocalTBNToWorld(in vec3 dir, in vec3 normal)
{
    mat3 tbn = getTBNMatrix(normal);
    return (dir.x * tbn[0] + dir.y * tbn[1] + dir.z * tbn[2]);
}

float luminance(vec3 color) { return dot(color, vec3(.2126f, .7152f, .0722f)); }

#endif // _MATH_BASIC