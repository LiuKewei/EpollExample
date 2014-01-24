#ifndef IOTHREAD_H_INCLUDED
#define IOTHREAD_H_INCLUDED

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>

#ifndef __USE_UNIX98
#define __USE_UNIX98
#endif

enum shutdown
{
    ACTIVED,
    FORCE_STOP,
    WAIT_WORKING_TASKS,
    WAIT_ALL_TASKS
};

typedef struct tpool_job
{
    void*                (*runfunc)(void*); // task func
    void*                arg; // task func's parameter
    struct tpool_job * next;
} tpool_job_t;

typedef struct tpool
{
    enum shutdown    is_shutdown;
    int                max_thr_num;
    int                current_job_num;
    pthread_t        *thr_id;
    tpool_job_t      *queue_head;
    pthread_mutex_t queue_lock;
    pthread_cond_t  queue_ready;
} tpool_t;

bool tpool_init(int max_thr_num);

bool tpool_destroy(enum shutdown shutdown_way);

int tpool_add_job(void* (*runfunc)(void*), void* arg);


#endif // IOTHREAD_H_INCLUDED
