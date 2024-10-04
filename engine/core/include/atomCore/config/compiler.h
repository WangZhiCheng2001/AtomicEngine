#pragma once
//-------------------------------------------------------------------------------
// -> compiler
//      ATOM_COMPILER_GCC
//      ATOM_COMPILER_MSVC
//      ATOM_COMPILER_CLANG
//      ATOM_COMPILER_CLANG_CL
//
// -> cxx version
//      ATOM_CXX_11
//      ATOM_CXX_14
//      ATOM_CXX_17
//      ATOM_CXX_20
//      ATOM_CXX_VERSION
//
// -> other
//      ATOM_COMPILER_VERSION
//-------------------------------------------------------------------------------
#include "platform.h"

// compiler def
#if defined(__clang__) && !defined(_MSC_VER)
#define ATOM_COMPILER_CLANG 1
#elif defined(__GNUC__)
#define ATOM_COMPILER_GCC 1
#elif defined(_MSC_VER)
#if defined(__clang__) && !defined(ATOM_COMPILER_CLANG_CL)
#define ATOM_COMPILER_CLANG_CL 1
#elif !defined(ATOM_COMPILER_MSVC)
#define ATOM_COMPILER_MSVC 1
#endif
#endif

// cxx 11
#if !defined(ATOM_CXX_11) && defined(__cplusplus)
#if (__cplusplus >= 201103L)
#define ATOM_CXX_11 1
#elif defined(__GNUC__) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#define ATOM_CXX_11 1
#elif defined(_MSC_VER) && _MSC_VER >= 1600
#define ATOM_CXX_11 1
#endif
#endif

// cxx 14
#if !defined(ATOM_CXX_14) && defined(__cplusplus)
#if (__cplusplus >= 201402L)
#define ATOM_CXX_14 1
#elif defined(_MSC_VER) && (_MSC_VER >= 1900)
#define ATOM_CXX_14 1
#endif
#endif

// cxx 17
#if !defined(ATOM_CXX_17) && defined(__cplusplus)
#if (__cplusplus >= 201703L)
#define ATOM_CXX_17 1
#elif defined(_MSVC_LANG) && (_MSVC_LANG >= 201703L)
#define ATOM_CXX_17 1
#endif
#endif

// cxx 20
#if !defined(ATOM_CXX_20) && defined(__cplusplus)
#if (__cplusplus >= 202002L)
#define ATOM_CXX_20 1
#elif defined(_MSVC_LANG) && (_MSVC_LANG >= 202002L)
#define ATOM_CXX_20 1
#endif
#endif

// cxx version
#if defined(ATOM_CXX_20)
#define ATOM_CXX_VERSION 20
#elif defined(ATOM_CXX_17)
#define ATOM_CXX_VERSION 17
#elif defined(ATOM_CXX_14)
#define ATOM_CXX_VERSION 14
#elif defined(ATOM_CXX_11)
#define ATOM_CXX_VERSION 11
#endif

// fall back
#include "compiler_fallback.inc"