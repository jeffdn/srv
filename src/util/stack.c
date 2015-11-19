/* stack.c
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
#include <assert.h>

#include "util.h"
#include "stack.h"

/**
 * create a new stack
 */
sstack_t *stack_new(void)
{
    sstack_t *stk;

    stk = calloc(1, sizeof *stk);
    if (NULL == stk) {
        ERRF(__FILE__, __LINE__, "ERROR: allocating for a new stack!\n");
        exit(1);
    }

    stack_init(stk);

    return stk;
}

/**
 * initialize a new stack
 * @param stack the stack to initialize
 */
void stack_init(sstack_t * stk)
{
#ifdef DEBUG
    assert(NULL != stk);
#endif

    stk->count = 0;
    stk->first = NULL;
    stk->node = NULL;
    stk->data_alloc = NULL;
    stk->data_free = NULL;
}

/**
 * push an entry onto the stack
 * @param stack the stack to push on to
 * @param entry the data to place in the node
 */
void stack_push(sstack_t * stk, void *entry)
{
    stacknode_t *node;
    void *tmp;

#ifdef DEBUG
    assert(NULL != stk);
    assert(NULL != stk->data_alloc);
#endif

    if (NULL == entry) {
        tmp = stk->data_alloc(NULL);
        if (NULL == tmp) {
            /* sigh... */
            ERRF(__FILE__, __LINE__,
                 "ERROR: allocating for new data entry in stack node!\n");
            exit(1);
        }
    } else {
        tmp = stk->data_alloc(entry);
    }

    if (NULL == stk->node) {
        stk->node = calloc(1, sizeof *(stk->node));
        if (NULL == stk->node) {
            ERRF(__FILE__, __LINE__,
                 "ERROR: allocating for a new stack node!\n");
            exit(1);
        }

        stk->node->next = NULL;
        stk->node->prev = NULL;
        stk->node->data = tmp;
        stk->first = stk->node;
    } else {
        node = calloc(1, sizeof *node);
        if (NULL == node) {
            ERRF(__FILE__, __LINE__,
                 "ERROR: allocating for a new stack node!\n");
            exit(1);
        }

        node->prev = stk->node;
        node->next = NULL;
        node->data = tmp;
        stk->node = node;
    }

    ++stk->count;
}

/**
 * pop some data off the stack
 * @param stack the stack to get an entry from
 */
void *stack_pop(sstack_t * stk)
{
    stacknode_t *node;
    void *data;

#ifdef DEBUG
    assert(NULL != stk);
#endif

    if (NULL == stk->node || !(stk->count)) {
        /* nothing to report */
        return NULL;
    } else if (1 == stk->count) {
        data = stk->first->data;

        stk->first->data = NULL;
        stk->first->next = NULL;
        stk->first->prev = NULL;
        free(stk->first);
        stk->first = NULL;
        stk->node = NULL;
    } else if (2 == stk->count) {
        data = stk->first->data;
        stk->first->data = NULL;
        stk->first->next = NULL;
        stk->first->prev = NULL;
        free(stk->first);
        stk->first = stk->node;
    } else {
        data = stk->first->data;
        stk->first = stk->first->next;
    }

    --stk->count;

    return data;
}

/**
 * look at the data on the top of the stack without popping it
 * @param stack the stack to peek at
 */
void *stack_peek(sstack_t * stk)
{
    if (NULL == stk->node) {
        return NULL;
    }

    return stk->node->data;
}

/**
 * destroy a stack
 * @param stack the stack to destroy
 */
void stack_destroy(sstack_t * stk)
{
#ifdef DEBUG
    assert(NULL != stk);
#endif

    stack_remove_count(stk, stk->count);

    stk->count = 0;
    stk->data_alloc = NULL;
    stk->data_free = NULL;
}

/**
 * free a stack
 * @param stack the stack to free
 */
void stack_free(sstack_t * stk)
{
#ifdef DEBUG
    assert(NULL != stk);
#endif

    stack_destroy(stk);
    free(stk);
}

/**
 * set the stack's data allocation function
 * @param stack the stack to set
 * @param data_alloc the allocation funciton to use
 */
void stack_set_data_alloc(sstack_t * stk, void *(*data_alloc) (void *))
{
#ifdef DEBUG
    assert(NULL != stk);
    assert(NULL != data_alloc);
#endif

    stk->data_alloc = data_alloc;
}

/**
 * set the stack's data freeing function
 * @param stack the stack to set
 * @param data_free the freeing function to use
 */
void stack_set_data_free(sstack_t * stk, void (*data_free) (void *))
{
#ifdef DEBUG
    assert(NULL != stk);
    assert(NULL != data_free);
#endif

    stk->data_free = data_free;
}

/**
 * add the specified number of nodes to the stack
 * @param stack the stack to add nodes to
 * @param count the number of nodes to add
 */
int stack_add_count(sstack_t * stk, unsigned int count)
{
    unsigned int i;

#ifdef DEBUG
    assert(NULL != stk);
#endif

    for (i = 0; i < count; i++)
        stack_push(stk, NULL);

    return 1;
}

/**
 * remove the specified number of nodes from the stack
 * @param stack the stack to remove nodes from
 * @param count the number of nodes to remove
 */
int stack_remove_count(sstack_t * stk, unsigned int count)
{
    unsigned int i;
    void *tmp;

#ifdef DEBUG
    assert(NULL != stk);
#endif

    for (i = 0; i < count && stk->count; i++) {
        tmp = stack_pop(stk);
        stk->data_free(tmp);
    }

    return 1;
}
