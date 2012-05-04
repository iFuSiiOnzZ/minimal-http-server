#include "errors.h"

void badRequest(int client)
{
	int sz = 0;
	char buf[1024];
	memset(buf, 0, 1024);

	sz = sprintf(buf, "HTTP/1.0 400 Bad Request\r\n");
	send(client, buf, sz, 0);
	
	sz = sprintf(buf, "Content-type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = sprintf(buf, "\r\n");
	send(client, buf, sz, 0);
	
	sz = sprintf(buf, "<html><title>Bad Request</title><body><p>Your browser sent a bad request</p></body></html>");
	send(client, buf, sz, 0);
}

void notFound(int client)
{
	int sz = 0;
	char buf[1024];
	memset(buf, 0, 1024);

	sz = sprintf(buf, "HTTP/1.0 404 File Not Found\r\n");
	send(client, buf, sz, 0);
	
	sz = sprintf(buf, "Content-Type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = sprintf(buf, "\r\n");
	send(client, buf, sz, 0);
	
	sz = sprintf(buf, "<html><title>Not Found</title><body><p>The server could not find the resourse</p></body></html>");
	send(client, buf, sz, 0);
}

