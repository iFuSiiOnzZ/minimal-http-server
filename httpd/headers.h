#ifndef _HEADERS_H_
    #define _HEADERS_H_

    #include "platform.h"

    void http_ok(int client, const char *cType, platform_t *platform);
    void http_not_found(int client, platform_t *platform);

    void http_not_implemented(int client, platform_t *platform);
    void http_bad_request(int client, platform_t *platform);

    void http_forbidden(int client, platform_t *platform);
    void http_internal_server_error(int client, platform_t *platform);
#endif
