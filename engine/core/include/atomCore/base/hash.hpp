#pragma once

#include <cstddef>
#include <functional>

template <typename T, typename... Rest>
void hash_combine(size_t& seed, const T& v, const Rest&... rest)
{
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    (hash_combine(seed, rest), ...);
}