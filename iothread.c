#include "iothread.h"

static tpool_t* init_tpool = NULL;
static int thread_count = 0;

static void* thread_runfunc(void* arg)
{
    tpool_job_t * job;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    int id = ++thread_count;
    while(true)
    {
        pthread_mutex_lock(&init_tpool->queue_lock);
        //printf("thread_runfunc start %d\n", id );

        while(init_tpool->queue_head==NULL && init_tpool->is_shutdown == 0)
        {
            //printf("thread_runfunc waiting %d\n", id );
            pthread_cond_wait(&init_tpool->queue_ready, &init_tpool->queue_lock);
        }
        if(init_tpool->is_shutdown==WAIT_WORKING_TASKS)
        {
            printf("pthread_exit(NULL);\n");
            pthread_mutex_unlock(&init_tpool->queue_lock);
            pthread_exit(NULL);
        }
        else if(init_tpool->is_shutdown==WAIT_ALL_TASKS && init_tpool->current_job_num == 0)
        {
            printf("pthread_exit(NULL);\n");
            pthread_mutex_unlock(&init_tpool->queue_lock);
            pthread_exit(NULL);
        }
        job = init_tpool->queue_head;
        init_tpool->queue_head = job->next;
        init_tpool->current_job_num -= 1;
        pthread_mutex_unlock(&init_tpool->queue_lock);

        job->runfunc(job->arg);
        free(job);

    }
    printf("thread_runfunc exit\n");
    return NULL;
}

bool tpool_init(int max_thr_num)
{
    int i_func_ret;
    init_tpool = (tpool_t*)malloc(sizeof(tpool_t));
    if(init_tpool == NULL)
    {
        printf("init_tpool malloc failed!\n");
        return false;
    }
    init_tpool->max_thr_num = max_thr_num;
    init_tpool->is_shutdown = ACTIVED;
    init_tpool->current_job_num = 0;
    init_tpool->queue_head = NULL;

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);//

    if(pthread_mutex_init(&init_tpool->queue_lock, &attr) != 0)
    {
        printf("pthread_mutex_init failed!\n");
        return false;
    }

    if(pthread_cond_init(&init_tpool->queue_ready, NULL) != 0)
    {
        printf("pthread_cond_init failed!\n");
        return false;
    }

    pthread_attr_t tattr;
    i_func_ret = pthread_attr_init(&tattr);
    assert(i_func_ret == 0);
    i_func_ret = pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
    assert(i_func_ret == 0);

    init_tpool->thr_id = (pthread_t *)malloc(max_thr_num*sizeof(pthread_t));

    if(init_tpool->thr_id == NULL)
    {
        printf("The %d number of thr_id in struct tpool malloc failed!\n", max_thr_num);
        return false;
    }
    int per_thread = 0;
    for(; per_thread< max_thr_num; ++per_thread)
    {
        if( pthread_create(&init_tpool->thr_id[per_thread], &tattr, thread_runfunc, NULL ) != 0 )
        {
            printf("The NO.%d thread create failed!\n", per_thread+1);
            return false;
        }
    }
    return true;
}

bool tpool_destroy(enum shutdown shutdown_way)
{
    tpool_job_t * member;
    if(init_tpool->is_shutdown)
    {
        return true;
    }
    init_tpool->is_shutdown = shutdown_way;

    pthread_mutex_lock(&init_tpool->queue_lock);
    pthread_cond_broadcast(&init_tpool->queue_ready);
    pthread_mutex_unlock(&init_tpool->queue_lock);
    int per_thread = 0;
    if(shutdown_way == FORCE_STOP)
    {
        for(; per_thread < init_tpool->max_thr_num; ++per_thread)
        {
            pthread_cancel(init_tpool->thr_id[per_thread]);
        }
        per_thread = 0;
    }

    for(; per_thread < init_tpool->max_thr_num; ++per_thread)
    {
        pthread_join(init_tpool->thr_id[per_thread], NULL);
    }
    free(init_tpool->thr_id);

    while(init_tpool->queue_head)
    {
        member = init_tpool->queue_head;
        init_tpool->queue_head = member->next;
        free(member);
    }
    pthread_mutex_destroy(&init_tpool->queue_lock);
    pthread_cond_destroy(&init_tpool->queue_ready);

    free(init_tpool);

    return true;
}

int tpool_add_job(void* (*runfunc)(void*), void* arg)
{
    tpool_job_t *job, *member;
    if(runfunc == NULL)
    {
        printf("Invalid argument!\n");
        return -1;
    }
    job = (tpool_job_t *)malloc(sizeof(tpool_job_t));
    if(job == NULL)
    {
        printf("Job malloc failed!\n");
        return -1;
    }
    job->runfunc = runfunc;
    job->arg = arg;
    job->next = NULL;

    pthread_mutex_lock(&init_tpool->queue_lock);
    member = init_tpool->queue_head;
    if(member == NULL)
    {
        init_tpool->queue_head = job;
    }
    else
    {
        while(member->next)
        {
            member = member->next;
        }
        member->next = job;
    }
    init_tpool->current_job_num += 1;
    pthread_cond_signal(&init_tpool->queue_ready);
    pthread_mutex_unlock(&init_tpool->queue_lock);

    return 0;
}
