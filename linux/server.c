/*
    Project includes
*/
#include "../httpd/platform.h"
#include "../httpd/functions.h"

/*
    Lib includes
*/

/*
    C includes
*/
#include <stdio.h>
#include <stdlib.h>

#include <errno.h>
#include <dirent.h>

#include <signal.h>
#include <assert.h>

#include <string.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <unistd.h>

/*
    Defines and Enums
*/

// Prevents the compiler from reordering memory operations
#define __compiler_barrier() __asm__ __volatile__ ("" : : : "memory")

// Full memory barrier (includes compiler barriers for safety)
#define __memory_barrier() { __compiler_barrier(); __sync_synchronize(); __compiler_barrier(); }

/*
    Data structures
*/
typedef struct thread_pool_task_t
{
    struct thread_pool_task_t *next;
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

static inline size_t __atomic_read(volatile size_t *ptr)
{
    return __sync_add_and_fetch(ptr, 0);
}

static inline size_t __atomic_add(volatile size_t *ptr, size_t inc)
{
    return __sync_add_and_fetch(ptr, inc);
}

static inline size_t __atomic_sub(volatile size_t *ptr, size_t inc)
{
    return __sync_sub_and_fetch(ptr, inc);
}

static inline size_t __atomic_inc(volatile size_t *ptr)
{
    return __atomic_add(ptr, 1);
}

static inline size_t __atomic_dec(volatile size_t *ptr)
{
    return __atomic_sub(ptr, 1);
}

static inline size_t __atomic_cmp(volatile size_t *ptr, size_t cmp)
{
    return __atomic_read(ptr) == cmp;
}

///////////////////////////////////////////////////////////////////////////////

void fnc_release_socket(int socketId)
{
    shutdown(socketId, SHUT_RDWR);
    close(socketId);
}

int fnc_read_dir(void *__dir, dirent_t *__data)
{
    struct dirent *dir = readdir(__dir);
    if (dir == NULL) return 0;

    __data->d_name = dir->d_name;
    __data->d_type = dir->d_type;

    return 1;
}

int fnc_errno()
{
    return *__errno_location();
}

///////////////////////////////////////////////////////////////////////////////

static thread_pool_task_t *thread_pool_create_task(int socketId)
{
    thread_pool_task_t *job = (thread_pool_task_t *) malloc(sizeof(thread_pool_task_t));
    assert(job != NULL && "Can not reserve memory!");

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

    __atomic_dec(&pool->numberOfJobsInQueue);
    return job;
}

static void thread_pool_add_task(thread_pool_t *pool, int socketId)
{
    if(pool == NULL || __atomic_cmp(&pool->stop, 1)) { return; }
    thread_pool_task_t *task = thread_pool_create_task(socketId);

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

    __atomic_inc(&pool->numberOfJobsInQueue);
    pthread_cond_broadcast(&pool->condNewTask);
}

static void *thread_pool_worker(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;
    platform_t platform = {};

    platform.networkAPI.send = send;
    platform.networkAPI.recv = recv;
    platform.networkAPI.release = fnc_release_socket;

    platform.dirAPI.getcwd = getcwd;
    platform.dirAPI.readdir = fnc_read_dir;
    platform.dirAPI.opendir = (void * (*)(const char *))opendir;
    platform.dirAPI.closedir = (int (*)(void *))closedir;

    platform.getLastError = fnc_errno;

    while(1)
    {
        pthread_mutex_lock(&pool->mutex);
            while(pool->firstTask == NULL && __atomic_cmp(&pool->stop, 0))
            {
                pthread_cond_wait(&pool->condNewTask, &pool->mutex);
            }

            if(__atomic_cmp(&pool->stop, 1) && __atomic_cmp(&pool->numberOfJobsInQueue, 0))
            {
                pthread_mutex_unlock(&pool->mutex);
                break;
            }

            thread_pool_task_t *job = thread_pool_get_job(pool);
        pthread_mutex_unlock(&pool->mutex);

        if(job != NULL)
        {
            __atomic_inc(&pool->numberOfRunningTasks);
                fnc_process_request(job->socketId, &platform);
                thread_pool_destroy_job(job);
            __atomic_dec(&pool->numberOfRunningTasks);
        }

        if(__atomic_cmp(&pool->numberOfJobsInQueue, 0) && __atomic_cmp(&pool->numberOfRunningTasks, 0))
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
    pthread_mutex_lock(&pool->mutex);
        while(pool->numberOfRunningTasks != 0 || pool->numberOfJobsInQueue != 0)
        {
            pthread_cond_broadcast(&pool->condNewTask);
            pthread_cond_wait(&pool->condAllTaskCompleted, &pool->mutex);
        }
    pthread_mutex_unlock(&pool->mutex);
}

static void thread_pool_destroy(thread_pool_t *pool)
{
    pool->stop = 1;
    thread_pool_wait(pool);

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->condNewTask);
    pthread_cond_destroy(&pool->condAllTaskCompleted);

    free(pool);
}

///////////////////////////////////////////////////////////////////////////////

int main( int argc, char *argv[] )
{
    int socketDescriptor = 0;
    int socketPort       = 8080;

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
        unsigned int socketLength = sizeof(struct sockaddr_in);
        int socketDescriptorClient = accept(socketDescriptor, (struct sockaddr *) &s_client, &socketLength);

        if(socketDescriptorClient != -1)
        {
            thread_pool_add_task(pool, socketDescriptorClient);
        }
    }

    thread_pool_destroy(pool);
    close(socketDescriptor);

    return EXIT_SUCCESS;
}
