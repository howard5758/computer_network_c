#ifndef _SEGH_
#define _SEGH_

#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include "./filetable.h"

#define IP_LEN 16
#define PROTOCOL_LEN 10
#define RESERVED_LEN 10
#define MAX_FILENAME_LEN 200

//p2p seg types
#define DATA_REQ 0
#define DATA 1

//p2p data type
#define DELTA_REQ 0
#define FILE_REQ 1

//p2t seg types
#define REGISTER 0
#define ALIVE 1
#define FILE_TABLE_UPDATE 2

/**************p2p seg****************/
typedef struct p2p_segment {
    int src_peerIP;
    int dest_peerIP;

    int type; //0: DELTA_REQ or 1: DATA (see above)
    int length; // length of data
    int data_req_type;//delta=0; full file=1
    char filename[MAX_FILENAME_LEN];
    int seqNum;

    char *data;
} p2p_seg_t;


/**************p2t seg****************/
typedef struct p2t_segment {
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
    //file table
    file_table_t filetable;
}p2t_seg_t;

/**************t2p seg****************/

typedef struct t2p_segment {
    // time interval that the peer should sending alive message periodically
    int interval;
    // piece length
    int piece_len;
    file_table_t filetable;
} t2p_seg_t;


/**************send and receive****************/
int send_p2p_segment(int connection, p2p_seg_t *packet);
int receive_p2p_segment(int connection, p2p_seg_t *packet);

int send_p2t_segment(int connection, p2t_seg_t *packet);
int receive_p2t_segment(int connection, p2t_seg_t* packet);


int send_t2p_segment(int connection, t2p_seg_t* packet);
int receive_t2p_segment(int connection, t2p_seg_t* packet);

#endif
