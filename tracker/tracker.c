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


	//Listen until accept returns error
	while(1){
		new_socket = accept(listening_socket_fd, (struct sockaddr *)&TrackerAddress, 
                       (socklen_t*)&addrlen);
		if (new_socket < 0){
			printf("Error accepting a new connection\n");
			break;
		}


		printf("New Peer connecteed\n");
		new_peer(new_socket, &TrackerAddress);

		//CREATE A NEW HANDSHAKE THREAD


	}
	

    // valread = read( new_socket , buffer, 1024);
    // printf("%s\n",buffer );
    // send(new_socket , hello , strlen(hello) , 0 );
    // printf("Hello message sent\n");
    // return 0;
	



	// pthread_t handshake_thread;

	// pthread_create(&handshake_thread, NULL, tracker_hand_shake, (void *)0);






}



void tracker_init(){

	for (int i = 0; i < MAX_PEER_SLOTS; i++) {
		tracker_side_peer_table[i] = NULL; //Set all entries in the table to NULL
	}



	//Also remember mutexes


}

//Function to handle new peer
void new_peer(int sockfd, struct sockaddr_in address){
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
      	// char ip[IP_LEN];
        tracker_side_peer_table[i]->sockfd = sockfd;
        char* ip = inet_ntoa(address.sin_addr);
        memcpy(tracker_side_peer_table[i]->ip, ip ,IP_LEN);
		tracker_side_peer_table[i]->last_time_stamp = time(NULL);

		printf("Client connected with IP %s\n",tracker_side_peer_table[i]->ip);  
        return i;
      }

    }
  }

  //If here, Maximum number of peers already connected
  printf("%s\n","Maximum number of peers connected" );
  return -1;
}


// void *tracker_hand_shake() {



// 	tracker_send();

// 	tracker_recieve(int sockfd, );





// }




// void *monitor_peers()
// {


// 	time_t t0 = time(0);
// // ...
// 	time_t t1 = time(0);
// 	double datetime_diff_ms = difftime(t1, t0) * 1000.;




// }






void tracker_stop(){
    if (listening_socket_fd >=0){
        close(listening_socket_fd);
    }
    printf("Exiting Tracker\n");
}