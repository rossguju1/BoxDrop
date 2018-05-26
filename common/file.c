


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "file.h"




// create dropbox directory in the current working directory

void create_drop_box_directory()
{
	char *buf;
	char *ptrTocwd;
	long size = pathconf(".", _PC_PATH_MAX);

	char root[12] = "DropBoxroot";


	if ((buf = (char *)malloc((size_t)size)) != NULL) {
		ptrTocwd = getcwd(buf, (size_t)size);


		char *file_path = malloc(strlen(ptrTocwd) + strlen(root) + 1);

		sprintf(file_path, "%s/%s", ptrTocwd, root);

		mkdir(file_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

		chdir(file_path);

		free(file_path);

	}


	return;
}



char *get_file_path(char *filename)
{
	FILE* fd;

	char *buf;
	char *ptrTocwd;
	long size = pathconf(".", _PC_PATH_MAX);

	buf = (char *)malloc((size_t)size);


	ptrTocwd = getcwd(buf, (size_t)size);


	char *file_path = malloc(strlen(ptrTocwd) + strlen(filename) + 1);

	sprintf(file_path, "%s/%s", ptrTocwd, filename);

	return file_path;

}



int get_file_size(char *filename)
{

// struct stat st;
// stat(filename, &st);
// size = st.st_size;

	FILE* fd;

	fd = fopen(filename, "r");

	int fileSize;


	fseek(fd, 0L, SEEK_END);


	fileSize = ftell(fd);

	//fseek(fp, 0L, SEEK_SET);


	rewind(fd);

	return fileSize;
}




















