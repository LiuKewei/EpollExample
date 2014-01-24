/*
 * systhread.h
 *
 *  Created on: 2014-1-24
 *      Author: kedou
 */

#ifndef SYSTHREAD_H_
#define SYSTHREAD_H_

#include <pthread.h>
#include "memorypool.h"

#if KD_HAS_THREADS
struct kd_thread_mutex_t {
    kd_pool_t *pool;
    pthread_mutex_t mutex;
};
#endif

#endif /* SYSTHREAD_H_ */
