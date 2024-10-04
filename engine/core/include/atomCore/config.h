#pragma once

// platform headers
#ifndef __cplusplus
#include <stdbool.h>
#endif

#if __has_include("sys/types.h")
#include <sys/types.h>
#endif

#if __has_include("stdint.h")
#include <stdint.h>
#endif

#if __has_include("float.h")
#include <float.h>
#endif

// platform & compiler marcos
#include "./config/platform.h"
#include "./config/compiler.h"

// keywords
#include "./config/keywords.h"

#ifdef __cplusplus
#define ATOM_NULL nullptr
#else
#define ATOM_NULL 0
#endif

#ifdef __cplusplus
#ifndef ATOM_NULLPTR
#define ATOM_NULLPTR nullptr
#endif
#else
#ifndef ATOM_NULLPTR
#define ATOM_NULLPTR ATOM_NULL
#endif
#endif

#ifndef atom_max
#define atom_max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef atom_min
#define atom_min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define atom_round_up(value, multiple)   ((((value) + (multiple) - 1) / (multiple)) * (multiple))
#define atom_round_down(value, multiple) ((value) - (value) % (multiple))

#define atom_align(size, align) ((size + align - 1) & (~(align - 1)))

#ifdef _DEBUG
#include "assert.h"
#define atom_assert assert
#else
#define atom_assert(expr) (void)(expr);
#endif
#define atom_static_assert static_assert

#ifndef ATOM_API
#ifdef SHARED_MODULE
#define ATOM_API ATOM_EXPORT
#elif
#define ATOM_API ATOM_IMPORT
#endif
#endif

#ifndef ATOM_EXTERN_C_BEGIN
#ifdef __cplusplus
#define ATOM_EXTERN_C_BEGIN extern "C" {
#else
#define ATOM_EXTERN_C_BEGIN
#endif
#endif

#ifndef ATOM_EXTERN_C_END
#ifdef __cplusplus
#define ATOM_EXTERN_C_END }
#else
#define ATOM_EXTERN_C_END
#endif
#endif