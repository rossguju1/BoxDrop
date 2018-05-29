#ifndef PEER_H
#define PEER_H

typedef struct filetemp{		//PLACEHOLDER
	char data[50];
}file_t;

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
	// the number of files in the local file table -- optional
	int file_table_size;
	// file table of the client -- your own design
	// file_t file_table;
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
	struct filetemp file_table;
} ptp_tracker_t;



void peer_stop();

#define MAX_LEN 1024 /*Path length for a directory*/
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 16 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

void *monitor(void *arg);
void add_watches(int fd, char *root);


#endif //PEER_H