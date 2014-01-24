/*
 * memorypool.c
 *
 *  Created on: 2014-1-24
 *      Author: kedou
 */

#include "memorypool.h"
#include "commonerrno.h"
/*
 * Allocator
 */

kd_status_t kd_allocator_create(kd_allocator_t **allocator)
{
    kd_allocator_t *new_allocator;

    *allocator = NULL;

    if ((new_allocator = malloc(SIZEOF_ALLOCATOR_T)) == NULL)
        return KD_ENOMEM;

    memset(new_allocator, 0, SIZEOF_ALLOCATOR_T);
    new_allocator->max_free_index = KD_ALLOCATOR_MAX_FREE_UNLIMITED;

    *allocator = new_allocator;

    return KD_SUCCESS;
}
