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
#include "seg.h"

/****** send a segment over the overlay network *******/
/* The overlay network is simply a single TCP connection in lab 2.
 * TCP sends data as a byte stream. Delimiters for the start and end 
 * of the packet are added, where characters ``!&'' are used to 
 * indicate the start, while ``!#'' indicates the end. 
 * Return 1 in case of success, and -1 in case of failure. 
 */
int mnp_sendseg(int connection, seg_t* segPtr)
{
	char bufstart[2];
	char bufend[2]; 
	bufstart[0] = '!';
	bufstart[1] = '&';
	bufend[0] = '!';
	bufend[1] = '#';
	if (send(connection, bufstart, 2, 0) < 0) {
		return -1;
	}
	if (send(connection, segPtr, sizeof(seg_t), 0) < 0) {
		return -1;
	}
	if (send(connection, bufend, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

/****** receive a segment over the overlay network *******/
/* The overlay network is simply a single TCP connection in lab 2.
 * We recommend that you receive one byte at a time using recv(). 
 * Here you are looking for ``!&'' characters then seg_t and then 
 * ``!#''. After parsing a segment, it calls seglost() to emulate 
 * segment loss.
 * Returns 1 if successfully received a segment, -1 if any errors.
 */
int mnp_recvseg(int connection, seg_t* segPtr)
{
	char buf[sizeof(seg_t) + 2];
	char c;
	int idx = 0;

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
			if (c == '#') {
				buf[idx] = c;
				idx++;
				state = 0;
				idx = 0;
				if (seglost() > 0) {
					printf("seg lost!!!!!!\n");
					continue;
				}
				memcpy(segPtr, buf, sizeof(seg_t));
				return 1;
			} else if (c == '!') {
				buf[idx] = c;
				idx++;
			} else {
				buf[idx] = c;
				idx++;
				state = 2;
			}
		}
	}
	return -1;
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


