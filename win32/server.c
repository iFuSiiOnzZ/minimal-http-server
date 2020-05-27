/*
    Project includes
*/
#include "../httpd/platform.h"
#include "../httpd/functions.h"

/*
    Lib includes
*/

/*
    C includes
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>

#include <stdio.h>
#include <stdlib.h>

#include <direct.h>
#include "dirent.h"

#pragma comment(lib,"Ws2_32.lib")

///////////////////////////////////////////////////////////////////////////////

void fnc_release_socket(int socketId)
{
    shutdown(socketId, SD_BOTH);
    closesocket(socketId);
}

int fnc_read_dir(void *__dir, dirent_t *__data)
{
    struct dirent *dir = readdir(__dir);
    if (dir == NULL) return 0;

    __data->d_name = dir->d_name;
    __data->d_type = dir->d_type;

    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    (void)argc, (void)argv;
    WSADATA wsaData = { 0 };
    int err = WSAStartup(MAKEWORD(2,2), &wsaData);

    if (err != 0)
    {
        printf("WSAStartup failed with error: %d\n", err);
        return EXIT_FAILURE;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(serverSocket == INVALID_SOCKET)
    {
        printf("Client: socket() failed! Error code: %ld\n", WSAGetLastError());
        WSACleanup(); return EXIT_FAILURE;
    }

    struct sockaddr_in local = { 0 };
    local.sin_family        = AF_INET;
    local.sin_addr.s_addr   = INADDR_ANY;
    local.sin_port          = htons(8080);

    if (bind(serverSocket, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);

        WSACleanup();
        return EXIT_FAILURE;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);

        WSACleanup();
        return EXIT_FAILURE;
    }

    platform_t platform = { 0 };
    platform.networkAPI.send = (ssize_t(*)(int, const void *, size_t, int))send;
    platform.networkAPI.recv = (ssize_t(*)(int, void *, size_t, int))recv;
    platform.networkAPI.release = fnc_release_socket;

    platform.dirAPI.getcwd = (char*(*)(char *, size_t))_getcwd;
    platform.dirAPI.readdir = fnc_read_dir;
    platform.dirAPI.opendir = (void * (*)(const char *))opendir;
    platform.dirAPI.closedir = (int (*)(void *))closedir;


    while(1)
    {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);

        if (clientSocket != INVALID_SOCKET)
        {
            fnc_process_request((int)clientSocket, &platform);
        }
    }

    WSACleanup();
    return EXIT_SUCCESS;
}
