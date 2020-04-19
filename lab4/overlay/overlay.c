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
#define LISTEN_BACKLOG 10

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

	network_conn = -1;

	/* register a signal handler which is sued to terminate the process */
	signal(SIGINT, overlay_stop);

	/* print out all the neighbors */
	int nbrNum = topology_getNbrNum();
	for (int i = 0; i < nbrNum; i++) {
		printf("Overlay: neighbor %d:%d\n", i + 1, nt[i].nodeID);
	}
	//exit(9);
	/* start the waitNbrs thread to wait for incoming connections from neighbors with larger node IDs */
	pthread_t waitNbrs_thread;
	pthread_create(&waitNbrs_thread, NULL, waitNbrs, (void *) 0);

	/* wait for other nodes to start */
	sleep(OVERLAY_START_DELAY);

	/* connect to neighbors with smaller node IDs */
	connectNbrs();

	/* wait for waitNbrs thread to return */
	pthread_join(waitNbrs_thread, NULL);
	//sleep(5);
	//exit(9);
	/* at this point, all connections to the neighbors are created */

	/* create threads listening to all the neighbors */
	for (int i = 0; i < nbrNum; i++) {
		int *idx = (int *) malloc(sizeof(int));
		*idx = i;
		pthread_t nbr_listen_thread;
		pthread_create(&nbr_listen_thread, NULL, listen_to_neighbor, (void *) idx);
	}
	printf("Overlay: node initialized...\n");
	printf("Overlay: waiting for connection from NP process...\n");

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

	// initialize the variables
	struct sockaddr_in list_server;
	int list_sock, comm_sock;
	int nbr_num = topology_getNbrNum();
	int myID = topology_getMyNodeID();
	int nbrID;
	int big_nbr = 0;
	// count the number of nodes with greater nodeID
	for(int i = 0; i < nbr_num; i++){
		if(nt[i].nodeID > myID) big_nbr++;
	}
	// socket stuff
	list_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (list_sock < 0){
		perror("opening socket stream");
		exit(1);
	}
	list_server.sin_family = AF_INET;
	list_server.sin_addr.s_addr = htonl(INADDR_ANY);
	list_server.sin_port = htons(CONNECTION_PORT);
	if (bind(list_sock, (struct sockaddr *) &list_server, sizeof(list_server))) {
		perror("binding socket name");
		exit(2);
	}
	printf("Listening at port %d\n", ntohs(list_server.sin_port));
	printf("To find your MacOS IP address, ifconfig | grep 'inet '\n");
	listen(list_sock, LISTEN_BACKLOG);
	// try to connect
	while(big_nbr > 0){
		// set size for client address
		struct sockaddr_in cli_addr;
		socklen_t cli_len;
		cli_len = sizeof cli_addr;
		// accept client msg
		comm_sock = accept(list_sock, (struct sockaddr *)&cli_addr,&cli_len);
		if (comm_sock == -1){
			perror("accept");
			continue;
		}
		else{
			// obtain neighbor ID and add to neighbor table
			nbrID = topology_getNodeIDfromip(&cli_addr.sin_addr);
			if(nt_addconn(nt, nbrID, comm_sock) == 0) printf("failed to add %i nbr to nt\n", nbrID);
			else big_nbr = big_nbr - 1;
		}
	}
	close(list_sock);
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
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	int myID = topology_getMyNodeID();
	int comm_sock;
	// connect to nodes with smaller ID
	for(int i = 0; i < nbr_num; i++){
		if(myID > nt[i].nodeID){

			printf("try %i\n", nt[i].nodeID);
			// socket stuff
			comm_sock = socket(AF_INET, SOCK_STREAM, 0);
			if(comm_sock < 0) {
				perror("opening socket");
				exit(2);
			}
			struct sockaddr_in server;
  			server.sin_family = AF_INET;
  			server.sin_port = htons(CONNECTION_PORT);
  			server.sin_addr.s_addr = nt[i].nodeIP;
  			if(connect(comm_sock, (struct sockaddr *)&server, sizeof(server)) < 0){
  				perror("connecting stream socket");
    			exit(4);
  			}
  			// after connected, add to neighbor table
  			if(nt_addconn(nt, nt[i].nodeID, comm_sock) == 0) printf("failed to add %i nbr to nt\n", nt[i].nodeID);
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
	// initialize variables
	int i = *(int*) arg;
	struct packet pkt;
	// continue receiving and forwarding packets to MNP
	while(recvpkt(&pkt, nt[i].conn) == 1){
		//printf("recvpkt %i\n", nt[i].nodeID);
		if(pkt.header.type == MNP){
			printf("recvpkt to %i\n", pkt.header.dest_nodeID);
		}
		forwardpktToMNP(&pkt, network_conn);

	}
	// free and close stuff
	close(nt[i].conn);
	nt[i].conn = -1;
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

	// initialize some variables
	struct packet pkt;
	int next;
	// socket stuff
	struct sockaddr_in server;
	int list_sock;
	list_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (list_sock < 0){
		perror("opening socket stream");
		exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(OVERLAY_PORT);
	if (bind(list_sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding socket name");
		exit(2);
	}
	listen(list_sock, LISTEN_BACKLOG);
	// continue listen for MNP process
	while(1){

		struct sockaddr_in cli_addr;
		socklen_t cli_len;
		cli_len = sizeof cli_addr;
		
		network_conn = accept(list_sock, (struct sockaddr *)&cli_addr,&cli_len);
		if (network_conn == -1){
			perror("accept");
			continue;
		}
		else{
			// get packet from MNP to send
			while(getpktToSend(&pkt, &next, network_conn) == 1){

				int nbr_num = topology_getNbrNum();
				if(next == BROADCAST_NODEID){
					for(int i = 0; i < nbr_num; i++){
						//printf("sendpkt broadcast %i\n", nt[i].nodeID);
						sendpkt(&pkt, nt[i].conn);
					}
				}
				else{
					for(int i = 0; i < nbr_num; i++){
						if(nt[i].nodeID == next && nt[i].conn >= 0){
							printf("sendpkt %i\n", nt[i].nodeID);
							sendpkt(&pkt, nt[i].conn);
						}
					}
				}
			}
			close(network_conn);
		}
	}

}

// TODO: 
// 1) if(net_conn != -1)
//      close(net_conn)
// 2) destory neighbor table
// 3) exit(0)
void overlay_stop() {
	if(network_conn != -1){
		close(network_conn);
		network_conn = -1;
	}
	nt_destroy(nt);
	exit(0);
}


