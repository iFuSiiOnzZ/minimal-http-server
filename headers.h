#ifndef _HEADERS_H_
    #define _HEADERS_H_

    void http_ok(int client, char *cType);
    void http_not_found(int client);

    void http_not_implemented(int client);
    void http_bad_request(int client);

    void http_forbidden(int client);
    void http_internal_server_error(int client);
#endif
