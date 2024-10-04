// usage of this file: define 'no-definition' for unjudged platform macros

#ifdef __ATOM_PLATFORM_FALLBACK_INC__
    #error "platform_fallback.inc included twice"
#endif

//-------------------------------------------------------------------------------
// -> platform
//      ATOM_PLAT_WIN32
//      ATOM_PLAT_WIN64
//      ATOM_PLAT_WINDOWS
//      ATOM_PLAT_UNIX
//
// -> architecture
//      ATOM_ARCH_X86
//      ATOM_ARCH_X86_64
//      ATOM_ARCH_ARM32
//      ATOM_ARCH_ARM64
//      ATOM_ARCH_32BIT
//      ATOM_ARCH_64BIT
//      ATOM_ARCH_LITTLE_ENDIAN
//      ATOM_ARCH_BIG_ENDIAN
//
// -> SIMD
//      ATOM_ARCH_SSE
//      ATOM_ARCH_SSE2
//      ATOM_ARCH_AVX
//      ATOM_ARCH_AVX2
//-------------------------------------------------------------------------------

// platform
#ifndef ATOM_PLAT_WIN32
    #define ATOM_PLAT_WIN32 0
#endif
#ifndef ATOM_PLAT_WIN64
    #define ATOM_PLAT_WIN64 0
#endif
#ifndef ATOM_PLAT_UNIX
    #define ATOM_PLAT_UNIX 0
#endif

// architecture
#ifndef ATOM_ARCH_X86
    #define ATOM_ARCH_X86 0
#endif
#ifndef ATOM_ARCH_X86_64
    #define ATOM_ARCH_X86_64 0
#endif
#ifndef ATOM_ARCH_ARM32
    #define ATOM_ARCH_ARM32 0
#endif
#ifndef ATOM_ARCH_ARM64
    #define ATOM_ARCH_ARM64 0
#endif
#ifndef ATOM_ARCH_32BIT
    #define ATOM_ARCH_32BIT 0
#endif
#ifndef ATOM_ARCH_64BIT
    #define ATOM_ARCH_64BIT 0
#endif
#ifndef ATOM_ARCH_LITTLE_ENDIAN
    #define ATOM_ARCH_LITTLE_ENDIAN 0
#endif
#ifndef ATOM_ARCH_BIG_ENDIAN
    #define ATOM_ARCH_BIG_ENDIAN 0
#endif

// SIMD
#ifndef ATOM_ARCH_SSE
    #define ATOM_ARCH_SSE 0
#endif
#ifndef ATOM_ARCH_SSE2
    #define ATOM_ARCH_SSE2 0
#endif
#ifndef ATOM_ARCH_AVX
    #define ATOM_ARCH_AVX 0
#endif
#ifndef ATOM_ARCH_AVX2
    #define ATOM_ARCH_AVX2 0
#endif

#define __ATOM_PLATFORM_FALLBACK_INC__