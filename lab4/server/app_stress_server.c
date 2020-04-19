/*
 * server/app_stress_server.c -- implementation of the stress test 
 * server application. It has following steps:
 * 1. connects to local MNP process; 
 * 2. initializes the MRT client by calling mrt_server_init(), 
 * 		creates a socket and accepts client request by calling 
 * 		mrt_server_sock() and mrt_server_connect(); 
 * 3. receives data and saves it to receivedtext.txt file; 
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
// port numbers of the overlay connection
#define CLIENTPORT1 87
#define SVRPORT1 88

// after the received file data is saved, the server waits WAITTIME seconds, and then closes the overlay_connection
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

	//initialize MRT server
	mrt_server_init(network_conn);

	// create a MRT server sock at port SVRPORT1 
	int sockfd = mrt_server_sock(SVRPORT1);
	if(sockfd < 0) {
		fprintf(stderr, "can't create MRT server\n");
		exit(1);
	}
	// listen and accept overlay_connection from a MRT client
	mrt_server_accept(sockfd);
	printf("Server: accepted connection\n");

	// receive the file size first and then receive the file data
	int fileLen;
	mrt_server_recv(sockfd,&fileLen,sizeof(int));
	printf("Server: received file length\n");
	char* buf = (char*) malloc(fileLen);
	mrt_server_recv(sockfd,buf,fileLen);
	printf("Server: received file\n");

	// save the received file data in receivedtext.txt
	printf("Server: saving file\n");
	FILE* f = fopen("receivedtext.txt","a");
	if (f == NULL) {
		fprintf(stderr, "failed to open file\n");
		exit(2);
	}
	fwrite(buf,fileLen,1,f);
	fclose(f);
	free(buf);

	// wait for a while
	sleep(WAITTIME);

	// close MRT server 
	if(mrt_server_close(sockfd) < 0) {
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
