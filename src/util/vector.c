/* vector.c
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "util.h"
#include "vector.h"

/**
 * create a new vector
 * @param slots the initial number of slots to allocate. If 0, we guess.
 * @param size the size of each data member we'll be inserting
 */
vector_t *vector_new(unsigned int slots, unsigned int size)
{
    vector_t *vec = calloc(1, sizeof *vec);

    if (NULL == vec) {
        ERRF(__FILE__, __LINE__, "allocating for a new vector!\n");
        return NULL;
    }

    vector_init(vec, slots, size);

    return vec;
}

/**
 * initialize an allocated vector
 * @param vec the vector to initialize
 * @param slots the number of slots to allocate.
 * @param size the size of each member
 */
int vector_init(vector_t * vec, unsigned int slots, unsigned int size)
{
#ifdef DEBUG
    assert(NULL != vec);
    assert(0 != size);
#endif

    if (!slots) {
        /* guess 16 *shrug* */
        vec->slots = 16;
    } else {
        vec->slots = slots;
    }

    vec->size = size;
    vec->count = 0;

    vec->data = calloc(1, vec->slots * vec->size);
    if (NULL == vec->data) {
        ERRF(__FILE__, __LINE__,
             "allocating vector data table (slots=%u,size=%u)!\n",
             vec->slots, vec->size);
        return 0;
    }

    memset(vec->data, '\0', (vec->slots * vec->size));

    return 1;
}

/**
 * add a value to the vector
 * @param vec the vector to append the value to
 * @param val the vlue to add
 */
int vector_push(vector_t * vec, void *val)
{
#ifdef DEBUG
    assert(NULL != vec);
    assert(NULL != val);
#endif

    if (vec->count == (vec->slots - 1)) {
        /* too big, add more slots */
        if (!vector_resize(vec)) {
            /* sorry! */
            ERRF(__FILE__, __LINE__, "could not resize vector, failed...\n");
            return 0;
        }
    }

    vector_set_at(vec, vec->count, val);

    return 1;
}

/**
 * set a value at a certain point on the vector
 * @param vec the vector whose value to modify
 * @param index the location of the value to modify
 * @param val the new value to insert
 */
int vector_set_at(vector_t * vec, unsigned int index, void *val)
{
#ifdef DEBUG
    assert(NULL != vec);
    assert(NULL != val);
#endif

    while (index >= vec->slots) {
        /* resize until our table is big enough. */
        if (!vector_resize(vec)) {
            ERRF(__FILE__, __LINE__,
                 "could not set value at specified location\n");
            return 0;
        }
    }

    memcpy((char *)vec->data + (index * vec->size), val, vec->size);
    ++vec->count;

    return 1;
}

/**
 * get a value at a certain point in the vector
 * @param vec the vector to get the value from
 * @param index the location of the value to get
 */
void *vector_get_at(vector_t * vec, unsigned int index)
{
#ifdef DEBUG
    assert(NULL != vec);
#endif

    if (index >= vec->slots) {
        /* definitely not here. */
        return NULL;
    }

    return (char *)vec->data + (index * vec->size);
}

/**
 * double the number of slots available to a vector
 * @param vec the vector to grow
 */
int vector_resize(vector_t * vec)
{
    void *tmp;

#ifdef DEBUG
    assert(NULL != vec);
#endif

    tmp = realloc(vec->data, vec->slots * 2 * vec->size);
    if (NULL == tmp) {
        ERRF(__FILE__, __LINE__, "reallocating vector data buffer\n");
        return 0;
    }

    vec->data = tmp;
    vec->slots *= 2;

    return 1;
}

/**
 * clear the vector
 * @param vec the vector to clear
 */
void vector_clear(vector_t * vec)
{
#ifdef DEBUG
    assert(NULL != vec);
#endif

    memset((char *)vec, 0, vec->slots * vec->size);
}

/**
 * destroy the vector
 * @param vec the vector to destroy
 */
void vector_destroy(vector_t *vec)
{
#ifdef DEBUG
    assert(NULL != vec);
#endif

    if (NULL != vec->data)
        free(vec->data);

    vec->slots = 0;
    vec->count = 0;
    vec->size = 0;
    vec->data = NULL;
}

/**
 * free the vector
 * @param vec the vector to free
 */
void vector_free(vector_t * vec)
{
#ifdef DEBUG
    assert(NULL != vec);
#endif

    vector_destroy(vec);
    free(vec);
}
