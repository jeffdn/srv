/* thread.h
 * Copyright (c) 2007
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

#ifndef SRV_THREAD_H
#define SRV_THREAD_H

#include <pthread.h>

#include "vector.h"
#include "stack.h"

typedef void *(*tfunc) (void *);

struct _worker_t {
	int id, busy;
	pthread_t th;
	pthread_mutex_t mt;
	int job;
};

typedef struct _tpool_t {
	struct _worker_t boss;
	struct _worker_t *pool;
	unsigned int cnt;
	unsigned int jobcount;

	/* controller function */
	tfunc controller;
	/* handler function */
	tfunc handler;

	/* stack and mutex shield */
	pthread_mutex_t mutex;
	sstack_t work;
} tpool_t;

/* allocate */
tpool_t *tpool_create(unsigned int, tfunc, tfunc);
/* create */
int tpool_init(tpool_t *, unsigned int, tfunc, tfunc);
/* destroy */
void tpool_destroy(tpool_t *);
/* add a job */
int tpool_add_work(tpool_t *, void *);
/* get a job */
void *tpool_get_work(tpool_t *);
/* check pending */
int tpool_pending_jobs(tpool_t *);

#endif
