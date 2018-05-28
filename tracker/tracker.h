/*

 CS60 - DropBox Project

 tracker.h

*/
#include <time.h>

#ifndef TRACKER_H
#define TRACKER_H

/**************** global types ****************/
typedef struct _tracker_side_peer_t {

	//Remote peer IP address, 16 bytes

	char ip[IP_LEN];

	//Last alive timestamp of this peer

	//unsigned long last_time_stamp;
	time_t last_time_stamp;

	//TCP connection to this remote peer

	int sockfd;

	//Pointer to the next peer, linked list, removed for now

	//struct _tracker_side_peer_t *next;

} tracker_peer_t;



// void *tracker_hand_shake();


// void *monitor_peers();

// void tracker_recieve(tracker_table_t *table, seg_t *segmentBuffer);

// void tracker_send(tracker_table_t *table, );

int connect_to_peer();

#endif //TRACKER_H