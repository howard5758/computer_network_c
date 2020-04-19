/*
 * overlay/overlay.h: definitions of functions used by ON process. 
 *
 * CS60, March 2018. 
 */

#ifndef OVERLAY_H 
#define OVERLAY_H

#include "../common/constants.h"
#include "../common/pkt.h"
#include "neighbortable.h"


/**
 * This thread opens a TCP port on CONNECTION_PORT and waits for the 
 * incoming connection from all the neighbors with node IDs larger 
 * than my nodeID. After all the incoming connections are 
 * established, this thread terminates. 
 */
void* waitNbrs(void* arg);


/**
 * This function connects to all the neighbors with node IDs smaller 
 * than my nodeID. 
 *
 * Return 1 after all the outgoing connections are established, 
 * otherwise -1
 */
int connectNbrs();


/**
 * This function opens a TCP port on OVERLAY_PORT, and waits for the 
 * incoming connection from local MNP process. After the local MNP 
 * process is connected, this function keeps getting send_arg_t 
 * structures from MNP process, and sends the packets to the next 
 * hop in the overlay network.
 */
void waitNetwork();


/**
 * This thread keeps receiving packets from a neighbor. It handles 
 * the received packets by forwarding the packets to the MNP process.
 * listen_to_neighbor threads are started after all the TCP 
 * connections to the neighbors are established
 *
 * @param arg arguments of the thread
 */
void* listen_to_neighbor(void* arg);


/**
 * This function stops the overlay. It closes all the connections 
 * and frees all the dynamically allocated memory. It is called when 
 * receiving a signal SIGINT
 */
void overlay_stop(); 

#endif //OVERLAY_H
