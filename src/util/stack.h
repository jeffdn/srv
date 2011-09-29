/* stack.h
 * Copyright (c) 2006
 * Jeff Nettleton
 */

#ifndef UTIL_STACK_H
#define UTIL_STACK_H

typedef struct _stacknode_t {
	void *data;
	struct _stacknode_t *next;
	struct _stacknode_t *prev;
} stacknode_t;

typedef struct _sstack_t {
	unsigned int count;

	stacknode_t *node;
	stacknode_t *first;

	void *(*data_alloc) (void *);
	void (*data_free) (void *);
} sstack_t;

/* create a new stack */
sstack_t *stack_new(void);
/* initialize an allocated stack */
void stack_init(sstack_t *);
/* push something onto the stack */
void stack_push(sstack_t *, void *);
/* pop something off of the stack */
void *stack_pop(sstack_t *);
/* look at the top of the stack */
void *stack_peek(sstack_t *);
/* destroy a stack */
void stack_destroy(sstack_t *);
/* free a stack */
void stack_free(sstack_t *);

/* utility functions */
/* set the allocation function for the stack */
void stack_set_data_alloc(sstack_t *, void *(*data_alloc) (void *));
/* set the freeing function for the stack */
void stack_set_data_free(sstack_t *, void (*data_free) (void *));
/* add a specified number of nodes to the stack */
int stack_add_count(sstack_t *, unsigned int);
/* remove up to the specified number of nodes from the stack */
int stack_remove_count(sstack_t *, unsigned int);

#endif
