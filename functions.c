#include "functions.h"
#include "headers.h"

int requestMethod(char *method, int sockID)
{
	int OK = 1;

	if(strcmp("GET", method))
	{
		if(!strcmp("POST", method))
		{
			notImplemented((int) sockID);
		}
		else
		{
			badRequest((int) sockID);
		}

		close(sockID);
		OK = -1;
	}

	return(OK);
}

FILE *openFile(char *fileName, int sockID)
{
	FILE *pFile = NULL;

	if((pFile = fopen(fileName, "r")) == NULL)
	{
		if(errno == EACCES)
		{
			forbidden((int) sockID);
		}
		else
		{
			notFound((int) sockID);
		}
		close(sockID);
	}

	return(pFile);
}

void acceptPetition(long sockID)
{
	struct headers hdr;
	char buffer[MAX_BUFFER];
	char ctype[MAX_CTYPE];
	FILE *pFile = NULL;

	memset(ctype, 0, MAX_CTYPE);
	memset(buffer, 0, MAX_BUFFER);
	getHeader(&hdr, (int)sockID);

	if(requestMethod(hdr.metode, sockID) == -1)
	{
		pthread_exit(NULL);
	}

	if((pFile = openFile(hdr.uri, sockID)) == NULL)
	{
		pthread_exit(NULL);
	}

	cType(hdr.uri, ctype);
	fileFound((int) sockID, ctype);
	int sz = 0;

	while((sz = fread(buffer, 1, MAX_BUFFER, pFile)) > 0)
	{
		send(sockID, buffer, sz, 0);
	}

	fclose(pFile);
	close((int) sockID);
	pthread_exit(NULL);
}

void getHeader(struct headers *hdr, int sockID)
{
	int i = 0;
	int c = '\n';
	memset(hdr, 0, sizeof(struct headers));

	for(i = 0; (i < HDR_METODE_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
	{
		if(c != '\r')
		{
			hdr->metode[i] = c;
		}
	}

	for(i = 0; (i < HDR_URI_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
	{
		if(c != '\r')
		{
			hdr->uri[i] = c;
		}
	}

	if(strlen(hdr->uri) > 1)
	{
		for(i = 0; i < strlen(hdr->uri); i++)
		{
			hdr->uri[i] = hdr->uri[i + 1];
		}
	}
	else
	{
		strncpy(hdr->uri, HTM_INDEX, HDR_URI_SZ - 1);
	}

	for(i = 0; (i < HDR_VERSION_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
	{
		if(c != '\r')
		{
			hdr->version[i] = c;
		}
	}
}

void cType(char *fileName, char *cType)
{
	char *c = strrchr(fileName, '.');
	char ext[5]; memset(ext, 0, 5);
	strncpy(ext, c + 1, 4);

	if(!strcmp(ext, "htm") || !strcmp(ext, "html"))
	{
		strcat(cType, "text/html");
	}
	else if(!strcmp(ext,"gif"))
	{
		strcat(cType, "image/gif");
	}
	else if(!strcmp(ext,"jpg") || !strcmp(ext, "jpeg"))
	{
		strcat(cType, "image/jpeg");
	}
	else if(!strcmp(ext,"png"))
	{
		strcat(cType, "image/png");
	}
	else if(!strcmp(ext,"txt"))
	{
		strcat(cType, "text/plain");
	}
}