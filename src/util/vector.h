/* vector.h
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

#ifndef UTIL_VECTOR_H
#define UTIL_VECTOR_H

typedef struct _vector_t {
	unsigned int count;
	unsigned int slots;
	unsigned int size;

	void *data;
} vector_t;

/* create a new vector */
vector_t *vector_new(unsigned int, unsigned int);
/* initialize an allocated vector */
int vector_init(vector_t *, unsigned int, unsigned int);
/* append a value to the vector */
int vector_push(vector_t *, void *);
/* set a value at a certain position */
int vector_set_at(vector_t *, unsigned int, void *);
/* get the value at a certain position */
void *vector_get_at(vector_t *, unsigned int);
/* clear the whole vector */
void vector_clear(vector_t *);
/* resize the vector */
int vector_resize(vector_t *);
/* destroy the vector */
void vector_destroy(vector_t *);
/* free the vector */
void vector_free(vector_t *);

#endif
