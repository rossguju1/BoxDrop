/*

 CS60 - DropBox Project

 tracker.h

*/
#include <time.h>

#ifndef TRACKER_H
#define TRACKER_H

/**************** global types ****************/
typedef struct filetemp{		//PLACEHOLDER
	char data[50];
}file_t;

typedef struct _tracker_side_peer_t {

	//Remote peer IP address, 16 bytes
	char ip[IP_LEN];
	//Last alive timestamp of this peer
	time_t last_time_stamp;
	//TCP connection to this remote peer
	int sockfd;
	//Pointer to the next peer, linked list, removed for now

	//struct _tracker_side_peer_t *next;

} tracker_peer_t;

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


//Struct to communicate between main thread and handshake thread
typedef struct handshake_seg {
	int sockfd;
	struct in_addr   sin_addr;
} handshake_seg_t;


// void *tracker_hand_shake();


// void *monitor_peers();

// void tracker_recieve(tracker_table_t *table, seg_t *segmentBuffer);

// void tracker_send(tracker_table_t *table, );

int connect_to_peer();

#endif //TRACKER_H