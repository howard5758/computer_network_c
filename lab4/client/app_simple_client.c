/*
 * client/app_simple_client.c -- implementation of the simple client 
 * application. It has following steps:
 * 1. connects to local MNP process; 
 * 2. initializes the MRT client by calling mrt_client_init(), 
 * 		which creates 2 sockets and connects the server by calling 
 * 		mrt_client_sock() and mrt_client_connect() twice; 
 * 3. sends short strings to the server from these two connections; 
 * 4. after some time, disconnects from the server by calling 
 * 		mrt_client_disconnect(); 
 * 5. closes the socket by calling mrt_client_close(), disconnects 
 *    the local MNP process.
 * 
 * Input: none
 * Output: client states
 * 
 * CS60, March 2018. 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include "../common/constants.h"
#include "../topology/topology.h"
#include "mrt_client.h"

/**************** local constants ****************/
// port numbers of two overlay connections
#define CLIENTPORT1 87
#define SVRPORT1 88
#define CLIENTPORT2 89
#define SVRPORT2 90

// after connecting to the MNP process, wait STARTDELAY for server to start
#define STARTDELAY 1

// after the strings are sent, wait WAITTIME seconds, and then close the connections
#define WAITTIME 5


/**************** local function prototypes ****************/
int connectToNetwork();
void disconnectToNetwork(int network_conn); 

/**************** main function ****************/
int main() {
	// random seed for loss rate
	srand(time(NULL));

	// connect to MNP process and get the TCP socket descriptor
	int network_conn = connectToNetwork();
	if (network_conn < 0) {
		fprintf(stderr, "fail to start overlay\n");
		exit(1);
	}

	// initialize MRT client
	mrt_client_init(network_conn);
	sleep(STARTDELAY);

	// choose a node to connect
	char hostname[50];
	printf("Enter server name to connect:");
	scanf("%s", hostname);
	int svr_nodeID = topology_getNodeIDfromname(hostname);
	if (svr_nodeID == -1) {
		fprintf(stderr, "host name error!\n");
		exit(1);
	} else {
		printf("connecting to node %d\n", svr_nodeID);
	}	

	// create a MRT client sock on port CLIENTPORT1 and connect to MRT 
	// server port SVRPORT1
	int sockfd = mrt_client_sock(CLIENTPORT1);
	if(sockfd < 0) {
		fprintf(stderr, "fail to create MRT client sock");
		exit(1);
	}
	if(mrt_client_connect(sockfd, svr_nodeID, SVRPORT1) < 0) {
		fprintf(stderr, "fail to connect to MRT server %s\n", hostname);
		exit(1);
	}
	printf("client connected to server %s, client port:%d, server port %d\n", hostname, CLIENTPORT1, SVRPORT1);
	
	// create a MRT client sock on port CLIENTPORT2 and connect to MRT 
	// server port SVRPORT2
	int sockfd2 = mrt_client_sock(CLIENTPORT2);
	if(sockfd2 < 0) {
		fprintf(stderr, "fail to create MRT client sock");
		exit(1);
	}
	if(mrt_client_connect(sockfd2, svr_nodeID, SVRPORT2) < 0) {
		fprintf(stderr, "fail to connect to MRT server %s\n", hostname);
		exit(1);
	}
	printf("client connected to server %s, client port:%d, server port %d\n", hostname, CLIENTPORT1, SVRPORT1);

	// send strings through the first overlay connection
  char mydata[6] = "hello";
	int i;
	for(i=0;i<5;i++){
		mrt_client_send(sockfd, mydata, 6);
		printf("send string:%s to overlay_connection 1\n",mydata);
  }

	// send strings through the second overlay connection
  char mydata2[7] = "byebye";
	for(i=0;i<5;i++){
		mrt_client_send(sockfd2, mydata2, 7);
		printf("send string:%s to overlay_connection 2\n",mydata2);
  }

	// wait for a while and close the connections
	sleep(WAITTIME);

	if(mrt_client_disconnect(sockfd)<0) {
		fprintf(stderr, "fail to disconnect from MRT server\n");
		exit(1);
	}
	if(mrt_client_close(sockfd)<0) {
		fprintf(stderr, "fail to close MRT client\n");
		exit(1);
	}
	
	if(mrt_client_disconnect(sockfd2)<0) {
		fprintf(stderr, "fail to disconnect from MRT server\n");
		exit(1);
	}
	if(mrt_client_close(sockfd2)<0) {
		fprintf(stderr, "fail to close MRT client\n");
		exit(1);
	}

	// disconnect from the MNP process
	disconnectToNetwork(network_conn);

	return 0; 
}

/**************** local functions ****************/

// This function connects to the local MNP process on port 
// NETWORK_PORT. The TCP socket descriptor returned will be used by 
// MRT to send segments.
// Return the socket description if successful, otherwise -1. 
int connectToNetwork() {
	struct sockaddr_in servaddr;

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(NETWORK_PORT);

	int network_conn = socket(AF_INET, SOCK_STREAM, 0);
	if (network_conn < 0)
		return -1;
	if (connect(network_conn, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)
		return -1;

	// successfully connected
	return network_conn;
}

// This function disconnects from the local MNP process by closing 
// the TCP connection.
void disconnectToNetwork(int network_conn) {
	close(network_conn);
}
