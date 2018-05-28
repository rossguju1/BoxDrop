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
#include <netdb.h>
#include <signal.h>
//#include "trackertable.h"
#include "../common/constants.h"
#include "../common/file.h"

/****** local functions *********/

void tracker_stop(); 

/****** local constants *********/


/**************** global variables ****************/
int listening_socket_fd = -1;         //Listening socket

//pthread_create(HandShake thread) when new peer joins

//Listen on handshake port




#define MAX_PEER_SLOTS 10

//void *main_pthread();

//Receive messages from a specific peer

//If needed respond using peer-tracker handshake protocol (PTP)
//

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