/* module.c
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "util.h"
#include "module.h"

/**
 * create a new module
 * @param name the name of the module (for hash lookups, etc)
 * @param path the path to the module
 */
module_t *module_new(const char *name, const char *path)
{
    module_t *module;

#ifdef DEBUG
    assert(NULL != name);
    assert(NULL != path);
#endif

    module = calloc(1, sizeof *module);
    if (NULL == module) {
        ERRF(__FILE__, __LINE__, "allocating for a new module!\n");
        return NULL;
    }

    module_init(module, name, path);

    return module;
}

/**
 * initialize an allocate module
 * @param module the allocated module to initialize
 * @param name the name of the module (for hash lookups, etc)
 * @param path the path to the module
 */
int module_init(module_t * module, const char *name, const char *path)
{
#ifdef DEBUG
    assert(NULL != module);
    assert(NULL != name);
    assert(NULL != path);
#endif

    module->name = strdup(name);

#ifdef WIN32
    module->path = calloc(1, strlen(path) + strlen(name) + 6);
    snprintf(module->path, strlen(path) + strlen(name) + 6, "%s/%s.dll",
             path, name);
#else
    module->path = calloc(1, strlen(path) + strlen(name) + 5);
    snprintf(module->path, strlen(path) + strlen(name) + 5, "%s/%s.so",
             path, name);
#endif

    module->module = NULL;
    module->loaded = 0;

    return 1;
}

/**
 * load the module
 * @param module the module to laod
 */
int module_load(module_t * module)
{
#ifdef DEBUG
    assert(NULL != module);
#endif

#ifdef WIN32
    module->module = LoadLibrary(module->path);
    if (NULL == module->module) {
        ERRF(__FILE__, __LINE__, "loading module from file %s!\n",
             module->path);
        return 0;
    }
#else
    module->module = dlopen(module->path, RTLD_LAZY);
    if (NULL == module->module) {
        ERRF(__FILE__, __LINE__, "loading module from file %s: %s!\n",
             module->path, dlerror());
        return 0;
    }
#endif

    module->loaded = 1;

    return 1;
}

/**
 * unload the module
 * @param module the module to unload
 */
void module_unload(module_t * module)
{
#ifdef DEBUG
    assert(NULL != module);
#endif

    if (module->loaded) {
        /* k, we gotta do this! */
#ifdef WIN32
        FreeLibrary(module->module);
        module->loaded = 0;
#else
        dlclose(module->module);
        module->loaded = 0;
#endif
    }
}

/**
 * get the module
 * @param module the module to grab the dlopen()'d object from
 */
dlptr_t module_get(module_t * module)
{
#ifdef DEBUG
    assert(NULL != module);
#endif

    if (module->loaded) {
        /* k, we have dlopen()'d the module */
        return module->module;
    }

    return NULL;
}

/**
 * get a symbol from the module
 * @param module the module to dlsym() the symbol from
 * @param name the name of the symbol to snag
 */
funcptr_t module_get_symbol(module_t * module, const char *name)
{
    funcptr_t symbol;

#ifdef DEBUG
    assert(NULL != module);
    assert(NULL != name);
#endif

    if (!module->loaded) {
        /* um...hello... */
        return NULL;
    }
#ifdef WIN32
    if (NULL != (symbol = GetProcAddress(module->module, name))) {
        return symbol;
    } else {
        ERRF(__FILE__, __LINE__, "loading symbol %s from %s!\n",
             name, module->name);
    }
#else
    if (NULL != (symbol = dlsym(module->module, name))) {
        return symbol;
    } else {
        ERRF(__FILE__, __LINE__, "loading symbol %s from %s: %s!\n",
             name, module->name, dlerror());
    }
#endif

    return NULL;
}

/**
 * destroy the module
 * @param module the module to destroy
 */
void module_destroy(module_t * module)
{
#ifdef DEBUG
    assert(NULL != module);
#endif

    if (module->name)
        free(module->name);
    if (module->path)
        free(module->path);

    module_unload(module);
}

/**
 * free the module object
 * @param module the modujle to free
 */
void module_free(module_t * module)
{
#ifdef DEBUG
    assert(NULL != module);
#endif

    module_destroy(module);
    free(module);
}
