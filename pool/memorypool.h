/*
 * memorypool.h
 *
 *  Created on: 2014-1-22
 *      Author: kedou
 */

#ifndef MEMORYPOOL_H_
#define MEMORYPOOL_H_

#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>

#define MAX_INDEX 20

#define KD_MEMNODE_T_SIZE KD_ALIGN_DEFAULT(sizeof(kd_memnode_t))
#define KD_ALLOCATOR_MAX_FREE_UNLIMITED   0
#define KD_ALIGN(size, boundary) \
(((size) + ((boundary) - 1)) & ~((boundary) - 1))
#define KD_ALIGN_DEFAULT(size) KD_ALIGN(size, 8)


typedef struct cleanup_t cleanup_t;


#if KD_POOL_DEBUG

typedef struct debug_node_t debug_node_t;

struct debug_node_t {
	debug_node_t *next;
	uint32_t index;
	void *beginp[64];
	void *endp[64];
};

#define SIZEOF_DEBUG_NODE_T KD_ALIGN_DEFAULT(sizeof(debug_node_t))

#endif /* APR_POOL_DEBUG */

typedef struct kd_allocator_t kd_allocator_t;
typedef struct kd_memnode_t kd_memnode_t;

/** basic memory node structure */
struct kd_memnode_t {
	kd_memnode_t *next; /**< next memnode */
	kd_memnode_t **ref; /**< reference to self */
	uint32_t index; /**< size */
	uint32_t free_index; /**< how much free */
	char *first_avail; /**< pointer to first free memory */
	char *endp; /**< pointer to end of free memory */
};

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

#define SIZEOF_ALLOCATOR_T APR_ALIGN_DEFAULT(sizeof(kd_allocator_t))

struct kd_pool_t {
	kd_pool_t *parent;
	kd_pool_t *child;
	kd_pool_t *sibling;
	kd_pool_t **ref; //用于指向内存池本身
	cleanup_t *cleanups;
	kd_allocator_t *allocator;
	struct process_chain *subprocesses;
	kd_abortfunc_t abort_fn;
	kd_hash_t *user_data;
	const char *tag;
#if !KD_POOL_DEBUG
	kd_memnode_t *active;
	kd_memnode_t *self; /* The node containing the pool itself */
	char *self_first_avail;
#else /* KD_POOL_DEBUG */
	debug_node_t *nodes;
	const char *file_line;
	uint32_t creation_flags;
	unsigned int stat_alloc;
	unsigned int stat_total_alloc;
	unsigned int stat_clear;
	#if KD_HAS_THREADS
		pthread_t owner;
		pthread_mutex_t *mutex;
	#endif /* KD_HAS_THREADS */
#endif /* KD_POOL_DEBUG */
#ifdef NETWAREs
	pid_t owner_proc;
#endif /* defined(NETWARE) */
};
typedef struct kd_pool_t kd_pool_t;

#endif /* MEMORYPOOL_H_ */
