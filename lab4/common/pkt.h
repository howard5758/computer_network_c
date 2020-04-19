/*
 * common/pkt.h: packet definitions and interfaces to send and 
 * receive packets.
 *
 * CS60, March 2018. 
 */

#ifndef PKT_H
#define PKT_H

#include "constants.h"

/**************** constants ****************/

// packet type definition, used for type field in packet header
#define	ROUTE_UPDATE 1
#define MNP 2	

/**************** global types ****************/

// MNP packet format definition
typedef struct mnpheader {
  int src_nodeID;		          // source node ID
  int dest_nodeID;		        // destination node ID
  unsigned short int length;	// length of the data in the packet
  unsigned short int type;	  // type of the packet 
} mnp_hdr_t;

typedef struct packet {
  mnp_hdr_t header;
  char data[MAX_PKT_LEN];
} mnp_pkt_t;

/* *******
 * route update packet definition
 * for a route update packet, the route update information will be 
 * stored in the data field of a packet 
 */

// a route update entry
typedef struct routeupdate_entry {
  unsigned int nodeID;  // destination nodeID
  unsigned int cost;    // link cost from the source (src_nodeID in header) to destination node
} routeupdate_entry_t;

// route update packet format
typedef struct pktrt{
  unsigned int entryNum;	// # of entries in this update packet
  routeupdate_entry_t entry[MAX_NODE_NUM];
} pkt_routeupdate_t;


// data structure used in overlay_sendpkt(). overlay_sendpkt() is 
// called by the MNP process to request the ON process to send a 
// packet to the overlay network. The ON process receives this data 
// structure by calling getpktToSend(). It then sends the packet to 
// the next hop by calling sendpkt().
typedef struct sendPktArgument {
  int nextNodeID; // node ID of the next hop
  mnp_pkt_t pkt;  // the packet to be sent
} sendpkt_arg_t;


/**************** functions ****************/

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
int overlay_sendpkt(int nextNodeID, mnp_pkt_t* pkt, int overlay_conn);

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
int overlay_recvpkt(mnp_pkt_t* pkt, int overlay_conn);

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
int getpktToSend(mnp_pkt_t* pkt, int* nextNode,int network_conn);

// forwardpktToMNP() function is called after the ON process 
// receives a packet from a neighbor in the overlay network. The ON 
// process calls this function to forward the packet to MNP process. 
// The parameter network_conn is the TCP connection's socket 
// descriptor between the MNP and ON processes. The packet is sent 
// with delimiters !& and !#. 
// Return 1 if success, and -1 otherwise.
int forwardpktToMNP(mnp_pkt_t *pkt, int network_conn);

// sendpkt() function is called by the ON process to send a packet 
// received from the MNP process to the next hop. Parameter conn is 
// the TCP connection's socket descriptor to the next hop. The 
// packet is sent over the TCP connection between the ON process and 
// a neighboring node, and delimiters !& and !# are used. 
// Return 1 if success, and -1 otherwise.
int sendpkt(mnp_pkt_t* pkt, int conn);

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
int recvpkt(mnp_pkt_t* pkt, int conn);

#endif //PKT_H
