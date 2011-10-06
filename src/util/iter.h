/* iterator.h
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

#ifndef UTIL_ITER_H
#define UTIL_ITER_H

#include "hash.h"
#include "vector.h"

/* hash iterator */

#define hash_iter_foreach(iter) \
    for (hash_iter_init (iter, iter->ht); hash_iter_next (iter);)

typedef struct _hash_iter_t {
	unsigned int pos;
	unsigned int depth;

	/* pointer to a place in the hash's data table */
	hash_entry_t *he;
	/* pointer to the hash table */
	hash_t *ht;

	/* the data */
	void *first;
	void *second;
} hash_iter_t;

/* create a new hash iterator */
hash_iter_t *hash_iter_new(hash_t *);
/* initialize a new hash iterator */
void hash_iter_init(hash_iter_t *, hash_t *);
/* move to the next position in the table */
int hash_iter_next(hash_iter_t *);
/* move back a position in the table */
int hash_iter_prev(hash_iter_t *);
/* check if we are at the beginning */
int hash_iter_begin(hash_iter_t *);
/* check if we are at the end */
int hash_iter_end(hash_iter_t *);
/* destroy a hash iterator */
void hash_iter_destroy(hash_iter_t *);
/* free a hash iterator */
void hash_iter_free(hash_iter_t *);

/* vector iterator */

#define vector_iter_foreach(iter) \
    for (vector_iter_init (iter, iter->vec); vector_iter_next (iter);)

typedef struct _vector_iter_t {
	unsigned int pos;

	/* our vector */
	vector_t *vec;

	/* the data */
	void *data;
} vector_iter_t;

/* create a new hash iterator */
vector_iter_t *vector_iter_new(vector_t *);
/* initialize a new hash iterator */
void vector_iter_init(vector_iter_t *, vector_t *);
/* move to the next position in the table */
int vector_iter_next(vector_iter_t *);
/* move back a position in the table */
int vector_iter_prev(vector_iter_t *);
/* check if we are at the beginning */
int vector_iter_begin(vector_iter_t *);
/* check if we are at the end */
int vector_iter_end(vector_iter_t *);
/* destroy a hash iterator */
void vector_iter_destroy(vector_iter_t *);
/* free a hash iterator */
void vector_iter_free(vector_iter_t *);

#endif
