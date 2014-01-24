/*
 * sysio.h
 *
 *  Created on: 2014-1-24
 *      Author: kedou
 */

#ifndef SYSIO_H_
#define SYSIO_H_

#include <stdint.h>

struct kd_file_t {
	kd_pool_t *pool;
	int filedes;
	char *fname;
	int32_t flags;
	int eof_hit;
	int is_pipe;
//apr_interval_time_t timeout;
	int buffered;
	enum {
		BLK_UNKNOWN, BLK_OFF, BLK_ON
	} blocking;
	int ungetchar; /* Last char provided by an unget op. (-1 = no char)*/
#ifndef WAITIO_USES_POLL
	/* if there is a timeout set, then this pollset is used */
//apr_pollset_t *pollset;
#endif
	/* Stuff for buffered mode */
	char *buffer;
	int bufpos; /* Read/Write position in buffer */
	unsigned long dataRead; /* amount of valid data read into buffer */
	int direction; /* buffer being used for 0 = read, 1 = write */
	unsigned long filePtr; /* position in file of handle */
#if KD_HAS_THREADS
struct kd_thread_mutex_t *thlock;
#endif
};

#endif /* SYSIO_H_ */
