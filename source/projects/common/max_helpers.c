// max_helpers.c
// Implementation of shared Max/MSP helper functions

#include "max_helpers.h"
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// ----------------------------------------------------------------------------
// Path Resolution

t_string* mxh_get_external_path(t_class* c, const char* subpath) {
    char external_path[MAX_PATH_CHARS];
    char external_name[MAX_PATH_CHARS];
    char conform_path[MAX_PATH_CHARS];
    short path_id = class_getpath(c);
    t_string* result;

#ifdef __APPLE__
    const char* ext_filename = "%s.mxo";
#else
    const char* ext_filename = "%s.mxe64";
#endif

    snprintf_zero(external_name, MAX_PATH_CHARS, ext_filename, c->c_sym->s_name);
    path_toabsolutesystempath(path_id, external_name, external_path);
    path_nameconform(external_path, conform_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);
    result = string_new(external_path);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    return result;
}

t_string* mxh_get_package_path(t_class* c, const char* subpath) {
    t_string* result;
    t_string* external_path = mxh_get_external_path(c, NULL);

    const char* ext_path_c = string_getptr(external_path);

    // dirname() modifies its input, so we need to make a copy
    char* path_copy = strdup(ext_path_c);
    if (!path_copy) {
        object_free(external_path);
        return NULL;
    }

    char* dir1 = dirname(path_copy);
    char* dir1_copy = strdup(dir1);
    free(path_copy);

    if (!dir1_copy) {
        object_free(external_path);
        return NULL;
    }

    char* dir2 = dirname(dir1_copy);
    result = string_new(dir2);
    free(dir1_copy);

    if (subpath != NULL) {
        string_append(result, subpath);
    }

    object_free(external_path);

    return result;
}

// ----------------------------------------------------------------------------
// File Loading

int mxh_load_lua_file(t_class* c, t_symbol* filename,
                               int (*load_func)(void*, const char*),
                               void* context)
{
    if (filename == gensym("")) {
        return 0;
    }

    char norm_path[MAX_PATH_CHARS];
    path_nameconform(filename->s_name, norm_path, PATH_STYLE_MAX, PATH_TYPE_BOOT);

    // Try absolute path first
    if (access(norm_path, F_OK) == 0) {
        post("loading: %s", norm_path);
        return load_func(context, norm_path);
    }

    // Try in the examples folder
    t_string* path = mxh_get_package_path(c, "/examples/");
    if (!path) {
        error("max_helpers: failed to get package path");
        return -1;
    }

    string_append(path, filename->s_name);
    const char* lua_file = string_getptr(path);
    post("loading: %s", lua_file);

    int result = load_func(context, lua_file);
    object_free(path);

    return result;
}
