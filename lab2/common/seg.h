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

// send a segment
int mnp_sendseg(int connection, seg_t* segPtr);

// receive a segment
int mnp_recvseg(int connection, seg_t* segPtr);

// emulate segment loss
int seglost();

#endif //SEG_H
