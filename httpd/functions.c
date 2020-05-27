/*
    Project includes
*/
#include "functions.h"
#include "headers.h"

/*
    Lib includes
*/

/*
    C includes
*/
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
/*
    Defines
*/
#define HDR_URI_SZ      1024
#define HDR_METHOD_SZ   5
#define HDR_VERSION_SZ  10

#define MAX_BUFFER      1024

#define NumberOfElements(x) (sizeof(x) / sizeof((x)[0]))

#if defined(_WIN32)
    #define strtok_r strtok_s
    #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
    #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

/*
    Enums
*/
enum FD_TYPES { FD_FILES, FD_DIRS, FD_OTHERS, FD_ERROR };
enum METHOD_TYPE { METHOD_GET, METHOD_POST, METHOD_PUT, METHOD_DELETE, METHOD_OTHER };

/*
    Data structure
*/
typedef struct http_headers_t
{
    char version[HDR_VERSION_SZ];
    char method[HDR_METHOD_SZ];
    char uri[HDR_URI_SZ];
} http_headers_t;

typedef struct mime_types_t
{
    const char *extension;
    const char *type;
} mime_types_t;

/*
    Global variables
*/
static mime_types_t gMimeTypes[] =
{
    { "htm" , "text/html" },
    { "html", "text/html" },

    { "scss", "text/x-scss" },
    { "css" , "text/css"    },

    { "jpg" , "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "bmp" , "image/bmp"  },
    { "gif" , "image/gif"  },
    { "png" , "image/png"  },

    { "js" , "application/x-javascript" },

    { "woff2", "font/woff2" },
    { "woff" , "font/woff"  },
    { "ttf"  , "font/ttf"   }
};

///////////////////////////////////////////////////////////////////////////////

static int fnc_get_request_method(char *method)
{
    if(!strcmp("GET", method))
    {
        return METHOD_GET;
    }
    else if(!strcmp("PUT", method))
    {
        return METHOD_PUT;
    }
    else if(!strcmp("POST", method))
    {
        return METHOD_POST;
    }
    else if(!strcmp("DELETE", method))
    {
        return METHOD_DELETE;
    }

    return METHOD_OTHER;
}

static const char *fnc_get_file_extension(const char *fileName)
{
    char *fileExtension = strrchr(fileName, '.');
    if (fileExtension == NULL) return "text/plain";;

    // NOTE(Andrei): Remove the dot
    ++fileExtension;

    for(size_t i = 0; i < NumberOfElements(gMimeTypes); ++i)
    {
        if(!strcmp(fileExtension, gMimeTypes[i].extension))
        {
            return gMimeTypes[i].type;
        }
    }

    return "text/plain";
}

static void fnc_send_file(const char *file, int socketId, platform_t *platform)
{
    size_t sz = 0;
    FILE *pFile = NULL;

    char buffer[MAX_BUFFER];
    http_ok(socketId, fnc_get_file_extension(file), platform);

    if((pFile = fopen(file, "rb")) == NULL)
    {
        if(platform->getLastError() == ERROR_EACCES)
        {
            http_forbidden(socketId, platform);
        }
        else
        {
            http_internal_server_error(socketId, platform);
        }

        return;
    }

    while((sz = fread(buffer, 1, MAX_BUFFER, pFile)) > 0)
    {
        platform->networkAPI.send(socketId, buffer, sz, 0);
    }

    fclose(pFile);
}

static void fnc_send_directory(const char *directory, const char *httpPath, int socketId, platform_t *platform)
{
    void *dir = NULL;
    dirent_t myContent;

    if((dir = platform->dirAPI.opendir(directory)) == NULL )
    {
        if(platform->getLastError() == ERROR_EACCES)
        {
            http_forbidden(socketId, platform);
        }
        else
        {
            http_internal_server_error(socketId, platform);
        }

        return;
    }

    http_ok(socketId, "text/html", platform);
    const char *httpFilePath = directory + strlen(httpPath);

    if (!strcmp(httpFilePath, "/"))
    {
        httpFilePath = "";
    }

    int sz = 0;
    char buffer[MAX_BUFFER] = { 0 };

    while(platform->dirAPI.readdir(dir, &myContent))
    {
        if(strcmp(myContent.d_name, ".") != 0 && strcmp(myContent.d_name, "..") != 0)
        {
            sz = snprintf(buffer, MAX_BUFFER, "<a href=\"%s/%s\">%s</a><br />", httpFilePath, myContent.d_name, myContent.d_name);
            platform->networkAPI.send(socketId, buffer, sz, 0);
        }
    }

    platform->dirAPI.closedir(dir);
}

static void fnc_parse_header(http_headers_t *hdr, int socketId, platform_t *platform)
{
    char buffer[MAX_BUFFER] = { 0 };
    int bytesReceived  = 0;

    while(1)
    {
        int recvBytes = platform->networkAPI.recv(socketId, buffer + bytesReceived, MAX_BUFFER - bytesReceived, 0);

        if(recvBytes < 0)
        {
            break;
        }

        bytesReceived += recvBytes;
        buffer[bytesReceived] = 0;

        if(strstr(buffer, "\r\n\r\n") || bytesReceived == MAX_BUFFER)
        {
            break;
        }
    }

    if (bytesReceived <= 0)
    {
        return;
    }

    char *saveptr = NULL, *data = strtok_r(buffer, " \r\n", &saveptr);
    strcpy(hdr->method, data);

    data = strtok_r(NULL, " \r\n", &saveptr);
    strcpy(hdr->uri, data);

    data = strtok_r(NULL, " \r\n", &saveptr);
    strcpy(hdr->version, data);
}

static int fnc_get_path_type(const char *path)
{
    struct stat st = { 0 };

    if(stat(path, &st))
    {
        return FD_ERROR;
    }

    if(S_ISDIR(st.st_mode))
    {
        return FD_DIRS;
    }
    else if(S_ISREG(st.st_mode))
    {
        return FD_FILES;
    }

    return FD_OTHERS;
}

void fnc_process_request(int socketId, platform_t *platform)
{
    http_headers_t hdr = { 0 };
    fnc_parse_header(&hdr, socketId, platform);

    /*
    printf("Method  : %s\n", hdr.method);
    printf("Uri     : %s\n", hdr.uri);
    printf("Version : %s\n", hdr.version);
    printf("SocketId: %d\n\n", socketId);
    */

    if(fnc_get_request_method(hdr.method) != METHOD_GET)
    {
        http_not_implemented(socketId, platform);
        goto close_connection;
    }

    char currentDir[MAX_BUFFER], httpPath[MAX_BUFFER];
    platform->dirAPI.getcwd(httpPath, MAX_BUFFER);

    strncpy(currentDir, httpPath, MAX_BUFFER);
    strncat(currentDir, hdr.uri, MAX_BUFFER - strlen(currentDir));

    switch (fnc_get_path_type(currentDir))
    {
        case FD_FILES:
        {
            fnc_send_file(currentDir, socketId, platform);
        } break;

        case FD_DIRS:
        {
            fnc_send_directory(currentDir, httpPath, socketId, platform);
        } break;

        default:
        {
            http_not_found(socketId, platform);
        } break;
    }

close_connection:
    platform->networkAPI.release(socketId);
}
