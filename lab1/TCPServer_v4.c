#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFSIZE 1024
#define LISTEN_BACKLOG 5

int main(const int argc, char *argv[]){

	int list_sock, comm_sock, max;
	struct sockaddr_in server;
	char buf[BUFSIZE];
	int bytes_read;
	struct sockaddr_in cli_addr;
	socklen_t cli_len;
	fd_set master;
	fd_set temp;

	// check user inputs
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

	// add master socket to master fd_set
	FD_SET(list_sock, &master);
	// only one at the moment
	max = list_sock;

	while(true){

		// refresh temp everytime
		temp = master;
		// select
		if (select(max+1, &temp, NULL, NULL, NULL) >= 0)
			printf("select ok\n");

		// loop through fd_set
		for (int i = 0; i <= max; i++){
			if(FD_ISSET(i, &temp)){
				
				// master socket, accept new connection from client
				if(i == list_sock){
					cli_len = sizeof cli_addr;
					comm_sock = accept(list_sock, (struct sockaddr *)&cli_addr,&cli_len);
					if (comm_sock == -1){
						perror("accept");
						continue;
					}
					printf("New connection, socket fd is %i, IP is : %s\n", comm_sock, inet_ntoa(cli_addr.sin_addr));
					
					// add to master fd_set
					FD_SET(comm_sock, &master);

					// refresh max
					if (comm_sock > max)
						max = comm_sock;
				}
				// receive msg
				else{
					// same stuff as the other versions
					memset(buf, 0, sizeof(buf)); // fill buffer with nulls
					if ((bytes_read = read(i, buf, BUFSIZE-1)) < 0){
						perror("reading stream message");
						continue;
					}
					if (bytes_read == 0){
						printf("Ending connection\n");
						continue;
					}
					else{
						if (write(i, buf, sizeof(buf)) < 0){
							perror("writing on stream socket");
							exit(3);
						}
					}
					// close and remove from master fd_set
					close(i);
					FD_CLR(i, &master);
				}
			}
		}
	}

	return 0;
}