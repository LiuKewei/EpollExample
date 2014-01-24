/*
 * sysproc.h
 *
 *  Created on: 2014-1-24
 *      Author: kedou
 */

#ifndef SYSPROC_H_
#define SYSPROC_H_

#include <sys/types.h>

typedef enum {
	KD_KILL_NEVER, /**< process is never sent any signals */
	KD_KILL_ALWAYS, /**< process is sent SIGKILL on KD_pool_t cleanup*/
	KD_KILL_AFTER_TIMEOUT, /**< SIGTERM, wait 3 seconds, SIGKILL */
	KD_JUST_WAIT, /**< wait forever for the process to complete */
	KD_KILL_ONLY_ONCE /**< send SIGTERM and then wait */
} kd_kill_conditions_e;

typedef struct kd_proc_t {
	pid_t pid;
	kd_file_t *in;
	kd_file_t *out;
	kd_file_t *err;
} kd_proc_t;

/** A list of processes */
struct process_chain {
	/** The process ID */
	kd_proc_t *proc;
	kd_kill_conditions_e kill_how;
	/** The next process in the list */
	struct process_chain *next;
};

#endif /* SYSPROC_H_ */
