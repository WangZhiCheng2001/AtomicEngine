#pragma once

#include <atomCore/config.h>

// #define atom_hash128_t XXH128_hash_t

ATOM_EXTERN_C ATOM_API size_t   atom_hash(const void* input, size_t length, size_t seed);
ATOM_EXTERN_C ATOM_API uint32_t atom_hash_32(const void* input, size_t length, uint32_t seed);
ATOM_EXTERN_C ATOM_API uint64_t atom_hash_64_without_seed(const void* input, size_t length);
ATOM_EXTERN_C ATOM_API uint64_t atom_hash_64(const void* input, size_t length, uint64_t seed);