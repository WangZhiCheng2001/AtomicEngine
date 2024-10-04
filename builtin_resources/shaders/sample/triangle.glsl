#ifndef _SAMPLE_TRIANGLE
#define _SAMPLE_TRIANGLE

#include "../structure/global_geometry.glsl"
#include "../utils/space.glsl"

#define EXTRACT_BARYCENTRIC_COORD_ATTRIBUTE(coord, attri0, attri1, attri2) \
    (attri0 * (1.f - coord.x - coord.y) + coord.x * attri1 + coord.y * attri2)

struct TriangleSampleRecord {
    vec3  position;
    float texCoordU;
    vec3  normal;
    float texCoordV;
    vec3  color;
    uint  materialIndex;
    vec2  dpdx;
    vec2  dpdy;
};

#define _TRIANGLE_ATTRIBUTE_EXTRACT_HELPER(coord, attriName) \
    res.attriName = EXTRACT_BARYCENTRIC_COORD_ATTRIBUTE(coord, vert0.attriName, vert1.attriName, vert2.attriName)

TriangleSampleRecord getTriangleAttribute(in uint primitiveIndex, in vec2 barycentricCoord)
{
    TriangleSampleRecord res;

    VertexAttribute vert0 = vertices[indices[primitiveIndex * 3 + 0]], vert1 = vertices[indices[primitiveIndex * 3 + 1]],
                    vert2 = vertices[indices[primitiveIndex * 3 + 2]];

    _TRIANGLE_ATTRIBUTE_EXTRACT_HELPER(barycentricCoord, position);
    _TRIANGLE_ATTRIBUTE_EXTRACT_HELPER(barycentricCoord, normal);
    _TRIANGLE_ATTRIBUTE_EXTRACT_HELPER(barycentricCoord, color);
    res.normal        = normalize(res.normal);
    res.normal        = (length(res.normal) < 1e-8) ? faces[primitiveIndex].normal : res.normal;
    vec2 uv           = EXTRACT_BARYCENTRIC_COORD_ATTRIBUTE(barycentricCoord, vert0.texCoord, vert1.texCoord, vert2.texCoord);
    res.texCoordU     = uv.x;
    res.texCoordV     = uv.y;
    res.materialIndex = faces[primitiveIndex].materialIndex;
    res.dpdx          = vec2(1.f, 1.f);
    res.dpdy          = vec2(1.f, 1.f);

    return res;
}

#undef _TRIANGLE_ATTRIBUTE_EXTRACT_HELPER
#undef EXTRACT_BARYCENTRIC_COORD_ATTRIBUTE

// Taken from http://filmicworlds.com/blog/visibility-buffer-rendering-with-material-graphs/
struct BarycentricDeriv {
    vec3 lambda;
    vec3 ddx;
    vec3 ddy;
};

struct InterpolatedVec1 {
    float value;
    float ddx;
    float ddy;
};

struct InterpolatedVec2 {
    vec2 value;
    vec2 ddx;
    vec2 ddy;
};

struct InterpolatedVec3 {
    vec3 value;
    vec3 ddx;
    vec3 ddy;
};

BarycentricDeriv calBarycentricDeriv(in vec4 pt0, in vec4 pt1, in vec4 pt2, in vec2 pixelNdc, in vec2 winSize)
{
    BarycentricDeriv ret;

    vec3 invW = 1.f / vec3(pt0.w, pt1.w, pt2.w);

    vec2 ndc0 = pt0.xy * invW.x;
    vec2 ndc1 = pt1.xy * invW.y;
    vec2 ndc2 = pt2.xy * invW.z;

    float invDet = 1.f / determinant(mat2(ndc2 - ndc1, ndc0 - ndc1));
    ret.ddx      = vec3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
    ret.ddy      = vec3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;
    float ddxSum = dot(ret.ddx, vec3(1, 1, 1));
    float ddySum = dot(ret.ddy, vec3(1, 1, 1));

    vec2  deltaVec   = pixelNdc - ndc0;
    float interpInvW = invW.x + deltaVec.x * ddxSum + deltaVec.y * ddySum;
    float interpW    = 1.f / interpInvW;

    ret.lambda.x = interpW * (invW[0] + deltaVec.x * ret.ddx.x + deltaVec.y * ret.ddy.x);
    ret.lambda.y = interpW * (0.0f + deltaVec.x * ret.ddx.y + deltaVec.y * ret.ddy.y);
    ret.lambda.z = interpW * (0.0f + deltaVec.x * ret.ddx.z + deltaVec.y * ret.ddy.z);

    ret.ddx *= (2.0f / winSize.x);
    ret.ddy *= (2.0f / winSize.y);
    ddxSum  *= (2.0f / winSize.x);
    ddySum  *= (2.0f / winSize.y);

    ret.ddy *= -1.0f;
    ddySum  *= -1.0f;

    float interpW_ddx = 1.0f / (interpInvW + ddxSum);
    float interpW_ddy = 1.0f / (interpInvW + ddySum);

    ret.ddx = interpW_ddx * (ret.lambda * interpInvW + ret.ddx) - ret.lambda;
    ret.ddy = interpW_ddy * (ret.lambda * interpInvW + ret.ddy) - ret.lambda;

    return ret;
}

InterpolatedVec1 interpolateAttribute1D(in vec3 v, in BarycentricDeriv deriv)
{
    InterpolatedVec1 ret;
    ret.value = dot(v, deriv.lambda);
    ret.ddx   = dot(v, deriv.ddx);
    ret.ddy   = dot(v, deriv.ddy);
    return ret;
}

InterpolatedVec2 interpolateAttribute2D(in mat3x2 v, in BarycentricDeriv deriv)
{
    InterpolatedVec2 ret;
    InterpolatedVec1 x = interpolateAttribute1D(vec3(v[0].x, v[1].x, v[2].x), deriv),
                     y = interpolateAttribute1D(vec3(v[0].y, v[1].y, v[2].y), deriv);
    ret.value          = vec2(x.value, y.value);
    ret.ddx            = vec2(x.ddx, y.ddx);
    ret.ddy            = vec2(x.ddy, y.ddy);
    return ret;
}

InterpolatedVec3 interpolateAttribute3D(in mat3 v, in BarycentricDeriv deriv)
{
    InterpolatedVec3 ret;
    InterpolatedVec1 x = interpolateAttribute1D(vec3(v[0].x, v[1].x, v[2].x), deriv),
                     y = interpolateAttribute1D(vec3(v[0].y, v[1].y, v[2].y), deriv),
                     z = interpolateAttribute1D(vec3(v[0].z, v[1].z, v[2].z), deriv);
    ret.value          = vec3(x.value, y.value, z.value);
    ret.ddx            = vec3(x.ddx, y.ddx, z.ddx);
    ret.ddy            = vec3(x.ddy, y.ddy, z.ddy);
    return ret;
}

TriangleSampleRecord sampleVisibilityBuffer(in vec2  screenUV,
                                            in vec2  resolution,
                                            in mat4  matrixView,
                                            in mat4  matrixProj,
                                            out uint primitiveIndex)
{
    const uint unpackedIndices = packUnorm4x8(texture(gbuffer, screenUV));
    // const uint instanceIndex  = (unpackedIndices >> 23) & 255;
    primitiveIndex             = (unpackedIndices & ((1 << 23) - 1)) - 1;
    if (primitiveIndex == 0U) {
        primitiveIndex = ~0U;
        return TriangleSampleRecord(vec3(.0f), .0f, vec3(.0f), .0f, vec3(.0f), ~0U, vec2(1.f), vec2(1.f));
    }

    VertexAttribute vert0 = vertices[indices[primitiveIndex * 3 + 0]], vert1 = vertices[indices[primitiveIndex * 3 + 1]],
                    vert2 = vertices[indices[primitiveIndex * 3 + 2]];
    vec4 pos0             = matrixProj * matrixView * vec4(vert0.position, 1.f);
    vec4 pos1             = matrixProj * matrixView * vec4(vert1.position, 1.f);
    vec4 pos2             = matrixProj * matrixView * vec4(vert2.position, 1.f);

    BarycentricDeriv dev = calBarycentricDeriv(pos0, pos1, pos2, screenUV * 2 - 1, resolution);

    TriangleSampleRecord res;
    res.materialIndex = faces[primitiveIndex].materialIndex;
    mat3 matAttri     = mat3(vert0.position, vert1.position, vert2.position);
    res.position      = interpolateAttribute3D(matAttri, dev).value;

    matAttri   = mat3(vert0.normal, vert1.normal, vert2.normal);
    res.normal = normalize(interpolateAttribute3D(matAttri, dev).value);
    res.normal = (length(res.normal) < 1e-8) ? faces[primitiveIndex].normal : res.normal;

    matAttri  = mat3(vert0.color, vert1.color, vert2.color);
    res.color = interpolateAttribute3D(matAttri, dev).value;

    mat3x2           matAttri_ = mat3x2(vert0.texCoord, vert1.texCoord, vert2.texCoord);
    InterpolatedVec2 uv        = interpolateAttribute2D(matAttri_, dev);
    res.texCoordU              = uv.value.x;
    res.texCoordV              = uv.value.y;
    res.dpdx                   = uv.ddx;
    res.dpdy                   = uv.ddy;

    return res;
}

// TriangleSampleRecord sampleVisibilityBuffer(in vec2 screenUV, in mat4 matrixView, in mat4 matrixProj, out uint
// primitiveIndex)
// {
//     const uint unpackedIndices = packUnorm4x8(texture(gbuffer, screenUV));
//     // const uint instanceIndex  = (unpackedIndices >> 23) & 255;
//     primitiveIndex             = (unpackedIndices & ((1 << 23) - 1)) - 1;
//     if (primitiveIndex == 0U) {
//         primitiveIndex = ~0U;
//         return TriangleSampleRecord(vec3(.0f), .0f, vec3(.0f), .0f, vec3(.0f), ~0U);
//     }

//     VertexAttribute vert0 = vertices[indices[primitiveIndex * 3 + 0]], vert1 = vertices[indices[primitiveIndex * 3 + 1]],
//                     vert2  = vertices[indices[primitiveIndex * 3 + 2]];
//     vec4 pos0              = matrixProj * matrixView * vec4(vert0.position, 1.f);
//     vec4 pos1              = matrixProj * matrixView * vec4(vert1.position, 1.f);
//     vec4 pos2              = matrixProj * matrixView * vec4(vert2.position, 1.f);
//     pos0.w                 = 1.f / pos0.w;
//     pos1.w                 = 1.f / pos1.w;
//     pos2.w                 = 1.f / pos2.w;
//     pos0.xyz              *= pos0.w;
//     pos1.xyz              *= pos1.w;
//     pos2.xyz              *= pos2.w;

//     const vec3  v0 = vec3(pos0.xy - screenUV, .0f), v1 = vec3(pos1.xy - screenUV, .0f), v2 = vec3(pos2.xy - screenUV, .0f);
//     const float area1 = length(cross(v0, v1)), area2 = length(cross(v1, v2)), area3 = length(cross(v2, v0));
//     vec3        barycentricCoord = vec3(area2, area3, area1) / (area1 + area2, area3);

//     TriangleSampleRecord res;
//     vec3                 invW     = vec3(pos0.w, pos1.w, pos2.w);
//     float                w        = 1.f / dot(invW, barycentricCoord);
//     mat3                 matAttri = transpose(mat3(vert0.position, vert1.position, vert2.position));
//     res.position                  = vec3(dot(invW, matAttri[0] * barycentricCoord),
//                         dot(invW, matAttri[1] * barycentricCoord),
//                         dot(invW, matAttri[2] * barycentricCoord))
//                    * w;
//     // float                z    = matrixProj[2][2] * w + matrixProj[3][2];
//     // vec4                 pos  = inverse(matrixProj * matrixView) * vec4(screenUV * w, z, w);
//     // res.position              = pos.xyz / pos.w;

//     matAttri   = transpose(mat3(vert0.normal, vert1.normal, vert2.normal));
//     res.normal = normalize(vec3(dot(invW, matAttri[0] * barycentricCoord),
//                                 dot(invW, matAttri[1] * barycentricCoord),
//                                 dot(invW, matAttri[2] * barycentricCoord))
//                            * w);
//     res.normal = (length(res.normal) < 1e-8) ? faces[primitiveIndex].normal : res.normal;

//     matAttri  = transpose(mat3(vert0.color, vert1.color, vert2.color));
//     res.color = vec3(dot(invW, matAttri[0] * barycentricCoord),
//                      dot(invW, matAttri[1] * barycentricCoord),
//                      dot(invW, matAttri[2] * barycentricCoord))
//                 * w;

//     mat2x3 matAttri_ = transpose(mat3x2(vert0.texCoord, vert1.texCoord, vert2.texCoord));
//     vec2   uv        = vec2(dot(invW, matAttri_[0] * barycentricCoord), dot(invW, matAttri_[1] * barycentricCoord)) * w;
//     res.texCoordU    = uv.x;
//     res.texCoordV    = uv.y;

//     res.materialIndex = faces[primitiveIndex].materialIndex;

//     return res;
// }

#endif // _SAMPLE_TRIANGLE