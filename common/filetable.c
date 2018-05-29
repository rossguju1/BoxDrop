
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
#include <time.h>
#include <netdb.h>
#include "file.h"
#include "filetable.h"

void *create_fileTable(void)
{

	fileTable_t *table = (fileTable_t*)malloc(sizeof(fileTable_t));

	if ( table == NULL ) {
		return NULL;       //error allocating memory
	} else {
		table->head = NULL;  // set first node head to be NULL
		return table;
	}
}

Node_t *node_new(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip, char* OGPeer)
{

	Node_t* node = malloc(sizeof(Node_t));

	if (node == NULL) {


		return NULL;
	} else {

		node->filename = malloc(strlen(filename) + 1);
		node->newpeerip = malloc(strlen(newpeerip) + 1);
		node->OGPeer = malloc(strlen(OGPeer) + 1);


		if (node->filename == NULL || node->newpeerip == NULL) {
			free(node->filename);
			free(node->newpeerip);
			free(node->OGPeer);
			return NULL;
		} else {
			node->status = 1;
			strcpy(node->filename, filename);
			strcpy(node->newpeerip, newpeerip);
			strcpy(node->OGPeer, OGPeer);
			node->size = size;
			node->timestamp = timestamp;
			node->next = NULL;
			return node;
		}

	}
}


bool node_insert(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip)
{

	if (table == NULL || filename == NULL)  {
		printf("%s\n", "table or filename is NULL and file was not inserted");
		return false;
	}
	if (size < 0) {
		return false;
	}
	// allocate a new node to be added to the list
	Node_t *new = node_new(table, filename, size, timestamp, newpeerip, newpeerip);
	if (new == NULL) {
		return false; //return false if new node is not created
	}

	if (table->head == NULL) {  //If set has no items
		table->head = new;  //Make the new Node the head of the set
		return true;
	} else {
		Node_t *currentNode = table->head;
		//compare file for head

		while (currentNode->next != NULL) { //reach the end of the list
			if (strcmp(currentNode->filename, filename) == 0 ) { //If key exists, return update file info except for OGPeer

				strcpy(currentNode->filename, filename);

				strcpy(currentNode->newpeerip, newpeerip);

				currentNode->timestamp = timestamp;

				currentNode->size = size;



				return true;
			} else {
				currentNode = currentNode->next;
				continue;
			}
		}
		currentNode->next = new;
		return true;
	}

	// never gonna reach here unless serious error
	return false;

}

char* get_ip_file(fileTable_t *table, char* filename)
{
	if (table == NULL) {
		return NULL; // bad set
	} else if (table->head == NULL) {
		return NULL; // set is empty
	} else if (filename == NULL) {
		return NULL; // key is NULL
	} else {
		Node_t *currentNode = table->head;
		while ( currentNode != NULL ) { //iterate through all items
			if (strcmp(currentNode->filename, filename) == 0 ) { //If key matches, return item
				return currentNode->newpeerip;
			}
			currentNode = currentNode->next;
		}
		return NULL; //not found
	}
}

char* get_originalip_file(fileTable_t *table, char* filename)
{
	if (table == NULL) {
		return NULL; // bad set
	} else if (table->head == NULL) {
		return NULL; // set is empty
	} else if (filename == NULL) {
		return NULL; // key is NULL
	} else {
		Node_t *currentNode = table->head;
		while ( currentNode != NULL ) { //iterate through all items
			if (strcmp(currentNode->filename, filename) == 0 ) { //If key matches, return item
				return currentNode->OGPeer;
			}
			currentNode = currentNode->next;
		}
		return NULL; //not found
	}
}

unsigned long int get_file_timestamp(fileTable_t *table, char* filename)
{

	if (table == NULL) {
		return -1; // bad set
	} else if (table->head == NULL) {
		return -1; // set is empty
	} else if (filename == NULL) {
		return -1; // key is NULL
	} else {
		Node_t *currentNode = table->head;
		while ( currentNode != NULL ) { //iterate through all items
			if (strcmp(currentNode->filename, filename) == 0 ) { //If key matches, return item
				return currentNode->timestamp;
			}
			currentNode = currentNode->next;
		}
		return -1; //not found
	}
	// never going to get here
	return -1;
}

bool find_file(fileTable_t *table, char* filename)
{

	if (table == NULL) {
		return false; // bad set
	} else if (table->head == NULL) {
		return false; // set is empty
	} else if (filename == NULL) {
		return false; // key is NULL
	} else {
		Node_t *currentNode = table->head;
		while ( currentNode != NULL ) { //iterate through all items
			if (strcmp(currentNode->filename, filename) == 0 ) { //If key matches, return item
				return true;
			}
			currentNode = currentNode->next;
		}
		return false; //not found
	}
	// never going to get here
	return false;
}


void deleter(fileTable_t *table, char* filename)
{

	for (Node_t *node = table->head; node != NULL; node = node->next) {
		if (strcmp(node->filename, filename) == 0 ) { //If key matches, return item
			free(node->filename);
			free(node->newpeerip);
			free(node->OGPeer);
			node->size = 0;
			node->timestamp = 0;
			node->filename = NULL;
			node->newpeerip = NULL;
			node->OGPeer = NULL;

			//continue;

		}

	}
}

int get_file_count(fileTable_t *table) {

	int counter;

	counter = 1;

	for (Node_t *node = table->head; node != NULL; node = node->next) {
		if (node->filename != NULL) {
			counter++;
		}
	}


	return counter;
}


fileTable_t *fileTable_REGISTER() {

	char hostname[2048];
	int node_id;
	gethostname(hostname, 2048);
	struct hostent *host;
	char* ip_address;





	host = gethostbyname(hostname);
	ip_address = inet_ntoa(*((struct in_addr*)host->h_addr_list[0]));

	//printf("ip %s\n", hostname);

	unsigned long int timestamp = time(NULL);


	struct dirent *DIRentry;  // Pointer for directory entry
	// chdir("/Users/rossguju/Desktop/anon/CS60SP18/CS60-TeamDDOS-DropBox/common");

	DIR *DIRp = opendir(".");

	if (DIRp == NULL)
	{
		printf("Could not open dropbox root directory" );
		return NULL;
	}
	fileTable_t *localFileTable = create_fileTable();
	// printf("file number %d \n", get_number_of_files());

	while ((DIRentry = readdir(DIRp)) != NULL) {



		int size = get_file_size(DIRentry->d_name);

		if (node_insert(localFileTable, DIRentry->d_name, (size - 2), timestamp, ip_address)) {

			continue;
		}

	}
	closedir(DIRp);

	// deleter(localFileTable, ".");
	// deleter(localFileTable, "..");


	return localFileTable;
}



/*
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
*/


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


int check_update_local(fileTable_t* localFiletable, fileTable_t* trackerFile_table)
{
	int counter = 0;
	for (Node_t *node = trackerFile_table->head; node != NULL; node = node->next) {
		char* track_input = node->filename;

		if (!find_file(localFiletable, track_input)) {
			if (node_insert(localFiletable, track_input, node->size, node->timestamp, node->newpeerip)) {
				printf("local file is missing file : %s\n", node->filename);
				counter++;
				continue;
			}

		}
	}
	return counter;
}

int check_update_tracker(fileTable_t* localFiletable, fileTable_t* trackerFile_table) {
	int counter = 0;

	for (Node_t *node = localFiletable->head; node != NULL; node = node->next) {

		if (!find_file(trackerFile_table, node->filename)) {

			if (node_insert(trackerFile_table, node->filename, node->size, node->timestamp, node->newpeerip)) {
				printf("local file is missing file : %s\n", node->filename);
				counter++;
				continue;
			}

		}

	}
	return counter;
}


int check_modified(fileTable_t* write_update_table, fileTable_t* read_update_table) {
	int counter = 0;

	for (Node_t *node = read_update_table->head; node != NULL; node = node->next) {
		if (find_file(write_update_table, node->filename)) {

			if (get_file_timestamp(write_update_table, node->filename) != get_file_timestamp(read_update_table, node->filename)) {
				int gtime = get_file_timestamp(read_update_table, node->filename);
				if (node_insert(write_update_table, node->filename, node->size, gtime, node->newpeerip)) {
					printf("local file needs to be updated because timeStamp is different : %s\n", node->filename);
					counter++;
					continue;
				}

			}

		}
	}
	return counter;
}



