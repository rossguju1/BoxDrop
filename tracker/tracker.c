/*

 CS60 - DropBox Project

 tracker.c

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <ctype.h>
#include <pthread.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <time.h>
//#include "trackertable.h"
#include "../common/constants.h"
#include "../common/file.h"
#include "tracker.h"

/****** local functions *********/

void tracker_stop();


/****** local constants *********/


/**************** global variables ****************/
int listening_socket_fd = -1;         //Listening socket
tracker_peer_t *tracker_side_peer_table[MAX_PEER_SLOTS]; // Tracker side Peer table

//pthread_create(HandShake thread) when new peer joins

//Listen on handshake port




#define MAX_PEER_SLOTS 10

//void *main_pthread();

//Receive messages from a specific peer

//If needed respond using peer-tracker handshake protocol (PTP)
//

//Initializes the tables and other stuff for tracker
void tracker_init();

void *tracker_hand_shake();

void* handshake(void* arg);

//monitor and accept alive message from peers
//Remove dead peers if timeout occurs
void *monitor_peers();

// void tracker_recieve(tracker_table_t *table, seg_t *segmentBuffer);

// void tracker_send(tracker_table_t *table, );

int connect_to_peer();




//Main thread: listen on the handshake port, 
//and create a Handshake thread when a new peer joins.
int main()
{

	int new_socket = 0;
	signal(SIGINT, tracker_stop);

	struct sockaddr_in TrackerAddress;
	int addrlen = sizeof(TrackerAddress);

	printf("%s\n", "Starting up tracker main thread");

	tracker_init();
	//Create tracker table
	// tracker_table_t *Table = trackertable_create();

	//Open socket in TRACKER_PORT
	listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_socket_fd < 0) {
		perror("opening socket failed");
		exit(-1);
	}

	TrackerAddress.sin_family = AF_INET;
	TrackerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	TrackerAddress.sin_port = htons(TRACKER_PORT);
	// binding
	if (bind(listening_socket_fd, (struct sockaddr *) &TrackerAddress, sizeof(TrackerAddress)) < 0) {
		perror("Error when Binding");
		exit(-1);
	}

	//Listen on the handshake port
	if (listen(listening_socket_fd, MAX_PEER_SLOTS) < 0) {
		perror("Error when listening");
		exit(-1);
	}

	printf("Listening on Port %d\n", TRACKER_PORT);
	printf("Waiting for connections......\n");

	//Create a handshake thread
	pthread_t monitor_thread;
	pthread_create(&monitor_thread, NULL, monitor_peers, (void *) 0);	

	//Listen until accept returns error
	while(1){
		new_socket = accept(listening_socket_fd, (struct sockaddr *)&TrackerAddress, 
                       (socklen_t*)&addrlen);
		if (new_socket < 0){
			//printf("Error accepting a new connection\n");
			break;
		}


		printf("New Peer connecteed\n");

		//Data to send to handshake thread
		handshake_seg_t* toHandshakeThread = malloc(sizeof(handshake_seg_t));
		toHandshakeThread->sockfd = new_socket;
		toHandshakeThread->sin_addr = TrackerAddress.sin_addr;

		//Create a handshake thread
		pthread_t handshake_thread;
		pthread_create(&handshake_thread, NULL, handshake, (void *) toHandshakeThread);




	}
	








}



void tracker_init(){

	for (int i = 0; i < MAX_PEER_SLOTS; i++) {
		tracker_side_peer_table[i] = NULL; //Set all entries in the table to NULL
	}



	//Also remember mutexes


}

//Function to handle new peer
int new_peer(int sockfd, struct in_addr peerIP){
	//looks up the peer to find the first 
  //NULL entry, and creates a new peer entry using malloc() for that 
  //entry.
  for (int i = 0; i < MAX_PEER_SLOTS; i++) {
    if (tracker_side_peer_table[i] == NULL){
      tracker_side_peer_table[i] = malloc ( sizeof(struct _tracker_side_peer_t) );

      if (tracker_side_peer_table[i] == NULL){
        printf("%s\n","Error in malloc" );
        return -1;
      } else {
      
        tracker_side_peer_table[i]->sockfd = sockfd;
        char* ip = inet_ntoa(peerIP);
        memcpy(tracker_side_peer_table[i]->ip, ip ,IP_LEN);
		tracker_side_peer_table[i]->last_time_stamp = time(NULL);

		// //Create a handshake thread
		// pthread_t handshake_thread;
		// pthread_create(&handshake_thread, NULL, handshake, (void *) sockfd);

		printf("Client connected with IP %s\n",tracker_side_peer_table[i]->ip);  
        return i;
      }

    }
  }

  //If here, Maximum number of peers already connected
  printf("%s\n","Maximum number of peers connected" );
  return -1;
}

//Handshake thread, individual for each peer
void* handshake(void* arg) {
	printf("%s\n","new handshake thread" );
	int tableindex = -1; //The index of the current peer in tracker_side_peer_table[]

	//Parse input and free input memory
	handshake_seg_t* toHandshakeThread = (handshake_seg_t*) arg;
	int sockfd = toHandshakeThread->sockfd;
	struct in_addr peerIP = toHandshakeThread->sin_addr;
	free(arg);

	ptp_peer_t* receivedseg = malloc(sizeof(ptp_peer_t) );
	ptp_tracker_t* segtosend = malloc(sizeof(ptp_tracker_t) );

	//Recieve data from Peer
	while( (read( sockfd , receivedseg, sizeof(ptp_peer_t))) > 0 ){
		//Keep reading until error occours
		if (receivedseg->type == REGISTER){			//REGISTER MESSAGE
			printf("Peer trying to register\n");
			tableindex = new_peer(sockfd, peerIP);
			if (tableindex >= 0){
			 	//Acknowledge register
			 	
			 	segtosend->interval = HEARTBEAT_INTERVAL;
			 	printf("Sent interval is %d\n",segtosend->interval );
			 	segtosend->piece_len = PIECE_LEN;
			 	send(sockfd , segtosend , sizeof(ptp_tracker_t), 0 );
			 	//Also need to send file table
			 	//segtosend->file_table = something;			-TODO
			 	//segtosend->file_table_size = something;		-TODO

			} else {
				//Either error or maximum peers connected
				printf("Either error or maximum peers connected!!!!\n");
			} 

		} else if (receivedseg->type == KEEP_ALIVE){
			if (tracker_side_peer_table[tableindex] != NULL){
				tracker_side_peer_table[tableindex]->last_time_stamp = time(NULL);
			}

		} else if (receivedseg->type == FILE_UPDATE){
			printf("Peer sent file update\n");
		} else if (receivedseg->type == PEER_CLOSE){
			printf("Peer sent close\n");
			disconnectpeer(tableindex);
		}
		 else {
			printf("Unknown type of segment received for sockfd %d\n",sockfd );
		}

	}
	printf("Exiting Handshake Thread for sockfd %d\n",sockfd );
	free(receivedseg);
	free(segtosend);
	disconnectpeer(tableindex);
	return;

 

}




//REmoves dead peers if timeout occours
void *monitor_peers()
{

	while(1){
		int count = 0;
		for (int i = 0; i < MAX_PEER_SLOTS; i++) {
			if (tracker_side_peer_table[i] == NULL){
				continue;
			}
			count++;
			   time_t currenttime = time(NULL);
			   int difference = (int) (currenttime - tracker_side_peer_table[i]->last_time_stamp);
			   printf("difference is %d\n",difference );
			   if (difference > HEARTBEAT_TIMEOUT){

			   	//Disconnect the client
			   	disconnectpeer(i);
			   }

		}
		printf("Current number of peers connected is %d\n",count );
		sleep(MONITOR_INTERVAL);
	}

}



void disconnectpeer(int index){
	if (tracker_side_peer_table[index] == NULL){
		printf("Client already disconnected!\n");
		return;
	}

	//Maybe send a closing message???
	printf("%s\n","Trying to disconnect peer" );
	close(tracker_side_peer_table[index]->sockfd);
	free(tracker_side_peer_table[index]);
	tracker_side_peer_table[index] = NULL;
	printf("Peer at index %d disconnected\n", index );

}


void tracker_stop(){
    if (listening_socket_fd >=0){
        close(listening_socket_fd);
    }
    printf("Exiting Tracker\n");
}