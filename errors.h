#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#define MAX_BUFFER_ERROR	1024

void badRequest(int client);
void notFound(int client);
