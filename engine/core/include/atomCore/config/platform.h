#pragma once

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

// windows platform
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64)
#ifdef _WIN64
#define ATOM_PLAT_WIN64 1
#else
#define ATOM_PLAT_WIN32 1
#endif
#endif

#if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#define ATOM_PLAT_UNIX 1
#endif

// architecture
#ifndef ATOM_MANUAL_CONFIG_CPU_ARCHITECTURE
#if defined(__x86_64__) || defined(_M_X64) || defined(_AMD64_) || defined(_M_AMD64)
#define ATOM_ARCH_X86_64        1
#define ATOM_ARCH_64BIT         1
#define ATOM_ARCH_LITTLE_ENDIAN 1
#define ATOM_ARCH_SSE           1
#define ATOM_ARCH_SSE2          1
#elif defined(__i386) || defined(_M_IX86) || defined(_X86_)
#define ATOM_ARCH_X86           1
#define ATOM_ARCH_32BIT         1
#define ATOM_ARCH_LITTLE_ENDIAN 1
#define ATOM_ARCH_SSE           1
#define ATOM_ARCH_SSE2          1
#elif defined(__aarch64__) || defined(__AARCH64) || defined(_M_ARM64)
#define ATOM_ARCH_ARM64         1
#define ATOM_ARCH_64BIT         1
#define ATOM_ARCH_LITTLE_ENDIAN 1
#define ATOM_ARCH_SSE           1
#define ATOM_ARCH_SSE2          1
#elif defined(__arm__) || defined(_M_ARM)
#define ATOM_ARCH_ARM32         1
#define ATOM_PLATFORM_32BIT     1
#define ATOM_ARCH_LITTLE_ENDIAN 1
#else
#error Unrecognized CPU was used.
#endif
#endif

// SIMD
#if defined(__AVX__)
#define ATOM_ARCH_AVX 1
#endif
#if defined(__AVX2__)
#define ATOM_ARCH_AVX2 1
#endif

// fallback
#include "platform_fallback.inc"

// other platform def
#define ATOM_PLAT_WINDOWS ATOM_PLAT_WIN32 || ATOM_PLAT_WIN64

// Platform Specific Configure
#define ATOM_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE

#ifdef _WIN32
#ifndef ATOM_WINDOWS_CONFIGURES
#define ATOM_WINDOWS_CONFIGURES
#endif
#ifdef ATOM_WINDOWS_CONFIGURES
#define ATOM_RUNTIME_USE_MIMALLOC
#endif
#endif

#undef ATOM_HEADER_SCOPE_DEFINING_PLATFORM_CONFIGURE