#pragma once

#include <cstdint>
#include <memory>

#include "metaProgram.hpp"

inline void hash_combine(std::size_t &seed, const std::size_t &hashCode)
{
    const auto temp = hashCode + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= temp;
}

template <typename T>
inline void hash_param(std::size_t &seed, const T &arg)
{
    std::hash<T> hasher{};
    hash_combine(seed, hasher(arg));
}

template <typename T>
    requires is_container_type<T>
inline void hash_param(std::size_t &seed, const T &argContainer)
{
    for (const auto &arg : argContainer)
        hash_param(seed, arg);
}

template <typename T, typename... Ts>
inline void hash_param(std::size_t &seed, const T &arg, const Ts &...args)
{
    hash_param(seed, arg);
    hash_param(seed, args...);
}

template <typename T>
struct Aligned8Hasher
{
    std::size_t operator()(const T &obj) const
    {
        const uint8_t size = sizeof(T) / sizeof(uint8_t);
        const uint8_t *vBits = reinterpret_cast<const uint8_t *>(&obj);
        std::size_t seed = 0;
        for (uint8_t i = 0u; i < size; i++)
            hash_combine(seed, vBits[i]);
        return seed;
    }
};

template <typename T>
struct Aligned16Hasher
{
    std::size_t operator()(const T &obj) const
    {
        const uint16_t size = sizeof(T) / sizeof(uint16_t);
        const uint16_t *vBits = reinterpret_cast<const uint16_t *>(&obj);
        std::size_t seed = 0;
        for (uint16_t i = 0u; i < size; i++)
            hash_combine(seed, vBits[i]);
        return seed;
    }
};

template <typename T>
struct Aligned32Hasher
{
    std::size_t operator()(const T &obj) const
    {
        const uint32_t size = sizeof(T) / sizeof(uint32_t);
        const uint32_t *vBits = reinterpret_cast<const uint32_t *>(&obj);
        std::size_t seed = 0;
        for (uint32_t i = 0u; i < size; i++)
            hash_combine(seed, vBits[i]);
        return seed;
    }
};

template <typename T>
struct Aligned64Hasher
{
    std::size_t operator()(const T &obj) const
    {
        const uint64_t size = sizeof(T) / sizeof(uint64_t);
        const uint64_t *vBits = reinterpret_cast<const uint64_t *>(&obj);
        std::size_t seed = 0;
        for (uint64_t i = 0u; i < size; i++)
            hash_combine(seed, vBits[i]);
        return seed;
    }
};