
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
#include "../common/file.h"

/**************** local functions ****************/
void peer_stop(); 

/**************** global variables ****************/
int tracker_connection = -1;         //connection to the tracker

int main()
{
	int sock_fd;
	struct sockaddr_in address;

    signal(SIGINT, peer_stop);


    // choose an IP address to connect
    char ipadr[50];
    printf("Enter IP address of tracker to connect:");
    scanf("%s", ipadr);

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
    if(inet_pton(AF_INET, ipadr, &address.sin_addr)<=0) 
    {
        printf("\nInvalid address to connect \n");
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

    sleep(100);
    peer_stop();

	
}


void peer_stop(){
    if (tracker_connection >=0){
        close(tracker_connection);
    }
    printf("Exiting Peer\n");
}
