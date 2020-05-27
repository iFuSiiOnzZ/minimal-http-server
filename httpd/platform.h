#ifndef _PLATFORM_H_
    #define _PLATFORM_H_

    #include <stddef.h>
    #include <sys/types.h>

    #ifdef _WIN32
        typedef int ssize_t;
    #endif

    typedef struct file_content_t
    {
        char *fileContent;
        size_t size;
    } file_content_t;

    typedef struct dirent_t
    {
        unsigned int d_type;
        char *d_name;
    } dirent_t;

    typedef struct directory_api_t
    {
        char*   (*getcwd)(char *__buf, size_t __size);
        int     (*readdir)(void *__dir, dirent_t *__data);
        void*   (*opendir)(const char *__name);
        int     (*closedir)(void *__dir);
    } directory_api_t;

    typedef struct file_api_t
    {
        file_content_t (*loadFileIntoMemory)(const char *__file);
        void           (*releaseFileFromMemory)(file_content_t *__fileContent);
    } file_api_t;

    typedef struct network_api_t
    {
        void    (*release)(int __fd);
        ssize_t (*recv)(int __fd, void *__buf, size_t __n, int __flags);
        ssize_t (*send)(int __fd, const void *__buf, size_t __n, int __flags);
    } network_api_t;

    typedef struct platform_t
    {
        network_api_t networkAPI;
        directory_api_t dirAPI;
        file_api_t fileAPI;

        int (*getLastError)();
    } platform_t;

    #define ERROR_EPERM        1  /* Operation not permitted */
    #define ERROR_ENOENT       2  /* No such file or directory */
    #define ERROR_ESRCH        3  /* No such process */
    #define ERROR_EINTR        4  /* Interrupted system call */
    #define ERROR_EIO          5  /* I/O error */
    #define ERROR_ENXIO        6  /* No such device or address */
    #define ERROR_E2BIG        7  /* Argument list too long */
    #define ERROR_ENOEXEC      8  /* Exec format error */
    #define ERROR_EBADF        9  /* Bad file number */
    #define ERROR_ECHILD      10  /* No child processes */
    #define ERROR_EAGAIN      11  /* Try again */
    #define ERROR_ENOMEM      12  /* Out of memory */
    #define ERROR_EACCES      13  /* Permission denied */
    #define ERROR_EFAULT      14  /* Bad address */
    #define ERROR_ENOTBLK     15  /* Block device required */
    #define ERROR_EBUSY       16  /* Device or resource busy */
    #define ERROR_EEXIST      17  /* File exists */
    #define ERROR_EXDEV       18  /* Cross-device link */
    #define ERROR_ENODEV      19  /* No such device */
    #define ERROR_ENOTDIR     20  /* Not a directory */
    #define ERROR_EISDIR      21  /* Is a directory */
    #define ERROR_EINVAL      22  /* Invalid argument */
    #define ERROR_ENFILE      23  /* File table overflow */
    #define ERROR_EMFILE      24  /* Too many open files */
    #define ERROR_ENOTTY      25  /* Not a typewriter */
    #define ERROR_ETXTBSY     26  /* Text file busy */
    #define ERROR_EFBIG       27  /* File too large */
    #define ERROR_ENOSPC      28  /* No space left on device */
    #define ERROR_ESPIPE      29  /* Illegal seek */
    #define ERROR_EROFS       30  /* Read-only file system */
    #define ERROR_EMLINK      31  /* Too many links */
    #define ERROR_EPIPE       32  /* Broken pipe */
    #define ERROR_EDOM        33  /* Math argument out of domain of func */
    #define ERROR_ERANGE      34  /* Math result not representable */
#endif
