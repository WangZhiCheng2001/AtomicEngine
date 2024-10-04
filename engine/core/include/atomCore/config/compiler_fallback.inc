#ifdef __ATOM_COMPILER_FALLBACK_INC__
    #error "compiler_fallback.inc included twice"
#endif
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
//      ATOM_CXX_VER
//-------------------------------------------------------------------------------

// compiler
#ifndef ATOM_COMPILER_GCC
    #define ATOM_COMPILER_GCC 0
#endif
#ifndef ATOM_COMPILER_MSVC
    #define ATOM_COMPILER_MSVC 0
#endif
#ifndef ATOM_COMPILER_CLANG
    #define ATOM_COMPILER_CLANG 0
#endif
#ifndef ATOM_COMPILER_CLANG_CL
    #define ATOM_COMPILER_CLANG_CL 0
#endif

// cxx version
#ifndef ATOM_CXX_11
    #define ATOM_CXX_11 0
#endif
#ifndef ATOM_CXX_14
    #define ATOM_CXX_14 0
#endif
#ifndef ATOM_CXX_17
    #define ATOM_CXX_17 0
#endif
#ifndef ATOM_CXX_20
    #define ATOM_CXX_20 0
#endif

#define __ATOM_COMPILER_FALLBACK_INC__