/*
 * client/mrt_client.h: definitions of client states, related data 
 * structures, and client socket interface definitions. 
 *
 * CS60, March 2018. 
 */


#ifndef MRTCLIENT_H
#define MRTCLIENT_H

#include <pthread.h>
#include "../common/seg.h"

/**************** global types ****************/
// client states
enum client_state_t {
	CLIENT_CLOSED,
	CLIENT_SYNSENT,
	CLIENT_CONNECTED,
	CLIENT_FINWAIT
};

// unit to store segments in send buffer. 
typedef struct segBuf {
	seg_t seg;
	unsigned int sentTime;
	struct segBuf* next;
} segBuf_t;


// client transport control block (TCB), containing overlay 
// connection information at the client side. 
typedef struct client_tcb {
	unsigned int svr_nodeID;        //node ID of server, similar as IP address, currently unused
	unsigned int svr_portNum;       //port number of server
	unsigned int client_nodeID;     //node ID of client, similar as IP address, currently unused
	unsigned int client_portNum;    //port number of client
	unsigned int state;     				//state of client

	// fields below are used for data transmission 
	unsigned int next_seqNum;
	pthread_mutex_t* bufMutex;
	segBuf_t* sendBufHead;
	segBuf_t* sendBufunSent;
	segBuf_t* sendBufTail;
	unsigned int unAck_segNum;
} client_tcb_t;


/**************** functions ****************/
/*
 * In what follows, we provide the prototype definition for each 
 * call and limited pseudo code representation of the function. 
 *
 * This is not meant to be comprehensive - more a guideline. You are 
 * free to design the code as you wish.
 *
 * NOTE: When designing all functions you should consider all 
 * possible states of the FSM using a switch statement. Typically, 
 * the FSM has to be in a certain state determined by the FSM design 
 * to carry out a certain action.
 */


/**
 * This function initializes the TCB table marking all entries NULL. 
 * It also initializes a global variable for the overlay TCP socket 
 * descriptor "conn" used as an input parameter for mnp_sendseg() 
 * and mnp_recvseg(). Finally, the function starts the seghandler 
 * thread to handle incoming segments.
 *
 * Note: There is only one seghandler at the client side which
 * handles call connections for the client.
 *
 * @param conn - socket descriptor
 */
void mrt_client_init(int conn);


/**
 * This function looks up the client TCB table to find the first 
 * NULL entry, and creates a new TCB entry using malloc() for that 
 * entry. All fields in the TCB are initialized, e.g., TCB state is 
 * set to CLOSED and the server port set to the function call 
 * parameter server port. The TCB table entry index should be 
 * returned as the new socket ID to the server and be used to 
 * identify the connection on the server side. If no entry in the 
 * TCB table is available the function returns -1.
 *
 * @param port - port number
 * @return 1 for success, -1 for failure
 */
int mrt_client_sock(unsigned int client_port);


/**
 * This function is used to connect to the server. It takes the 
 * socket ID and the server's port number as input parameters. The 
 * socket ID is used to find the TCB entry. This function sets up 
 * the TCB's server port number and a SYN segment to send to
 * the server using mnp_sendseg(). After the SYN segment is sent, a 
 * timer is started. If no SYNACK is received after SYNSEG_TIMEOUT 
 * timeout, then the SYN is retransmitted. If SYNACK is received, 
 * return 1. Otherwise, if the number of SYNs sent > SYN_MAX_RETRY, 
 * transition to CLOSED state and return -1.
 *
 * @param sockfd - socket file descriptor
 * @param server_port - server port for connection
 * @return 1 for successful change; -1 for failure
 */
int mrt_client_connect(int sockfd, unsigned int server_port);


/**
 * This function sends data to a MRT server. 
 *
 * @param sockfd
 * @param data
 * @param length
 * @return 1 for success; -1 for failure
 */
int mrt_client_send(int sockfd, void* data, unsigned int length);


/**
 * This function disconnects from the server. It takes the socket 
 * ID as an input parameter. The socket ID is used to find the TCB 
 * entry in the TCB table. This function sends a FIN segment to the 
 * server. After the FIN segment is sent the state should transition 
 * to FINWAIT and a timer started. If the state == CLOSED after the 
 * timeout the FINACK was successfully received. Else, if after a 
 * number of retries FIN_MAX_RETRY the state is still FINWAIT then
 * the state transitions to CLOSED and -1 is returned.
 *
 * @param sockfd
 * @return 1 for success; -1 for failure
 */
int mrt_client_disconnect(int sockfd);


/**
 * This function calls free() to free the TCB entry. It marks that 
 * entry in TCB as NULL and returns 1 if succeeded (i.e., was in the 
 * right state to complete a close) and -1 if fails (i.e., in the 
 * wrong state).
 *
 * @param sockfd
 * @return 1 for success; -1 for failure
 */
int mrt_client_close(int sockfd);

/**
 * This is a thread started by mrt_client_init(). It handles all the 
 * incoming segments from the server. The design of seghanlder is an 
 * infinite loop that calls mnp_recvseg(). If mnp_recvseg() fails 
 * then the overlay connection is closed and the thread is 
 * terminated. Depending on the state of the connection when a 
 * segment is received (based on the incoming segment) various 
 * actions are taken. See the client FSM for more details.
 *
 * @param arg
 * @return 1 for success; -1 for failure
 */
void *sendBuf_timer(void* sock);

void *seghandler(void* arg);


#endif //MRTCLIENT_H
