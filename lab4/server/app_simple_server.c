/*
 * server/app_simple_server.c -- implementation of the simple server 
 * application. It has following steps:
 * 1. connects to local MNP process; 
 * 2. initializes the MRT server by calling mrt_server_init(), 
 * 		creates 2 sockets and accepts client connections by 
 *    calling mrt_server_sock() and mrt_server_accept() twice; 
 * 3. receives short strings from the client from two connections; 
 * 4. closes the sockets by calling mrt_server_close(), disconnects 
 *    the local MNP process. 
 * 
 * Input: none
 * Output: server states
 * 
 * CS60, March 2018. 
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include "mrt_server.h"

/**************** local constants ****************/
// port numbers of two overlay connections
#define CLIENTPORT1 87
#define SVRPORT1 88
#define CLIENTPORT2 89
#define SVRPORT2 90

// after the strings are received, the server waits WAITTIME seconds, and then closes the connections
#define WAITTIME 10

/**************** local function prototypes ****************/
int connectToNetwork(); 
void disconnectToNetwork(int network_conn); 

/**************** main function ****************/
int main() {
	// random seed for segment loss
	srand(time(NULL));

	// connect to local MNP process and get the TCP socket descriptor
	int network_conn = connectToNetwork();
	if (network_conn < 0) {
		fprintf(stderr, "can not start overlay\n");
	}

	// initialize MRT server
	mrt_server_init(network_conn);

	// create a MRT server sock at port SVRPORT1 
	int sockfd = mrt_server_sock(SVRPORT1);
	if(sockfd < 0) {
		fprintf(stderr, "can't create MRT server\n");
		exit(1);
	}
	// listen and accept overlay_connection from a MRT client
	mrt_server_accept(sockfd);

	// create a MRT server sock at port SVRPORT2
	int sockfd2 = mrt_server_sock(SVRPORT2);
	if(sockfd2 < 0) {
		fprintf(stderr, "can't create MRT server\n");
		exit(1);
	}
	// listen and accept overlay_connection from a MRT client
	mrt_server_accept(sockfd2);


	char buf1[6];
	char buf2[7];
	int i;
	// receive strings from first overlay connection
	for(i=0;i<5;i++) {
		mrt_server_recv(sockfd,buf1,6);
		printf("recv string: %s from overlay_connection 1\n",buf1);
	}
	// receive strings from second overlay connection
	for(i=0;i<5;i++) {
		mrt_server_recv(sockfd2,buf2,7);
		printf("recv string: %s from overlay_connection 2\n",buf2);
	}

	sleep(WAITTIME);

	// close MRT server 
	if(mrt_server_close(sockfd) < 0) {
		fprintf(stderr, "can't destroy MRT server\n");
		exit(1);
	}				
	if(mrt_server_close(sockfd2) < 0) {
		fprintf(stderr, "can't destroy MRT server\n");
		exit(1);
	}				

	// disconnect from the local MNP process
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
