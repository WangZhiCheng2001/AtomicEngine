#define UUID_SYSTEM_GENERATOR
#include <uuid.h>

#include <atomCore/base/guid.h>

// for __imp_CoCreateGuid not found problem
#ifdef ATOM_PLAT_WINDOWS
#pragma comment(lib, "Ole32.lib")
#endif

using namespace uuids;

ATOM_EXTERN_C ATOM_API void atom_make_guid(atom_guid_t* guid) { *((uuid*)guid) = uuid_system_generator{}(); }