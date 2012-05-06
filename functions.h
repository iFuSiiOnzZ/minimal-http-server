#ifndef _FUNCTIONS_H_
	#define _FUNCTIONS_H_

	#include <netinet/in.h>
	#include <stdio.h>
	#include <errno.h>
	

	#define HDR_URI_SZ      100
	#define HDR_METODE_SZ   5
	#define HDR_VERSION_SZ  10

	#define MAX_BUFFER      1024
	#define HTM_INDEX       "index.htm"

	struct headers
	{
	    char version[HDR_VERSION_SZ];
	    char metode[HDR_METODE_SZ];
	    char uri[HDR_URI_SZ];
	};

	void 	cType(char *fileName, char *cType);
	void 	acceptPetition(long sockID);
	void 	getHeader(struct headers *hdr, int sockID);

	int 	requestMethod(char *method, int sockID);
	FILE	*openFile(char *fileName, int sockID);
#endif