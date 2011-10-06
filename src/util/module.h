/* module.h
 * Copyright (c) 2006
 * Jeff Nettleton
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 */

#ifndef UTIL_MODULE_H
#define UTIL_MODULE_H

/* I apologize in advance for how ugly this is. This
 * is however one of the reasons I have seperated the
 * base, so that the MOS code can be free of this sort
 * of shenanigans.
 */

#ifdef WIN32
typedef HMODULE dlptr_t;
typedef FARPROC funcptr_t;
#else
typedef void *dlptr_t;
typedef void *funcptr_t;
#endif

typedef struct _module_t {
	char *name;
	char *path;

	/* weeee! */
	dlptr_t module;
	unsigned int loaded;
} module_t;

/* create a new module */
module_t *module_new(const char *, const char *);
/* initialize a module */
int module_init(module_t *, const char *, const char *);
/* load the module */
int module_load(module_t *);
/* unload the module */
void module_unload(module_t *);

/* get module */
dlptr_t module_get(module_t *);
/* get a symbol */
funcptr_t module_get_symbol(module_t *, const char *);

/* destroy a module object */
void module_destroy(module_t *);
/* free a module object */
void module_free(module_t *);

#endif
