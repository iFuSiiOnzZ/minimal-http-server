#ifndef _PLATFORM_H_
    #define _PLATFORM_H_

    #include <stddef.h>
    #include <sys/types.h>

    #ifdef WIN32
        typedef int ssize_t;
    #endif

    typedef struct file_content_t
    {
        char *fileContent;
        size_t size;
    } file_content_t;

    typedef struct file_api_t
    {
        file_content_t (*loadFileIntoMemory)(const char *file);
        void (*releaseFileFromMemory)(file_content_t *fileContent);
    } file_api_t;

    typedef struct network_api_t
    {
        void (*release)(int __fd);
        ssize_t (*recv)(int __fd, void *__buf, size_t __n, int __flags);
        ssize_t (*send)(int __fd, const void *__buf, size_t __n, int __flags);
    } network_api_t;

    typedef struct platform_t
    {
        network_api_t networkAPI;
        file_api_t fileAPI;
    } platform_t;
#endif
