
#ifndef FILETABLE_H
#define FILETABLE_H




typedef struct node {

	int status;
//the size of the file
	int size;
//the name of the file
	char *filename;
//the timestamp when the file is modified or created
	unsigned long int timestamp;
//pointer to build the linked list
	struct node *next;
//for the file table on peers, it is the ip address of the peer
//for the file table on tracker, it records the ip of all peers which has the
//newest edition of the file
	char* newpeerip;

	char* OGPeer; // original ganster peer that added the file

} Node_t;

typedef struct fileTable {
	struct node *head;
} fileTable_t;

Node_t *node_new(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip, char* OGPeer);

void *create_fileTable(void);

Node_t *node_new(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip, char* OGPeer);

bool node_insert(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip);

char* get_ip_file(fileTable_t *table, char* filename);

char* get_originalip_file(fileTable_t *table, char* filename);

unsigned long int get_file_timestamp(fileTable_t *table, char* filename);

void deleter(fileTable_t *table, char* filename);

fileTable_t *fileTable_REGISTER();

#endif







