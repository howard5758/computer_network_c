/*
 * server/app_simple_server.c -- implementation of the simple server 
 * application. It has following steps:
 * 1. starts the overlay by creating a TCP connection between 
 * 		the server and client; 
 * 2. initializes the MRT server by calling mrt_server_init(), 
 * 		creates 2 sockets and accepts client connections by 
 *    calling mrt_server_sock() and mrt_server_accept() twice; 
 * 3. receives short strings from the client from two connections; 
 * 4. closes the sockets by calling mrt_server_close(), where 
 *    overlay is stopped by calling overlay_end(). 
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
int overlay_start(); 
void overlay_stop(int overlay_conn); 

/**************** main function ****************/
int main() {
	// random seed for segment loss
	srand(time(NULL));

	// start overlay and get the overlay TCP socket descriptor
	int overlay_conn = overlay_start();
	if(overlay_conn < 0) {
		fprintf(stderr, "can not start overlay\n");
	}

	// initialize MRT server
	mrt_server_init(overlay_conn);

	printf("server_init\n");

	// create a MRT server sock at port SVRPORT1 
	int sockfd = mrt_server_sock(SVRPORT1);
	if(sockfd < 0) {
		fprintf(stderr, "can't create MRT server\n");
		exit(1);
	}
	printf("sock_init\n");
	// listen and accept overlay_connection from a MRT client
	mrt_server_accept(sockfd);
	printf("connected\n");



	// create a MRT server sock at port SVRPORT2
	int sockfd2 = mrt_server_sock(SVRPORT2);
	if(sockfd2 < 0) {
		fprintf(stderr, "can't create MRT server\n");
		exit(1);
	}
	// listen and accept overlay_connection from a MRT client
	mrt_server_accept(sockfd2);
	printf("connected\n");
	//exit(9);


	char buf1[6];
	char buf2[7];
	int i;
	//sleep(10);
	// receive strings from first overlay connection
	//mrt_server_recv(sockfd,buf1,6);
	//printf("recv string: %s from overlay_connection 1\n",buf1);
	//mrt_server_recv(sockfd,buf1,6);
	//printf("recv string: %s from overlay_connection 1\n",buf1);
	//sleep(2);
	//exit(9);
	for(i=0;i<5;i++) {
		mrt_server_recv(sockfd,buf1,6);
		printf("recv string: %s from overlay_connection 1\n",buf1);
	}
	//sleep(2);
	//exit(9);
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

	//stop the overlay
	overlay_stop(overlay_conn);

	return 0;
}

/**************** local functions ****************/

// start the overlay by creating a TCP connection between the 
// client and the server. 
// return the TCP socket descriptor, or -1 if any failures.  
// the returned socket descriptor will be used by MRT to send 
// segments.
int overlay_start() {
	// init server address 
	struct sockaddr_in serverAddr; 
	memset(&serverAddr, 0, sizeof(serverAddr)); // clear the space
	serverAddr.sin_family = AF_INET;		// IPv4 address family
  serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // any incoming interface
  serverAddr.sin_port = htons(OVERLAY_PORT);

  // create socket
  int serverSock = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSock < 0) {
		return -1; 
	}

	// bind, listen, and accept
	if (bind(serverSock, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0) {
  	return -1;  
  } 
  if (listen(serverSock, 1) < 0) {
  	return -1;
  }
  printf("waiting for connection\n");

  struct sockaddr clientAddr; 
  socklen_t clientAddrLen;
  int clientSock = accept(serverSock, (struct sockaddr*) &clientAddr, &clientAddrLen);
  return clientSock; 
}

// stop the overlay by closing the TCP connection between the server
// and the client
void overlay_stop(int overlay_conn) {
	close(overlay_conn);
}
