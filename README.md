# Minimal HTTP server
Minimal multi-platform, Windows and Linux, implementation of a light HTTP server in C.

## Features
* Accepts GET petitions
* Multi-platform (Windows and Linux [easy to add a new ones by providing the services layer])
* Multi-threaded user request (making use of the thread pool, twice as thread as hardware threads)

## Folders structure
```
|->httpd
    |-> platform.h         Operating system services (abstraction layer)
    |-> functions.h        Main entry point for the user request
    |-> functions.c        Contains all the implementation needed to process the user request
    |-> headers.h          Minimal HTTP headers used by the server (200, 400, 500, etc)
    |-> headers.c          Implementation of the HTTP headers
|-> linux
    |-> build              Build script (it uses gcc)
    |-> build.c            Unity build file
    |-> server.c           Program entry point and abstraction layer definition
|-> win32
    |-> build.bat          Build script (cl.exe has to be loaded [vcvars.bat])
    |-> build.c            Unity build file
    |-> server.c           Program entry point and abstraction layer definition
```

## How to build
```
[win32]  mkdir c:\webserver\src\
[win32]  cd c:\webserver\src\

[linux]  mkdir -p /home/<user>/webserver/src/
[linux]  cd /home/<user>/webserver/src/

git clone https://github.com/iFuSiiOnzZ/http.git .

[win32] cd win32
[win32] "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
[win32] .\build.bat --build-type=Release (or Debug)

[linux] cd linux
[linux] sh ./build.sh --build-type=Release (or Debug)

* the output it will be found ../../bin (c:\webserver\bin or /home/<user>/webserver/bin)
* you can use whatever path you want this are just an example
```
