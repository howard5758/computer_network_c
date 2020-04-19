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
 * This thread sends out route update packets every RU_INTERVAL time.
 * The route update packet contains this node's distance vector. 
 * Broadcasting is done by set the dest_nodeID in packet header as 
 * BROADCAST_NODEID and use overlay_sendpkt() to send the packet out.
 *
 * @param arg
 */
void *routeupdate_daemon(void *arg);

/**
 * This thread handles incoming packets from the ON process. 
 * It receives packets from the ON process by calling 
 * overlay_recvpkt(). 
 * If the packet is a MNP packet and the destination node is this 
 * node, forward the packet to the MRT process.
 * If the packet is a MNP packet and the destination node is not 
 * this node, forward the packet to the next hop according to the 
 * routing table.
 * If this packet is an Route Update packet, update the distance 
 * vector table and the routing table.
 */
void *pkthandler(void *arg);


/**
 * This function stops the MNP process. 
 * It closes all the connections and frees all dynamically 
 * allocated memory. It is called when the MNP process receives a 
 * signal SIGINT. 
 */
void network_stop();

/**
 * This function opens a port on NETWORK_PORT and waits for the TCP 
 * connection from local MRT process.
 * After the local MRT process is connected, this function keeps 
 * receiving sendseg_arg_ts which contains the segments and their 
 * destination node addresses from the MRT process.
 * The received segments are then encapsulated into packets (one 
 * segment in one packet), and sent to the next hop using 
 * overlay_sendpkt(). The next hop is retrieved from routing table.
 * When a local MRT process is disconnected, this function waits for 
 * the next MRT process to connect.
 */
void waitTranport();

#endif //NETWORK_H
