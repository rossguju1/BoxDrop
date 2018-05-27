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
#include "trackertable.h"
#include "../common/file.h"

/****** local functions *********/



/****** local constants *********/

//pthread_create(HandShake thread) when new peer joins

//Listen on handshake port


#define TRACKER_PORT 3465

#define MAX_PEER_SLOTS 10

//void *main_pthread();

//Receive messages from a specific peer

//If needed respond using peer-tracker handshake protocol (PTP)
//

void *tracker_hand_shake();

//monitor and accept alive message from peers
//Remove dead peers if timeout occurs
void *monitor_peers();

void tracker_recieve(tracker_table_t *table, seg_t *segmentBuffer);

void tracker_send(tracker_table_t *table, );

int connect_to_peer();





int main()
{

	int socket_fd;

	fd_set SelectSet;

	struct sockaddr_in ServerAddress;

	printf("%s\n", "Starting up tracker");

	tracker_table_t *Table = trackertable_create();

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_fd < 0) {
		perror("opening socket failed");
		exit(-1);
	}


	int opt_val = 1;
	setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));

	memset(&ServerAddress, 0, sizeof(ServerAddress));


	ServerAddress.sin_family = AF_INET;

	ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	ServerAddress.sin_port = htons(TRACKER_PORT);


// binding
	if (bind(socket_fd, (struct sockaddr *) &ServerAddress, sizeof(ServerAddress)) < 0) {
		perror("Error when Binding");
		exit(-1);
	}

	if (listen(socket_fd, MAX_PEER_SLOTS) < 0) {
		perror("Error when listening");
		exit(-1);
	}

	printf("Listening on Port %d\n", TRACKER_PORT);
	printf("Waiting for connections......\n");





	pthread_t handshake_thread;

	pthread_create(&handshake_thread, NULL, tracker_hand_shake, (void *)0);






}





void *tracker_hand_shake() {



	tracker_send();

	tracker_recieve(int sockfd, );





}




void *monitor_peers()
{


	time_t t0 = time(0);
// ...
	time_t t1 = time(0);
	double datetime_diff_ms = difftime(t1, t0) * 1000.;




}








