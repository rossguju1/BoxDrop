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
#include "filetable.h"


// typedef struct node {

// 	int status;
// //the size of the file
// 	int size;
// //the name of the file
// 	char *filename;
// //the timestamp when the file is modified or created
// 	unsigned long int timestamp;
// //pointer to build the linked list
// 	struct node *next;
// //for the file table on peers, it is the ip address of the peer
// //for the file table on tracker, it records the ip of all peers which has the
// //newest edition of the file
// 	char* newpeerip;

// 	char* OGPeer; // original ganster peer that added the file

// } Node_t;

// typedef struct fileTable {
// 	struct node *head;
// } fileTable_t;

// Node_t *node_new(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip, char* OGPeer);

// void *create_fileTable(void);

// Node_t *node_new(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip, char* OGPeer);

// bool node_insert(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip);

// char* get_ip_file(fileTable_t *table, char* filename);

// char* get_originalip_file(fileTable_t *table, char* filename);

// unsigned long int get_file_timestamp(fileTable_t *table, char* filename);

// void delete(fileTable_t *table, char* filename);
/*

int main() {

	printf("%s\n", "making filetable");
	fileTable_t* maketable = create_fileTable();

	printf("%s\n", "made filetable");


	char node_1[27] = "file1.c";
	char node_1_ip[45] = "126.2.222.111";
	unsigned long int time1 = 132;
	int size1 = 111;

	printf("%s\n", "inserting node 1");
	if (node_insert(maketable, node_1, size1, time1, node_1_ip)) {

		printf("%s\n", "Inserted node1 !!!!");
	}



	char node_2[27] = "file2.c";
	char node_2_ip[45] = "201.0.111.608";
	unsigned long int time2 = 223;
	int size2 = 2222;


	printf("%s\n", "inserting node 2");
	if (node_insert(maketable, node_2, size2, time2, node_2_ip)) {

		printf("%s\n", "Inserted node2 !!!!");
	}



	char node_3[27] = "file3.c";
	char node_3_ip[45] = "385.9.000.129";
	unsigned long int time3 = 320;
	int size3 = 333333;



	printf("%s\n", "inserting node 3");
	if (node_insert(maketable, node_3, size3, time3, node_3_ip)) {

		printf("%s\n", "Inserted node3 !!!!");
	}




	char node_4[27] = "file4.c";
	char node_4_ip[45] = "426.2.432.494";
	unsigned long int time4 = 458;
	int size4 = 44444;

	printf("%s\n", "inserting node 4");
	if (node_insert(maketable, node_4, size4, time4, node_4_ip)) {

		printf("%s\n", "Inserted node3 !!!!");
	}




	for (Node_t *node = maketable->head; node != NULL; node = node->next) {

		printf("file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
	}
	char* charter = get_ip_file(maketable, node_2);

	char* ipip2 = get_originalip_file(maketable, node_2);

	unsigned long int timestamper2 = get_file_timestamp(maketable, node_2);


	printf("node2 current ip %s OG ip %s time stamp %lu\n", charter, ipip2, timestamper2);

	char* charter3 = get_ip_file(maketable, node_3);

	char* ipip3 =  get_originalip_file(maketable, node_3);

	unsigned long int timestamper3 = get_file_timestamp(maketable, node_3);

	printf("node3 current ip %s, OG ip %s time stamp %lu\n", charter3, ipip3, timestamper3);


	char dummy[123] = "dummy";

	unsigned long int dummynum = 676;


	if (node_insert(maketable, node_3, size3,  dummynum, dummy)) {

		printf("%s\n", "Inserted node3 !!!!");
	}




	for (Node_t *node = maketable->head; node != NULL; node = node->next) {

		printf("file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
	}


	char new2[12] = "new2file";

	char new2ip[12] = "new2ip";

	unsigned long int time2_new = 00;


	if (node_insert(maketable, node_2, timestamper2,  time2_new, new2ip)) {

		printf("%s\n", "Inserted node3 !!!!");
	}

	charter = get_ip_file(maketable, node_2);

	ipip2 = get_originalip_file(maketable, node_2);

	timestamper2 = get_file_timestamp(maketable, node_2);


	printf("~1node2 current ip %s OG ip %s time stamp %lu\n", charter, ipip2, timestamper2);

	for (Node_t *node = maketable->head; node != NULL; node = node->next) {

		printf("~1file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
	}

	if (node_insert(maketable, node_2, 0, dummynum, new2ip)) {

		printf("%s\n", "Inserted node3 !!!!");
	}

	charter = get_ip_file(maketable, node_2);

	ipip2 = get_originalip_file(maketable, node_2);

	timestamper2 = get_file_timestamp(maketable, node_2);


	printf("~2node2 current ip %s OG ip %s time stamp %lu\n", charter, ipip2, timestamper2);



	for (Node_t *node = maketable->head; node != NULL; node = node->next) {

		printf("~2file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
	}

	if (node_insert(maketable, node_2, 0, dummynum, new2ip)) {

		printf("%s\n", "Inserted node3 !!!!");
	}

	charter = get_ip_file(maketable, node_2);

	ipip2 = get_originalip_file(maketable, node_2);

	timestamper2 = get_file_timestamp(maketable, node_2);


	printf("~3node2 current ip %s OG ip %s time stamp %lu\n", charter, ipip2, timestamper2);



	for (Node_t *node = maketable->head; node != NULL; node = node->next) {

		printf("~3file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
	}

	if (node_insert(maketable, node_2, 0, dummynum, dummy)) {

		printf("%s\n", "Inserted node3 !!!!");
	}

	charter = get_ip_file(maketable, node_2);

	ipip2 = get_originalip_file(maketable, node_2);

	timestamper2 = get_file_timestamp(maketable, node_2);


	printf("~4node2 current ip %s OG ip %s time stamp %lu\n", charter, ipip2, timestamper2);




	for (Node_t *node = maketable->head; node != NULL; node = node->next) {

		printf("~4file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
	}


	delete(maketable, node_2);

	for (Node_t *node = maketable->head; node != NULL; node = node->next) {
		if (node->filename != NULL) {
			printf("file name: %s size: %d time: %lu ip: %s original : %s\n\n", node->filename, node->size, node->timestamp, node->newpeerip, node->OGPeer);
		}
	}

	return 0;
}
*/




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

	if (table == NULL || filename == NULL) {
		printf("%s\n", "table or filename is NULL and file was not inserted");
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


void deleter(fileTable_t *table, char* filename)
{

	for (Node_t *node = table->head; node != NULL; node = node->next) {
		if (strcmp(node->filename, filename) == 0 ) { //If key matches, return item
			free(node->filename);
			free(node->newpeerip);
			free(node->OGPeer);
			node->size = 0;
			node->timestamp = 1;
			node->filename = NULL;
			node->newpeerip = NULL;
			node->OGPeer = NULL;

			//continue;

		}

	}
}













