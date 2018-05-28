


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
#include "constants.h"

void update_drop_box_directory()
{
	// need this function for first time initialization. it gets the file table from the tracker
	// and matches against current folder
	// it reads all the files in the directory
	// and updates files that are outdated and downloads those files which are not present.

}





// create dropbox directory in the current working directory

void create_drop_box_directory()
{
	char *buf;
	char *ptrTocwd;
	long size = pathconf(".", _PC_PATH_MAX);

	char root[12] = DIRECTORY_NAME;


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



	FILE* fd;

	fd = fopen(filename, "r");

	int fileSize;


	fseek(fd, 0L, SEEK_END);


	fileSize = ftell(fd);



	rewind(fd);

	return fileSize;
}

int get_number_of_files()
{
	struct dirent *Dirptr;

	// opendir() returns a pointer of DIR type.
	DIR *Dirp = opendir(".");

	int counter;

	counter = 0;

	if (Dirp == NULL)
	{
		printf("Could not open current directory" );
		return 0;
	}


	while ((Dirptr = readdir(Dirp)) != NULL) {

		counter++;

	}


	closedir(Dirp);




	return counter;

}

















