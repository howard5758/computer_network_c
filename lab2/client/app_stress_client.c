/*
 * client/app_stress_client.c -- implementation of the stress test 
 * client application. It has following steps:
 * 1. starts the overlay by creating a TCP connection between 
 * 		the server and client; 
 * 2. initializes the MRT client by calling mrt_client_init(), 
 * 		which creates a socket and connects the server by calling 
 * 		mrt_client_sock() and mrt_client_connect(); 
 * 3. sends text data from file send_this_text.txt to the server; 
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
#include <unistd.h>			// sleep()
#include "../common/constants.h"
#include "mrt_client.h"

/**************** local constants ****************/
// port numbers of the overlay connection
#define CLIENTPORT1 87
#define SVRPORT1 88

// after the file is sent, the client waits for WAITTIME seconds, 
// and then closes the connection
#define WAITTIME 1

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

	// create a MRT client sock on port CLIENTPORT1 and connect to MRT 
	// server port SVRPORT1
	int sockfd = mrt_client_sock(CLIENTPORT1);
	if(sockfd < 0) {
		fprintf(stderr, "fail to create MRT client sock");
		exit(1);
	}
	if(mrt_client_connect(sockfd,SVRPORT1) < 0) {
		fprintf(stderr, "fail to connect to MRT server\n");
		exit(1);
	}
	printf("client connected to server, client port:%d, server port %d\n",CLIENTPORT1,SVRPORT1);
	
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
	// get server name
	char hostname[50] = "127.0.0.1";
	//printf("Enter server name to connect:");
	//scanf("%s",hostname);

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
