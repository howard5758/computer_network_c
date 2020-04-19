/*
 * common/constants.h: constants used by mini reliable transport (
 * MRT) protocol
 *
 * CS60, March 2018 
 */


#ifndef CONSTANTS_H
#define CONSTANTS_H

// Overlay port opened by the server. the client will connect to 
// this port. You should choose a random port to avoid conflicts 
// with your classmates. Because you may log onto the same computer.
#define OVERLAY_PORT 9009

// The MAX connections can be supported by MRT. Your TCB table 
// should contain MAX_TRANSPORT_CONNECTIONS entries
#define MAX_TRANSPORT_CONNECTIONS 10

// Maximal segment length
// MAX_SEG_LEN = 1500 - sizeof(seg header) - sizeof(ip header)
#define MAX_SEG_LEN  1464

// Packet loss rate is 10%
#define PKT_LOSS_RATE 0.1

// SYN_TIMEOUT value in nanoseconds
#define SYNSEG_TIMEOUT_NS 500000000

// FIN_TIMEOUT value in nanoseconds
#define FINSEG_TIMEOUT_NS 500000000

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
#define DATA_TIMEOUT 100000

// GBN window size
#define GBN_WINDOW 10

// The packet loss rate
#define LOSS_RATE 0.1

#endif //CONSTANTS_H
