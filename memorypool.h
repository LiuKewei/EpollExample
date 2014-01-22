/*
 * memorypool.h
 *
 *  Created on: 2014-1-22
 *      Author: kedou
 */

#ifndef MEMORYPOOL_H_
#define MEMORYPOOL_H_

#include <stdint.h>

/** basic memory node structure */
struct kd_memnode_t {
	kd_memnode_t *next; /**< next memnode */
	kd_memnode_t **ref; /**< reference to self */
	uint32_t index; /**< size */
	uint32_t free_index; /**< how much free */
	char *first_avail; /**< pointer to first free memory */
	char *endp; /**< pointer to end of free memory */
};

#define KD_MEMNODE_T_SIZE sizeof(kd_memnode_t)
#define KD_ALIGN(size, boundary) \
(((size) + ((boundary) - 1)) & ~((boundary) - 1))
#define KD_ALIGN_DEFAULT(size) KD_ALIGN(size, 8)

#define MAX_INDEX 20

struct kd_allocator_t {
	uint32_t max_index;
	uint32_t max_free_index;
	uint32_t current_free_index;
#if KD_HAS_THREADS
	pthread_mutex_t *mutex;
#endif /* KD_HAS_THREADS */
	kd_pool_t *owner;
	kd_memnode_t *free[MAX_INDEX];
};



#endif /* MEMORYPOOL_H_ */
