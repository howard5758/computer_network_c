/*
 * client/app_simple_client.c -- implementation of the simple client 
 * application. It has following steps:
 * 1. starts the overlay by creating a TCP connection between 
 * 		the server and client; 
 * 2. initializes the MRT client by calling mrt_client_init(), 
 * 		which creates 2 sockets and connects the server by calling 
 * 		mrt_client_sock() and mrt_client_connect() twice; 
 * 3. sends short strings to the server from these two connections; 
 * 4. after some time, disconnects from the server by calling 
 * 		mrt_client_disconnect(); 
 * 5. closes the socket by calling mrt_client_close(), where overlay 
 * 		is stopped by calling overlay_end(). 
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
#include "mrt_client.h"

/**************** local constants ****************/
// port numbers of two overlay connections
#define CLIENTPORT1 87
#define SVRPORT1 88
#define CLIENTPORT2 89
#define SVRPORT2 90

// after the strings are sent, wait WAITTIME seconds, and then close the connections
#define WAITTIME 5


/**************** local function prototypes ****************/
int overlay_start(); 
void overlay_stop(int overlay_conn); 

/**************** main function ****************/
int main() {
	// random seed for loss rate
	srand(time(NULL));

	// start overlay and get the overlay TCP socket descriptor	
	int overlay_conn = overlay_start();
	if(overlay_conn < 0) {
		fprintf(stderr, "fail to start overlay\n");
		exit(1);
	}

	// initialize MRT client
	mrt_client_init(overlay_conn);
	printf("client init\n");

	// create a MRT client sock on port CLIENTPORT1 and connect to MRT 
	// server port SVRPORT1
	int sockfd = mrt_client_sock(CLIENTPORT1);
	if(sockfd < 0) {
		fprintf(stderr, "fail to create MRT client sock");
		exit(1);
	}
	printf("sock init\n");
	if(mrt_client_connect(sockfd,SVRPORT1) < 0) {
		fprintf(stderr, "fail to connect to MRT server\n");
		exit(1);
	}
	printf("client connected to server, client port:%d, server port %d\n", CLIENTPORT1, SVRPORT1);

	// create a MRT client sock on port CLIENTPORT2 and connect to MRT 
	// server port SVRPORT2
	int sockfd2 = mrt_client_sock(CLIENTPORT2);
	if(sockfd2 < 0) {
		fprintf(stderr, "fail to create MRT client sock");
		exit(1);
	}
	if(mrt_client_connect(sockfd2,SVRPORT2) < 0) {
		fprintf(stderr, "fail to connect to MRT server\n");
		exit(1);
	}
	printf("client connected to server, client port:%d, server port %d\n",CLIENTPORT2, SVRPORT2);

	//exit(9);
	// send strings through the first overlay connection
  	char mydata[6] = "hello";
	int i;
	//mrt_client_send(sockfd, mydata, 6);
	//printf("send string:%s to overlay_connection 1\n",mydata);
	//mrt_client_send(sockfd, mydata, 6);
	//printf("send string:%s to overlay_connection 1\n",mydata);
	//sleep(2);
	//exit(9);

	for(i=0;i<5;i++){
		mrt_client_send(sockfd, mydata, 6);
		printf("send string:%s to overlay_connection 1\n",mydata);
  }
  //sleep(2);
  //exit(9);
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

	// stop the overlay
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
	// get server name
	char hostname[50];
	printf("Enter server name to connect:");
	scanf("%s",hostname);

	// init server address
	struct sockaddr_in server;  // address of the server
	memset(&server, 0, sizeof(server)); // clear the space
	server.sin_family = AF_INET; 
	server.sin_port = htons(OVERLAY_PORT);
	// Look up the hostname specified on command line
  struct hostent *hostp = gethostbyname(hostname); // server hostname
  if (hostp == NULL) {
    fprintf(stderr, "unknown host '%s'\n", hostname);
    exit(3);
  }  
  memcpy(&server.sin_addr, hostp->h_addr_list[0], hostp->h_length);

  // create socket 
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		return -1;
	}

	// connect
	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0){
		return -1;
	}

  return sock;
}

// stop the overlay by closing the TCP connection between the server
// and the client
void overlay_stop(int overlay_conn) {
	close(overlay_conn);
}
