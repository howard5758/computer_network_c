#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define LISTEN_BACKLOG 5

int main(const int argc, char *argv[]){


	int list_sock, comm_sock;
	struct sockaddr_in server;
	char buf[BUFSIZE];
	int bytes_read;
	struct sockaddr_in cli_addr;
	socklen_t cli_len;

	// get port
	int SERV_PORT;
	if (argc != 2){
		printf("input format: \n");
		fprintf(stderr, "./TCPServer_v1 port");
		printf("\n");
		exit(1);
	}
	else{
		SERV_PORT = atoi(argv[1]);
	}

	// master socket
	list_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (list_sock < 0){
		perror("opening socket stream");
		exit(1);
	}

	// initiate
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

			printf("Client %s connected\n", inet_ntoa(cli_addr.sin_addr));

			// fill buffer with nulls
			memset(buf, 0, sizeof(buf)); 

			if ((bytes_read = read(comm_sock, buf, BUFSIZE-1)) < 0){
				perror("reading stream message");
				continue;
			}
			if (bytes_read == 0){
				printf("Ending connection\n");
				continue;
			}
			else{
				// if read success!
				printf("%s\n", buf);
				// write back to client
				if (write(comm_sock, buf, sizeof(buf)) < 0){
					perror("writing on stream socket");
					continue;
				}
				close(comm_sock);
			}

		}
	}

	return 0;
}