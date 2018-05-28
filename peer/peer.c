
/*

 CS60 - DropBox Project

 peer.c

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
#include "peer.h"
#include "../common/file.h"

/**************** local functions ****************/
void peer_stop(); 
void* keepAlive(void* arg);

/**************** global variables ****************/
int tracker_connection = -1;         //connection to the tracker
int interval = -1;
int piece_len = -1;

int main()
{
	int sock_fd;
	struct sockaddr_in address;

    signal(SIGINT, peer_stop);

    // choose a node to connect
    char hostname[50];
    printf("Enter server name to connect:");
    scanf("%s", hostname);
    struct hostent* host;
    host = gethostbyname(hostname);     //get host structure from gethostbyname
    if (host== NULL){
        printf("%s\n","Invalid Server" );
        return -1;
    }
    char* ip = inet_ntoa(*((struct in_addr *) host->h_addr_list[0]));

    // Create socket
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("\n\nsocket failed\n\n");
        return -1;
    }
    printf("\n\n%s\n","Peer Socket created" );

    
    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(TRACKER_PORT);




   // Convert IPv4 addresses from IP to binary form and pack it
    if(inet_pton(AF_INET, ip, &address.sin_addr)<=0) 
    {
        printf("\nInvalid Tracker address to connect \n");
        return -1;
    }

    //Connect 
    if (connect(sock_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        printf("\nConnection to Tracker Failed \n");
        return -1;
    }
    tracker_connection = sock_fd;
    printf("%s\n","Peer connected to Tracker" );

    talkto_tracker();


    printf("%s\n","Ending peer" );
    peer_stop();

	
}

void talkto_tracker(){
    
    //First register
    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    segtosend->type = REGISTER;
    send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
    free(segtosend);

    //Create an heartbeat thread (keepAlive)
    pthread_t heartbeat_thread;
    pthread_create(&heartbeat_thread, NULL, keepAlive, (void *) tracker_connection);

    
    ptp_tracker_t* receivedseg = malloc(sizeof(ptp_tracker_t) );
    //Keep receiving data from tracker
    while( (read( tracker_connection , receivedseg, sizeof(ptp_tracker_t))) > 0 ){
        printf("%s\n","Message received from tracker" );
        printf("Received interval is %d\n",receivedseg->interval );
        interval = receivedseg->interval;
        piece_len = receivedseg->piece_len;

    }

}

void* keepAlive(void* arg) {

    ptp_peer_t* segtosend = malloc(sizeof(ptp_peer_t) );
    //Until connected to tracker
    while (tracker_connection >=0 ){
        segtosend->type = KEEP_ALIVE;
        //segtosend.peer_ip = 
        if (interval > 0){
            send(tracker_connection , segtosend , sizeof(ptp_peer_t), 0 );
            sleep(interval);
        } else {
            //If sleep interval is not defined, default is 5 seconds
            sleep(5);
        }

    }



    }


void peer_stop(){
    if (tracker_connection >=0){
        close(tracker_connection);
        tracker_connection = -1;
    }
    printf("Exiting Peer\n");
}
