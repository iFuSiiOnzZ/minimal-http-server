#include "functions.h"
#include "headers.h"

#include <netinet/in.h>
#include <sys/stat.h>

#include <pthread.h>
#include <string.h>

#include <dirent.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>

int requestMethod(char *method, int sockID)
{
    if(!strcmp("GET", method))
    {
        return 1;
    }

    if(!strcmp("POST", method))
        notImplemented(sockID);
    {
    }
    else
    {
        badRequest(sockID);
    }

    shutdown(sockID, SHUT_RDWR);
    close(sockID);

    return -1;
}

void acceptPetition(int sockID)
{
    struct headers hdr = { 0 };
    getHeader(&hdr, sockID);

    if(requestMethod(hdr.method, sockID) != -1)
    {   
        if(getFDType(hdr.uri) == FILES)
        {
            sendFile(hdr.uri, sockID);
        }
        else if(getFDType(hdr.uri) == DIRS)
        {
            sendDir(hdr.uri, sockID);
        }
        else
        {
            notFound(sockID);
        }
    }
    else
    {
        notImplemented(sockID);
    }

    shutdown(sockID, SHUT_RDWR);
    close(sockID);

    pthread_exit(NULL);
}

void sendFile(char *file, int sockID)
{
    int sz = 0;
    FILE *pFile = NULL;
    char buffer[MAX_BUFFER];

    memset(buffer, 0, MAX_BUFFER);
    fileFound(sockID, cType(file));

    if((pFile = fopen(file, "r")) == NULL)
    {
        if(errno == EACCES)
        {
            forbidden(sockID);
        }
        else
        {
            sz = snprintf(buffer, MAX_BUFFER, "Error unexpected end of transmission");
            send(sockID, buffer, sz, 0);
        }

        close(sockID);
        pthread_exit(NULL);
    }
    
    while((sz = fread(buffer, 1, MAX_BUFFER, pFile)) > 0)
    {
        send(sockID, buffer, sz, 0);
    }

    fclose(pFile);
}

void sendDir(char *dr, int sockID)
{
    int sz = 0;
    DIR *dir = NULL;
    struct dirent *myDir = NULL;

    char buffer[MAX_BUFFER] = { 0 };
    char curentDir[MAX_BUFFER] = { 0 };


    fileFound((int) sockID, "text/html");

    if((dir = opendir(dr)) == NULL )
    {
        if(errno == EACCES)
        {
            forbidden(sockID);
        }
        else
        {
            sz = snprintf(buffer, MAX_BUFFER, "Error unexpected end of transmission");
            send(sockID, buffer, sz, 0);
        }
        
        shutdown(sockID, SHUT_RDWR);
        close(sockID);

        pthread_exit(NULL);
    }

    getcwd(curentDir, MAX_BUFFER);
    while((myDir = readdir(dir)) != NULL)
    {
        if(strcmp(myDir->d_name, ".") != 0 && strcmp(myDir->d_name, "..") != 0)
        {
            sz = snprintf(buffer, MAX_BUFFER, "<a href=\"%s/%s\">%s</a><br />", (strlen(dr + strlen(curentDir)) > 1)? dr + strlen(curentDir) : "", myDir->d_name, myDir->d_name);
            send(sockID, buffer, sz, 0);
        }
    }

    closedir(dir);
}

void getHeader(struct headers *hdr, int sockID)
{
    int i = 0, j = 0;
    int c = '\n';

    memset(hdr, 0, sizeof(struct headers));
    getcwd(hdr->uri, HDR_URI_SZ);

    for(i = 0; (i < HDR_METHOD_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
    {
        if(c != '\r')
        {
            hdr->method[i] = c;
        }
    }

    for(i = strlen(hdr->uri), j = i; (i < HDR_URI_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
    {
        if(c != '\r')
        {
            hdr->uri[i] = c;
        }
    }

    for(i = 0; (i < HDR_VERSION_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
    {
        if(c != '\r')
        {
            hdr->version[i] = c;
        }
    }

    printf("HTTP version: %s\n", hdr->version);
    printf("HTTP request: %s\n", hdr->method);
    printf("HTTP uri:     %s\n", hdr->uri);
}

char *cType(char *fileName)
{
    char *fileExtension = strrchr(fileName, '.');
    
    if(fileExtension == NULL)
    {
        return "text/plain";
    }

    if(!strcmp(fileExtension, "htm") || !strcmp(fileExtension, "html"))
    {
        return "text/html";
    }
    else if(!strcmp(fileExtension,"jpg") || !strcmp(fileExtension, "jpeg"))
    {
        return "image/jpeg";
    }
    else if(!strcmp(fileExtension,"gif"))
    {
        return "image/gif";
    }
    else if(!strcmp(fileExtension,"png"))
    {
        return "image/png";
    }
    else
    {
        return "text/plain";
    }
}

int getFDType(char *path)
{
    struct stat st = { 0 };

    if(stat(path, &st))
    {
        return ERROR;
    }

    if(st.st_mode & S_IFDIR)
    {
        return DIRS;
    }
    else if(st.st_mode & S_IFREG)
    {
        return FILES;
    }

    return ELSE;
}
