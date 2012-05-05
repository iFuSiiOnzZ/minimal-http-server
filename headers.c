#include "headers.h"

void badRequest(int client)
{
	int sz = 0;
	char buf[MAX_BUFFER];
	memset(buf, 0, (size_t) MAX_BUFFER);

	sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 400 Bad Request\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Bad Request</title><body><p>Your browser sent a bad request</p></body></html>");
	send(client, buf, sz, 0);
}

void notImplemented(int client)
{
	int sz = 0;
	char buf[MAX_BUFFER];
	memset(buf, 0, (size_t) MAX_BUFFER);

	sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 501 Not Implemente\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Forbidden</title><body><p>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</p></body></html>");
	send(client, buf, sz, 0);
}

void forbidden(int client)
{
	int sz = 0;
	char buf[MAX_BUFFER];
	memset(buf, 0, (size_t) MAX_BUFFER);

	sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 403 Forbidden\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Forbidden</title><body><p>You don't have acces to this content</p></body></html>");
	send(client, buf, sz, 0);
}

void notFound(int client)
{
	int sz = 0;
	char buf[MAX_BUFFER];
	memset(buf, 0, (size_t) MAX_BUFFER);

	sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 404 File Not Found\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-Type: text/html\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>File Not Found</title><body><p>The server could not find the resourse</p></body></html>");
	send(client, buf, sz, 0);
}

void fileFound(int client, char *cType)
{

	int sz = 0;
	char buf[MAX_BUFFER];	memset(buf, 0, (size_t) MAX_BUFFER);

	sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 200 OK\r\n");
	send(client, buf, sz, 0);
	

	sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-Type: %s\r\n", cType);
	send(client, buf, sz, 0);
	
	sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
	send(client, buf, sz, 0);
}