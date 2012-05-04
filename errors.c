#include "errors.h"

void badRequest(int client)
{
	int sz = 0;
	char buf[MAX_BUFFER_ERROR];
	memset(buf, 0, MAX_BUFFER_ERROR);

	sz = snprintf(buf, MAX_BUFFER_ERROR, "HTTP/1.0 400 Bad Request\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, MAX_BUFFER_ERROR, "Content-type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, MAX_BUFFER_ERROR, "\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, MAX_BUFFER_ERROR, "<html><title>Bad Request</title><body><p>Your browser sent a bad request</p></body></html>");
	send(client, buf, sz, 0);
}

void notFound(int client)
{
	int sz = 0;
	char buf[MAX_BUFFER_ERROR];
	memset(buf, 0, MAX_BUFFER_ERROR);

	sz = snprintf(buf, MAX_BUFFER_ERROR, "HTTP/1.0 404 File Not Found\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, MAX_BUFFER_ERROR, "Content-Type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, MAX_BUFFER_ERROR, "\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, MAX_BUFFER_ERROR, "<html><title>Not Found</title><body><p>The server could not find the resourse</p></body></html>");
	send(client, buf, sz, 0);
}

