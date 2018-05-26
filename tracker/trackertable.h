/*



trackertable.h


*/

#include "trackertable.h"





#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H



typedef struct _tracker_side_peer_t {

	//Remote peer IP address, 16 bytes

	char ip[IP_LEN];

	//Last alive timestamp of this peer

	unsigned long last_time_stamp;

	//TCP connection to this remote peer

	int sockfd;

	//Pointer to the next peer, linked list

	struct _tracker_side_peer_t *next;

} tracker_peer_t;




/* routing table as a hash table with MAX_ROUTINGTABLE_SLOTS slots.
 * Each slot is a linked list of routing entries.
typedef struct routing_table {
	routingtable_entry_t *hash[MAX_ROUTINGTABLE_SLOTS];
} routingtable_t;

*/

typedef struct set {

	struct _tracker_side_peer_t *head;       // head of the set

} set_t;


typedef struct tracking_table  {

	struct set **table;		//*hash[MAX_ROUTINGTABLE_SLOTS];

} tracker_table_t;



































































#endif //ROUTINGTABLE_H


