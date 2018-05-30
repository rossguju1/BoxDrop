#ifndef FILETABLE_H
#define FILETABLE_H

#include "constants.h"

// typedef struct node {
// 	int status;
// 	int size;
// 	char *filename;
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


typedef struct node{
	int status;
	int size;
	char filename[FILE_NAME_LEN];
	int file_name_size;
	time_t latest_timestamp;
	struct in_addr IP_Peers_with_latest_file[MAX_PEER_SLOTS];
    int num_peers;
	char OGPeer[IP_LEN];
} node_t;

typedef struct fileTable{
	int numfiles;
	struct node nodes[MAX_FILES];
} fileTable_t;





/* The packet data structure sending from peer to tracker */
typedef struct segment_peer {
	// protocol length
	int protocol_len;
	// protocol name
	char protocol_name[PROTOCOL_LEN + 1];
	// packet type : register, keep alive, update file table
	int type;
	// reserved space, you could use this space for your convenient, 8 bytes by default
	char reserved[RESERVED_LEN];
	// the peer ip address sending this packet
	char peer_ip[IP_LEN];
	// listening port number in p2p
	int port;
	// file table of the client -- your own design
	struct fileTable file_table;
    // file info
    struct node file_information;
}ptp_peer_t;


/* The packet data structure sending from tracker to peer */
typedef struct segment_tracker{
	// time interval that the peer should sending alive message periodically
	int interval;
	// piece length
	int piece_len;
	// file number in the file table -- optional
	int file_table_size;
	// file table of the tracker -- your own design
	struct fileTable file_table;
} ptp_tracker_t;


fileTable_t *create_fileTable();

// Node_t *node_new(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip, char* OGPeer);

// bool node_insert(fileTable_t* table, char *filename, int size, unsigned long int timestamp, char *newpeerip);

// char* get_ip_file(fileTable_t *table, char* filename);

// char* get_originalip_file(fileTable_t *table, char* filename);

// unsigned long int get_file_timestamp(fileTable_t *table, char* filename);

// void deleter(fileTable_t *table, char* filename);


// fileTable_t *fileTable_REGISTER();

// int check_update_local(fileTable_t* localFiletable, fileTable_t* trackerFile_table);

// int check_update_tracker(fileTable_t* localFiletable, fileTable_t* trackerFile_table);

// bool find_file(fileTable_t *table, char* filename);

// int check_modified(fileTable_t* write_update_table, fileTable_t* read_update_table);



#endif




