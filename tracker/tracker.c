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
#include "../common/filetable.h"
#include "../common/file.h"
#include "tracker.h"

/****** local functions *********/

void tracker_stop();
void broadcast_to_peer(int peer_sockfd);
void handleFileUpdate(ptp_peer_t *  recv_seg, struct in_addr ip, int sockfd);
/****** local constants *********/


/**************** global variables ****************/
int listening_socket_fd = -1;         //Listening socket
tracker_peer_t *tracker_side_peer_table[MAX_PEER_SLOTS]; // Tracker side Peer table
fileTable_t *global_filetable;

pthread_mutex_t * filetable_mutex; //file_table mutex
//pthread_create(HandShake thread) when new peer joins

//Listen on handshake port



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
int new_peer(int sockfd, struct in_addr peerIP);

bool isPeerAlreadyConnected(struct in_addr peerIP);



//Main thread: listen on the handshake port, 
//and create a Handshake thread when a new peer joins.
int main()
{

	int new_socket = 0;
	signal(SIGINT, tracker_stop);

	struct sockaddr_in TrackerAddress;
	int addrlen = sizeof(TrackerAddress);

	printf("%s\n", "Main: Starting up tracker main thread");

	tracker_init();
	//Create tracker table
	// tracker_table_t *Table = trackertable_create();

	//Open socket in TRACKER_PORT
	listening_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listening_socket_fd < 0) {
		perror("Main: opening socket failed");
		exit(-1);
	}

	TrackerAddress.sin_family = AF_INET;
	TrackerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	TrackerAddress.sin_port = htons(TRACKER_PORT);
	// binding
	if (bind(listening_socket_fd, (struct sockaddr *) &TrackerAddress, sizeof(TrackerAddress)) < 0) {
		perror("Main: Error when Binding");
		exit(-1);
	}

	//Listen on the handshake port
	if (listen(listening_socket_fd, MAX_PEER_SLOTS) < 0) {
		perror("Main: Error when listening");
		exit(-1);
	}

	printf("Main: Listening on Port %d\n", TRACKER_PORT);
	printf("Main: Waiting for connections......\n");

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
		printf("Main: HandShake thread created\n");
	}
	


}


//Handshake thread, individual for each peer
void* handshake(void* arg) {
	int tableindex = -1; //The index of the current peer in tracker_side_peer_table[]

	//Parse input and free input memory
	handshake_seg_t* toHandshakeThread = (handshake_seg_t*) arg;
	int sockfd = toHandshakeThread->sockfd;
	struct in_addr peerIP = toHandshakeThread->sin_addr;
	// printf("Handshake Thread for peer with IP %ul \n", peerIP.s_addr );
	free(arg);

	printf("Size of ptp peer is %ld\n", sizeof(ptp_peer_t));
	ptp_peer_t* receivedseg = malloc(sizeof(ptp_peer_t) );
	

	//Recieve data from Peer
	while( (read( sockfd , receivedseg, sizeof(ptp_peer_t))) > 0 ){
		//Keep reading until error occours
		if (receivedseg->type == REGISTER){			//REGISTER MESSAGE
			printf("RECEIVED REGISTER\n");
			if (!isPeerAlreadyConnected(peerIP)){
				tableindex = new_peer(sockfd, peerIP); 
				if (tableindex >= 0){
				 	//Acknowledge registration from peer
				 	broadcast_to_peer(sockfd);
				} else {
					//Either error or maximum peers connected
					printf("Either error or maximum peers connected!!!!\n");
				} 
			}
			

		} else if (receivedseg->type == KEEP_ALIVE){
			// printf("%s\n","RECEIVED KEEPALIVE" );
			if (tracker_side_peer_table[tableindex] != NULL){
				tracker_side_peer_table[tableindex]->last_time_stamp = time(NULL);
				broadcast_to_peer(sockfd);
			}


		} else if (receivedseg->type == FILE_UPDATE){
			printf("Peer sent file update\n");
            handleFileUpdate(receivedseg, peerIP, sockfd);
		} else if (receivedseg->type == PEER_CLOSE){
			printf("Peer sent close\n");
			disconnectpeer(tableindex);
		}
		 else {
			printf("Unknown type of segment received for sockfd %d\n",sockfd );
		}
		//free(receivedseg);

	}
	printf("Exiting Handshake Thread for sockfd %d\n",sockfd );
	memset(&receivedseg, 0, sizeof(receivedseg));
	free(receivedseg);
	disconnectpeer(tableindex);
	return NULL;

}






void tracker_init(){

	for (int i = 0; i < MAX_PEER_SLOTS; i++) {
		tracker_side_peer_table[i] = NULL; //Set all entries in the table to NULL
	}

	global_filetable = create_fileTable();
    filetable_mutex = malloc( sizeof(pthread_mutex_t) );
    pthread_mutex_init(filetable_mutex,NULL);


}


bool isPeerAlreadyConnected(struct in_addr peerIP){
	for (int i = 0; i < MAX_PEER_SLOTS; i++) {
		if (tracker_side_peer_table[i] != NULL){
			char* ip = inet_ntoa(peerIP);
			if (strcmp(ip,tracker_side_peer_table[i]->ip) == 0){
				printf("already connected\n");
				return true;
			}
		}
	}
	return false;
			

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


		printf("Client connected with IP %s\n",tracker_side_peer_table[i]->ip);  
        return i;
      }

    }
  }

  //If here, Maximum number of peers already connected
  printf("%s\n","Maximum number of peers connected" );
  return -1;
}


//Sends the updated filetable along with other stuff to all peers
//Except the peer with given sockfd
void broadcast_to_all_peers_except( int excluded_sockfd){
	for (int i = 0; i < MAX_PEER_SLOTS; i++) {
		if (tracker_side_peer_table[i] != NULL && tracker_side_peer_table[i]->sockfd != excluded_sockfd) {
			broadcast_to_peer(tracker_side_peer_table[i]->sockfd);
		}
	}		
}
//Sends the updated filetable along with other stuff to given peer sockfd
void broadcast_to_peer(int peer_sockfd){
	ptp_tracker_t* segtosend = malloc(sizeof(ptp_tracker_t) );
	segtosend->interval = HEARTBEAT_INTERVAL;
	segtosend->piece_len = PIECE_LEN;
	segtosend->file_table_size = global_filetable->numfiles;
	memcpy(&(segtosend->file_table), global_filetable, sizeof(fileTable_t));
	printf("%s\n","Sending to peer" );
	send(peer_sockfd , segtosend , sizeof(ptp_tracker_t), 0 );
	free(segtosend);
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
			   if (difference > HEARTBEAT_TIMEOUT){
			   	//Disconnect the client
			   	printf("MOnitor peers: heartbeat timed out\n");
			   	disconnectpeer(i);
			   }

		}
		printf("monitor_peers: Current number of peers connected is %d\n",count );

		if (count ==0){
			if (global_filetable->numfiles != 0){
				//clear filetable
				printf("Monitor Peers: clearing filetable on tracker\n");
				 pthread_mutex_lock(filetable_mutex);
				global_filetable->numfiles = 0;
				pthread_mutex_unlock(filetable_mutex);
			}
			
			
		}
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

void printfileTable()
{
	printf("Number of files in Filetable is %d\n", global_filetable->numfiles);
	for (int i = 0 ; i<global_filetable->numfiles;i++)
	{
		printf("%s  %d\n", global_filetable->nodes[i].filename, global_filetable->nodes[i].status);
		// printf("%d \n", global_filetable->nodes[i].file_name_size);
	}
}
void handleFileUpdate(ptp_peer_t *  recv_seg, struct in_addr ip, int sockfd){
    // 1.     updateFileTableAfterUpdate(recv_seg);

	printf("Got this file as update %s with status %d\n", recv_seg->file_information.filename,recv_seg->file_information.status );
    if (recv_seg->file_information.status == DELETED)
	{
		//updateFileTable(recv_seg->file_information);g]
		for (int i = 0 ; i < global_filetable->numfiles; i++)
		{
			if (strncmp(global_filetable->nodes[i].filename, recv_seg->file_information.filename, recv_seg->file_information.file_name_size)==0)
			{
				pthread_mutex_lock(filetable_mutex);
				global_filetable->nodes[i].status = DELETED;
				pthread_mutex_unlock(filetable_mutex);
			}

		}

	}
	else if (recv_seg->file_information.status == MODIFIED)
	{
		pthread_mutex_lock(filetable_mutex);
		for (int i = 0 ; i < global_filetable->numfiles; i++) {

			if (strncmp(global_filetable->nodes[i].filename, recv_seg->file_information.filename,
						recv_seg->file_information.file_name_size) == 0) {
				pthread_mutex_lock(filetable_mutex);
				global_filetable->nodes[i].latest_timestamp = recv_seg->file_information.latest_timestamp;
				struct in_addr* desttocopy = &(global_filetable->nodes[i].IP_Peers_with_latest_file[0]);
				memcpy(desttocopy, &ip, sizeof(struct in_addr) );
				global_filetable->nodes[i].num_peers = 1;
				pthread_mutex_unlock(filetable_mutex);
			}
		}
		pthread_mutex_unlock(filetable_mutex);

	}

	else if (recv_seg->file_information.status == MODIFIED_DONE)
	{
		pthread_mutex_lock(filetable_mutex);

		for (int i = 0 ; i < global_filetable->numfiles; i++) {
			if (strncmp(global_filetable->nodes[i].filename, recv_seg->file_information.filename,
						recv_seg->file_information.file_name_size) == 0) {
				pthread_mutex_lock(filetable_mutex);
				struct in_addr* desttocopy = &(global_filetable->nodes[i].IP_Peers_with_latest_file[global_filetable->nodes[i].num_peers]);
				memcpy(desttocopy, &ip, sizeof(struct in_addr) );
				global_filetable->nodes[i].num_peers++;
				pthread_mutex_unlock(filetable_mutex);
			}
		}
		pthread_mutex_unlock(filetable_mutex);


	}
    else{
    	//This means a file was added by a peer
        if (global_filetable->numfiles <= MAX_FILES)
        {
            pthread_mutex_lock(filetable_mutex);
            //check if file exists
            bool found = false;
            for (int i = 0 ; i < global_filetable->numfiles; i++) {
				if (strncmp(global_filetable->nodes[i].filename, recv_seg->file_information.filename,
						recv_seg->file_information.file_name_size) == 0) {
					found = true;


					break;
				}
			}
			if (!found){ //first time
				global_filetable->nodes[global_filetable->numfiles].status = ADDED;
	            global_filetable->nodes[global_filetable->numfiles].latest_timestamp = recv_seg->file_information.latest_timestamp;
	            memcpy(global_filetable->nodes[global_filetable->numfiles].filename, recv_seg->file_information.filename,
	                   sizeof(recv_seg->file_information.filename));
				global_filetable->nodes[global_filetable->numfiles].file_name_size = recv_seg->file_information.file_name_size;

	            //enter ip in list of ips
	            struct in_addr* desttocopy = &(global_filetable->nodes[global_filetable->numfiles].IP_Peers_with_latest_file[0]);
	            memcpy(desttocopy, &ip, sizeof(struct in_addr) );
	            global_filetable->nodes[global_filetable->numfiles].num_peers = 1;
	            global_filetable->numfiles++;

			} else {
				
				struct in_addr* desttocopy = &(global_filetable->nodes[global_filetable->numfiles].IP_Peers_with_latest_file[global_filetable->nodes[global_filetable->numfiles].num_peers]);
	            memcpy(desttocopy, &ip, sizeof(struct in_addr) );
	            global_filetable->nodes[global_filetable->numfiles].num_peers++;
			}




           
            pthread_mutex_unlock(filetable_mutex);
        }
    }
	printfileTable();
    // 2 .  broadcast file table to other peers;
	broadcast_to_all_peers_except(sockfd);
}


void tracker_stop(){

    if (listening_socket_fd >=0){
        close(listening_socket_fd);
    }
    pthread_mutex_destroy(filetable_mutex);
    printf("Exiting Tracker\n");
}


