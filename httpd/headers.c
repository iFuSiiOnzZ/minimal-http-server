/*
    Project includes
*/
#include "headers.h"

/*
    Lib includes
*/

/*
    C includes
*/
#include <stdio.h>

/*
    Defines
*/
#define MAX_BUFFER  1024

///////////////////////////////////////////////////////////////////////////////

void http_bad_request(int client, platform_t *platform)
{
    int sz = 0;
    char buf[MAX_BUFFER];

    sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 400 Bad Request\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-type: text/html\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Bad Request</title><body><p>Your browser sent a bad request</p></body></html>");
    platform->networkAPI.send(client, buf, sz, 0);
}

void http_not_implemented(int client, platform_t *platform)
{
    int sz = 0;
    char buf[MAX_BUFFER];

    sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 501 Not Implemented\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-type: text/html\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Not Implemented</title><body><p>The server either does not recognise the request method, or it lacks the ability to fulfill the request.</p></body></html>");
    platform->networkAPI.send(client, buf, sz, 0);
}

void http_forbidden(int client, platform_t *platform)
{
    int sz = 0;
    char buf[MAX_BUFFER];

    sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 403 Forbidden\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-type: text/html\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Forbidden</title><body><p>You don't have acces to this content</p></body></html>");
    platform->networkAPI.send(client, buf, sz, 0);
}

void http_not_found(int client, platform_t *platform)
{
    int sz = 0;
    char buf[MAX_BUFFER];

    sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 404 File Not Found\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-Type: text/html\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>File Not Found</title><body><p>The server could not find the resourse</p></body></html>");
    platform->networkAPI.send(client, buf, sz, 0);
}

void http_internal_server_error(int client, platform_t *platform)
{
    int sz = 0;
    char buf[MAX_BUFFER];

    sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 500 Internal Server Error\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-Type: text/html\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "<html><title>Internal Server Error</title><body><p>Internal Server Error</p></body></html>");
    platform->networkAPI.send(client, buf, sz, 0);
}

void http_ok(int client, const char *cType, platform_t *platform)
{
    int sz = 0;
    char buf[MAX_BUFFER];

    sz = snprintf(buf, (size_t) MAX_BUFFER, "HTTP/1.0 200 OK\r\n");
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "Content-Type: %s\r\n", cType);
    platform->networkAPI.send(client, buf, sz, 0);

    sz = snprintf(buf, (size_t) MAX_BUFFER, "\r\n");
    platform->networkAPI.send(client, buf, sz, 0);
}
