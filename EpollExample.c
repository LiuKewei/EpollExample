#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

#include<sys/types.h>
#include<sys/stat.h>

#include "socktasks.h"
#include "threadpool.h"

#define SERV_PORT             38250       // 服务器端口
#define LISTENQ               128         // listen sock 参数
#define MAX_EPOLL_EVENT_COUNT 500         // 同时监听的 epoll 数
#define NETWORK_READ_THREAD_COUNT 350
#define SOCKET_ERROR -1



static int epfd;
static int listenfd;

static pthread_mutex_t mutex_thread_read;
static pthread_cond_t cond_thread_read;
static pthread_mutex_t mutex_thread_write;
static pthread_cond_t cond_thread_write;

static sock_content_t_list network_read_tasks = NULL;
static sock_content_t_list network_write_tasks = NULL;

static enum shutdown thread_destroy_way = FORCE_STOP;





//////////////////////////////////////////////////////////////////////////////////////////
void setnonblocking(int sock)
{
    int opts;
    opts = fcntl(sock, F_GETFL);
    if(opts < 0)
    {
        printf("fcntl(sock,GETFL)");
        exit(1);
    }

    opts = opts | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, opts)<0)
    {
        printf("fcntl(sock,SETFL,opts)");
        exit(1);
    }
}

void del_from_network(sock_content_t_list sc_content)
{
    assert(sc_content!=NULL);
    int fd = sc_content->data.fd;
    if (fd != -1)
    {
        struct epoll_event ev;
        ev.data.fd = fd;
        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);

        close(fd);
        sc_content->data.fd = -1;
        free(sc_content);
        sc_content = NULL;
    }
}

void * thread_read_task(void *args)
{
    sock_content_t_list read_sock_content = NULL;
    struct epoll_event ev;
    while(true)
    {
        pthread_mutex_lock(&mutex_thread_read);
        while(network_read_tasks->next == NULL)
        {
            pthread_testcancel();
            pthread_cond_wait(&cond_thread_read,&mutex_thread_read);
            pthread_testcancel();
        }
        read_sock_content = task_list_pop(network_read_tasks);
        assert(read_sock_content!=NULL);
        assert(read_sock_content->next==NULL);
        pthread_mutex_unlock(&mutex_thread_read);

        if(read_sock_content->data.fd != -2 && strcmp(read_sock_content->data.io_buf, "HEADER") != 0)
        {
            int read_count = 0;
            int total_read = 0;

            pthread_testcancel();
            bzero(read_sock_content->data.io_buf, MAX_IO_BUF_SIZE);
            read_count = recv(read_sock_content->data.fd, read_sock_content->data.io_buf + total_read, MAX_IO_BUF_SIZE-1, 0);
            pthread_testcancel();
            if(read_count > 0)
            {
                total_read += read_count;
                do
                {
                    read_count = read(read_sock_content->data.fd, read_sock_content->data.io_buf + total_read, MAX_IO_BUF_SIZE-1);
                    if (read_count <= 0)
                    {
                        if (errno == EINTR)
                            continue;
                        if (errno == EAGAIN)
                        {
                            //printf("total_read : %d\n", total_read);
                            printf("%s\n", read_sock_content->data.io_buf);
                            ev.data.ptr = read_sock_content;
                            ev.events = EPOLLOUT | EPOLLET;
                            epoll_ctl(epfd, EPOLL_CTL_MOD, read_sock_content->data.fd, &ev);
                            break;
                        }
                        printf("2 error read_count < 0, errno[%d]", errno);

                        del_from_network(read_sock_content);
                        read_sock_content = NULL;
                        break;
                    }
                    total_read += read_count;
                }
                while(1);
            }
            else if(read_count < 0)
            {
                if(errno == EAGAIN)
                {
                    continue;
                }
                del_from_network(read_sock_content);
                read_sock_content = NULL;
                printf("read_count < 0        del_from_network(read_sock_content);\n");
                continue;
            }
            else if (read_count == 0)
            {
                del_from_network(read_sock_content);
                read_sock_content = NULL;
                printf("read_count == 0        del_from_network(read_sock_content);\n");
                continue;
            }
        }
    }
}


void * thread_write_task(void *args)
{
    sock_content_t_list write_sock_content = NULL;
    struct epoll_event ev;
    while(true)
    {
        pthread_mutex_lock(&mutex_thread_write);
        while(network_write_tasks->next == NULL)
        {
            pthread_testcancel();
            pthread_cond_wait(&cond_thread_write,&mutex_thread_write);
            pthread_testcancel();
        }
        write_sock_content = task_list_pop(network_write_tasks);
        assert(write_sock_content!=NULL);
        assert(write_sock_content->next==NULL);
        pthread_mutex_unlock(&mutex_thread_write);

        if(write_sock_content->data.fd != -2 && strcmp(write_sock_content->data.io_buf, "HEADER") != 0)
        {
            int write_counts = 0;
            int total_write = 0;
            int data_size = strlen(write_sock_content->data.io_buf);
            total_write = data_size;
            while (total_write > 0)
            {
                write_counts = write(write_sock_content->data.fd, write_sock_content->data.io_buf + data_size - total_write, total_write);
                if (write_counts < total_write)
                {
                    if (write_counts == -1 && errno != EAGAIN)
                    {
                        printf("write error\n");
                    }
                    break;
                }
                total_write -= write_counts;
            }
            ev.events = EPOLLIN | EPOLLET;
            ev.data.ptr = write_sock_content;
            epoll_ctl(epfd, EPOLL_CTL_MOD, write_sock_content->data.fd, &ev);
        }
    }
}



int main()
{
    bool b_func_ret;
    int i_func_ret;
    /** thread jobs structure **/
    network_read_tasks = task_list_init();
    assert(network_read_tasks != NULL);
    network_write_tasks = task_list_init();
    assert(network_write_tasks != NULL);
    b_func_ret = tpool_init(NETWORK_READ_THREAD_COUNT);
    assert(b_func_ret == true);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);//
    pthread_mutex_init(&mutex_thread_read, &attr);
    pthread_cond_init(&cond_thread_read, NULL);
    pthread_mutex_init(&mutex_thread_write, &attr);
    pthread_cond_init(&cond_thread_write, NULL);

    int job_num = 0;
    for(; job_num < (NETWORK_READ_THREAD_COUNT/2); ++job_num)
    {
        tpool_add_job(thread_read_task, &job_num);
    }
    job_num = 0;
    for(; job_num < (NETWORK_READ_THREAD_COUNT/2); ++job_num)
    {
        tpool_add_job(thread_write_task, &job_num);
    }

    sock_content_t_list sc_clients = NULL;

    /** epoll structure **/
    int opt_val = 0;
    int nfds = 0;
    int connfd = 0;
#ifdef _IPV6_
    struct sockaddr_in6 client_addr6 = {0};
    socklen_t client_len = sizeof(client_addr6);
    struct sockaddr_in6 server_addr6;

    listenfd = socket(AF_INET6, SOCK_STREAM, 0);
#else
    struct sockaddr_in clientaddr = {0};
    socklen_t clilen = sizeof(clientaddr);
    struct sockaddr_in serveraddr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
#endif
    struct epoll_event reg_ev, cur_ev, dealback_evs[MAX_EPOLL_EVENT_COUNT];
    epfd = epoll_create(MAX_EPOLL_EVENT_COUNT);

    assert(listenfd != -1);
    setnonblocking(listenfd);

    reg_ev.data.fd = listenfd;
    reg_ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &reg_ev);
    opt_val = 0;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));//不允许 套接字 与已经被使用的ip地址绑定

#ifdef _IPV6_
    bzero(&server_addr6, sizeof(server_addr6));
    server_addr6.sin6_family = AF_INET6;
    server_addr6.sin6_addr = in6addr_any;
    server_addr6.sin6_port = htons(SERV_PORT);

    i_func_ret = bind(listenfd, (struct sockaddr *)&server_addr6, sizeof(server_addr6));
    assert(i_func_ret != -1);
#else
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    i_func_ret = bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    assert(i_func_ret != -1);
#endif

    i_func_ret = listen(listenfd, LISTENQ);
    assert(i_func_ret != -1);

    struct epoll_event ev;
    int sockfd = 0;
    while(1)
    {
        nfds = epoll_wait(epfd, dealback_evs, MAX_EPOLL_EVENT_COUNT, -1);
        int i = 0;
        for(; i< nfds; i++)
        {
            cur_ev = dealback_evs[i];
            if(cur_ev.data.fd == listenfd)
            {
#ifdef _IPV6_
                while ( (connfd = accept(listenfd, (struct sockaddr *)&client_addr6, &client_len)) > 0 )
#else
                while ( (connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &clilen)) > 0 )
#endif
                {
                    if(connfd == -1)
                    {
                        printf("accept error: %d\n", errno);
                        continue;
                    }
                    setnonblocking(connfd);
                    int nRecvBuf=128*1024;//设置为32K
                    setsockopt(connfd, SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int));
                    //发送缓冲区
                    int nSendBuf=128*1024;//设置为32K
                    setsockopt(connfd, SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int));

                    int nNetTimeout=1000;//1秒
                    //发送时限
                    setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&nNetTimeout, sizeof(int));
                    //接收时限
                    setsockopt(connfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&nNetTimeout, sizeof(int));

#ifdef _IPV6_
                    char client_ip6[100];
                    const char * remote_addr = inet_ntop(AF_INET6, &client_addr6.sin6_addr, client_ip6, 100);
                    int remote_port = ntohs(client_addr6.sin6_port);

                    printf("connect from : %s, %d\n", remote_addr, remote_port);
                    printf("connect from : %s\n", client_ip6);
#else
                    const char *remote_addr = inet_ntoa(clientaddr.sin_addr);
                    int  remote_port = ntohs(clientaddr.sin_port);
                    printf("connect from : %s, %d\n", remote_addr, remote_port);
#endif
                    sc_clients = (sock_content_t_list)malloc(sizeof(sock_content_t));
                    assert(sc_clients != NULL);
                    bzero(sc_clients->data.io_buf, sizeof(sc_clients->data.io_buf));
                    sc_clients->data.fd = connfd;

                    ev.data.ptr = sc_clients;
                    ev.events = EPOLLIN | EPOLLET;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);
                }
            }
            else if (cur_ev.events & EPOLLIN)
            {
                sock_content_t_list in_sc_data = (sock_content_t_list)cur_ev.data.ptr;
                sockfd = in_sc_data->data.fd;
                if( sockfd < 0 )
                {
                    break;
                }
                pthread_mutex_lock(&mutex_thread_read);
                task_list_appand(network_read_tasks, in_sc_data);
                //pthread_cond_signal(&cond_thread_read);
                pthread_cond_broadcast(&cond_thread_read);
                pthread_mutex_unlock(&mutex_thread_read);
            }
            else if (cur_ev.events & EPOLLOUT)
            {
                sock_content_t_list out_sc_data = (sock_content_t_list)cur_ev.data.ptr;
                //sockfd = out_sc_data->data.fd;

                pthread_mutex_lock(&mutex_thread_write);
                task_list_appand(network_write_tasks, out_sc_data);
                pthread_cond_broadcast(&cond_thread_write);
                pthread_mutex_unlock(&mutex_thread_write);
/*
                int write_counts = 0;
                int total_write = 0;
                int data_size = strlen(out_sc_data->data.io_buf);
                total_write = data_size;
                while (total_write > 0)
                {
                    write_counts = write(sockfd, out_sc_data->data.io_buf + data_size - total_write, total_write);
                    if (write_counts < total_write)
                    {
                        if (write_counts == -1 && errno != EAGAIN)
                        {
                            printf("write error");
                        }
                        break;
                    }
                    total_write -= write_counts;
                }

                ev.events = EPOLLIN | EPOLLET;
                ev.data.ptr = out_sc_data;
                epoll_ctl(epfd, EPOLL_CTL_MOD, sockfd, &ev);
*/
            }
            else if (cur_ev.events & EPOLLHUP)
            {
                printf("else if(cur_ev.events & EPOLLHUP)");
                thread_destroy_way = FORCE_STOP;
                task_list_destroy(network_read_tasks);
                task_list_destroy(network_write_tasks);
                tpool_destroy(thread_destroy_way);
                //del_from_network(context);
            }
            else
            {
                printf("[warming]other epoll event: %d", cur_ev.events);
                break;
            }
        }
    }
    thread_destroy_way = FORCE_STOP;
    task_list_destroy(network_read_tasks);
    task_list_destroy(network_write_tasks);
    tpool_destroy(thread_destroy_way);

    return 1;
}
