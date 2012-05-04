#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "errors.h"

int main( int argc, char *argv[] )
{
    int socketDescriptor        = 0;
    int socketDescriptorClient  = 0;
    int socketLength            = 0;
    int socketPort              = 8080;
    
    struct sockaddr_in  s_server;
    struct sockaddr_in  s_client;
    
    if((socketDescriptor = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        printf( "Error al crear el socket\n" );
        exit( EXIT_FAILURE );
    }
    
    memset((void *)&s_client, 0, (size_t) sizeof(struct sockaddr_in));
    memset((void *)&s_server, 0, (size_t) sizeof(struct sockaddr_in));
    
    s_server.sin_family         = AF_INET;
    s_server.sin_addr.s_addr    = inet_addr("127.0.0.1");
    s_server.sin_port           = htons(socketPort);
    
    if(bind(socketDescriptor, (struct sockaddr *) &s_server, (socklen_t) sizeof(struct sockaddr_in)) == -1)
    {
        printf("Error al asociar el puerto y el socket!\n");
        exit(EXIT_FAILURE);
    }
    
    if(listen(socketDescriptor, 1) == -1)
    {
        printf("Error al listen!\n");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        socketLength = sizeof(struct sockaddr_in);
        socketDescriptorClient = accept(socketDescriptor, (struct sockaddr *) &s_client, &socketLength);
        notFound(socketDescriptorClient);
        close(socketDescriptorClient);
    }


    
    close(socketDescriptor);
    return(EXIT_SUCCESS);
}
