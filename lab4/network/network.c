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
#include "../common/seg.h"
#include "../topology/topology.h"
#include "network.h"
#include "nbrcosttable.h"
#include "dvtable.h"
#include "routing_table.h"

/**************** constants ****************/
#define NETWORK_WAITTIME 60

/**************** global variables ****************/
int overlay_connection; 		//connection to the overlay
int transport_connection; 	//connection to the transport
nbr_cost_entry_t *nbr_cost_table; //neighbor cost table
dv_t *dv_table; 									//distance vector table
pthread_mutex_t *dv_mutex; 				//dvtable mutex
routingtable_t *routing_table; 		//routing table
pthread_mutex_t *routingtable_mutex; //routing_table mutex

/**************** local function prototypes ****************/
int connectToOverlay(); 
void *routeupdate_daemon(void *arg); 
void *pkthandler(void *arg); 
void waitTransport(); 
void network_stop(); 

/**************** main function ****************/
/* TODO: entry point for the network: 
 *  1) initialize neighbor cost table, distance vector table, mutex
 *     routing table, connections to overlay and transport
 *  2) print out the three tables
 *  3) set up signal handler for SIGINT 
 * 	4) set up overlay connection 
 *  5) create packet handling thread
 *  6) create route update thread
 *  7) wait NETWORK_WAITTIME for routes to be established and then 
 *     print routing table
 *  8) wait for the MRT process to connect (waitTransport())
 */
int main(int argc, char *argv[]) {

	printf("network layer is starting, pls wait...\n");
	nbr_cost_table = nbrcosttable_create();
	dv_table = dvtable_create();
	routing_table = routingtable_create();
	nbrcosttable_print(nbr_cost_table);
	dvtable_print(dv_table);
	routingtable_print(routing_table);
	/* register a signal handler which is sued to terminate the process */
	signal(SIGINT, network_stop);

	// mutex
	dv_mutex = malloc(sizeof(*dv_mutex));
	pthread_mutex_init(dv_mutex, NULL);
	routingtable_mutex = malloc(sizeof(*routingtable_mutex));
	pthread_mutex_init(routingtable_mutex, NULL);

	// connections
	overlay_connection = -1;
	transport_connection = -1;

	// connect to overlay
	overlay_connection = connectToOverlay();
	if(overlay_connection == -1) perror("try to connect to overlay\n");

	// threads
	pthread_t pkt_handle;
	pthread_t route_update;
	pthread_create(&pkt_handle, NULL, pkthandler, (void *)0);
	pthread_create(&route_update, NULL, routeupdate_daemon, (void *)0);

	
	sleep(NETWORK_WAITTIME);
	// wait connection from MRT process
	printf("final dvtable + routingtable\n");
	dvtable_print(dv_table);
	routingtable_print(routing_table);
	printf("waiting for connection from MRT process...\n");
	
	waitTransport();
	
	

	return 0;
}

/**************** local functions *************/

// This function is used to for the network layer process to 
// connect to the local overlay process on port OVERLAY_PORT. 
// return connection descriptor if success, -1 otherwise. 
// Pseudocode
// 1) Fill in sockaddr_in for socket
// 2) Create socket
// 3) return the socket descriptor 
int connectToOverlay() {
	struct sockaddr_in servaddr;
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(OVERLAY_PORT);
	int overlay_conn = socket(AF_INET, SOCK_STREAM, 0);
	if (overlay_conn < 0)
		return -1;
	if (connect(overlay_conn, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)
		return -1;

	// successfully connected
	return overlay_conn;
}

// TODO: This thread handles incoming packets from the ON process.   
// Pseudocode
// 1) while recv packet from overlay connection
// 2) 	if it is a ROUTE_UPDATE packet
// 3) 		fetch data (distance vector) and src node ID
// 4) 		lock dv table and update dv table
// 5) 		update routing table if necessary with lock
// 6)     release locks
// 7)   if it is a MNP packet to itself 
// 8)     forward it to MRT layer w/ forwardsegToMRT()
// 9) 	if it is a MNP packet to others
// 10)    find the node ID of the next hop based on the routing table
// 11)    send it to the next hop w/ overlay_sendpkt()
// 2) close overlay conn
// 3) exit thread
void *pkthandler(void *arg) {
	// initialize some variables
	struct packet pkt;
	int myID = topology_getMyNodeID();
	int nextNodeID;
	while(overlay_recvpkt(&pkt, overlay_connection) == 1){

		if(pkt.header.type == ROUTE_UPDATE){
			// obtain route update from pkt
			struct pktrt *ru;
			ru = (struct pktrt*)pkt.data;
			int src_nodeID = pkt.header.src_nodeID;
			int entryNum = ru->entryNum;

			//lock dvtable
			pthread_mutex_lock(dv_mutex);
			// update dv_table
			for(int i = 0; i < entryNum; i++){
				if(ru->entry[i].cost < dvtable_getcost(dv_table, src_nodeID, ru->entry[i].nodeID)){
					if(dvtable_setcost(dv_table, src_nodeID, ru->entry[i].nodeID, ru->entry[i].cost) == -1)
						printf("dvtable_setcos failed!\n");
				}

				int new_dist = dvtable_getcost(dv_table, myID, src_nodeID) + ru->entry[i].cost;
				if(new_dist < dvtable_getcost(dv_table, myID, ru->entry[i].nodeID)){
					if(dvtable_setcost(dv_table, myID, ru->entry[i].nodeID, new_dist) == -1)
						printf("dvtable_setcos failed!\n");

					// update routingtable
					pthread_mutex_lock(routingtable_mutex);
					routingtable_setnextnode(routing_table, ru->entry[i].nodeID, src_nodeID);
					pthread_mutex_unlock(routingtable_mutex);
				}
			}
			pthread_mutex_unlock(dv_mutex);

		}
		else{
			// forward to MRT
			if(pkt.header.dest_nodeID == myID){
				printf("forwardsegToMRT\n");
				struct segment seg;
				seg = *(struct segment *)&pkt.data;
				forwardsegToMRT(transport_connection, pkt.header.src_nodeID, &seg);
			}
			// forward to next node
			else{

				routingtable_entry_t *head;
				head = routing_table->hash[makehash(pkt.header.dest_nodeID)];
				while(head->next != NULL){
					head = head->next;
					if(head->destNodeID == pkt.header.dest_nodeID){
						nextNodeID = head->nextNodeID;
						break;
					}
				}
				nextNodeID = head->nextNodeID;
				printf("foward to %i\n", nextNodeID);
				overlay_sendpkt(nextNodeID, &pkt, overlay_connection);
			}
		}
	}
	close(overlay_connection);
	overlay_connection = -1;

	pthread_exit(NULL);
}

// TODO: This thread sends out route update packets every 
// ROUTEUPDATE_INTERVAL. The route update packet contains this 
// node's distance vector. 
// Broadcasting is done by set the dest_nodeID in packet header as 
// BROADCAST_NODEID and use overlay_sendpkt() to send it out. 
// Pseudocode
// 1) get my node ID and number of neighbors
// 2) while(1)
//    Fill in mnp_pkt header with myNodeID, BROADCAST_NODEID and 
//			ROUTE_UPDATE
//    Cast the MNP packet data as pkt_routeupdate_t type, set the 
//      entryNum as the number of neighbors
// 		Lock the dv table, put distance vector into the packet data
//    Unlock the dv table
//    Set the length in packet header: sizeof(entryNum) + entryNum*
//      sizeof(routeupdate_entry_t)
//    if(overlay_sendpkt(BROADCAST_NODEID,&ru,overlay_conn < 0)
//      close(overlay_conn)
//      exit
//    Sleep ROUTEUPDATE_INTERVAL
void *routeupdate_daemon(void *arg) {
	// initialize some variables
	int myID = topology_getMyNodeID();
	int nbr_num = topology_getNbrNum();
	int node_num = topology_getNodeNum();
	struct packet pkt;
	// send broadcast every ROUTEUPDATE_INTERVAL
	while(1){
		// initialize packet
		pkt.header.src_nodeID = myID;
		pkt.header.dest_nodeID = BROADCAST_NODEID;
		pkt.header.type = ROUTE_UPDATE;

		struct pktrt *ru;
		ru = (struct pktrt *)&pkt.data;
		ru->entryNum = node_num;

		// store dv_table into route update
		pthread_mutex_lock(dv_mutex);
		for(int i = 0; i <= node_num; i++){
			struct routeupdate_entry ru_entry;
			ru_entry.nodeID = dv_table[nbr_num].dvEntry[i].nodeID;
			ru_entry.cost = dv_table[nbr_num].dvEntry[i].cost;
			ru->entry[i] = ru_entry; 
		}
		pthread_mutex_unlock(dv_mutex);
		pkt.header.length = sizeof(ru->entryNum) + ru->entryNum*sizeof(routeupdate_entry_t);

		// send route update broadcast
		if(overlay_sendpkt(BROADCAST_NODEID, &pkt, overlay_connection) < 0){
			close(overlay_connection);
			exit(9);
		}
		sleep(ROUTEUPDATE_INTERVAL);
	}
}

// TODO: this function opens a port on NETWORK_PORT and waits for 
// the TCP connection from local MRT process. 
// Pseudocode
// 1) create a socket listening on NETWORK_PORT
// 2) while (1)
// 3)   accept an connection from local MRT process
// 4) 	while (getsegToSend()) keep receiving segment and 
//      destination node ID from MRT
// 5) 		encapsulate the segment into a MNP packet
// 6)     find the node ID of the next hop based on the routing table
// 7)     send the packet to next hop using overlay_sendpkt()
// 8)   close the connection to local MRT
void waitTransport() {
	// some variables
	int dest_NodeID;
	int next_NodeID;
	int myID = topology_getMyNodeID();
	struct segment seg;
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
	server.sin_port = htons(NETWORK_PORT);
	if (bind(list_sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding socket name");
		exit(2);
	}
	listen(list_sock, 10);
	// mrt connection
	while(1){

		struct sockaddr_in cli_addr;
		socklen_t cli_len;
		cli_len = sizeof cli_addr;
		
		transport_connection = accept(list_sock, (struct sockaddr *)&cli_addr,&cli_len);
		if (transport_connection == -1){
			perror("accept");
			continue;
		}
		else{
			// continue sending segments from mrt to overlay
			while(getsegToSend(transport_connection, &dest_NodeID, &seg) == 1){

				printf("getsegToSend\n");
				// initialize new pkt
				struct packet pkt;
				memcpy(pkt.data, &seg, sizeof(seg));
				pkt.header.src_nodeID = myID;
				pkt.header.dest_nodeID = dest_NodeID;
				pkt.header.type = MNP;
				pkt.header.length = sizeof(seg);

				// find next_node in routing table
				routingtable_entry_t *head;
				head = routing_table->hash[makehash(pkt.header.dest_nodeID)];
				while(head->next != NULL){
					if(head->destNodeID == pkt.header.dest_nodeID){
						next_NodeID = head->nextNodeID;
						break;
					}
					head = head->next;
				}
				next_NodeID = head->nextNodeID;
				printf("overlay_sendpkt %i\n", pkt.header.dest_nodeID);
				overlay_sendpkt(next_NodeID, &pkt, overlay_connection);
			}
		}
		close(transport_connection);
		transport_connection = -1;
	}
}

// TODO: This function stops the MNP process. It closes all the 
// connections and frees all the dynamically allocated memory. 
// It is called when the MNP process receives a signal SIGINT.
// 1) close overlay connection if it exists
// 2) close the connection to MRT if it exists
// 3) destroy tables, free mutex
// 2) exit
void network_stop() {
	// destroy and free everything
	nbrcosttable_destroy(nbr_cost_table);
	dvtable_destroy(dv_table);
	routingtable_destroy(routing_table);
	if(overlay_connection == 1) close(overlay_connection);
	if(transport_connection == 1) close(transport_connection);
	free(dv_mutex);
	free(routingtable_mutex);
}

