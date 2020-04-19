/*
 * network/network.h: definitions of functions for the MNP process.
 *
 * CS60, March 2018. 
 */

#ifndef NETWORK_H
#define NETWORK_H


/**
 * This function is for the MNP process to connect to the local ON 
 * process on port OVERLAY_PORT.
 *
 * Return connection descriptor if success, otherwise -1
 */
int connectToOverlay();


/**
 * This thread sends out route update packets every RU_INTERVAL time
 * In this lab this thread only broadcasts empty route update 
 * packets to all neighbors. Broadcasting is done by set the 
 * dest_nodeID in packet header as BROADCAST_NODEID. 
 *
 * @param arg
 */
void *routeupdate_daemon(void *arg);

/**
 * This thread handles incoming packets from the ON process. 
 * It receives packets from the ON process by calling 
 * overlay_recvpkt(). 
 * In this lab, after receiving a packet, this thread just outputs 
 * the packet received information without handling the packet. 
 */
void *pkthandler(void *arg);


/**
 * This function stops the MNP process. 
 * It closes all the connections and frees all dynamically 
 * allocated memory. It is called when the MNP process receives a 
 * signal SIGINT. 
 */
void network_stop();

#endif //NETWORK_H
