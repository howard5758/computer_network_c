/*
 * common/seg.c: implementation of interfaces to send and 
 * receive segments, as well as emulation of segment losses, defined
 * in common/seg.h
 * 
 * CS60, March 2018. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <memory.h>
#include <assert.h>
#include <string.h>
#include "seg.h"

/**************** constants ****************/
#define START1 0
#define START2 1
#define RECV 2
#define STOP1 3


/**************** functions ****************/

/**
 * MRT process uses this function to send a segment and its 
 * destination node ID in a sendseg_arg_t structure to MNP process 
 * to send out.
 * TCP sends data as a byte stream. Delimiters for the start and end 
 * of the packet are added, where characters ``!&'' are used to 
 * indicate the start, while ``!#'' indicates the end. 
 * Return 1 if a sendseg_arg_t is successfully sent, otherwise -1. 
 */
int mnp_sendseg(int network_conn, int dest_nodeID, seg_t *segPtr) 
{
	sendseg_arg_t arg;
	arg.nodeID = dest_nodeID;
	arg.seg = *segPtr;
	char buf_start[2] = "!&";
	char buf_end[2] = "!#";

	if (send(network_conn, buf_start, 2, 0) < 0) {
		return -1;
	}

	if (send(network_conn, &arg, sizeof(sendseg_arg_t), 0) < 0) {
		return -1;
	}

	if (send(network_conn, buf_end, 2, 0) < 0) {
		return -1;
	}

	return 1;
}

/**
 * MRT process uses this function to receive a sendseg_arg_t 
 * structure which contains a segment and its src node ID from the 
 * MNP process.  
 * Here we are looking for ``!&'' characters then seg_t and then 
 * ``!#''. After parsing a segment, it calls seglost() to emulate 
 * segment loss.
 * Return 1 if a sendseg_arg_t is successfully received, else -1.
 */
int mnp_recvseg(int network_conn, int *src_nodeID, seg_t *segPtr)
{
	sendseg_arg_t arg;
	char buf[sizeof(sendseg_arg_t) + 2];
	char c;
	int index = 0;

	int state = START1;
	while (recv(network_conn, &c, 1, 0) > 0) {
		switch (state) {
			case START1:
				if (c == '!') {
					state = START2;
					break;
				}
			case START2:
				if (c == '&') {
					state = RECV;
				} else {
					state = START1;
				}
				break;
			case RECV:
				if (c == '!') {
					buf[index] = c;
					index++;
					state = STOP1;
				} else {
					buf[index] = c;
					index++;
				}
				break;
			case STOP1:
				if (c == '#') {
					buf[index] = c;
					assert(index - 1 == sizeof(arg));
					memcpy(&arg, buf, index - 1);
					*src_nodeID = arg.nodeID;
					*segPtr = arg.seg;

					state = START1;
					index = 0;

					/* add segment error */
					if (seglost(segPtr) > 0) {
						continue;
					}
					return 1;
				} else if (c == '!') {
					buf[index] = c;
					index++;
				} else {
					buf[index] = c;
					index++;
					state = RECV;
				}
				break;
			default:
				break;
		}
	}
	return -1;
}

/**
 * MNP process uses this function to receive a sendseg_arg_t 
 * structure which contains a segment and its destination node ID 
 * from the MRT process.
 * Parameter tran_conn is the TCP socket descriptor of the 
 * connection between the MRT and MNP processes.
 * Return 1 if a sendseg_arg_t is successfully received, else -1.
 */
int getsegToSend(int tran_conn, int *dest_nodeID, seg_t *segPtr) {
	sendseg_arg_t arg;
	char buf[sizeof(sendseg_arg_t) + 2];
	char c;
	int index = 0;

	int state = START1;
	while (recv(tran_conn, &c, 1, 0) > 0) {
		switch (state) {
			case START1:
				if (c == '!')
					state = START2;
				break;
			case START2:
				if (c == '&')
					state = RECV;
				else
					state = START1;
				break;
			case RECV:
				if (c == '!') {
					buf[index] = c;
					index++;
					state = STOP1;
				} else {
					buf[index] = c;
					index++;
				}
				break;
			case STOP1:
				if (c == '#') {
					buf[index] = c;
					assert(index - 1 == sizeof(arg));
					memcpy(&arg, buf, index - 1);
					*dest_nodeID = arg.nodeID;
					*segPtr = arg.seg;

					return 1;
				} else if (c == '!') {
					buf[index] = c;
					index++;
				} else {
					buf[index] = c;
					index++;
					state = RECV;
				}
				break;
			default:
				break;

		}
	}
	return -1;
}

/**
 * MNP process uses this function to send a sendseg_arg_t structure
 * which contains a segment and its src node ID to the MRT process.
 * Parameter tran_conn is the TCP socket descriptor of the 
 * connection between the MRT and MNP processes.
 * Return 1 if a sendseg_arg_t is successfully sent, else -1.
 */
int forwardsegToMRT(int tran_conn, int src_nodeID, seg_t *segPtr) {
	sendseg_arg_t arg;
	arg.nodeID = src_nodeID;
	arg.seg = *segPtr;
	char buf_start[2] = "!&";
	char buf_end[2] = "!#";

	if (send(tran_conn, buf_start, 2, 0) < 0) {
		return -1;
	}

	if (send(tran_conn, &arg, sizeof(sendseg_arg_t), 0) < 0) {
		return -1;
	}

	if (send(tran_conn, buf_end, 2, 0) < 0) {
		return -1;
	}

	return 1;
}

/*************** emulate segment loss **************/
/* a segment has PKT_LOST_RATE probability to be lost 
 * if the segment is lost, return 1; otherwise return 0 
 */
int seglost() {
	int random = rand() % 100;
	if (random < PKT_LOSS_RATE * 100)
		return 1;
	else
		return 0;
}


