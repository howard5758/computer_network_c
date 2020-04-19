/*
 * client/app_stress_client.c -- implementation of the stress test 
 * client application. It has following steps:
 * 1. connects to local MNP process; 
 * 2. initializes the MRT client by calling mrt_client_init(), 
 * 		which creates a socket and connects the server by calling 
 * 		mrt_client_sock() and mrt_client_connect(); 
 * 3. sends text data from file send_this_text.txt to the server; 
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
#include <unistd.h>			// sleep()
#include "../common/constants.h"
#include "../topology/topology.h"
#include "mrt_client.h"

/**************** local constants ****************/
// port numbers of the overlay connection
#define CLIENTPORT1 87
#define SVRPORT1 88

// after connecting to the local MNP process, wait STARTDELAY for server to start.
#define STARTDELAY 1

// after the file is sent, the client waits for WAITTIME seconds, 
// and then closes the connection
#define WAITTIME 1

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

	// choose a host to connect to
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
	
	// get sampletext.txt file length, create a buffer and read the 
	// file data in
	FILE *f = fopen("send_this_text.txt","r");
	if(f == NULL) {
		fprintf(stderr, "failed to open file\n");
		exit(2);
	}
	fseek(f,0,SEEK_END);
	int fileLen = ftell(f);
	fseek(f,0,SEEK_SET);
	char *buffer = (char*)malloc(fileLen);
	fread(buffer,fileLen,1,f);
	fclose(f);

	// send file length first, then send the whole file
	mrt_client_send(sockfd,&fileLen,sizeof(int));
	printf("Client: start sending actual file\n");
  mrt_client_send(sockfd, buffer, fileLen);
	free(buffer);
	printf("Client: done with sending actual file\n");

	// wait for a while and close the connections
	sleep(WAITTIME);

	printf("Client: start disconnecting\n");
	if(mrt_client_disconnect(sockfd) < 0) {
		fprintf(stderr, "fail to disconnect from MRT server\n");
		exit(1);
	}
	if(mrt_client_close(sockfd) < 0) {
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
