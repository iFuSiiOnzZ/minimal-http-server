#ifndef _HEADERS_H_
    #define _HEADERS_H_
    
    #define MAX_BUFFER  1024
    #define MAX_CTYPE   50

    void fileFound(int client, char *cType);
    void notImplemented(int client);
    void badRequest(int client);
    void forbidden(int client);
    void notFound(int client);
#endif
