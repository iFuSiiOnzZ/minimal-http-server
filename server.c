#include <stdlib.h>
#include "headers.h"
#include "functions.h"


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