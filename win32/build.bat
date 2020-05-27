@echo off
setlocal EnableDelayedExpansion

set LOCAL_PATH=%~dp0
set "FILE_N=-[%~n0]:"

set BUILD_TYPE=Release
set REMOVE_INTERMEDIATE=false

:arg-parse
if not "%1"=="" (
    if "%1"=="--build-type" (
        set BUILD_TYPE=%2
        shift
    )

    if "%1"=="--clean" (
        set REMOVE_INTERMEDIATE=true
    )

    if "%1"=="-h" (
        goto help
    )

    if "%1"=="--help" (
        goto help
    )

    shift
    goto arg-parse
)

REM
REM Check if arguments are valid
REM
if %BUILD_TYPE% neq Release if %BUILD_TYPE% neq Debug (
    goto unknown_build_type
)

REM
REM Create build directories
REM
set BIN_PATH=%LOCAL_PATH%..\..\bin
set BUILD_PATH=%LOCAL_PATH%..\..\bin\win32obj

if not exist %BIN_PATH% MKDIR %BIN_PATH%
if not exist %BUILD_PATH% MKDIR %BUILD_PATH%

REM
REM Set compiler flags in function of the build type
REM
set CompilerFlags= /D_CRT_SECURE_NO_WARNINGS /nologo /MP /W4 /Oi /GR /Fo"%BUILD_PATH%\\" /Fe"%BIN_PATH%\\"
set LinkerFlags= /NOLOGO /SUBSYSTEM:CONSOLE /INCREMENTAL:NO

if %BUILD_TYPE% == Debug (
    set CompilerFlags= /Od /MTd /Z7 /GS /Gs /RTC1 !CompilerFlags! /Fd"%BIN_PATH%\\"
) else (
    set CompilerFlags= /WX /O2 /MT !CompilerFlags!
)

REM
REM Compile
REM
cl build.c %CompilerFlags% /link %LinkerFlags%
if %ERRORLEVEL% neq 0 (
    goto bad_exit
)

goto good_exit
REM ============================================================================
REM -- Messages and Errors -----------------------------------------------------
REM ============================================================================

:help
    echo build.bat [--build-type=Release^|Debug]
    echo By default: --build-type=Release
    echo    --build-type: type of build, release or debug
    goto good_exit

:unknown_build_type
    echo.
    echo %FILE_N% [ERROR] Unknown build type
    echo %FILE_N% [INFO ] Allowd values are "Release" or "Debug"
    goto bad_exit

:good_exit
    endlocal
    exit /b 0

:bad_exit
    endlocal
    exit /b %ERRORLEVEL%
