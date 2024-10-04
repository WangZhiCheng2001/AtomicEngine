#include <xxhash.h>

#include <atomCore/base/hash.h>

ATOM_EXTERN_C ATOM_API size_t atom_hash(const void* input, size_t length, size_t seed)
{
#if SIZE_MAX == UINT64_MAX
    return (size_t)atom_hash_64(input, length, seed);
#elif SIZE_MAX == UINT32_MAX
    return (size_t)atom_hash_32(input, length, seed);
#else
#error "unsupported hash size!"
#endif
}

ATOM_EXTERN_C ATOM_API uint32_t atom_hash_32(const void* input, size_t length, uint32_t seed)
{
    return (uint32_t)XXH32(input, length, (XXH32_hash_t)seed);
}

ATOM_EXTERN_C ATOM_API uint64_t atom_hash_64_without_seed(const void* input, size_t length)
{
    return (uint64_t)XXH3_64bits(input, length);
}

ATOM_EXTERN_C ATOM_API uint64_t atom_hash_64(const void* input, size_t length, uint64_t seed)
{
    return (uint64_t)XXH3_64bits_withSeed(input, length, (XXH64_hash_t)seed);
}

// ATOM_EXTERN_C ATOM_API atom_hash128_t atom_hash_128(const void* input, size_t length) { XXH3_128bits(input, length); }
//
// ATOM_EXTERN_C ATOM_API atom_hash128_t atom_hash_128(const void* input, size_t length, uint64_t seed)
//{
//     XXH3_128bits_withSeed(input, length, (XXH64_hash_t)seed);
// }