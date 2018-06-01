/*
 * overlay/overlay.c: implementation of an ON process. It has 
 * following steps: 
 * 1. connect to all neighbors
 * 2. start listen_to_neighbor threads, each of which keeps 
 *    receiving packets from a neighbor and forwarding the received 
 * 		packets to the MNP process.
 * 3. waits for the connection from MNP process. 
 * 4. after connecting to a MNP process, keep receiving 
 *    sendpkt_arg_t structures from the MNP process and sending the 
 *    received packets out to the overlay network.
 *
 * CS60, March 2018. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <sys/utsname.h>
#include <assert.h>

#include "../common/constants.h"
#include "../common/pkt.h"
#include "../topology/topology.h"
#include "overlay.h"
#include "neighbortable.h"

/**************** local constants ****************/
// start the ON processes on all the overlay hosts within this period of time
#define OVERLAY_START_DELAY 60

/**************** global variables ****************/
nbr_entry_t* nt; // neighbor table
int network_conn; // TCP connection

/**************** local function prototypes ****************/
void* waitNbrs(void* arg); 
int connectNbrs(); 
void* listen_to_neighbor(void* arg); 
void waitNetwork(); 
void overlay_stop(); 

/**************** main function ****************/
// entry point of the overlay 
int main() {
	/* start overlay initialization */
	printf("Overlay: Node %d initializing...\n", topology_getMyNodeID());

	/* create a neighbor table */
	nt = nt_create();
	/* initialize network_conn to -1, means no MNP process is connected yet */
	network_conn = -1;

	/* register a signal handler which is sued to terminate the process */
	signal(SIGINT, overlay_stop);

	/* print out all the neighbors */
	int nbrNum = topology_getNbrNum();
	for (int i = 0; i < nbrNum; i++) {
		printf("Overlay: neighbor %d:%d\n", i + 1, nt[i].nodeID);
	}

	/* start the waitNbrs thread to wait for incoming connections from neighbors with larger node IDs */
	pthread_t waitNbrs_thread;
	pthread_create(&waitNbrs_thread, NULL, waitNbrs, (void *) 0);

	/* wait for other nodes to start */
	sleep(OVERLAY_START_DELAY);

	/* connect to neighbors with smaller node IDs */
	connectNbrs();

	/* wait for waitNbrs thread to return */
	pthread_join(waitNbrs_thread, NULL);

	/* at this point, all connections to the neighbors are created */

	/* create threads listening to all the neighbors */
	for (int i = 0; i < nbrNum; i++) {
		int *idx = (int *) malloc(sizeof(int));
		*idx = i;
		pthread_t nbr_listen_thread;
		pthread_create(&nbr_listen_thread, NULL, listen_to_neighbor, (void *) idx);
	}
	printf("Overlay: node initialized...\n");
	printf("Overlay: waiting for connection from MNP process...\n");

	/* waiting for connection from MNP process */
	waitNetwork();
}

/**************** functions ****************/
// TODO: thread function waiting for incoming connections from 
// neighbors with larger node IDs
// 1) Get number of neighbors, nbrNum
// 2) Get my node ID, myNodeID
// 3) Count number of incoming neighbors, incoming_neighbors
// 4) Create socket to listen on at port CONNECTION_PORT
// 5) while(incoming_neighbors > 0)
//      accept connection on socket
//      get id of neighbor, nbrID
//      add nbrID and connection to neighbor table
// 6) close the listening socket
void* waitNbrs(void* arg) {
	//get number of neighbors
	int nbrNum = topology_getNbrNum();
	if(nbrNum<0){
		pthread_exit(NULL);
	}

	//get my node ID
	int myNodeID = topology_getMyNodeID();
	if (myNodeID<0){
		pthread_exit(NULL);
	}

	//count the number of incoming neighbors
	int num_of_incoming_nbr = 0;
	int *nbrArr = topology_getNbrArray();
	if (nbrArr==NULL){
		pthread_exit(NULL);
	}
	for (int i = 0; i < nbrNum; ++i){
		if (nbrArr[i] > myNodeID){
			num_of_incoming_nbr++;
		}
	}

	//Create socket to listen
	int main_socket= socket(AF_INET, SOCK_STREAM, 0);
	if(main_socket<0){
		perror("socket opening");
		pthread_exit(NULL);
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(CONNECTION_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(main_socket, (struct sockaddr *)&server, sizeof(server))<0){
		perror("binding socket");
		pthread_exit(NULL);
	}
	printf("Listening at port %d\n", ntohs(server.sin_port));

	if(listen(main_socket, MAX_NODE_NUM)<0){
		perror("listen");
		pthread_exit(NULL);
	}

	struct sockaddr_in cli_addr;
	int clilen=sizeof(cli_addr);

	while(num_of_incoming_nbr >0){
		//pick a connection and descibed it with now_socket
		int now_socket = accept(main_socket, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen);
		if(now_socket==-1){
			perror("accept");
			continue;
		}

		//print the client ip addresss
		char client_ip[INET_ADDRSTRLEN];
		inet_ntop( AF_INET, &(cli_addr.sin_addr), client_ip, INET_ADDRSTRLEN );
		printf("Neighbor %s is connected in! Remaining neighbors: %d!\n", client_ip, num_of_incoming_nbr);

		//get neighbor id
		int nbr_id = topology_getNodeIDfromip(&(cli_addr.sin_addr));

		//fill the table
		if(nt==NULL){
			fprintf(stderr, "nt is null [in waitNbrs]\n");
			pthread_exit(NULL);
		}
		for (int i = 0; i < nbrNum; ++i){
			if (nt[i].nodeID == nbr_id){
				nt[i].conn = now_socket;
				break;
			}
		}

		num_of_incoming_nbr--;

	}

	//free resources
	close(main_socket);
	free(nbrArr);
	pthread_detach(pthread_self());

	pthread_exit(NULL);
}

// TODO: connect to neighbors with smaller node IDs
// 1) nbrNum = get neighbor num
// 2) myNodeID = getMyNodeID
// 3) for each neighbor
//      if myNodeID > nt[i].nodeID
//        make a socket on CONNECTION_PORT and connect to it
//        add this neighbor to neighbor table with connection
// 4) return 1
int connectNbrs() {
	//get number of neighbors
	int nbrNum = topology_getNbrNum();
	if(nbrNum<0){
		return -1;
	}

	//get my node ID
	int myNodeID = topology_getMyNodeID();
	if (myNodeID<0){
		return -1;
	}

	//connect to neighbors
	for (int i = 0; i < nbrNum; ++i){
		if (nt[i].nodeID < myNodeID){
			int out_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (out_socket<0){
				perror("Socket openning error");
				return -1;
			}

			//neighbor address intialization
			struct sockaddr_in server;
			server.sin_family = AF_INET;
			server.sin_port = htons(CONNECTION_PORT);
			server.sin_addr.s_addr = nt[i].nodeIP;

			//connect to neighbor
			if(connect(out_socket, (struct sockaddr*)&server, sizeof(server))<0){
				perror("connectting socket");
				return -1;
			}
			printf("Connected to neighbor %d!\n", nt[i].nodeID);

			//add the connection in table
			nt[i].conn = out_socket;
		}
	}

	return 1;
}

// TODO: thread function for listening to all the neighbors
// 1) while(receive packet on neighbor connection)
//      foward packet to MNP
// 2) close neighbor conn
// 3) kill thread
void* listen_to_neighbor(void* arg) {
	//check input
	if (arg==NULL){
		pthread_exit(NULL);
	}
	int idx = *((int *)arg);
	free(arg);

	int nbrNum = topology_getNbrNum();
	if(nbrNum<0){
		pthread_exit(NULL);
	}
	if (idx >= nbrNum){ //check if it's valid
		pthread_exit(NULL);
	}

	//get the socket descriptor out
	int socket=-1;
	if(nt!=NULL) //it might be null if in this moment, overlay_stop() gets called
		socket = nt[idx].conn;

	//collecting packet comming in
	mnp_pkt_t packet;
	while(recvpkt(&packet , socket)==1){
		
		if(forwardpktToMNP(&packet, network_conn)<0){
			fprintf(stderr, "forwardpktToMNP fail [in listen_to_neighbor]\n");
		}		

		memset(&packet, 0, sizeof(packet));
	}

	//close neighbor conn
	close(socket);
	if(nt!=NULL) //it might be null if in this moment, overlay_stop() gets called
		nt[idx].conn=-1; //for not close it again in overlay_stop()

	//free soucrce
	pthread_detach(pthread_self());

	pthread_exit(NULL);
}

// TODO: wait for connection from MNP process 
// 1) Create socket on OVERLAY_PORT to listen to
// 2) while (1)
//      net_conn = accept connection on socket
//      while(get packets to send on net_conn)
//        nbrNum = get neighbor num
//        if(nextNode == BROADCAST_NODEID)
//          for each nbr
//            if nt[i].conn >= 0
//              sendpkt(pkt,nt[i].conn)
//        else
//          for each nbr
//            if(nt[i].nodeID == nextNodeID && nt[i].conn >= 0)
//              sendpkt(pkt,nt[i].conn)
//      close(net_conn)
void waitNetwork() {
	//build socket to wait for connection
	int main_socket= socket(AF_INET, SOCK_STREAM, 0);
	if(main_socket<0){
		perror("socket opening");
		pthread_exit(NULL);
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(OVERLAY_PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(main_socket, (struct sockaddr *)&server, sizeof(server))<0){
		perror("binding socket");
		return;
	}
	printf("Listening at port %d\n", ntohs(server.sin_port));

	if(listen(main_socket, MAX_NODE_NUM)<0){
		perror("listen");
		return;
	}

	//wait for connection and sending packet from MNP if connected
	struct sockaddr_in cli_addr;
	int clilen=sizeof(cli_addr);
	memset(&cli_addr, 0, clilen);
	while(1){
		//pick a connection and descibed it with network_conn
		network_conn = accept(main_socket, (struct sockaddr *)&cli_addr, (socklen_t*)&clilen);
		if(network_conn==-1){
			perror("accept");
			continue;
		}
		mnp_pkt_t packet;
		int nextNode=-1;
		int nbrNum = topology_getNbrNum();
		if(nt==NULL){
			fprintf(stderr, "nt is null [in waitNetwork]\n");
			break;
		}
		while(getpktToSend(&packet, &nextNode, network_conn)==1){
			if (nextNode==BROADCAST_NODEID){//broadcast
				for (int i = 0; i < nbrNum; ++i){
					if (nt[i].conn >=0 ){
						sendpkt(&packet, nt[i].conn);
					}
				}
			}else{//single-cast
				for (int i = 0; i < nbrNum; ++i){
					if (nt[i].nodeID == nextNode && nt[i].conn >=0 ){
						sendpkt(&packet, nt[i].conn);
					}
				}
			}

			//for next iteration
			memset(&packet, 0, sizeof(packet));
			memset(&cli_addr, 0, clilen);
			nextNode=-1;
		}
		
		//free resources
		close(network_conn);
		network_conn=-1;
	}

	//free resouces
	close(main_socket);//seems will never reach here
}

// TODO: 
// 1) if(net_conn != -1)
//      close(net_conn)
// 2) destory neighbor table
// 3) exit(0)
void overlay_stop() {
	//close connection to MNP
	if (network_conn!=-1){
		close(network_conn);
	}

	//if nt is null, exit directly
	if (nt==NULL){
		exit(0);
	}

	//close all connection 
	int nbrNum = topology_getNbrNum();
	for (int i = 0; i < nbrNum; ++i){
		if (nt[i].conn!=-1){  // if it's not in close state
			close(nt[i].conn);
		}
	}

	//free neighbor table  [connection to neighbors should be closed in listen_to_nbr]
	free(nt);

	exit(0);
}


