#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "headers.h"

#define HDR_URI_SZ      100
#define HDR_METODE_SZ   4
#define HDR_VERSION_SZ  10

#define MAX_BUFFER      1024
#define HTM_INDEX       "index.htm"

struct headers
{
    char version[HDR_VERSION_SZ];
    char metode[HDR_METODE_SZ];
    char uri[HDR_URI_SZ];
};

void cType(char *fileName, char *cType);
void acceptPetition(long sockID);
void getHeader(struct headers *hdr, int sockID);

int main( int argc, char *argv[] )
{
    int socketDescriptorClient  = 0;
    int socketDescriptor        = 0;
    pthread_t thread            = 0;
    int socketLength            = 0;
    int socketPort              = 8080;
    
    struct sockaddr_in  s_server;
    struct sockaddr_in  s_client;
    
    if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf( "Error creating the socket\n" );
        exit( EXIT_FAILURE );
    }
    
    memset((void *)&s_client, 0, (size_t) sizeof(struct sockaddr_in));
    memset((void *)&s_server, 0, (size_t) sizeof(struct sockaddr_in));
    
    s_server.sin_family         = AF_INET;
    s_server.sin_addr.s_addr    = inet_addr("127.0.0.1");
    s_server.sin_port           = htons(socketPort);
    
    if(bind(socketDescriptor, (struct sockaddr *) &s_server, (socklen_t) sizeof(struct sockaddr_in)) == -1)
    {
        printf("Error at associate port and socket!\n");
        exit(EXIT_FAILURE);
    }
    
    if(listen(socketDescriptor, 5) == -1)
    {
        printf("Error at listen!\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        socketLength = sizeof(struct sockaddr_in);
        if((socketDescriptorClient = accept(socketDescriptor, (struct sockaddr *) &s_client, &socketLength)) != -1)
        {
            if(pthread_create(&thread, NULL, (void *) &acceptPetition, (void *) ((long) socketDescriptorClient)) != 0)
            {
                fprintf(stderr, "SockedID: %d \t %s\n", socketDescriptorClient, "Error creating thread");
            }
        }
    }

    close(socketDescriptor);
    pthread_exit(NULL);
}

void acceptPetition(long sockID)
{
    struct headers hdr;
    char buffer[MAX_BUFFER];
    char ctype[MAX_CTYPE];
    FILE *pFile = NULL;

    memset(ctype, 0, MAX_CTYPE);
    memset(buffer, 0, MAX_BUFFER);
    getHeader(&hdr, (int)sockID);

    /*
    printf("Version: %s\n", hdr.version);
    printf("Metode: %s\n", hdr.metode);
    printf("Uri: %s\n", hdr.uri);
    */
    
    if((pFile = fopen(hdr.uri, "r")) == NULL)
    {
        if(errno == EACCES)
        {

        }
        else
        {
            notFound((int) sockID);
        }
    }
    else
    {
        cType(hdr.uri, ctype);
        fileFound((int) sockID, ctype);
        int sz = 0;

        while((sz = fread(buffer, 1, MAX_BUFFER, pFile)) > 0)
        {
           send(sockID, buffer, sz, 0);
        }

        fclose(pFile);
    }

    close((int) sockID);
    pthread_exit(NULL);
}

void getHeader(struct headers *hdr, int sockID)
{
    int i = 0;
    int c = '\n';

    memset(hdr, 0, sizeof(struct headers));

    for(i = 0; i < HDR_METODE_SZ && (recv(sockID, &c, 1, 0) != -1) && (c != ' ' && c != '\n'); i++)
    {
        if(c != '\r')
        {
            hdr->metode[i] = c;
        }
    }

    for(i = 0; i < HDR_URI_SZ && (recv(sockID, &c, 1, 0) != -1) && (c != ' ' && c != '\n'); i++)
    {
        if(c != '\r')
        {
            hdr->uri[i] = c;
        }
    }

    
    if(strlen(hdr->uri) > 1)
    {
        for(i = 0; i < strlen(hdr->uri); i++)
        {
            hdr->uri[i] = hdr->uri[i + 1];
        }
    }
    else
    {
        strncpy(hdr->uri, HTM_INDEX, HDR_URI_SZ - 1);
    } 

    for(i = 0; i < HDR_VERSION_SZ && (recv(sockID, &c, 1, 0) != -1) && (c != ' ' && c != '\n'); i++)
    {

        if(c != '\r')
        {
            hdr->version[i] = c;
        }
    }
}

void cType(char *fileName, char *cType)
{
    char *c = strrchr(fileName, '.');
    char ext[5]; memset(ext, 0, 5);
    strncpy(ext, c + 1, 4);

    if(!strcmp(ext, "htm") || !strcmp(ext, "html"))
    {
        strcat(cType, "text/html");
    }
    else if(!strcmp(ext,"gif"))
    {
        strcat(cType, "image/gif");
    }
    else if(!strcmp(ext,"jpg") || !strcmp(ext, "jpeg"))
    {
        strcat(cType, "image/jpeg");
    }
    else if(!strcmp(ext,"png"))
    {
        strcat(cType, "image/png");
    }
    else if(!strcmp(ext,"txt"))
    {
        strcat(cType, "text/plain");
    }
}