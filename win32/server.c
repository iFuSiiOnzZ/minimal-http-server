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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <direct.h>
#include "dirent.h"

/*
    Defines and Enums
*/
#pragma comment(lib,"Ws2_32.lib")

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
    CONDITION_VARIABLE condNewTask, condAllTaskCompleted;
    CRITICAL_SECTION mutex;

    volatile long numberOfJobsInQueue, numberOfRunningTasks;
    volatile long numberOfThreads, stop;
} thread_pool_t;

///////////////////////////////////////////////////////////////////////////////

static inline size_t __atomic_read(volatile long *ptr)
{
    return InterlockedAdd(ptr, 0);
}

static inline size_t __atomic_add(volatile long *ptr, long inc)
{
    return InterlockedAdd(ptr, inc);
}

static inline size_t __atomic_sub(volatile long *ptr, long inc)
{
    return InterlockedAdd(ptr, -inc);
}

static inline size_t __atomic_inc(volatile long *ptr)
{
    return InterlockedIncrement(ptr);
}

static inline size_t __atomic_dec(volatile long *ptr)
{
    return InterlockedDecrement(ptr);
}

static inline size_t __atomic_cmp(volatile long *ptr, long cmp)
{
    return __atomic_read(ptr) == cmp;
}

///////////////////////////////////////////////////////////////////////////////

void fnc_release_socket(SOCKET socketId)
{
    shutdown(socketId, SD_BOTH);
    closesocket(socketId);
}

int fnc_read_dir(void *__dir, dirent_t *__data)
{
    struct dirent *dir = readdir(__dir);
    if (dir == NULL) return 0;

    __data->d_name = dir->d_name;
    __data->d_type = dir->d_type;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

static thread_pool_task_t *thread_pool_create_task(int socketId)
{
    thread_pool_task_t *task = (thread_pool_task_t *)malloc(sizeof(thread_pool_task_t));
    assert(task != NULL && "Can not reserve memory for the task");

    task->next = NULL;
    task->socketId = socketId;

    return task;
}

static void thread_pool_destroy_task(thread_pool_task_t *task)
{
    if(task)
    {
        free(task);
    }
}

static thread_pool_task_t *thread_pool_get_task(thread_pool_t *pool)
{
    if(pool == NULL || pool->firstTask == NULL)
    {
        return NULL;
    }

    EnterCriticalSection(&pool->mutex);
        thread_pool_task_t *task = pool->firstTask;
        pool->firstTask = task->next;

        if(pool->firstTask == NULL)
        {
            pool->lastTask = NULL;
        }
    LeaveCriticalSection(&pool->mutex);

    __atomic_dec(&pool->numberOfJobsInQueue);
    return task;
}

static void thread_pool_add_task(thread_pool_t *pool, int socketId)
{
    if(pool == NULL || __atomic_cmp(&pool->stop, 1)) { return; }
    thread_pool_task_t *task = thread_pool_create_task(socketId);

    EnterCriticalSection(&pool->mutex);
        if(pool->firstTask == NULL)
        {
            pool->firstTask = task;
            pool->lastTask = task;
        }
        else
        {
            pool->lastTask->next = task;
            pool->lastTask = task;
        }
    LeaveCriticalSection(&pool->mutex);

    __atomic_inc(&pool->numberOfJobsInQueue);
    WakeAllConditionVariable(&pool->condNewTask);
}

static DWORD thread_pool_worker(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;
    platform_t platform = { 0 };

    platform.networkAPI.send = (ssize_t(*)(int, const void *, size_t, int))send;
    platform.networkAPI.recv = (ssize_t(*)(int, void *, size_t, int))recv;
    platform.networkAPI.release = (void(*)(int))fnc_release_socket;

    platform.dirAPI.getcwd = (char*(*)(char *, size_t))_getcwd;
    platform.dirAPI.readdir = fnc_read_dir;
    platform.dirAPI.opendir = (void * (*)(const char *))opendir;
    platform.dirAPI.closedir = (int (*)(void *))closedir;

    platform.getLastError = WSAGetLastError;

    while(1)
    {
        EnterCriticalSection(&pool->mutex);
            if(__atomic_cmp(&pool->stop, 0) && pool->firstTask == NULL)
            {
                SleepConditionVariableCS(&pool->condNewTask, &pool->mutex, INFINITE);
            }

            if(__atomic_cmp(&pool->stop, 1) && __atomic_cmp(&pool->numberOfJobsInQueue, 0))
            {
                LeaveCriticalSection(&pool->mutex);
                break;
            }

            thread_pool_task_t *task = thread_pool_get_task(pool);
        LeaveCriticalSection(&pool->mutex);

        if(task != NULL)
        {
            __atomic_inc(&pool->numberOfRunningTasks);
                fnc_process_request(task->socketId, &platform);
                thread_pool_destroy_task(task);
            __atomic_dec(&pool->numberOfRunningTasks);
        }

        if(__atomic_cmp(&pool->numberOfJobsInQueue, 0) && __atomic_cmp(&pool->numberOfRunningTasks, 0))
        {
            WakeAllConditionVariable(&pool->condAllTaskCompleted);
        }
    }

    return 0;
}

static thread_pool_t *thread_pool_create(long numberOfThreads)
{
    if(numberOfThreads == 0)
    {
        numberOfThreads = 1;
    }

    thread_pool_t *pool = (thread_pool_t *)malloc(sizeof(thread_pool_t));
    assert(pool != NULL && "Can not reserve memory for the pool");
    ZeroMemory(pool, sizeof(thread_pool_t));

    InitializeCriticalSection(&pool->mutex);
    InitializeConditionVariable(&pool->condNewTask);
    InitializeConditionVariable(&pool->condAllTaskCompleted);

    for(long i = 0; i < numberOfThreads; ++i)
    {
        HANDLE hThread = CreateThread(NULL, 0, thread_pool_worker, pool, 0, NULL);
        CloseHandle(hThread);
    }

    pool->numberOfThreads = numberOfThreads;
    return pool;
}

static void thread_pool_wait(thread_pool_t *pool)
{
    EnterCriticalSection(&pool->mutex);
        while (pool->numberOfRunningTasks != 0 || pool->numberOfJobsInQueue != 0)
        {
            WakeAllConditionVariable(&pool->condNewTask);
            SleepConditionVariableCS(&pool->condAllTaskCompleted, &pool->mutex, INFINITE);
        }
    LeaveCriticalSection(&pool->mutex);
}

static void thread_pool_destroy(thread_pool_t *pool)
{
    InterlockedExchange(&pool->stop, 1);
    thread_pool_wait(pool);

    DeleteCriticalSection(&pool->mutex);
    free(pool);
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    (void)argc, (void)argv;
    WSADATA wsaData = { 0 };
    int err = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (err != 0)
    {
        printf("WSAStartup failed with error: %d\n", err);
        return EXIT_FAILURE;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == INVALID_SOCKET)
    {
        printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());
        WSACleanup(); return EXIT_FAILURE;
    }

    struct sockaddr_in local = { 0 };
    local.sin_family        = AF_INET;
    local.sin_addr.s_addr   = INADDR_ANY;
    local.sin_port          = htons(8080);

    if (bind(serverSocket, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);

        WSACleanup();
        return EXIT_FAILURE;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);

        WSACleanup();
        return EXIT_FAILURE;
    }

    SYSTEM_INFO systemInfo = { 0 };
    GetSystemInfo(&systemInfo);

    long numberOfThread = systemInfo.dwNumberOfProcessors * 2;
    thread_pool_t *pool = thread_pool_create(numberOfThread);

    while(1)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);

        if (clientSocket != INVALID_SOCKET)
        {
            thread_pool_add_task(pool, (int)clientSocket);
        }
    }

    thread_pool_destroy(pool);
    fnc_release_socket(serverSocket);

    WSACleanup();
    return EXIT_SUCCESS;
}
