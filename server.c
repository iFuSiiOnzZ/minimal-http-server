/*
    Project includes
*/
#include "functions.h"

/*
    Lib includes
*/

/*
    C includes
*/
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <assert.h>

#include <string.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <unistd.h>

/*
    Data structures
*/
typedef void (*http_process_fnc_t)(int arg);

typedef struct thread_pool_task_t
{
    struct thread_pool_task_t *next;
    http_process_fnc_t httpCallBack;
    int socketId;
} thread_pool_task_t;

typedef struct thread_pool_t
{
    thread_pool_task_t *firstTask, *lastTask;
    pthread_cond_t condNewTask, condAllTaskCompleted;
    pthread_mutex_t mutex;

    volatile size_t numberOfJobsInQueue, numberOfRunningTasks;
    volatile size_t numberOfThreads, stop;
} thread_pool_t;

///////////////////////////////////////////////////////////////////////////////

static inline size_t atomic_read(volatile size_t *ptr)
{
    return __sync_add_and_fetch(ptr, 0);
}

static thread_pool_task_t *thread_pool_create_task(http_process_fnc_t httpCallBack, int socketId)
{
    thread_pool_task_t *job = (thread_pool_task_t *) malloc(sizeof(thread_pool_task_t));
    assert(job != NULL && "Can not reserve memory!");

    job->httpCallBack = httpCallBack;
    job->socketId = socketId;
    job->next = NULL;

    return job;
}

static void thread_pool_destroy_job(thread_pool_task_t *job)
{
    if(job != NULL)
    {
        free(job);
    }
}

static thread_pool_task_t *thread_pool_get_job(thread_pool_t *pool)
{
    if(pool == NULL || pool->firstTask == NULL)
    {
        return NULL;
    }

    thread_pool_task_t *job = pool->firstTask;
    pool->firstTask = pool->firstTask->next;

    if(pool->firstTask == NULL)
    {
        pool->lastTask = NULL;
    }

    __sync_sub_and_fetch(&pool->numberOfJobsInQueue, 1);
    return job;
}

static void thread_pool_add_task(thread_pool_t *pool, http_process_fnc_t callBack, int socketId)
{
    if(pool == NULL || callBack == NULL || atomic_read(&pool->stop) == 1) { return; }
    thread_pool_task_t *task = thread_pool_create_task(callBack, socketId);

    pthread_mutex_lock(&pool->mutex);
        if(pool->firstTask == NULL)
        {
            pool->firstTask = task;
            pool->lastTask = pool->firstTask;
        }
        else
        {
            pool->lastTask->next = task;
            pool->lastTask = task;
        }
    pthread_mutex_unlock(&pool->mutex);

    __sync_add_and_fetch(&pool->numberOfJobsInQueue, 1);
    pthread_cond_broadcast(&pool->condNewTask);
}

static void *thread_pool_worker(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;

    while(1)
    {
        pthread_mutex_lock(&pool->mutex);
            while(pool->firstTask == NULL && atomic_read(&pool->stop) == 0)
            {
                pthread_cond_wait(&pool->condNewTask, &pool->mutex);
            }

            if(atomic_read(&pool->stop) == 1 && atomic_read(&pool->numberOfJobsInQueue) == 0)
            {
                pthread_mutex_unlock(&pool->mutex);
                break;
            }

            thread_pool_task_t *job = thread_pool_get_job(pool);
        pthread_mutex_unlock(&pool->mutex);

        if(job != NULL)
        {
            __sync_add_and_fetch(&pool->numberOfRunningTasks, 1);
                job->httpCallBack(job->socketId);
                thread_pool_destroy_job(job);
            __sync_sub_and_fetch(&pool->numberOfRunningTasks, 1);
        }

        if(atomic_read(&pool->numberOfJobsInQueue) == 0 && atomic_read(&pool->numberOfRunningTasks) == 0)
        {
            pthread_cond_signal(&pool->condAllTaskCompleted);
        }
    }

    return NULL;
}

static thread_pool_t *thread_pool_create(size_t numberOfThreads)
{
    if(numberOfThreads == 0)
    {
        numberOfThreads = 1;
    }

    thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    memset(pool, 0, sizeof(thread_pool_t));

    pthread_cond_init(&pool->condNewTask, NULL);
    pthread_mutex_init(&pool->mutex, NULL);

    pthread_t thread = 0;
    for(size_t i = 0; i < numberOfThreads; ++i)
    {
        pthread_create(&thread, NULL, thread_pool_worker, pool);
        pthread_detach(thread);
    }

    pool->numberOfThreads = numberOfThreads;
    return pool;
}

static void thread_pool_wait(thread_pool_t *pool)
{
    if(pool == NULL)
    {
        return;
    }

    pthread_mutex_lock(&pool->mutex);
        while(atomic_read(&pool->numberOfRunningTasks) != 0 || atomic_read(&pool->numberOfJobsInQueue) != 0)
        {
            pthread_cond_broadcast(&pool->condNewTask);
            pthread_cond_wait(&pool->condAllTaskCompleted, &pool->mutex);
        }
    pthread_mutex_unlock(&pool->mutex);
}

static void thread_pool_destroy(thread_pool_t *pool)
{
    if(pool == NULL)
    {
        return;
    }

    __sync_lock_test_and_set(&pool->stop, 1);
    thread_pool_wait(pool);

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->condNewTask);
    pthread_cond_destroy(&pool->condAllTaskCompleted);

    free(pool);
}

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] )
{
    int socketDescriptorClient  = 0;
    int socketDescriptor        = 0;
    int socketLength            = 0;
    int socketPort              = 8080;

    struct sockaddr_in  s_server = { 0 };
    struct sockaddr_in  s_client = { 0 };

    if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf("Error creating the socket\n");
        return EXIT_FAILURE;
    }

    s_server.sin_family         = AF_INET;
    s_server.sin_addr.s_addr    = inet_addr("127.0.0.1");
    s_server.sin_port           = htons(socketPort);

    if(bind(socketDescriptor, (struct sockaddr *) &s_server, (socklen_t) sizeof(struct sockaddr_in)) == -1)
    {
        printf("Error at associate port and socket!\n");
        close(socketDescriptor);
        return EXIT_FAILURE;
    }

    if(listen(socketDescriptor, 5) == -1)
    {
        printf("Error at listen!\n");
        close(socketDescriptor);
        return EXIT_FAILURE;
    }

    size_t numberOfThreads = sysconf(_SC_NPROCESSORS_ONLN) * 2;
    thread_pool_t *pool = thread_pool_create(numberOfThreads);

    // https://blog.erratasec.com/2018/10/tcpip-sockets-and-sigpipe.html
    signal(SIGPIPE, SIG_IGN);

    while(1)
    {
        socketLength = sizeof(struct sockaddr_in);
        if((socketDescriptorClient = accept(socketDescriptor, (struct sockaddr *) &s_client, &socketLength)) != -1)
        {
            thread_pool_add_task(pool, (void *) &fnc_process_request, socketDescriptorClient);
        }
    }

    thread_pool_destroy(pool);
    close(socketDescriptor);

    return EXIT_SUCCESS;
}
