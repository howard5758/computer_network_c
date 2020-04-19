/*
 * common/pkt.c: implementation of interfaces to send and 
 * receive packets.
 *
 * CS60, March 2018. 
 */

#include <sys/socket.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#include "pkt.h"

/**************** constants ****************/
#define PKTSTART1 0
#define PKTSTART2 1
#define PKTRECV 2
#define PKTSTOP1 3

// overlay_sendpkt() is called by the MNP process to request the ON 
// process to send a packet to the overlay network. The ON and MNP 
// processes are connected with a local TCP connection. The packet 
// and its next hop's nodeID are encapsulated in a sendpkt_arg_t 
// data structure and sent to the ON process. The parameter 
// overlay_conn is the TCP connection's socket descriptor between 
// the MNP and ON processes. When sending the sendpkt_arg_t data 
// structure over the TCP connection between the MNP 
// process and the ON process, use '!&'  and '!#' as delimiters. 
// Return 1 if success, and -1 otherwise.
int overlay_sendpkt(int nextNodeID, mnp_pkt_t* pkt, int overlay_conn)
{
	sendpkt_arg_t arg;
	memset(&arg, 0, sizeof(arg));
	arg.nextNodeID = nextNodeID;
	arg.pkt = *pkt;

	char buf_start[2] = "!&";
	char buf_end[2] = "!#";

	if (send(overlay_conn, buf_start, 2, 0) < 0) {
		return -1;
	}
	if (send(overlay_conn, &arg, sizeof(sendpkt_arg_t), 0) < 0) {
		return -1;
	}
	if (send(overlay_conn, buf_end, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

// overlay_recvpkt() function is called by the MNP process to 
// receive a packet from the ON process. The parameter overlay_conn 
// is the TCP connection's socket descriptor between MNP and ON 
// processes. To receive the packet, this function uses a simple 
// finite state machine (FSM): 
// PKTSTART1 -- starting point 
// PKTSTART2 -- '!' received, expecting '&' to receive data 
// PKTRECV -- '&' received, start receiving data
// PKTSTOP1 -- '!' received, expecting '#' to finish receiving data 
// Return 1 if success, and -1 otherwise.
int overlay_recvpkt(mnp_pkt_t* pkt, int overlay_conn)
{
	char buf[sizeof(mnp_pkt_t) + 2];
	char c;
	int idx = 0;

	int state = PKTSTART1;
	while (recv(overlay_conn, &c, 1, 0) > 0) {
		switch (state) {
			case PKTSTART1:
				if (c == '!')
					state = PKTSTART2;
				break;
			case PKTSTART2:
				if (c == '&')
					state = PKTRECV;
				else
					state = PKTSTART1;
				break;
			case PKTRECV:
				if (c == '!') {
					buf[idx] = c;
					idx++;
					state = PKTSTOP1;
				} else {
					buf[idx] = c;
					idx++;
				}
				break;
			case PKTSTOP1:
				if (c == '#') {
					buf[idx] = c;
					assert(idx - 1 == sizeof(mnp_pkt_t));
					memcpy(pkt, buf, idx - 1);

					return 1;
				} else if (c == '!') {
					buf[idx] = c;
					idx++;
				} else {
					buf[idx] = c;
					idx++;
					state = PKTRECV;
				}
				break;
			default:
				break;

		}
	}
	return -1;
}

// This function is called by the ON process to receive a 
// sendpkt_arg_t data structure. The parameter network_conn is the 
// TCP connection's socket descriptor between MNP and ON processes. 
// To receive the packet, this function uses a simple finite state 
// machine (FSM): 
// PKTSTART1 -- starting point 
// PKTSTART2 -- '!' received, expecting '&' to receive data 
// PKTRECV -- '&' received, start receiving data
// PKTSTOP1 -- '!' received, expecting '#' to finish receiving data
// Return 1 if success, and -1 otherwise.
int getpktToSend(mnp_pkt_t* pkt, int* nextNode,int network_conn)
{
	sendpkt_arg_t arg;
	char buf[sizeof(sendpkt_arg_t) + 2];
	char c;
	int idx = 0;
	int state = PKTSTART1;

	while (recv(network_conn, &c, 1, 0) > 0) {
		
		switch (state) {
			case PKTSTART1:
				if (c == '!')
					state = PKTSTART2;
				break;
			case PKTSTART2:
				if (c == '&')
					state = PKTRECV;
				else
					state = PKTSTART1;
				break;
			case PKTRECV:
				if (c == '!') {
					buf[idx] = c;
					idx++;
					state = PKTSTOP1;
				} else {
					buf[idx] = c;
					idx++;
				}
				break;
			case PKTSTOP1:
				if (c == '#') {
					buf[idx] = c;
					assert(idx - 1 == sizeof(sendpkt_arg_t));
					memcpy(&arg, buf, idx - 1);
					*pkt = arg.pkt;
					*nextNode = arg.nextNodeID;
					return 1;
				} else if (c == '!') {
					buf[idx] = c;
					idx++;
				} else {
					buf[idx] = c;
					idx++;
					state = PKTRECV;
				}
				break;
			default:
				break;

		}
	}
	return -1;
}

// forwardpktToMNP() function is called after the ON process 
// receives a packet from a neighbor in the overlay network. The ON 
// process calls this function to forward the packet to MNP process. 
// The parameter network_conn is the TCP connection's socket 
// descriptor between the MNP and ON processes. The packet is sent 
// with delimiters !& and !#. 
// Return 1 if success, and -1 otherwise.
int forwardpktToMNP(mnp_pkt_t *pkt, int network_conn)
{
	char buf_start[2] = "!&";
	char buf_end[2] = "!#";
	if (send(network_conn, buf_start, 2, 0) < 0) {
		return -1;
	}
	if (send(network_conn, pkt, sizeof(mnp_pkt_t), 0) < 0) {
		return -1;
	}
	if (send(network_conn, buf_end, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

// sendpkt() function is called by the ON process to send a packet 
// received from the MNP process to the next hop. Parameter conn is 
// the TCP connection's socket descriptor to the next hop. The 
// packet is sent over the TCP connection between the ON process and 
// a neighboring node, and delimiters !& and !# are used. 
// Return 1 if success, and -1 otherwise.
int sendpkt(mnp_pkt_t* pkt, int conn)
{
	char buf_start[2] = "!&";
	char buf_end[2] = "!#";

	if (send(conn, buf_start, 2, 0) < 0) {
		return -1;
	}
	int pktsize = sizeof(mnp_hdr_t) + pkt->header.length;
	if (send(conn, pkt, pktsize, 0) < 0) {
		return -1;
	}
	if (send(conn, buf_end, 2, 0) < 0) {
		return -1;
	}
	return 1;
}

// recvpkt() function is called by the ON process to receive 
// a packet from a neighbor in the overlay network. Parameter conn 
// is the TCP connection's socket descriptor to a neighbor. The 
// packet is sent over the TCP connection  between the ON process 
// and the neighbor, and delimiters !& and !# are used. 
// To receive the packet, this function uses a simple finite state 
// machine (FSM): 
// PKTSTART1 -- starting point 
// PKTSTART2 -- '!' received, expecting '&' to receive data 
// PKTRECV -- '&' received, start receiving data
// PKTSTOP1 -- '!' received, expecting '#' to finish receiving data 
// Return 1 if success, and -1 otherwise.
int recvpkt(mnp_pkt_t* pkt, int conn)
{
	char buf[sizeof(mnp_pkt_t) + 2];
	char c;
	int index = 0;

	int state = PKTSTART1;
	while (recv(conn, &c, 1, 0) > 0) {
		switch (state) {
			case PKTSTART1:
				if (c == '!')
					state = PKTSTART2;
				break;
			case PKTSTART2:
				if (c == '&')
					state = PKTRECV;
				else
					state = PKTSTART1;
				break;
			case PKTRECV:
				if (c == '!') {
					buf[index] = c;
					index++;
					state = PKTSTOP1;
				} else {
					buf[index] = c;
					index++;
				}
				break;
			case PKTSTOP1:
				if (c == '#') {
					buf[index] = c;
					memcpy(pkt, buf, index - 1);

					return 1;
				} else if (c == '!') {
					buf[index] = c;
					index++;
				} else {
					buf[index] = c;
					index++;
					state = PKTRECV;
				}
				break;
			default:
				break;

		}
	}
	return -1;
}
