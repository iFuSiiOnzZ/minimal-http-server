#ifndef _FUNCTIONS_H_
	#define _FUNCTIONS_H_

	#define HDR_URI_SZ      1024
	#define HDR_METHOD_SZ   5
	#define HDR_VERSION_SZ  10

	#define MAX_BUFFER      1024
	#define HTM_INDEX       "index.htm"

	enum FD_TYPES {FILES = 0, DIRS = 1, ELSE = 2, ERROR = 4};

	struct headers
	{
	    char version[HDR_VERSION_SZ];
	    char method[HDR_METHOD_SZ];
	    char uri[HDR_URI_SZ];
	};

	void 	acceptPetition(long sockID);
	void	sendDir(char *dir, int sockID);
	void	sendFile(char *file, int sockID);
	void 	getHeader(struct headers *hdr, int sockID);

	int 	requestMethod(char *method, int sockID);
	int		getFDType(char *path);
	
	char 	*cType(char *fileName);
#endif
