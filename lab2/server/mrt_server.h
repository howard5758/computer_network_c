/*
 * client/mrt_server.h: definitions of server states, related data 
 * structures, and server socket interface definitions. 
 *
 * CS60, March 2018. 
 */

#ifndef MRTSERVER_H
#define MRTSERVER_H

#include "../common/seg.h"
#include "../common/constants.h"

/**************** global types ****************/
// server states used in FSM 
enum svr_state_t {
	SVR_CLOSED,
	SVR_LISTENING,
	SVR_CONNECTED,
	SVR_CLOSEWAIT
};

// server transport control block (TCB), containing overlay 
// connection information at the server side. 
typedef struct svr_tcb {
	unsigned int svr_nodeID;        // node ID of server, similar as IP address, currently unused
	unsigned int svr_portNum;       // port number of server
	unsigned int client_nodeID;     // node ID of client, similar as IP address, currently unused
	unsigned int client_portNum;    // port number of client
	unsigned int state;            // state of server

	// the following fields are for data transmission
	unsigned int expect_seqNum;
	char *recvBuf;
	unsigned int usedBufLen;
	pthread_mutex_t *bufMutex;
} server_tcb_t;

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
 * descriptor "conn" used as input parameter for mnp_sendseg() and 
 * mnp_recvseg(). Finally, the function starts the seghandler thread 
 * to handle the incoming segments.
 *
 * Note: There is only one seghandler for the server side which
 * handles call connections for the client.
 *
 * @param conn - socket descriptor
 */
void mrt_server_init(int conn);


/**
 * This function looks up the server TCB table to find the first 
 * NULL entry, and creates a new TCB entry using malloc() for that 
 * entry. All fields in the TCB are initialized, e.g., TCB state is 
 * set to CLOSED and the server port set to the function call 
 * parameter server port. The TCB table entry index should be 
 * returned as the new socket ID to the client and be used to 
 * identify the connection on the client side. If no entry in the 
 * TCB table is available the function returns -1.
 *
 * @param port - port number
 * @return 1 for success; -1 for failure
 */
int mrt_server_sock(unsigned int port);


/**
 * This function gets the TCB pointer using the sockfd and changes 
 * the state of the connection to LISTENING. It then starts a timer 
 * to "busy wait" until the TCB's state changes to CONNECTED 
 * (seghandler does this when a SYN is received). It waits in an 
 * infinite loop for the state transition before proceeding and to 
 * return 1 when the state change happens, dropping out of the busy 
 * wait loop. You can implement this blocking wait in different 
 * ways, if you wish.
 *
 * @param sockfd - socket file descriptor
 * @return 1 for successful change; -1 for failure
 */
int mrt_server_accept(int sockfd);


/**
 * Receive data to a MRT client. Recall this is a unidirectional 
 * transport where DATA flows from the client to the server. 
 * Signaling/control messages such as SYN, SYNACK, etc. flow in both 
 * directions. 
 *
 * @param sockfd - socket file descriptor
 * @param buf - receiving buffer
 * @param length - buffer length
 * @return 1 for success; -1 for failure
 */
int mrt_server_recv(int sockfd, void *buf, unsigned int length);


/**
 * This function calls free() to free the TCB entry. It marks that 
 * entry in TCB as NULL and returns 1 if succeeded (i.e., was in the 
 * right state to complete a close) and -1 if fails (i.e., in the 
 * wrong state).
 *
 * @param sockfd - socket file descriptor
 * @return 1 for success; -1 for failure
 */
int mrt_server_close(int sockfd);


/**
 * This is a thread started by mrt_server_init(). It handles all the 
 * incoming segments from the client. The design of seghanlder is an 
 * infinite loop that calls mnp_recvseg(). If mnp_recvseg() fails 
 * then the overlay connection is closed and the thread is 
 * terminated. Depending on the state of the connection when a 
 * segment is received (based on the incoming segment) various
 * actions are taken. See the server FSM for more details.
 *
 * @param arg - arguments for the thread
 * @return 1 for success; -1 for failure
 */
void *seghandler(void *arg);

#endif //MRTSERVER_H
