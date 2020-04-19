#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFSIZE 1024
#define LISTEN_BACKLOG 5

//thread handle function
void *thread_handle(void *thread_sock){

	int bytes_read;

	printf("Handled by thead: %lu\n", (unsigned long)pthread_self());

	// same stuff as previous versions
	int sock = *(int*)thread_sock;
	char buf[BUFSIZE];
	memset(buf, 0, sizeof(buf)); // fill buffer with nulls
	if ((bytes_read = read(sock, buf, BUFSIZE-1)) < 0)
		perror("reading stream message");
	if (bytes_read == 0)
			printf("Ending connection\n");
	else{
		if (write(sock, buf, sizeof(buf)) < 0){
			perror("writing on stream socket");
			exit(3);
		}
	}

	// free new_sock memory
	free(thread_sock);
	return NULL;
}

int main(const int argc, char *argv[]){

	// check user inputs
	int list_sock, comm_sock, *new_sock;
	struct sockaddr_in server;
	int SERV_PORT;
	struct sockaddr_in cli_addr;
	socklen_t cli_len;

	if (argc != 2){
		printf("input format: \n");
		fprintf(stderr, "./TCPServer_v3 port");
		printf("\n");
		exit(1);
	}
	else{
		SERV_PORT = atoi(argv[1]);
	}
	
	// initialize listening socket
	list_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (list_sock < 0){
		perror("opening socket stream");
		exit(1);
	}

	// initiate server
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(SERV_PORT);

	// bind 
	if (bind(list_sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding socket name");
		exit(2);
	}
	printf("Listening at port %d\n", ntohs(server.sin_port));
	printf("To find your MacOS IP address, ifconfig | grep 'inet '\n");

	//listen
	listen(list_sock, LISTEN_BACKLOG);
	while(true){
		
		// set size for client address
		cli_len = sizeof cli_addr;
		// accept client msg
		comm_sock = accept(list_sock, (struct sockaddr *)&cli_addr,&cli_len);
		if (comm_sock == -1){
			perror("accept");
			continue;
		}
		else {
			// if connect success, create new thread nad malloc for new_sock
			printf("Client %s connected\n", inet_ntoa(cli_addr.sin_addr));
			pthread_t new_thread;
			new_sock = malloc(1);
			*new_sock = comm_sock;
			pthread_create(&new_thread, NULL, thread_handle, (void*) new_sock);

		}
	}

	return 0;
}
