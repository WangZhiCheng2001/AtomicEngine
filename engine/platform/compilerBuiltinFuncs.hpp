#pragma once

#include <cstdint>

#include <metaProgram.hpp>

inline static uint32_t popcount(uint32_t u)
{
    u = (u & 0x55555555) + ((u >> 1) & 0x55555555);
    u = (u & 0x33333333) + ((u >> 2) & 0x33333333);
    u = (u & 0x0F0F0F0F) + ((u >> 4) & 0x0F0F0F0F);
    u = (u & 0x00FF00FF) + ((u >> 8) & 0x00FF00FF);
    u = (u & 0x0000FFFF) + ((u >> 16) & 0x0000FFFF);
    return u;
}
inline static int32_t popcount(int32_t u) { return popcount(static_cast<uint32_t>(u & 0x7FFFFFFF)); }
inline static uint64_t popcount(uint64_t u) { return popcount(static_cast<uint32_t>(u)) + popcount(static_cast<uint32_t>(u >> 32)); }
inline static int64_t popcount(int64_t u) { return popcount(static_cast<uint64_t>(u & 0x7FFFFFFFFFFFFFFF)); }