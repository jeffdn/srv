/* thread.h
 * Copyright (c) 2007
 * Jeff Nettleton
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
