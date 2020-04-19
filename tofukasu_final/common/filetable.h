#ifndef _FILETABLEH_
#define _FILETABLEH_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_PEER_NUM 10
#define IP_LEN 16

#define MAX_FILENAME_LEN 200

//file status
#define READ_N_WRITE 0
#define READ_ONLY 1
#define DELETED 2
#define SYNCED 3
#define WRITING_IN_PROGRESS 4

typedef struct file{
    int file_size;
    int delta_size;
    int status; //0,1,2,3, see above
    char filename[MAX_FILENAME_LEN];
    int peerNum;//#peers who have the latest file
    char peerIP[MAX_PEER_NUM][IP_LEN];//for peer this is the peer's own IP, for tracker this is the string of IPs fo all peers who have the file
    unsigned long latest_timestamp;
    unsigned long prev_timestamp;//should be initialized to 0;
    struct file *next;
    int isDir;//1: is dir; 0: not dir
} file_t;

typedef struct file_table {
    file_t *head;
    int size; //number of files in table
} file_table_t;

//int filetable_get_time ();

file_table_t *filetable_create();

void filetable_destroy(file_table_t *table);

//return 0 on success, -1 on failure
int filetable_add_file(file_table_t *table, int isDir, int status, char *filename, int file_size, int last_mod_time, int peerNum, char *peerIP[]);

//find file entry by filename
file_t *filetable_find_file(file_table_t *table, char *filename);

//when a peer is dead, remove the IP of the peer from all file table entries
//return 0 on success, -1 on failure
int filetable_remove_peerIP(file_table_t *table, char *peerIP);

void filetable_print(file_table_t *table);

#endif
