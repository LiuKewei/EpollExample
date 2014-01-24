#ifndef IOTASKS_H_INCLUDED
#define IOTASKS_H_INCLUDED

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/epoll.h>
#include <assert.h>

#define MAX_IO_BUF_SIZE       (1024 * 8)  // 最大读取数据 buf

typedef struct sock_data
{
    int       fd;
    char      io_buf[MAX_IO_BUF_SIZE];
} sock_data_t, *p_sock_data_t;

typedef struct sock_content
{
    sock_data_t            data;
    struct sock_content * next;
} sock_content_t, *sock_content_t_list;


// return the length of list, after init,
extern sock_content_t_list task_list_init();

// return the length of list, after appand,
extern int32_t task_list_appand(sock_content_t_list sc_lst, sock_content_t_list sc_data);

// return the length of list, after pop,
extern sock_content_t_list task_list_pop(sock_content_t_list sc_lst);

extern bool task_list_destroy(sock_content_t_list sc_lst);

#endif // IOTASKS_H_INCLUDED
