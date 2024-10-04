#ifndef _UTILS_RANDOM_
#define _UTILS_RANDOM_

uint halton2Inverse(in uint index, in uint digits)
{
    index = (index << 16) | (index >> 16);
    index = ((index & 0x00ff00ff) << 8) | ((index & 0xff00ff00) >> 8);
    index = ((index & 0x0f0f0f0f) << 4) | ((index & 0xf0f0f0f0) >> 4);
    index = ((index & 0x33333333) << 2) | ((index & 0xcccccccc) >> 2);
    index = ((index & 0x55555555) << 1) | ((index & 0xaaaaaaaa) >> 1);
    return index >> (32 - digits);
}

uint halton3Inverse(in uint index, in uint digits)
{
    uint result = 0;
    for (uint d = 0; d < digits; ++d) {
        result  = result * 3 + index % 3;
        index  /= 3;
    }
    return result;
}

uint getSampleOffsetBase(in uvec2 positionScreen)
{
    const uint64_t hx = uint64_t(halton2Inverse(positionScreen.x, widthPower));
    const uint64_t hy = uint64_t(halton3Inverse(positionScreen.y, heightPower));
    return uint((hx * widthInvElem + hy * heightInvElem) % sampleCount);
}

// from https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
uint pcg_hash(inout uint in_state)
{
    uint state = in_state * 747796405u + 2891336453u;
    uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    in_state   = (word >> 22u) ^ word;
    return in_state;
}

uint tea(uint val0, uint val1)
{
    uint v0 = val0;
    uint v1 = val1;
    uint s0 = 0;

    [[unroll]] for (uint n = 0; n < 16; n++) {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }

    return v0;
}

float random1D(in uint sampleOffset, inout uint samplingDim)
{
    samplingDim++;
    return randomSeq[(sampleOffset + samplingDim - 1) % randomSeq.length()];
}

vec2 random2D(in uint sampleOffset, inout uint samplingDim)
{
    return vec2(random1D(sampleOffset, samplingDim), random1D(sampleOffset, samplingDim));
}

vec3 random3D(in uint sampleOffset, inout uint samplingDim)
{
    return vec3(random1D(sampleOffset, samplingDim), random1D(sampleOffset, samplingDim), random1D(sampleOffset, samplingDim));
}

vec4 random4D(in uint sampleOffset, inout uint samplingDim)
{
    return vec4(random1D(sampleOffset, samplingDim),
                random1D(sampleOffset, samplingDim),
                random1D(sampleOffset, samplingDim),
                random1D(sampleOffset, samplingDim));
}

float random1D(inout uint in_state) { return pcg_hash(in_state) / float(uint(0xffffffff)); }

vec2 random2D(inout uint in_state) { return vec2(random1D(in_state), random1D(in_state)); }

vec3 random3D(inout uint in_state) { return vec3(random1D(in_state), random1D(in_state), random1D(in_state)); }

vec4 random4D(inout uint in_state)
{
    return vec4(random1D(in_state), random1D(in_state), random1D(in_state), random1D(in_state));
}

#endif // _UTILS_RANDOM_