#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_BUFFER	1024
#define MAX_CTYPE	50

void fileFound(int client, char *cType);
void badRequest(int client);
void notFound(int client);