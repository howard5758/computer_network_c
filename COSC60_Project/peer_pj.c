// 1. read config
// 2. register with tracker
// 3. heartbeat thread
// 4. start file monitor thread
// 5. p2p listen thread
// 6. infinite loop receive tracker file table
int heartbeat_sock, tracker_sock;
tracker_ip

int main(int argc, char *argv[]) {

	// read(config)
	// directory = config.directory
	// trackerIP = config.ip
	// heartbeat_sock = connect_to_tracker(trackerIP, port);
	// start heartbeat thread
	// start file monitor thread
	// start p2plisten thread
	//
	// while(true)
	// 		if(recv(tracker) == 1)
	//			for filename in FT.names
	//				case filename not in directory
	//					download full file
	//					add entry in FT
	//				case filename exist && file seq < tracker file prev seq
	//					download full file
	//					update FT with new seq
	//				case filename exist && file seq == tracker file prev seq
	//					download delta
	//					update FT with new seq
	//				case filename exist && file state == open
	//					set file permission to read only
	//				case filename exist && file state == deleted
	//					delete the file
	//					delete entry from FT
	return 0;
}

int connect_to_tracker(s_addr tracker_ip, int port){
	struct sockaddr_in server; 
	memset(&server, 0, sizeof(server)); 
	memcpy(&server.sin_addr, tracker_ip, tracker_ip_size);
	server.sin_family = AF_INET; 
	server.sin_port = htons(port);

	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
		return -1;

	// connect
	if (connect(sock, (struct sockaddr*) &server, sizeof(server)) < 0)
		return -1;

  	return sock;
}

// heartbeat thread
void *heart_beat(void *arg){
	// while(1)
	// 		create empty pkt
	//		type = heartbeat
	//		while(send(heartbeat_sock, pkt) == 1)
	//			sleep(interval);
	//		heartbeat_sock = connect_to_tracker(tracker_ip, port)
}

// p2p listen thread
void *p2p_listen_thread(void *arg){
	// while true
	// 		select connect
	// 		if(recv == 1)
	// 			obtain filename, chunk num.
	// 			create delta 
	// 			cut delta get chunk
	// 			send chunk
}



