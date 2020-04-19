#ifndef _PEERH_
#define _PEERH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "../common/seg.h"
#include "./monitor.h"
//#include "../common/seg.h"
//


#define TRACKER_ADDR "moose.cs.dartmouth.edu"
#define MONITOR_PORT 9613
#define HEARTBEAT_PORT 9619
#define P2PPort 3339
#define IP_LEN 16
#define REGISTER 0
#define HEARTBEAT 1
#define FT_UPDATE 2


#define MAX_FILENAME_LEN 200
#define MAX_PEER_IP 10*16

#define DELTA_FILE 0
#define FULL_FILE 1

//p2p seg types
#define DATA_REQ 0
#define DATA 1

#define BIGFILE_THRES 30 //(MB)



typedef struct pthread_list
{
	pthread_t thread;
	struct pthread_list * next;
}pthread_list;


typedef struct piece{
	int seqNum;
	char dataLength;
	char *data;
	struct piece *next;
}piece;

//Download Task List data structure
typedef struct download_task {
	int remaining_piece;
	int total_piece_num; //length of piece_seq_list
	int *piece_seq_list; // + for downloading, - for waiting (seq num start from 1)
	piece *piece_list;
	pthread_list *thread_list;
	int full_file_length;

	char filename[MAX_FILENAME_LEN]; //file name
	unsigned long int timeStamp;
	int should_thread_all_stop;//used in scan the download list in main thread: pthread_join && turn it off; used in add: turn it on
	int peerNum; //# of active peers
	char newPeerIp[MAX_PEER_IP]; //peers who have this file
	int file_type; //delta or full file
	pthread_mutex_t task_mutex;
	struct download_task * next;// next task

	char *delta_file;//to store delta_file (initially should set to NULL)
	int delta_file_length;//delta file size
	int isdelta_downloading;
}download_t;

//pthread task data structure
typedef struct  pthread_download_task
{
	download_t *task;
	int total_piece_num; //length of piece_seq_list
	int *piece_seq_list; // + for not finished, - for finished
	char peerIP[IP_LEN];
}thread_download_t;

typedef struct upload_arg {
	int sock;
	p2p_seg_t packet;
}upload_arg_t;


// peer main thread
int peer();
// get my ip
void get_my_ip();
// send register to tracker
int register_on_tracker();
// heart beat 
void *heart_beat_thread(void *arg);
// p2p listen
void *p2p_listen_thread(void *arg);
// file monitor
void *file_monitor_thread(void *arg);
// p2p download
void *p2p_download_thread(void *arg);
// p2p_upload
void *p2p_upload_thread(void *arg);
// 
void add_pthread_list(pthread_list **head, pthread_list * new_thread);
//
void add_piece(piece **head, piece * new_piece);
//
//@return  (-1: failure), (0: duplicate_and_not_add), (1: add)
int add_to_download_task(download_t **head, download_t *new_task);
//
int remove_from_download_task(download_t **head, download_t *new_task);
//
void free_one_task(download_t *task);
//
void clearAll();

#endif
