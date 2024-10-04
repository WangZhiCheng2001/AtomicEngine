#pragma once

#include <atomCore/config.h>

typedef struct {
    uint8_t value[16];
} atom_guid_t;

ATOM_EXTERN_C ATOM_API void atom_make_guid(atom_guid_t* guid);