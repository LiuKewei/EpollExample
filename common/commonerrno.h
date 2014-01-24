/*
 * commonerrno.h
 *
 *  Created on: 2014-1-24
 *      Author: kedou
 */

#ifndef COMMONERRNO_H_
#define COMMONERRNO_H_

#define KD_OS_ERRSPACE_SIZE 50000
#define KD_SUCCESS 0
#define KD_OS_START_ERROR 20000
#define KD_OS_START_STATUS (KD_OS_START_ERROR + KD_OS_ERRSPACE_SIZE)
#define KD_OS_START_USERERR (KD_OS_START_STATUS + KD_OS_ERRSPACE_SIZE)
#define KD_OS_START_CANONERR (KD_OS_START_USERERR + (KD_OS_ERRSPACE_SIZE * 10))
#define KD_OS_START_EAIERR (KD_OS_START_CANONERR + KD_OS_ERRSPACE_SIZE)
#define KD_OS_START_SYSERR (KD_OS_START_EAIERR + KD_OS_ERRSPACE_SIZE)

typedef int kd_status_t;

#define KD_ENOMEM (KD_OS_START_CANONERR + 7)


#endif /* COMMONERRNO_H_ */
