/* vector.h
 * Copyright (c) 2006
 * Jeff Nettleton
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
