/* thread.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "thread.h"
#include "stack.h"
#include "util.h"

/*
 * typedef struct _tpool_t {
 *     pthread_t *pool;
 *     unsigned int cnt;
 *     void (*handler)(void *);
 *     sstack_t work;
 * } tpool_t;
 */

/* allocate */
tpool_t *tpool_create(unsigned int cnt, tfunc controller, tfunc handler)
{
	tpool_t *tp = calloc(1, sizeof *tp);

	/* set it up */
	if (tp)
		tpool_init(tp, cnt, controller, handler);

	return tp;
}

/* create */
int tpool_init(tpool_t * tp, unsigned int cnt, tfunc controller, tfunc handler)
{
	int i;

#ifdef DEBUG
	assert(NULL != tp);
	assert(NULL != handler);
#endif

	tp->cnt = cnt;
	tp->pool = calloc(tp->cnt, sizeof *(tp->pool));
	tp->handler = handler;
	tp->controller = controller;
	tp->jobcount = 0;

	/* set up mutex */
	pthread_mutex_init(&tp->mutex, NULL);

	/* get work queue ready */
	stack_init(&tp->work);

	/* set up the control thread */
	pthread_create(&tp->boss.th, NULL, tp->controller, NULL);
	tp->boss.id = 0;
	tp->boss.job = 0;
	tp->boss.busy = 0;
	pthread_mutex_init(&tp->boss.mt, NULL);
	pthread_mutex_lock(&tp->boss.mt);
	pthread_detach(tp->boss.th);

	for (i = 0; i < (signed)tp->cnt; ++i) {
		if (pthread_create(&tp->pool[i].th, NULL, tp->handler, (void *)i)) {
			/* trouble... */
			ERRF(__FILE__, __LINE__, "error creating thread %d in pool.\n", i);
			return 0;
		}

		/* automatic cleanup */
		tp->pool[i].id = i;
		tp->pool[i].job = 0;
		tp->pool[i].busy = 0;
		pthread_mutex_init(&tp->pool[i].mt, NULL);
		pthread_mutex_lock(&tp->pool[i].mt);
		pthread_detach(tp->pool[i].th);
	}

	return 1;
}

/* destroy */
void tpool_destroy(tpool_t * tp)
{
	tp->cnt = 0;
	tp->handler = NULL;
	pthread_mutex_destroy(&tp->mutex);
	stack_destroy(&tp->work);
	memset(tp->pool, '\0', tp->cnt * sizeof(pthread_t));
	free(tp->pool);
}

/* add a job */
int tpool_add_work(tpool_t * tp, void *job)
{
	pthread_mutex_lock(&tp->mutex);
	stack_push(&tp->work, job);
	tp->jobcount++;
	pthread_mutex_unlock(&tp->mutex);

	pthread_mutex_unlock(&tp->boss.mt);

	return 1;
}

void *tpool_get_work(tpool_t * tp)
{
	void *job = NULL;

	if (tp->jobcount) {
		/* we have work... */
		pthread_mutex_lock(&tp->mutex);
		tp->jobcount--;
		job = stack_pop(&tp->work);
		pthread_mutex_unlock(&tp->mutex);
	}

	return job;
}

/* check pending */
int tpool_pending_jobs(tpool_t * tp)
{
	return tp->jobcount;
}
