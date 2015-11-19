/* iter.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "util.h"
#include "hash.h"
#include "iter.h"
#include "vector.h"

/**
 * create a new hash table iterator
 * @param ht the hash table to create the iterator for
 */
hash_iter_t *hash_iter_new(hash_t * ht)
{
    hash_iter_t *iter;

    iter = calloc(1, sizeof *iter);
    if (NULL == iter) {
        ERRF(__FILE__, __LINE__, "allocating for a new hash iterator!\n");
        exit(1);
    }

    hash_iter_init(iter, ht);

    return iter;
}

/**
 * initialize an allocate hash iterator
 * @param iter the hash iterator to initialize
 * @param ht the hash table to assign to the iterator
 */
void hash_iter_init(hash_iter_t * iter, hash_t * ht)
{
#ifdef DEBUG
    assert(NULL != iter);
    assert(NULL != ht);
#endif

    iter->pos = 0;
    iter->depth = 0;
    iter->ht = ht;
    iter->he = NULL;
    iter->first = NULL;
    iter->second = NULL;

    hash_iter_next(iter);
}

/**
 * move the iterator forward one place
 * @param iter the iter to advance
 */
int hash_iter_next(hash_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    if (NULL != iter->he) {
        if (NULL != iter->he->next) {
            /* there was a collision */
            iter->he = iter->he->next;

            iter->first = iter->he->key;
            iter->second = iter->he->val;

            ++iter->depth;

            return 1;
        } else {
            /* move up one... */
            ++iter->pos;
        }
    }

    /* reset the depth */
    iter->depth = 0;

    for (; iter->pos < iter->ht->slots; iter->pos++) {
        if (NULL != (iter->he = iter->ht->data[iter->pos])) {
            /* we hit an allocated entry... */
            iter->first = iter->he->key;
            iter->second = iter->he->val;

            return 1;
        }
    }

    return 0;
}

/**
 * step back in the hash table
 * @param iter the iter to retreat one step
 */
int hash_iter_prev(hash_iter_t * iter)
{
    unsigned int i;

#ifdef DEBUG
    assert(NULL != iter);
#endif

    if (hash_iter_begin(iter)) {
        /* already at the beginning */
        return 0;
    }

    if (iter->depth > 0) {
        for (iter->he = iter->ht->data[iter->pos], i = 0; i < iter->depth; i++) {
            /* descend to one level below where we are now */
            iter->he = iter->he->next;
        }

        --iter->depth;
        return 1;
    }

    for (; iter->pos; iter->pos--) {
        if (NULL != (iter->he = iter->ht->data[iter->pos])) {
            while (NULL != iter->he->next) {
                /* proceed along through the linked list */
                iter->he = iter->he->next;

                ++iter->depth;
            }

            return 1;
        }
    }

    return 0;
}

/**
 * destroy a hash iterator
 * @param iter the iterator to destroy
 */
void hash_iter_destroy(hash_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    iter->pos = 0;
    iter->depth = 0;
    iter->ht = NULL;
    iter->he = NULL;
    iter->first = NULL;
    iter->second = NULL;
}

/**
 * check if we are at the beginning of the hash table
 * @param iter the iterator to check
 */
int hash_iter_begin(hash_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    return (!iter->pos) ? 1 : 0;
}

/**
 * reset the position of the hash iterator
 * @param iter the iterator
 */
void hash_iter_reset(hash_iter_t * iter)
{
    hash_t *tmp;

#ifdef DEBUG
    assert(NULL != iter);
#endif

    tmp = iter->ht;
    hash_iter_destroy(iter);
    hash_iter_init(iter, tmp);
}

/**
 * free a hash iterator
 * @param iter the hash iter to free
 */
void hash_iter_free(hash_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    hash_iter_destroy(iter);
    free(iter);
}

/**
 * create a new vector table iterator
 * @param vec the vector table to create the iterator for
 */
vector_iter_t *vector_iter_new(vector_t * vec)
{
    vector_iter_t *iter;

    iter = calloc(1, sizeof *iter);
    if (NULL == iter) {
        ERRF(__FILE__, __LINE__, "allocating for a new vector iterator!\n");
        exit(1);
    }

    vector_iter_init(iter, vec);

    return iter;
}

/**
 * initialize an allocate vector iterator
 * @param iter the vector iterator to initialize
 * @param vec the vector table to assign to the iterator
 */
void vector_iter_init(vector_iter_t * iter, vector_t * vec)
{
#ifdef DEBUG
    assert(NULL != iter);
    assert(NULL != vec);
#endif

    iter->pos = 0;
    iter->vec = vec;
    iter->data = NULL;

    vector_iter_next(iter);
}

/**
 * move the iterator forward one place
 * @param iter the iter to advance
 */
int vector_iter_next(vector_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    if (iter->pos == (iter->vec->count - 1)) {
        /* at the end */
        return 0;
    }

    if (NULL == iter->data && !iter->pos) {
        /* running for the first time */
        iter->data = iter->vec->data;

        return 1;
    } else {
        /* lets doo this */
        ++iter->pos;
        iter->data = (char *)iter->vec->data + (iter->pos * iter->vec->size);

        return 1;
    }

    return 0;
}

/**
 * step back in the vector table
 * @param iter the iter to retreat one step
 */
int vector_iter_prev(vector_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    if (vector_iter_begin(iter)) {
        /* already at the beginning */
        return 0;
    }

    --iter->pos;
    iter->data = (char *)iter->vec->data + (iter->pos * iter->vec->size);

    return 1;
}

/**
 * destroy a vector iterator
 * @param iter the iterator to destroy
 */
void vector_iter_destroy(vector_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    iter->pos = 0;
    iter->data = NULL;
    iter->vec = NULL;
}

/**
 * check if we are at the beginning of the vector table
 * @param iter the iterator to check
 */
int vector_iter_begin(vector_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    return (!iter->pos) ? 1 : 0;
}

/**
 * reset the position of the vector iterator
 * @param iter the iterator
 */
void vector_iter_reset(vector_iter_t * iter)
{
    vector_t *tmp;

#ifdef DEBUG
    assert(NULL != iter);
#endif

    tmp = iter->vec;
    vector_iter_destroy(iter);
    vector_iter_init(iter, tmp);
}

/**
 * free a vector iterator
 * @param iter the vector iter to free
 */
void vector_iter_free(vector_iter_t * iter)
{
#ifdef DEBUG
    assert(NULL != iter);
#endif

    vector_iter_destroy(iter);
    free(iter);
}
