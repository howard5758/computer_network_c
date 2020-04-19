/*
 * common/seg.h: segment definitions and interfaces to send and 
 * receive segments.
 * It supports mnp_sendseg() and mnp_recvseg() for sending to the 
 * network layer. 
 *
 * CS60, March 2018. 
 */

#ifndef SEG_H
#define SEG_H

#include "constants.h"

/**************** constants ****************/

// segment type definition 
#define	SYN 0
#define	SYNACK 1
#define	FIN 2
#define	FINACK 3
#define	DATA 4
#define	DATAACK 5


/**************** global types ****************/

// segment header definition
typedef struct mrt_hdr {
	unsigned int src_port;        //source port number
	unsigned int dest_port;       //destination port number
	unsigned int seq_num;         //sequence number
	unsigned int ack_num;         //ack number
	unsigned short int length;    //segment data length
	unsigned short int  type;     //segment type
	unsigned short int  rcv_win;  //currently not used
	unsigned short int checksum;  //checksum for this segment
} mrt_hdr_t;

// segment definition
typedef struct segment {
	mrt_hdr_t header;
	char data[MAX_SEG_LEN];
} seg_t;

// the data structure exchanged between the MNP process and the MRT 
// process. It contains a node ID and a segment.
// For mnp_sendseg(), the node ID is the destination node ID of the 
// segment; for mnp_recvseg(), the node ID is the source node ID of 
// the segment.
typedef struct sendsegargument {
	int nodeID;        //node ID
	seg_t seg;        //a segment
} sendseg_arg_t;

/**************** functions ****************/
/*
 * In what follows, we provide the prototype definition for each 
 * call and limited pseudo code representation of the function. 
 *
 * This is not meant to be comprehensive - more a guideline. You are 
 * free to design the code as you wish.
 *
 * NOTE: mnp_sendseg() and mnp_recvseg() are services provided by 
 * the networking layer, i.e., the mini network protocol to the 
 * transport layer. 
 */

/**
 * MRT process uses this function to send a segment and its 
 * destination node ID in a sendseg_arg_t structure to MNP process 
 * to send out.
 * Parameter network_conn is the TCP socket descriptor of the 
 * connection between the MRT and MNP processes.
 * Return 1 if a sendseg_arg_t is successfully sent, otherwise -1.
 */
int mnp_sendseg(int network_conn, int dest_nodeID, seg_t *segPtr);

/**
 * MRT process uses this function to receive a sendseg_arg_t 
 * structure which contains a segment and its src node ID from the 
 * MNP process.
 * Parameter network_conn is the TCP socket descriptor of the 
 * connection between the MRT and MNP processes.
 * When a segment is received, use seglost to determine if the 
 * segment should be discarded. 
 * Return 1 if a sendseg_arg_t is successfully received, else -1.
 */
int mnp_recvseg(int network_conn, int *src_nodeID, seg_t *segPtr);

/**
 * MNP process uses this function to receive a sendseg_arg_t 
 * structure which contains a segment and its destination node ID 
 * from the MRT process.
 * Parameter tran_conn is the TCP socket descriptor of the 
 * connection between the MRT and MNP processes.
 * Return 1 if a sendseg_arg_t is successfully received, else -1.
 */
int getsegToSend(int tran_conn, int *dest_nodeID, seg_t *segPtr);

/**
 * MNP process uses this function to send a sendseg_arg_t structure
 * which contains a segment and its src node ID to the MRT process.
 * Parameter tran_conn is the TCP socket descriptor of the 
 * connection between the MRT and MNP processes.
 * Return 1 if a sendseg_arg_t is successfully sent, else -1.
 */
int forwardsegToMRT(int tran_conn, int src_nodeID, seg_t *segPtr);

// emulate segment loss
int seglost();

#endif //SEG_H
