#pragma once

#include <crtdbg.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

// TODO: try to make this cross-platform

ATOM_FORCEINLINE static void* _aligned_calloc(size_t nelem, size_t elsize, size_t alignment)
{
    void* memory = _aligned_malloc(nelem * elsize, alignment);
    if (memory != NULL) memset(memory, 0, nelem * elsize);
    return memory;
}

#define atom_malloc                                      malloc
#define atom_malloc_aligned                              _aligned_malloc
#define atom_malloc_alignedN(size, alignment, ...)       _aligned_malloc(size, alignment)
#define atom_calloc                                      calloc
#define atom_callocN(count, size, ...)                   calloc((count), (size))
#define atom_calloc_aligned                              _aligned_calloc
#define atom_realloc                                     realloc
#define atom_realloc_aligned                             _aligned_realloc
#define atom_realloc_alignedN(ptr, size, alignment, ...) _aligned_realloc((ptr), (size), (alignment))
#define atom_memalign                                    _aligned_malloc
#define atom_free                                        free
#define atom_freeN(ptr, ...)                             free(ptr)
#define atom_free_aligned                                _aligned_free
#define atom_free_alignedN(ptr, alignment, ...)          _aligned_free((ptr), (alignment))

#ifdef __cplusplus
#include <utility>

template <typename T, typename... Args>
T* atom_new_placed(void* memory, Args&&... args)
{
    return new (memory) T(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* atom_new(Args&&... args)
{
    return new T(std::forward<Args>(args)...);
}

template <typename T, typename... Args>
T* atom_new_sized(uint64_t size, Args&&... args)
{
    void* ptr = atom_calloc_aligned(1, size, alignof(T));
    return new (ptr) T(std::forward<Args>(args)...);
}

template <typename T>
void atom_delete_placed(T* object)
{
    object->~T();
}

template <typename T>
void atom_delete(T* object)
{
    delete object;
}

template <typename T, typename... Args>
ATOM_FORCEINLINE T make_zeroed(Args&&... args)
{
    std::aligned_storage_t<sizeof(T)> storage;
    ::memset(&storage, 0, sizeof(storage));
    auto res = new (&storage) T(std::forward<Args>(args)...);
    return *res;
}
#endif