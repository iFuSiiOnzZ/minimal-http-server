#include "functions.h"
#include "headers.h"

#include <netinet/in.h>
#include <sys/stat.h>

#include <pthread.h>
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
	
#include <fcntl.h>
#include <unistd.h>

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

        shutdown(sockID, SHUT_RDWR);
		close(sockID);
		OK = -1;
	}

	return(OK);
}

void acceptPetition(long sockID)
{
	struct headers hdr;
	getHeader(&hdr, (int)sockID);

	if(requestMethod(hdr.metode, sockID) != -1)
	{	
		if(getFDType(hdr.uri) == FILES)
		{
			sendFile(hdr.uri, (int) sockID);
		}
		else if(getFDType(hdr.uri) == DIRS)
		{
			sendDir(hdr.uri, (int) sockID);
		}
		else
		{
			notFound((int) sockID);
		}
	}
	else
	{
		notImplemented((int) sockID);
	}

    shutdown(sockID, SHUT_RDWR);
	close((int) sockID);
	pthread_exit(NULL);
}

void sendFile(char *file, int sockID)
{
	int sz = 0;
	FILE *pFile = NULL;
	char buffer[MAX_BUFFER];

	memset(buffer, 0, MAX_BUFFER);
	fileFound((int) sockID, cType(file));

	if((pFile = fopen(file, "r")) == NULL)
	{
		if(errno == EACCES)
		{
			forbidden((int) sockID);
		}
		else
		{
			sz = snprintf(buffer, MAX_BUFFER, "Error unexpected end of transmission");
			send(sockID, buffer, sz, 0);
		}

		close((int) sockID);
		pthread_exit(NULL);
	}
	
	while((sz = fread(buffer, 1, MAX_BUFFER, pFile)) > 0)
	{
		send(sockID, buffer, sz, 0);
	}

	fclose(pFile);
}

void sendDir(char *dr, int sockID)
{
	int sz = 0;
	DIR *dir = NULL;
	struct dirent *myDir;
	char buffer[MAX_BUFFER];
	char curentDir[MAX_BUFFER];

	memset(buffer, 0, MAX_BUFFER);
	memset(curentDir, 0, MAX_BUFFER);
	fileFound((int) sockID, "text/html");

	if((dir = opendir(dr)) == NULL )
	{
		if(errno == EACCES)
		{
			forbidden((int) sockID);
		}
		else
		{
			sz = snprintf(buffer, MAX_BUFFER, "Error unexpected end of transmission");
			send(sockID, buffer, sz, 0);
		}
		
		shutdown(sockID, SHUT_RDWR);
		close((int) sockID);
		pthread_exit(NULL);
	}

	getcwd(curentDir, MAX_BUFFER);
	while((myDir = readdir(dir)) != NULL)
	{
		if(strcmp(myDir->d_name, ".") != 0 && strcmp(myDir->d_name, "..") != 0)
		{
			sz = snprintf(buffer, MAX_BUFFER, "<a href=\"%s/%s\">%s</a><br />", (strlen(dr + strlen(curentDir)) > 1)? dr + strlen(curentDir) : "", myDir->d_name, myDir->d_name);
			send(sockID, buffer, sz, 0);
		}
	}

	closedir(dir);
}

void getHeader(struct headers *hdr, int sockID)
{
	int i = 0;
	int j = 0;
	int c = '\n';

	memset(hdr, 0, sizeof(struct headers));
	getcwd(hdr->uri, HDR_URI_SZ);

	for(i = 0; (i < HDR_METODE_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
	{
		if(c != '\r')
		{
			hdr->metode[i] = c;
		}
	}

	for(i = strlen(hdr->uri), j = i; (i < HDR_URI_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
	{
		if(c != '\r')
		{
			hdr->uri[i] = c;
		}
	}

	for(i = 0; (i < HDR_VERSION_SZ) && (recv(sockID, &c, 1, 0) > 0) && (c != ' ' && c != '\n'); i++)
	{
		if(c != '\r')
		{
			hdr->version[i] = c;
		}
	}
}

char *cType(char *fileName)
{
	char *c = strrchr(fileName, '.');
	
	if(c == NULL)
	{
		return("text/plain");
	}

	char ext[5]; 
	memset(ext, 0, 5);
	strncpy(ext, c + 1, 4);

	if(!strcmp(ext, "htm") || !strcmp(ext, "html"))
	{
		return("text/html");
	}
	else if(!strcmp(ext,"jpg") || !strcmp(ext, "jpeg"))
	{
		return("image/jpeg");
	}
	else if(!strcmp(ext,"gif"))
	{
		return("image/gif");
	}
	else if(!strcmp(ext,"png"))
	{
		return("image/png");
	}
	else
	{
		return("text/plain");
	}
}

int getFDType(char *path)
{
	struct stat st;
	enum FD_TYPES fd;

	if(stat(path,&st) == 0)
	{
		if(st.st_mode & S_IFDIR)
		{
			fd = DIRS;
		}
		else if(st.st_mode & S_IFREG)
		{
			fd = FILES;
		}
		else
		{
			fd = ELSE;
		}
	}
	else
	{
		fd = ERROR;
	}

	return(fd);
}
