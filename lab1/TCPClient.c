// general structure from cs50 sample code

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define BUFSIZE 1024
#define LISTEN_BACKLOG 5

int main(const int argc, char *argv[]){

	char *hostname;
	char *portt;
	int port;
	char *msg;
	char buf[BUFSIZE];
	int bytes_read;

	// check user input format
	if (argc != 4){
		printf("input format: \n");
		fprintf(stderr, "./TCPClient hostname port message");
		printf("\n");
		exit(1);
	}
	else{
		hostname = argv[1];
		portt = argv[2];
		port = atoi(argv[2]);
		msg = argv[3];
	}
	
	// master socket
	int comm_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (comm_sock < 0){
		perror("opening socket");
		exit(2);
	}

	// user get addrinfo to connect to server
	struct addrinfo hints, *info, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(hostname, portt, &hints, &info) < 0){
		printf("getaddrinfo error\n");
		exit(9);
	}

	for (p = info; p != NULL; p = p->ai_next){
		comm_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);

		if (connect(comm_sock, p->ai_addr, p->ai_addrlen) == -1){
			close(comm_sock);
			continue;
		}
		break;
	}

	if (p == NULL){
		perror("no connection");
		exit(10);
	}
	printf("Connected!\n");
	// uncomment this part to test server_v4
	//sleep(5);

	// write to server
	printf("sent!!!");
	if (write(comm_sock, msg, sizeof(msg)) < 0){
		perror("writing on stream socket");
		exit(6);
	}

	// fill buffer with nulls
	memset(buf, 0, sizeof(buf)); 
	if ((bytes_read = read(comm_sock, buf, BUFSIZE-1)) < 0)
		perror("reading stream message");
	if (bytes_read == 0)
		printf("Ending connection\n");
	else{
		printf("Received: %s\n", buf);
	}

	//sleep(10);
	close(comm_sock);

	return 0;
}