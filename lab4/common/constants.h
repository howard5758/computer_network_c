/*
 * common/constants.h: constants used by mini overlay network
 *
 * CS60, March 2018 
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

/*******************************************************************/
// transport layer parameters
/*******************************************************************/

// The MAX connections can be supported by MRT. Your TCB table 
// should contain MAX_TRANSPORT_CONNECTIONS entries
#define MAX_TRANSPORT_CONNECTIONS 10

// Maximal segment length
// MAX_SEG_LEN = 1500 - sizeof(seg header) - sizeof(ip header)
#define MAX_SEG_LEN  1464

// Packet loss rate is 10%
#define PKT_LOSS_RATE 0.1

// SYN_TIMEOUT value in nanoseconds
#define SYN_TIMEOUT 500000000

// FIN_TIMEOUT value in nanoseconds
#define FIN_TIMEOUT 500000000

// max number of SYN retransmissions in mrt_client_connect()
#define SYN_MAX_RETRY 5

// max number of FIN retransmissions in mrt_client_disconnect()
#define FIN_MAX_RETRY 5

// server close-wait timeout value in seconds
#define CLOSEWAIT_TIME 1

// sendBuf_timer thread's polling interval in nanoseconds
#define SENDBUF_POLLING_INTERVAL 100000000

// MRT client polls the receive buffer with this time interval in 
// order to check if requested data are available in mrt_srv_recv() 
// function in seconds
#define RECVBUF_POLLING_INTERVAL 1

// mrt_svr_accept() function uses this interval in nanoseconds to 
// busy wait on the TCB state
#define ACCEPT_POLLING_INTERVAL 100000000

// size of receive buffer
#define RECEIVE_BUF_SIZE 1000000

// DATA segment timeout value in microseconds
#define DATA_TIMEOUT 500000

// GBN window size
#define GBN_WINDOW 10



/*******************************************************************/
// overlay parameters
/*******************************************************************/

// this is the port number that is used for nodes to interconnect each other to form an overlay, you should change this to a random value to avoid conflicts with other students
#define CONNECTION_PORT 3423

// this is the port number that is opened by overlay process and connected by the network layer process, you should change this to a random value to avoid conflicts with other students
#define OVERLAY_PORT 3548

// max packet data length
#define MAX_PKT_LEN 1488 



/*******************************************************************/
// network layer parameters
/*******************************************************************/
// max node number supported by the overlay network 
#define MAX_NODE_NUM 10

// max number of slots in the routing table  
#define MAX_ROUTINGTABLE_SLOTS 10

// infinite link cost value, if two nodes are disconnected, they will have link cost INFINITE_COST
#define INFINITE_COST 999

// the network layer process opens this port, and waits for connection from transport layer process, you should change this to a random value to avoid conflicts with other students
#define NETWORK_PORT 4162

// this is the broadcasting nodeID. If the overlay layer process receives a packet destined to BROADCAST_NODEID from the network layer process, it should send this packet to all the neighbors
#define BROADCAST_NODEID 9999

// route update broadcasting interval in seconds
#define ROUTEUPDATE_INTERVAL 5

#endif //CONSTANTS_H
