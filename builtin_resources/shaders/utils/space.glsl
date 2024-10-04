#ifndef _UTILS_SPACE
#define _UTILS_SPACE

float convertToLinearDepth(in float depth, in float near, in float far)
{
    return (2.0 * near) / (near + far - depth * (far - near));
}

#endif // _UTILS_SPACE