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
#include <sys/stat.h>

#define IP_LEN 16
#define HEARTBEAT_PORT 3638
#define REGISTER 0
#define HEARTBEAT 1
#define FT_UPDATE 2


/* The packet data structure sending from peer to tracker */
typedef struct segment_peer {
	// protocol length
	int protocol_len;
	// protocol name
	char protocol_name[PROTOCOL_LEN + 1];
	// packet type : register, keep alive, update file table
	int type;
	// reserved space, you could use this space for your convenient, 8 bytes by default
	char reserved[RESERVED_LEN];
	// the peer ip address sending this packet
	char peer_ip[IP_LEN];
	// listening port number in p2p
	int port;
	// the number of files in the local file table -- optional
	int file_table_size;
	// file table of the client -- your own design
	file_t file_table;
}ptp_peer_t;

/* The packet data structure sending from tracker to peer */
typedef struct segment_tracker{
	// time interval that the peer should sending alive message periodically
	int interval;
	// piece length
	int piece_len;
	// file number in the file table -- optional
	int file_table_size;
	// file table of the tracker -- your own design
	file_t file_table;
} ptp_tracker_t;


//File table data structure
typedef struct file_table {
	int size; //file size
	char *name; //file name
	unsigned long int timeStamp, prevTimeStamp; //prevTS is sent assigned by Tracker, 
             												//to let peer know which to request (delta or whole file)
	int status;  // 0: r&w , 1: read-only , 2: deleted , 3: synced
	char * newPeerIp; // peer ip which has this file
	int ipNum; //# of peers which has this file (used for accessing [char *newPeerIp])
	struct file_table  *next;
}file_t;

 
//Constant 
const char trackerIP[IP_LEN];
const int P2PListenPort;

//Global Variable
char monitor_dir[100];
char myIp[IP_LEN];
int alive_pack_interval;
int piece_len ; 

//config
int tracker_socket;

void get_my_ip();
int connect_to(char ip[IP_LEN], int port);
int register_on_tracker();
void *heart_beat_thread(void *arg);
void *p2p_listen_thread(void *arg)

//Main thread:
int main(const int argc, char *argv){
	//---check input
	if(argc == 2 && argv[1] == 'tracker'){
		DoTrackerStuff();
	}
	else if(argc ==  && argv[1] == 'peer'){
		strcpy(monitor_dir, argv[2]);
		strcpy(trackerIP, argv[3]);
		DoPeerStuff();
	}
	else{
		printf("Wrong Input Format\n");
		printf("program peer/tracker monitor_dir trackerIP\n");
	}

	return 0;
}

//Peer Main thread:
void DoPeerStuff(){
	//get myself IP
	get_my_ip();

	//intialize download task list
	download_t *download_list_head = malloc();

	//loop forever
	while(1){
		//connect to tracker
		tracker_socket = connect_to(trackerIP, HEARTBEAT_PORT);
		if(tracker_socket == -1){
			continue;
		}

		//register
		int success = register_on_tracker(); 
		if(success < 0){// if failed, try to register again
			continue;
		}

		//start P2P listen thread
		pthread_t p2p_listen;
		pthread_create(&p2p_listen, NULL, p2p_listen_thread, (void *)0);
		//start file monitor thread
		pthread_t file_monitor;
		pthread_create(&file_monitor, NULL, file_monitor_thread, (void *)0);
		//start alive thread
		pthread_t heart_beat;
		pthread_create(&heart_beat, NULL, heart_beat_thread, (void *)0);

		//loop
		while(success){
			//recevie packet from tracker
			// p2t recv
			ptp_tracker_t packet = ...;
			file_t table = packet.file_table;

			//iterate througth the table
			file_t *head = &table;
			while(head != NULL){
				if(head->name not in directory && head->status == 0){
					//create a download_t
					download_t task  = ;
					//get file size, divide file into pieces, intialize piece_seq_list; calculate remaining_peice
					//set the name, timeStamp, file_type; intialize newPeerIp
					if(newPeerIp num >0)
						add_to_download_task();
				}
				else if (head->name in directory && head->prevTimeStamp > localfile->timeStamp){
					//download full file
					//create a download_t
				}
				else if(head->name in directory && head->prevTimeStamp == localfile->timeStamp){
					//download delta 
					//create a download_t
				}
				else if(head->name in directory && head->status == 1){
					// lock file
					chmod(head->name, S_IRUSR);
				}
				else if(head->name in directory && head->status == 2){
					//delete file
				}

			}


			//iterate through download task
			for (task in download_task){
				if(task is not complete){
					if(task.type is delta){
						//pick the first peer
						thread_download_t p_task  = malloc();
						p_task.task = task;
						p_task.peer_ip = ;
						pthread_create(&thrd1, NULL, p2p_download_thread, (void *) p_task);

					}else if(task.type is full file){
						//get the # of active peers, divide the job to peers (one peer one thread)
						for(every peer){ 
							thread_download_t p_task  = malloc();
							p_task.task = task;
							p_task.peer_ip = ;
							p_task.piece_seq_list = malloc();//initailize it, only consider those (-)
							//change the task.piece_seq_list to (+)
							pthread_create(&thrd1, NULL, p2p_download_thread, (void *) p_task);
						}
					}
				}else{//task is complete
					//resemble the file : if delta ; if full file
					//update file table
					//remove the task from download_list
				}
			}
		}

	}
}

// get my ip
void get_my_ip(){
	struct utsname unameData;
	uname(&unameData);
	struct hostent *hostp = gethostbyname(unameData.nodename);
	if(hostp == NULL){
		fprintf(stderr, "unknown host '%s'\n", unameData.nodename);
    	return -1;
	}
	memcpy(myIp, inet_ntoa((struct in_addr*)hostp->h_addr_list[0]), IP_LEN);
}

// connect to a server
int connect_to(char ip[IP_LEN], int port){

	int socket;
	struct sockaddr_in servaddr;
	socket = socket(AF_INET, SOCK_STREAM, 0);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &(servaddr.sin_addr));

	if (socket < 0)
		return -1;
	if (connect(socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)
		return -1;

	return socket;
}

//Connect to the tracker and do register
int register_on_tracker() {
    //construct register packet
    ptp_peer_t regis_pack;  
    regis_pack.type = REGISTER; 
    regis_pack.peer_ip = myIp; 
    regis_pack.port = P2PListenPort;

    //send it

    //receive accept packet
	ptp_tracker_t accept_pack;

    //get the value
   	alive_pack_interval = ;
    piece_len =  ;

	return 1;
}


//pthread task data structure
typedef struct  pthread_download_task
{
	download_t *task;
	int *piece_seq_list;
	int peer_ip;
}thread_download_t;

//Download Task List data structure
typedef struct download_task {
	char *name; //file name
	unsigned long int timeStamp;
	int remaining_peice;
	int *piece_seq_list; // + for downloading, - for waiting
	piece *piece_list;
	pthread_t *thread_list;

	char * newPeerIp; //peers who have this file
	int file_type; //delta or full file
	pthread_mutex_t task_mutex;
	struct download_task * next;// next task

	char *delta_file;//to store delta_file
}download_t;


int add_to_download_task(download_t **head, download_t task){
	//check if the task already exist
	//if it is, check timestamp, if newly one has newer timestamp, remove old one, go on and add; 
	                       //if newly one has older timestamp, quit (don't add)

	//if not, go on and add


	//add the task in it (link list add)
}

int remove_from_download_task(download_t **head, download_t task){
	//check if the task exist or not

	//cancel all the thread that is still running
	//free memory

	//delete from list
}

//p2p packet data structure
typedef struct p2p_packet{
	char *name; //file name
	int piece_seq_num;
	int file_type;

	char *data;
	int data_length;
}p2p_packet;


void* p2p_download_thread(void *arg){
	thread_download_t * task = (thread_download_t*) arg;
	int isConnectionBreak = 0; // if connection break during download, set it to 1

	//connect to peer (task.peerIp)
	download_socket = ;

	//start download
	if(task.type is delta){
		send_p2p_packet();
		receive_p2p_packet();
		//save the data to task.delta_file
	}else if(task.type is full file){
		for(piece in task.piece_seq_list){
			send_p2p_packet();
			receive_p2p_packet();
			//save the data to task.piece_list
		}
	}

	//clearing
	if(isConnectionBreak){
		//reset some state of the task (for reschedule download)
		//free resources
	}else{
		//free resources
	}
}

void * p2p_upload_thread(void *arg){
	while(receive packet){
		//check the packet 
			//if it want delta, send the whole delta
			//if it want a piece of file, send that piece
	}
}

void *file_monitor_thread(void *arg){
	//connet to tracker
	tracker_socket = ;

	//scan the directory "monitor_dir" && copy if necessary

	//create file table

	//send the file table

	//loop 
	while(1){
		//detect change
			//--block: if the file state is read-only, block it; 
				     //if the file table is synced, block it, change its state to r&w

		//update file table

		//send file table

		//copy the updated file
	}
}

void *heart_beat_thread(void *arg){
	while(1){
		//construct heartbeat packet
	    ptp_peer_t regis_pack;  
	    regis_pack.type = HEARTBEAT; 
	    regis_pack.peer_ip = myIp; 
	    regis_pack.port = P2PListenPort;

	    // p2t send

		sleep(alive_pack_interval);
	}
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


//-------------------------common method-------------------------------------

//send p2p packet
//@param:
int send_p2p_packet(int connection, p2p_packet *packet){
	char bufstart[2];
	char bufmid[2];
	char bufend[2]; 
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufmid[0]= '!';
	bufmid[1]= '^';
	bufend[0] = '!';
	bufend[1] = '#';
	if (send(connection, bufstart, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, packet, sizeof(packet), 0) < 0) {
		return -1;
	}
	if (send(connection, bufmid, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, packet.data, packet.data_length, 0) < 0) {
		return -1;
	}
	if (send(connection, bufend, 2, 0) < 0) {
		return -1;
	}
	return 1;
}


int receive_p2p_packet(int connection, p2p_packet* packet)
{
	char buf[sizeof(p2p_packet) + 2];
	char c;
	int idx = 0;

	char *data;

	/*
	 * state can be 0,1,2,3;
	 * 0 starting point
	 * 1 '!' received
	 * 2 '&' received, start receiving segment
	 * 3 '!' received,
	 * 4 '#' received, finish receiving segment
	 * */
	int state = 0;
	while (recv(connection, &c, 1, 0) > 0) {
		if (state == 0) {
			if (c == '!')
				state = 1;
		} else if (state == 1) {
			if (c == '&')
				state = 2;
			else
				state = 0;
		} else if (state == 2) {
			if (c == '!') {
				buf[idx] = c;
				idx++;
				state = 3;
			} else {
				buf[idx] = c;
				idx++;
			}
		} else if (state == 3) {
			if (c == '^') {
				buf[idx] = c;
				idx++;
				state = 4;
				memcpy(packet, buf, sizeof(p2p_packet));

				idx = 0;
				data  = malloc(sizeof(char) * (packet.data_length+2));
			} else if (c == '!') {
				buf[idx] = c;
				idx++;
			} else {
				buf[idx] = c;
				idx++;
				state = 2;
			}
		} else if (state == 4){
			if(c == '!'){
				data[idx] = c;
				idx++;
				state = 5;
			}else {
				data[idx] = c;
				idx++;
			}
		} else if(state == 5){
			if (c == '#') {
				data[idx] = c;
				idx++;
				state = 0;

				data[packet.data_length-1] = '\0';
				data[packet.data_length-2] = '\0';
				packet.data = data;

				data = NULL;
				idx = 0;

				return 1;
			} else if (c == '!') {
				data[idx] = c;
				idx++;
			} else {
				data[idx] = c;
				idx++;
				state = 4;
			}
		}
	}
	return -1;
}
