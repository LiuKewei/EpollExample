#include "iotasks.h"

// return the length of list, after init,
sock_content_t_list task_list_init()
{
    sock_content_t_list init_scl = NULL;
    init_scl = (sock_content_t_list)malloc(sizeof(sock_content_t));
    if(init_scl == NULL)
    {
        printf("'task_list_init' func initialize error!\n");
        return NULL;
    }
    init_scl->data.fd = -2;
    //init_scl->data.io_buf = {0,}; raise an error
    memset(init_scl->data.io_buf, 0, MAX_IO_BUF_SIZE);
    snprintf(init_scl->data.io_buf, MAX_IO_BUF_SIZE, "%s", "HEADER");
    init_scl->next = NULL;
    return init_scl;
}

// return the length of list, after appand,
int32_t task_list_appand(sock_content_t_list sc_lst, sock_content_t_list sc_data)
{
    assert(sc_data != NULL);
    if (sc_lst == NULL)
    {
        printf("The param type sock_content_t_list be sent to 'task_list_appand' func is NULL!\n");
        return -1;
    }
    int32_t scl_len = 0;//list contains header,
    sock_content_t_list tmp_scl;
    tmp_scl = sc_lst;
    while( tmp_scl->next != NULL )
    {
        ++scl_len;
        tmp_scl = tmp_scl->next;
    }
    tmp_scl->next = sc_data;
    //printf("sc_lst->next->data.fd : %d\n", sc_lst->next->data.fd);
    scl_len += 1;
    return scl_len;
}

// return the length of list, after pop,
sock_content_t_list task_list_pop(sock_content_t_list sc_lst)
{
    sock_content_t_list sc_pop = NULL;
    if (sc_lst == NULL)
    {
        printf("The param type sock_content_t_list be sent to 'task_list_pop' func is NULL!\n");
        return false;
    }
    sock_content_t_list tmp_scl;
    tmp_scl = sc_lst;
    if(tmp_scl->next != NULL)
    {
        sc_pop = tmp_scl->next;
        sc_lst->next = sc_lst->next->next;
        sc_pop->next = NULL;
    }
    else
    {
        sc_pop = tmp_scl;
        printf("The param type sock_content_t_list be sent to 'task_list_pop' func has no task element to pop-up !  Only pop-up the HEADER !\n");
    }
    return sc_pop;
}

bool task_list_destroy(sock_content_t_list sc_lst)
{
    if(sc_lst == NULL)
    {
        printf("The param type sock_content_t_list be sent to 'task_list_destroy' func no need to destroy!\n");
        return true;
    }
    sock_content_t_list tmp_scl;
    tmp_scl = sc_lst;
    while(tmp_scl->next != NULL)
    {
        sc_lst->next = sc_lst->next->next;
        free(tmp_scl->next);
        tmp_scl = sc_lst;
    }
    free(sc_lst);
    sc_lst = NULL;
    return true;
}






