// max_helpers.h
// Shared Max/MSP helper functions for luajit-max externals
// Path resolution, file loading, and Max integration utilities

#ifndef MAX_HELPERS_H
#define MAX_HELPERS_H

#include "ext.h"
#include "ext_obex.h"
#include "ext_strings.h"

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Path Resolution

// Get absolute path to the external itself
// Returns t_string that must be freed with object_free()
t_string* mxh_get_external_path(t_class* c, const char* subpath);

// Get path to package directory (two levels up from external)
// Returns t_string that must be freed with object_free()
t_string* mxh_get_package_path(t_class* c, const char* subpath);

// ----------------------------------------------------------------------------
// File Loading

// Resolve and load a Lua file, checking both absolute path and package examples folder
// Returns 0 on success, -1 on failure
int mxh_load_lua_file(t_class* c, t_symbol* filename,
                               int (*load_func)(void*, const char*),
                               void* context);

#ifdef __cplusplus
}
#endif

#endif // MAX_HELPERS_H
