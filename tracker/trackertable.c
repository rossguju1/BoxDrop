/*

trackertable.c





*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "trackertable.h"



#define MAX_PEER_SLOTS 10

#define IP_LEN 25




/********** local functions ***********/


set_t *set_new(void);

tracker_peer_t  *tracker_entry_node_new(int sockfd, char ip, unsigned long last_time_stamp);

bool set_insert(set_t *set, int sockfd, char ip, unsigned long last_time_stamp);

void set_delete(set_t *set);

bool hashtable_insert(tracker_table_t *tracker_table, int sockfd, char ip, unsigned long last_time_stamp);

void hashtable_delete(tracker_table_t *tracker_table);

int hashtable_find(tracker_table_t *tracker_table, int sockfd);

int set_find(set_t *set, int sockfd);





int makehash(int node)
{
	return node % MAX_PEER_SLOTS;
}


tracker_table_t *trackertable_create()
{

	int i; // counter for loop
	tracker_table_t *tracker_table = malloc(sizeof(tracker_table_t)); // actual hashtable
	if (!tracker_table) {
		return NULL;
	}

	tracker_table->table = calloc(MAX_PEER_SLOTS, sizeof(set_t)); //pointers to the headnodes


	printf("%s\n", "trackertable_Create 1");
	for (i = 0; i < MAX_PEER_SLOTS; i++) {
		//create an emtracker_tabley set for all indexes in table
		printf("%s\n", "making a new set");
		tracker_table->table[i] = set_new();
	}

	// for (int i = 0; i < nbrNum; i++) {
	//   printf("%s\n", "inserting");
	//   if (hashtable_insert(rt, nbrARRAY[i], nbrARRAY[i])) {
	//     printf("%s\n", "inserted");
	//     continue;
	//   }
	// }

	return tracker_table;
}

/** TODO:
 * This function destroys a routing table.
 * All dynamically allocated data structures for this routing table are freed.
 */
void trackertable_destroy(tracker_table_t *tracker_table)
{

	hashtable_delete(tracker_table);

}

/** TODO:
 * This function updates the routing table using the given
 * destination node ID and next hop's node ID.
 * If the routing entry for the given destination already exists,
 * update the existing routing entry.
 * If the routing entry of the given destination is not there, add
 * one with the given next node ID.
 * Each slot in the routing table contains a linked list of routing
 * entries due to conflicting hash nodeIDs (different hash nodeIDs -
 * destination node ID - may have the same hash value (slot entry
 * number).
 * To add an routing entry to the hash table:
 * First use the hash function makehash() to get the slot number in
 * which this routing entry should be stored.
 * Then append the routing entry to the linked list in that slot.
 */
void trackertable_setnextnode(tracker_table_t *tracker_table, int sockfd, char ip, unsigned long last_time_stamp)
{
	if (!(hashtable_insert(tracker_table, sockfd, ip, last_time_stamp))) {
		printf(" failed to insert routing entry in table\n");
	}


}

/** TODO:
 * This function looks up the destNodeID in the routing table.
 * Since routing table is a hash table, this opeartion has O(1) time complexity.
 * To find a routing entry for a destination node, first use the
 * hash function makehash() to get the slot number and then go
 * through the linked list in that slot to search for the routing
 * entry.
 * Return nextNodeID if the destNodeID is found, else -1.
 */
int trackertable_getnextnode(tracker_table_t *tracker_table, int sockfd)
{
	int next = hashtable_find(tracker_table, sockfd);

	if (next >= 0) {
		return next;
	}

	return -1;
}

/** TODO:
 * This function prints out the contents of the routing table.
 */
void trackertable_print(tracker_table_t *tracker_table) {
	printf("-------------routing table------------\n");

	for (int i = 0; i < MAX_PEER_SLOTS; i++) {

		printf("ROUTING TABLE SLOT number: %d \n\n", i);

		if (tracker_table->table[i] != NULL) {

			tracker_peer_t *currentNode = tracker_table->table[i]->head;
			while ( currentNode != NULL ) { //iterate through all costs


				currentNode = currentNode->next;
			}
		}
	}
	printf("--------------------------------------\n");
}


/*********************** set_new() ***********************/

// helper function

set_t *set_new(void)
{
	set_t *set = malloc(sizeof(set_t)); // allocate memory to *set pointer

	if ( set == NULL ) {
		return NULL;       //error allocating memory
	} else {
		set->head = NULL;  // set first node head to be NULL
		return set;
	}
}


/********************** set_insert() ***************************/

// helper function


bool set_insert(set_t *set, int sockfd, char ip, unsigned long last_time_stamp) {
	if (set != NULL && sockfd > 0) {
		// allocate a new node to be added to the list
		tracker_peer_t *new = tracker_entry_node_new(sockfd, ip, last_time_stamp);
		if (new == NULL) {
			return false; //return false if new node is not created
		}

		if (set->head == NULL) {  //If set has no costs
			set->head = new;  //Make the new Node the head of the set
			return true;
		} else {
			tracker_peer_t *lastNode = set->head;
			new->next = NULL;
			//compare nodeID for head
			while (lastNode->next != NULL) {  //reach the end of the list
				lastNode = lastNode->next;
			}
			lastNode->next = new;
			return true;
		}
	} else {
		printf("%s \n\n", "failed to insert routing entry in linked list");
	}
	return false;
}


/******************** routing_entry_node_new() *********************/

// helper function

tracker_peer_t  *tracker_entry_node_new(int sockfd, char ip, unsigned long last_time_stamp)
{
	tracker_peer_t *node = malloc(sizeof(tracker_peer_t ));

	if (node == NULL) {
		// error allocating memory for node; return error
		return NULL;
	} else {
		//node->destNodeID = (int)malloc(sizeof(int));
		if (node->sockfd < 0) {
			// error allocating memory for nodeID;
			// cleanup and return error
			free(node);
			return NULL;
		} else {
			// copy nodeID values into node->nodeID

			node->sockfd = sockfd;
			node->ip = ip;
			node->last_time_stamp = last_time_stamp;
			node->next = NULL;
			return node;
		}
	}
}



/***********  set_delete() ******/

// helper function

void set_delete(set_t *set)
{
	if (set != NULL) {
		for (tracker_peer_t *node = set->head; node != NULL; ) {
			tracker_peer_t *next = node->next;     // remember what comes next
			free(node);         // free the node
			node = next;          // and move on to next
		}
		free(set);
	}
}


/************** set_find() ***************/

// helper function

int set_find(set_t *set, int sockfd)
{
	if (set == NULL) {
		return -1; // bad set
	} else if (set->head == NULL) {
		return -1; // set is emtracker_tabley
	} else if (sockfd < 0 ) {
		return -1; // nodeID is negative
	} else {
		tracker_peer_t *currentNode = set->head;
		while ( currentNode != NULL ) { //iterate through all costs
			if (currentNode->sockfd == sockfd) { //If nodeID matches, return cost
				return currentNode->sockfd;
			}
			currentNode = currentNode->next;
		}
		return -1; //not found
	}

}

/******* hashtable_insert() ************/
// helper function

bool hashtable_insert(tracker_table_t *tracker_table, int sockfd, char ip, unsigned long last_time_stamp)
{
	if (tracker_table != NULL && sockfd > 0 && ip > 0) {
		int hash = makehash(sockfd);
		return (set_insert(tracker_table->table[hash], sockfd, ip, last_time_stamp));
	} else {
		return false;
	}

}

/********** hashtable_find() ****************/

// helper function

int hashtable_find(tracker_table_t *tracker_table, int sockfd)
{
	if (tracker_table != NULL && sockfd > 0) {
		int hash = makehash(sockfd);
		return set_find(tracker_table->table[hash], sockfd);

	} else {
		return -1;
	}

}

/************** hashtable_delete() ****************/

// helper function

void hashtable_delete(tracker_table_t *tracker_table)
{
	int i;
	if (tracker_table != NULL) {
		for (i = 0; i < MAX_PEER_SLOTS; i++) {
			set_delete(tracker_table->table[i]);
		}
		// free table of arrays
		free(tracker_table->table);
		//free hashtable
		free(tracker_table);
	}

}































































