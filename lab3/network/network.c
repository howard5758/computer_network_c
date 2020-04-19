/*
 * network/network.c: implementation of the MNP process. 
 *
 * CS60, March 2018. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#include <assert.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <unistd.h>

#include "../common/constants.h"
#include "../common/pkt.h"
#include "../topology/topology.h"
#include "network.h"

/**************** global variables ****************/
int overlay_conn; //connection to the overlay

/**************** local function prototypes ****************/
int connectToOverlay(); 
void *routeupdate_daemon(void *arg); 
void *pkthandler(void *arg); 
void network_stop(); 

/**************** main function ****************/
/* TODO: entry point for the network: 
 *  1) set up signal handler for SIGINT
 * 	2) set up overlay connection
 *  3) create packet handling thread
 *  4) create route update thread
 */
pthread_t pkt_handle;
pthread_t route_update;

int main(int argc, char *argv[]) {

	// 1)
	overlay_conn = -1;
	signal(SIGINT, network_stop);

	// 2)
	overlay_conn = connectToOverlay();

	// 3), 4)
	pthread_create(&pkt_handle, NULL, pkthandler, (void *)0);
	pthread_create(&route_update, NULL, routeupdate_daemon, (void *)0);

	while(1){
	}

	return 0;
}

/**************** local functions ****************/

// TODO: this function is used to for the network layer process to 
// connect to the local overlay process on port OVERLAY_PORT. 
// return connection descriptor if success, -1 otherwise. 
// Pseudocode
// 1) Fill in sockaddr_in for socket
// 2) Create socket
// 3) return the socket descriptor 
int connectToOverlay() {
	// all socket stuff
	int comm_sock;
	comm_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(comm_sock < 0) {
				perror("opening socket");
				exit(2);
	}

	struct sockaddr_in server;
  	server.sin_family = AF_INET;
  	server.sin_port = htons(OVERLAY_PORT);

	struct utsname unameData;
	uname(&unameData);
	struct hostent *hostp = gethostbyname(unameData.nodename);
	server.sin_addr.s_addr = ((struct in_addr *)hostp->h_addr_list[0])->s_addr;

	if(connect(comm_sock, (struct sockaddr *)&server, sizeof(server)) < 0){
  		perror("connecting stream socket");
    	exit(4);
  	}

	return comm_sock;
}

// TODO: This thread handles incoming packets from the ON process.
// We will implement the actual packet handling in lab 4. 
// In this lab, it only receives packets.  
// Pseudocode
// 1) while recv packet from overlay connection
//      print where it came from
// 2) close overlay conn
// 3) kill thread
void *pkthandler(void *arg) {
	// receive packet and print source
	struct packet pkt;
	while(overlay_recvpkt(&pkt, overlay_conn) == 1){
		printf("received pkt from node %i\n", pkt.header.src_nodeID);
	}
	close(overlay_conn);
	overlay_conn = -1;
	return NULL;
}

// TODO: This thread sends out route update packets every 
// ROUTEUPDATE_INTERVAL. In this lab, we do not add any data in the 
// packet. In lab 4, we will add distance vector as the packet data. 
// Broadcasting is done by set the dest_nodeID in packet header as 
// BROADCAST_NODEID and use overlay_sendpkt() to send the packet out 
// using BROADCAST_NODEID.  
// Pseudocode
// 1) while(1)
//    Fill in mnp_pkt header with myNodeID, BROADCAST_NODEID and 
//			ROUTE_UPDATE
//    Set mnp_pkt.header.length = 0
//    if(overlay_sendpkt(BROADCAST_NODEID,&ru,overlay_conn < 0)
//      close(overlay_conn)
//      exit
//    Sleep ROUTEUPDATE_INTERVAL
void *routeupdate_daemon(void *arg) {

	struct packet pkt;
	// send broadcast every ROUTEUPDATE_INTERVAL
	while(1){
		pkt.header.src_nodeID = topology_getMyNodeID();
		pkt.header.dest_nodeID = BROADCAST_NODEID;
		pkt.header.length = 0;
		pkt.header.type = ROUTE_UPDATE;
		if(overlay_sendpkt(BROADCAST_NODEID, &pkt, overlay_conn) < 0){
			close(overlay_conn);
			return NULL;
		}
		else{
			printf("broadcast\n");
		}
		sleep(ROUTEUPDATE_INTERVAL);
	}

	return NULL;
}

// TODO: This function stops the MNP process. It closes all the 
// connections and frees all the dynamically allocated memory. 
// It is called when the MNP process receives a signal SIGINT.
// 1) close overlay connection if it exists
// 2) exit
void network_stop() {
	// pthread_join(pkt_handle, NULL);
	// pthread_join(route_update, NULL);
	if(overlay_conn != -1){
		close(overlay_conn);
		overlay_conn = -1;
	}
	exit(9);
}

