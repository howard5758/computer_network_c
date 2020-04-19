#include "./peer.h"
//#include "../common/seg.h"
#include <netdb.h>
#include <sys/socket.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <zconf.h>
#include <errno.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <pthread.h>

#include <dirent.h>
#include <limits.h>
#include <signal.h>

#define LEN_DIR     1024 /* Assuming that the length of directory won't exceed 256 bytes */
#define MAX_EVENTS  1024 /*Max. number of events to process at one go*/
#define LEN_NAME    512 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/


//--------------------Important Note------------------//
//1. piece sequence number starts at one. So while using 
//   it to access one piece of file, use like this: (seqNum - 1)
//---------------------------------------------------//


///=======================Global variables===================///
int inotify_fd;
DIR_MNT_table *mnt_table;
MNT_config *config;
file_table_t *file_table;


//Constant 
const char trackerIP[IP_LEN];
const int P2PListenPort;

// monitor global
int inotify_fd;
DIR_MNT_table *mnt_table;
MNT_config *config;
file_table_t *file_table;

//Global Variable
char monitor_dir[100];
char myIp[IP_LEN];
int alive_pack_interval;
int piece_len ; 

//config
int monitor_socket;
int heartbeat_socket;

file_table_t *file_table;

char config_name[11] =".config";
//Peer Main thread:
//int peer(){
int peer(){
	printf("start peer\n");
	signal(SIGINT, clearAll);
	///get myself IP
	get_my_ip();

	//intialize download task list
	download_t *download_list_head = NULL;

	//loop forever
	while(1){
		//connect to tracker
		struct hostent *host = gethostbyname(TRACKER_ADDR);
		struct sockaddr_in servaddr;
		monitor_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (monitor_socket < 0)
			return -1;
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(MONITOR_PORT);
		memcpy(&servaddr.sin_addr, host->h_addr_list[0], host->h_length);
		if (connect(monitor_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)
			return -1;
		printf("connect to tracker\n");
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
		printf("start sleep\n");
		//sleep to let file monitor thread get my own file table
		sleep(10);
		while(success){
			//recevie packet from tracker
			t2p_seg_t packet;
			receive_t2p_segment(monitor_socket, &packet);
			printf("test receive filetable\n");
			file_table_t tracker_table = packet.filetable;
			printf("/************************ RECEIVED FT ***********************/\n");
			filetable_print(&tracker_table);
			printf("/************************ END RECEIVED FT ***********************/\n");
			//iterate througth the table
			file_t *head = tracker_table.head;

			//scan the received file table
			while(head != NULL){
				file_t *local_file = filetable_find_file(file_table, head->filename);
				if(!local_file && head->status == 0){//full file task
					//create a download_t
					download_t *task  = malloc(sizeof(download_t));
					memcpy(task->filename, head->filename, MAX_FILENAME_LEN); 
					task->timeStamp = head->latest_timestamp;
					task->full_file_length = head->file_size;
					task->total_piece_num = head->file_size/piece_len + ((head->file_size%piece_len==0)? 0: 1); 
					task->remaining_piece = task->total_piece_num;
					task->piece_seq_list = malloc(sizeof(int) * task->total_piece_num);
					for(int i=0; i<task->total_piece_num; i++){
						task->piece_seq_list[i]= -(i+1); //start from one: -1,-2,-3,-4,...
					}
					task->should_thread_all_stop=0; task->peerNum = head->peerNum;
					for (int i = 0; i < head->peerNum; ++i){
						for (int j = 0; j < IP_LEN; ++j){
							task->newPeerIp[i*IP_LEN + j] = head->peerIP[i][j];

						}
					}
					task->file_type = FULL_FILE;

					//add to download task list
					if(add_to_download_task(&download_list_head, task) == 1){
						printf("full file task for %s added, because it doesn't exist locally\n", head->filename);
					}

				}
				else if(local_file && head->status == 2){
					local_file->status = 2;
				 	//delete file
				 	//later
				}
				else if (local_file && head->prev_timestamp > local_file->latest_timestamp){//full file task
					//create a download_t
					download_t *task  = malloc(sizeof(download_t));
					memcpy(task->filename, head->filename, MAX_FILENAME_LEN); 
					task->timeStamp = head->latest_timestamp;
					task->full_file_length = head->file_size;
					task->total_piece_num = head->file_size/piece_len + ((head->file_size%piece_len==0)? 0: 1); 
					task->remaining_piece = task->total_piece_num;
					task->piece_seq_list = malloc(sizeof(int) * task->total_piece_num);
					for(int i=0; i<task->total_piece_num; i++){
						task->piece_seq_list[i]= -(i+1); //start from one: -1,-2,-3,-4,...
					}
					task->should_thread_all_stop=0; task->peerNum = head->peerNum;
					for (int i = 0; i < head->peerNum; ++i){
						for (int j = 0; j < IP_LEN; ++j){
							task->newPeerIp[i*IP_LEN + j] = head->peerIP[i][j];
						}
					}
					task->file_type = FULL_FILE;

					//add to download task list
					if(add_to_download_task(&download_list_head, task) ==1){
						printf("full file task for %s added, because local file is too old\n", head->filename);
					}

				}
				else if(local_file && head->prev_timestamp == local_file->latest_timestamp){//delta file task
					
					//create a download_t
					download_t *task  = malloc(sizeof(download_t));
					memcpy(task->filename, head->filename, MAX_FILENAME_LEN); task->timeStamp = head->latest_timestamp;
					task->should_thread_all_stop=0; task->peerNum = head->peerNum;
					for (int i = 0; i < head->peerNum; ++i){
						for (int j = 0; j < IP_LEN; ++j){
							task->newPeerIp[i*IP_LEN + j] = head->peerIP[i][j];
						}
					}
					task->file_type = DELTA_FILE;
					task->delta_file_length = head->delta_size;
					task->isdelta_downloading = 0;

					//add to download task list
					if(add_to_download_task(&download_list_head, task) == 1){
						printf("delta file task for %s added\n", head->filename);
					}

				}
				else if(local_file && head->status == 1){
				 	// lock file
				 	local_file->peerNum = head->peerNum;
					//strcpy(local_file->peerIP, head->peerIP);
					local_file->status = 1;
					chmod(head->filename, S_IRUSR);
				}
				else if(local_file && local_file->latest_timestamp == head->latest_timestamp){
					//local_file->peerNum = head->peerNum;
					//strcpy(local_file->peerIP, head->peerIP);
				}
				head = head->next;
			}

			printf("Scan the donwload list!\n");
			//scan the download list
			download_t * cur = download_list_head;
			while(cur!=NULL){
				if (cur->file_type == DELTA_FILE){
					if(cur->isdelta_downloading == 0 && cur->peerNum>0){ //not handled by a thread
						printf("Creating thread for delta task: %s!\n", cur->filename);
						thread_download_t *p_task  = malloc(sizeof(thread_download_t));
						p_task->task = cur;
						p_task->total_piece_num = 0; p_task->piece_seq_list =NULL;
						memcpy(p_task->peerIP, cur->newPeerIp, IP_LEN);
						
						pthread_list *thrd = malloc(sizeof(pthread_list));
						pthread_create(&thrd->thread, NULL, p2p_download_thread, (void *) p_task);

						add_pthread_list( &(cur->thread_list), thrd);
						cur->isdelta_downloading = 1;
					}else if(cur->isdelta_downloading == 1 &&cur->delta_file != NULL){ //download complete
						printf("delta task: %s completed!\n", cur->filename);
						//change the file table:
						file_t *download_file = filetable_find_file(file_table, cur->filename);
						if(download_file == NULL){
							fprintf(stderr, "[in scan download list] Error: can't find file table for the just downloaded delta!\n");
							continue;
						}
						download_file->status = SYNCED;
						download_file->delta_size = cur->delta_file_length;
						download_file->latest_timestamp = cur->timeStamp;

						//TODO: save delta and use the delta to save the file
						FILE *fp;
						char filename[MAX_FILENAME_LEN];
						strcpy(filename, config->delta);
						strcpy(strlen(filename) + filename -1, cur->filename);//-1 is to overwrite the '/0' in the end of config->delta
						printf("%s\n", filename);

						fp = fopen(filename, "wb");
						if(fp == NULL){
							fprintf(stderr, "[in scan download list] Error: can't write delta file!\n");
							continue;
						}
						fwrite(cur->delta_file, sizeof(char), cur->delta_file_length, fp);
						fclose(fp);


					}else{ //handle by a thread and not yet complete
						//do nothing I guess
					}
				}else if(cur->file_type ==  FULL_FILE){
					if(cur->remaining_piece <= 0){ //download complete
						printf("full file task: %s completed!\n", cur->filename);
						//change the file table:
						file_t *download_file = filetable_find_file(file_table, cur->filename);
						if(download_file == NULL){
							char **peerIP = malloc(1 * sizeof(char*));
                			char *myIP = getMyIP();
                			peerIP[0] = myIP;
							filetable_add_file(file_table, 0, SYNCED, cur->filename, cur->full_file_length, cur->timeStamp, 1, peerIP);
						}else{
							download_file->status = SYNCED;
							download_file->file_size = cur->full_file_length;
							download_file->latest_timestamp = cur->timeStamp;
						}
						
						//TODO:resemble the file and save it
						char *full_file = malloc(sizeof(char) * cur->total_piece_num * piece_len);
						memset(full_file, 0, cur->full_file_length);
						for (int i = 0; i < cur->total_piece_num; ++i){
							piece* pcur = cur->piece_list;
							while(pcur!=NULL){
								if(pcur->seqNum == i+1){
									memcpy(full_file + i*piece_len, pcur->data, pcur->dataLength);
									break;
								}
								pcur = pcur->next;
							}
						}

						FILE *fp;
						char filename[MAX_FILENAME_LEN];
						strcpy(filename, config->root);
						strcpy(strlen(filename) + filename -1, cur->filename);//-1 is to overwrite the '/0' in the end of config->delta
						printf("%s\n", filename);

						fp = fopen(filename, "wb");
						if(fp == NULL){
							fprintf(stderr, "[in scan download list] Error: can't write full file!\n");
							continue;
						}
						fwrite(full_file, sizeof(char), cur->full_file_length, fp);
						fclose(fp);


						free(full_file);

					}else if(cur->remaining_piece > 0 && cur->peerNum>0){
						int totol_remaining_piece = 0;//count the number of pieces that is not handled by a thread
						printf("total_piece_num: %d\n", cur->total_piece_num);
						for (int i = 0; i < cur->total_piece_num; ++i){
							printf("piece_seq_list: %d\n", cur->piece_seq_list[i]);
							cur->piece_seq_list[i] < 0 ? totol_remaining_piece++ : totol_remaining_piece;
						}
						printf("Creating thread for full file task: %s, remaining_piece: %d .\n", cur->filename, totol_remaining_piece);
						int num_of_piece_per_peer = totol_remaining_piece/cur->peerNum + ((totol_remaining_piece % cur->peerNum==0)? 0 :1);
						for (int i = 0; i < cur->peerNum && totol_remaining_piece>0; ++i){
							thread_download_t *p_task  = malloc(sizeof(thread_download_t));
							p_task->task = cur;
							p_task->total_piece_num = num_of_piece_per_peer<totol_remaining_piece? num_of_piece_per_peer: totol_remaining_piece;
							p_task->piece_seq_list = malloc(sizeof(int)* p_task->total_piece_num);
							int counter = p_task->total_piece_num - 1;
							for (int j = 0; j < cur->total_piece_num && counter>=0; ++j){
								if(cur->piece_seq_list[j] < 0){
									cur->piece_seq_list[j] = - cur->piece_seq_list[j];//turn it to (+)
									p_task->piece_seq_list[counter] = cur->piece_seq_list[j];
									counter--;
								}
							}
							memcpy(p_task->peerIP, cur->newPeerIp + IP_LEN*i, IP_LEN);

							pthread_list *thrd = malloc(sizeof(pthread_list));
							pthread_create(&thrd->thread, NULL, p2p_download_thread, (void *) p_task);
							add_pthread_list( &(cur->thread_list), thrd);

							//for next iteration
							totol_remaining_piece -= p_task->total_piece_num;
						}
					}
				}
				cur = cur->next;
			}//----end of while(cur!=NULL)
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
    	return;
	}
	memcpy(myIp, inet_ntoa(*(struct in_addr*)hostp->h_addr_list[0]), IP_LEN);
}


//Connect to the tracker and do register
int register_on_tracker() {
    //construct register packet
    p2t_seg_t regis_pack;  
    regis_pack.type = REGISTER;
    regis_pack.filetable.head = NULL; 
    memcpy(regis_pack.peer_ip, myIp, sizeof(myIp)); 
    regis_pack.port = MONITOR_PORT;

    //send it
    if(send_p2t_segment(monitor_socket, &regis_pack) != 1){
    	printf("send_p2t failed\n");
    	//return 0;
    }

    //receive accept packet
	t2p_seg_t accept_pack;
	if(receive_t2p_segment(monitor_socket, &accept_pack) != 1){
		printf("receive_t2p failed\n");
		//return 0;
	}

    //get the value
   	alive_pack_interval = accept_pack.interval;
    piece_len = accept_pack.piece_len;

    printf("interval %i\n", alive_pack_interval);
    printf("piece_len %i\n", piece_len);

	return 1;
}


void *heart_beat_thread(void *arg){
	printf("heart_beat_thread");
	while(1){

		//connect to tracker
		struct hostent *host = gethostbyname(TRACKER_ADDR);
		struct sockaddr_in servaddr;
		heartbeat_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (heartbeat_socket < 0)
			return 0;
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(HEARTBEAT_PORT);
		memcpy(&servaddr.sin_addr, host->h_addr_list[0], host->h_length);
		if (connect(heartbeat_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)
			return 0;
		printf("heart beat connected\n");
		//construct heartbeat packet
		while(1){
		    p2t_seg_t regis_pack;  
		    regis_pack.type = ALIVE; 
		    memcpy(regis_pack.peer_ip, myIp, sizeof(myIp));
		    //strcpy(regis_pack.peer_ip, myIp); 
		    regis_pack.port = P2PListenPort;

		    // p2t send
		    printf("send heartbeat\n");
		    if(send_p2t_segment(heartbeat_socket, &regis_pack) != 1){
		    	printf("heartbeat send_p2t failed\n");
		    }

			sleep(alive_pack_interval);
		}
	}
}

// p2p listen thread
void *p2p_listen_thread(void *arg){

	int listen_sock, comm_sock;
	struct sockaddr_in server;
	struct sockaddr_in cli_addr;
	socklen_t cli_len;

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0){
		perror("opening socket stream");
		exit(1);
	}
	// initiate
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(P2PPort);

	// bind 
	if (bind(listen_sock, (struct sockaddr *) &server, sizeof(server))) {
		perror("binding socket name");
		exit(2);
	}

	printf("p2p_listen_thread Listening at port %d\n", P2PPort);
	listen(listen_sock, 10);
	while(1){

		cli_len = sizeof cli_addr;
		comm_sock = accept(listen_sock, (struct sockaddr *)&cli_addr,&cli_len);
		if (comm_sock == -1){
			perror("accept");
			continue;
		}else{
			int *arg = malloc(sizeof(int));
			*arg = comm_sock;
			pthread_t new_upload_thread;
			pthread_create(&new_upload_thread, NULL, p2p_upload_thread, (void*) arg);
		}
	}
	return 0;
}

void *p2p_upload_thread(void *arg){
	printf("start to upload!\n");
	//get socket descriptor
	int *upload_sock_copy = (int *)arg;
	int upload_sock = *upload_sock_copy;
	free(upload_sock_copy);

	//receive request and send data
	p2p_seg_t request_pack;
	while(receive_p2p_segment(upload_sock, &request_pack) > 0){
		if(request_pack.type == DATA_REQ){
			if(request_pack.data_req_type == DELTA_REQ){//ask for delta file
				//get the delta file (cited from: https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c)
				char * buffer = NULL; //use to store the file
				long length;
				size_t num=0;
				char filename[MAX_FILENAME_LEN];
				strcpy(filename, config->delta);
				strcpy(strlen(filename) + filename, request_pack.filename);//-1 is to overwrite the '/0' in the end of config->delta
				printf("%s\n", filename);

				FILE * f = fopen (filename, "rb");
				if(f)
				{
					fseek(f, 0, SEEK_END);
					length = ftell(f);
					fseek(f, 0, SEEK_SET);
					buffer = malloc (length);
					if(buffer){
						num = fread(buffer, sizeof(char), length, f);
					}
					fclose(f);
				}
				if(num <= 0){
					fprintf(stderr, "Read delta file empty!\n");
					continue;
				}

				//send response packet
				p2p_seg_t data_pack;
				data_pack.type=DATA; 
				data_pack.length = 16;//--------need to change
				strcpy(data_pack.filename, request_pack.filename);
				data_pack.data = malloc(sizeof(char) * data_pack.length);
				char A[16]="Tofukasu-delta!";//--------need to change
				strcpy(data_pack.data, A);//--------need to change
				if(send_p2p_segment(upload_sock, &data_pack) < 0){
					fprintf(stderr, "[in p2p_upload_thread] Error: send p2p seg fail!\n");
				}

				//free resources
				free(buffer);
			}else if(request_pack.data_req_type == FILE_REQ){	
				//read the file out and the specific piece (cited from: https://stackoverflow.com/questions/174531/easiest-way-to-get-files-contents-in-c)
				char * buffer = NULL; //use to store the file
				char * data;// point to the specific piece
				long length;
				size_t num=0;
				char filename[MAX_FILENAME_LEN];
				strcpy(filename, config->root);
				strcpy(strlen(filename) + filename, request_pack.filename);//-1 is to overwrite the '/0' in the end of config->delta
				printf("%s\n", filename);

				FILE * f = fopen (filename, "rb");
				if(f)
				{
					fseek(f, 0, SEEK_END);
					length = ftell(f);
					fseek(f, 0, SEEK_SET);
					buffer = malloc (length);
					if(buffer){
						num = fread(buffer, sizeof(char), length, f);
					}
					fclose(f);
				}
				if(num <= 0){
					fprintf(stderr, "Read delta file empty!\n");
					continue;
				}else{
					data = buffer + (request_pack.seqNum-1) * piece_len;//seqNum starts at 1
				}

				//send response packet
				p2p_seg_t data_pack;
				data_pack.type=DATA; 
				data_pack.seqNum = request_pack.seqNum;
				data_pack.length = 15;//--------need to change
				strcpy(data_pack.filename, request_pack.filename);
				data_pack.data = malloc(sizeof(char) * data_pack.length);
				char A[15]="Tofukasu-file!";//--------need to change
				strcpy(data_pack.data, A);//--------need to change
				if(send_p2p_segment(upload_sock, &data_pack) < 0){
					fprintf(stderr, "[in p2p_upload_thread] Error: send p2p seg fail!\n");
				}

				//free resources
				free(buffer);
			}
		}

		//for next iteration
		if(request_pack.data!=NULL)
			free(request_pack.data);
		memset(&request_pack, 0, sizeof(request_pack));
	}
	return 0;
}



//--------------------------------------------------------------------------------------//
//---------------------------------Chao's part Start------------------------------------//
//--------------------------------------------------------------------------------------//


void* p2p_download_thread(void *arg){
	thread_download_t * p_task = (thread_download_t*) arg;
	int isConnectionBreak = 0; // if connection break during download, set it to 1
	int counter = -1;

	//connect to peer (p_task.peerIp)
	int download_socket = socket(AF_INET, SOCK_STREAM, 0);//socket intialization
	if (download_socket<0){
		perror("Socket openning error");
		return 0;
	}

	struct sockaddr_in server;//server address intialization
	server.sin_family = AF_INET;
	server.sin_port = htons(P2PPort);
	server.sin_addr.s_addr = inet_addr(p_task->peerIP);

	if(connect(download_socket, (struct sockaddr*)&server, sizeof(server))<0){//connect to peer
		perror("connectting socket");
		return 0;
	}

	//start download
	if(p_task->task->file_type == DELTA_FILE){
		//send request
		p2p_seg_t req_packet;
		req_packet.type = DATA_REQ;	req_packet.data_req_type = DELTA_FILE;
		strcpy(req_packet.filename, p_task->task->filename);
		if(send_p2p_segment(download_socket, &req_packet) < 0){
			isConnectionBreak = 1;
		}

		//receive data
		p2p_seg_t recv_packet;
		if(receive_p2p_segment(download_socket, &recv_packet) <0){
			isConnectionBreak = 1;
		}
		//save the data to task.delta_file 
		if(isConnectionBreak == 0){
			p_task->task->delta_file = malloc(sizeof(char) * recv_packet.length);
			p_task->task->delta_file_length = recv_packet.length;
			memcpy(p_task->task->delta_file, recv_packet.data, recv_packet.length);
			free(recv_packet.data);
		}
	}else if(p_task->task->file_type == FULL_FILE){
		counter = p_task->total_piece_num - 1;
		while(counter>=0 && p_task->task->should_thread_all_stop == 0){
			//send request
			p2p_seg_t req_packet;
			req_packet.type = DATA_REQ;	req_packet.data_req_type = FULL_FILE; 
			req_packet.seqNum = p_task->piece_seq_list[counter];
			strcpy(req_packet.filename, p_task->task->filename);
			if(send_p2p_segment(download_socket, &req_packet) <0){
				isConnectionBreak = 1;
				break;
			}

			//receive data
			p2p_seg_t recv_packet;
			if(receive_p2p_segment(download_socket, &recv_packet)){
				isConnectionBreak = 1;
				break;
			}
			//save the data to task.piece_list
			piece *new_piece = malloc(sizeof(piece));
			new_piece->seqNum = p_task->piece_seq_list[counter];
			new_piece->dataLength = recv_packet.length;
			new_piece->data = malloc(sizeof(char) *recv_packet.length);
			memcpy(new_piece->data, recv_packet.data, recv_packet.length);
			add_piece( &(p_task->task->piece_list), new_piece);

			p_task->task->remaining_piece--;
			p_task->piece_seq_list[counter] = - (p_task->piece_seq_list[counter]);//mark it as finished, turn it to (-)

			//for next iteration
			counter--;
		}
	}

	//reset state when connection break or the thread is being canceled (should_thread_all_stop==1)
	if( (isConnectionBreak == 1 && p_task->task->file_type == FULL_FILE) || (isConnectionBreak == 0 && counter >=0) ){
		//reset some state of the task (for reschedule download)
		for (int i = counter; i >= 0; i--){
			for (int j = 0; i < p_task->task->total_piece_num; j++){
				if(p_task->piece_seq_list[i] == p_task->task->piece_seq_list[j]){
					p_task->task->piece_seq_list[j] = - (p_task->task->piece_seq_list[j]);
					break;
				}
			}
		}
	}else if(isConnectionBreak == 1 && p_task->task->file_type == DELTA_FILE){
		p_task->task->delta_file = NULL;//reset state
		p_task->task->isdelta_downloading = 0;
	}

	//free resources
	close(download_socket);
	return 0;
}


void add_pthread_list(pthread_list **head, pthread_list * new_thread){
	//check input


	if(*head==NULL){//if head is null, let the new_thread to be the head
		*head=new_thread;
		return;
	}
	pthread_list *cur = *head;//if head is not null, find the tail element, and let it link to new_thread
	while(cur->next!=NULL){
		cur=cur->next;
	}
	cur->next=new_thread;
}



void add_piece(piece **head, piece * new_piece){
	//check input


	if(*head==NULL){//if head is null, let the new_piece to be the head
		*head=new_piece;
		return;
	}
	piece *cur = *head;//if head is not null, find the tail element, and let it link to new_piece
	while(cur->next!=NULL){
		cur=cur->next;
	}
	cur->next=new_piece;
}


//@return  (-1: failure), (0: duplicate_and_not_add), (1: add)
int add_to_download_task(download_t **head, download_t *new_task){
	//check input
	if(head == NULL || new_task ==NULL){
		fprintf(stderr, "[In add_to_download_task] Error: input invalid\n");
		return -1;
	}

	//if head is null, let the new element to be the head
	if(*head==NULL){
		*head=new_task;
		return 1;
	}

	//find duplicate
	download_t *cur = *head;
	while(cur!=NULL){
		if(strcmp(cur->filename, new_task->filename)==0){
			if(cur->timeStamp < new_task->timeStamp){ //newer version of that file comes
				remove_from_download_task(head, cur);
			}else if(cur->peerNum != new_task->peerNum){//active peer that has the file change
				//update the download list
				cur->peerNum = new_task->peerNum;
				memcpy(cur->newPeerIp, new_task->newPeerIp, new_task->peerNum);
				//reschedule or not
				if(cur->file_type ==FULL_FILE  && cur->remaining_piece > BIGFILE_THRES){
					cur->should_thread_all_stop = 1;
				}
				return 0;
			}else { //older version of file comes
				return 0;
			}
		}
		cur=cur->next;
	}

	//add the new_task in it
	download_t *cur1 = *head;
	while(cur1->next!=NULL){
		cur1=cur1->next;
	}
	cur1->next=new_task;
	new_task->next = NULL; //(just in case when create it not doing this)

	return 1;
}

int remove_from_download_task(download_t **head, download_t *new_task){
	//check input
	if(head == NULL || new_task ==NULL){
		fprintf(stderr, "[In remove_from_download_task] Error: input invalid\n");
		return -1;
	}

	//delete
	if(*head==new_task){
		*head=new_task->next;
		free_one_task(new_task);
		return 0;
	}
	download_t *cur = *head;
	while(cur!=NULL&&cur->next!=new_task){
		cur=cur->next;
	}
	if(cur==NULL){
		fprintf(stderr, "[In remove_from_download_task] Error: the task to be deleted is not in the list\n");
	}else{
		cur->next=new_task->next;
		free_one_task(new_task);
	}
	return 0;
}

void free_one_task(download_t *task){
	//check input
	if (task==NULL){
		fprintf(stderr, "[In free_one_task] Error: input invalid\n");
	}

	//cancel threads if they are still running and free them
	pthread_list *pre, *cur = task->thread_list;
	while(cur!=NULL){
		pre=cur;
		cur=cur->next;

		pthread_cancel(pre->thread);//send cancel request
		printf("HHHH, see if here!\n");
		pthread_join(pre->thread, NULL);//wait the thread to cancel
		free(pre);
	}

	//free piece_seq_list
	for (int i = 0; i < task->total_piece_num; ++i){
		free(task->piece_seq_list+i);
	}

	//free all pieces
	if(task->file_type == DELTA_FILE){
		free(task->delta_file);
	}else if(task->file_type == FULL_FILE){
		piece *pre2, *cur2 = task->piece_list;
		while(cur2!=NULL){
			pre2=cur2;
			cur2=cur2->next;
			free(pre2->data);
			free(pre2);
		}
	}

}



//--------------------------------------------------------------------------------------//
//---------------------------------Chao's part End---------------------------------------//
//--------------------------------------------------------------------------------------//


//// need new socket
void *file_monitor_thread(void *arg){
	/* Set inotify_fd as -1 */
    int inotify_fd = -1;

    /* Initialize Table */
    mnt_table = malloc(sizeof(DIR_MNT_table));
    mnt_table->entNum = 0;

    /* Read Config */
    config = malloc(sizeof(MNT_config));
    readConfigFile(config, config_name);

    /* Remove .backup dir from last session if exists */
    struct stat st = {0};
    if (stat(config->backup, &st) == 0) {
        rmdir(config->backup);
    }

    /* initialize file table  */
    file_table = filetable_create();

    /* initialize inotify watch */
    if ((inotify_fd = initWatch(mnt_table, config, file_table)) < 0){
        printf("error initWatch\n");
    }

    /* signal */
    // signal(SIGINT, clearAll);

    /* handle event */
    int length = 0, i = 0;
    char buffer[BUF_LEN];

    filetable_print(file_table);


    printf("start to read...\n");
    sendTracker(file_table, monitor_socket);

    while((length = read(inotify_fd, buffer, BUF_LEN )) > 0){
        /* return the list of change events happens. Read the change event one by one and process it accordingly. set i as pointer */
        i = 0;
        while(i < length){
            /* get event info */
            struct inotify_event *event = (struct inotify_event *)&buffer[i];
            if(event->len){
                char *preflix = getPathByWd(mnt_table, event->wd);//cannot be freed here!
                char *fullname;
                if(event->mask & IN_ISDIR){
                    /// add slash to the end, consistent with monitor table
                    fullname = malloc(strlen(preflix) + strlen(event->name) + 2);
                    strcpy(fullname, preflix);
                    strcat(fullname, event->name);
                    fullname[strlen(preflix) + strlen(event->name)] = '/';
                    fullname[strlen(preflix) + strlen(event->name) + 1] = '\0';
                }else{
                    /// file
                    /// filtering out vim backup
                    if(strcmp(get_filename_ext(event->name), "swp") == 0 || strcmp(get_filename_ext(event->name), "swx") == 0 ||
                       strcmp(event->name, "4913") == 0) {
                        i += EVENT_SIZE + event->len;
                        continue;
                    }
                    fullname = concatPath(preflix, event->name);
                }

                /// filtering out undesired event, i.e. vim backup or ignored dir
                if(watchFilter(config, fullname) < 0){
                    printf("filtering out %s\n", fullname);
                    free(fullname);
                    i += EVENT_SIZE + event->len;
                    continue;
                }

                /// handle event
                if((event->mask & IN_OPEN) && !(event->mask & IN_ISDIR)){
                    // TODO
                    fileOpened(fullname, file_table, config);

                }else if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)) {
                    // TODO
                    dirCreate(fullname, inotify_fd, file_table, mnt_table, config);

                }else if( event->mask & IN_DELETE ) {
                    if ( event->mask & IN_ISDIR ) {
                        // TODO
                        dirDelete(fullname, inotify_fd, file_table,mnt_table);

                    }else {
                        // TODO
                        if(fileDeleted(fullname, file_table, config) == 1){
                        	sendTracker(file_table, monitor_socket);
                        }

                    }
                }else if((event->mask & IN_CLOSE_WRITE) && !(event->mask & IN_ISDIR)){
                    if(fileCloseWrite(fullname, file_table, config) == 1){
                    	sendTracker(file_table, monitor_socket);
                    }
                }
//                else if((event->mask & IN_CLOSE_NOWRITE) && !(event->mask & IN_ISDIR)){
//                    fileCloseNoWrite(fullname, file_table, config);
//                }
//                else if((event->mask & IN_ACCESS) && !(event->mask & IN_ISDIR)){
//                    fileAccess(fullname, file_table, config);
//                }else if((event->mask & IN_MODIFY) && !(event->mask & IN_ISDIR)){
//                    fileInModified(fullname, file_table, config);
//                }
                free(fullname);
            }
            /* update pointer */
            i += EVENT_SIZE + event->len;
        }
    }
    
    return 0;
}
// monitor table clearAll
void clearAll(){
    printf("start to clear all...\n");
    /* free monitor config table */
    free(config->root);
    free(config->backup);
    free(config->delta);

    for(int i = 0; i < config->ignoreNum; i++){
        free(config->ignore[i]);
    }
    free(config);
    config = NULL;
    printf("Free config\n");

    /* free monitor watch table and rm watches */
    for(int i = 0; i < mnt_table->entNum; i++){
        if(mnt_table->table[i].dirName != NULL)
            free(mnt_table->table[i].dirName);
        if(mnt_table->table[i].watchD >= 0)
            inotify_rm_watch(inotify_fd, mnt_table->table[i].watchD);
    }
    free(mnt_table);
    mnt_table = NULL;
    printf("Free mnt_table\n");

    /* close inotify file descriptor*/
    close(inotify_fd);
    inotify_fd = -1;
    printf("close inotify\n");

    /* free file table */
    filetable_destroy(file_table);

    exit(0);
}
