/*
 * topology/topology.h: definitions of functions to parse the 
 * topology file 
 *
 * CS60, March 2018. 
 */

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <assert.h>
#include "../common/constants.h"

/**************** functions ****************/

/**
 * This function returns node ID of the given hostname. The node ID 
 * is an integer of the last 8 digits of the node's IP address. For 
 * example, a node with IP address 202.120.92.3 will have node ID 3. 
 *
 * @param hostname
 * Return -1 if the node ID can't be retrieved, otherwise 1
 */
int topology_getNodeIDfromname(char* hostname);

/**
 * This function returns the node ID based on an IP address. 
 *
 * Return -1 if my node ID can't be retrieved, otherwise 1
 */
int topology_getNodeIDfromip(struct in_addr* addr);

/**
 * This functions returns my node ID. 
 *
 * Return -1 if my node ID can't be retrieved, otherwise 1
 */
int topology_getMyNodeID();

/**
 * This functions parses the topology information stored in 
 * topology.dat
 *
 * Return the number of neighbors
 */
int topology_getNbrNum();

/**
 * This functions parses the topology information stored in 
 * topology.dat
 *
 * Return the number of total nodes in the overlay
 */
int topology_getNodeNum();

/**
 * This functions parses the topology information stored in 
 * topology.dat, and returns a dynamically allocated array which 
 * contains all the nodes' IDs in the overlay network. 
 *
 */
int* topology_getNodeArray();

/**
 * This functions parses the topology information stored in 
 * topology.dat and returns a dynamically allocated array which 
 * contains all the neighbors'IDs
 */
int* topology_getNbrArray();

/**
 * This functions parses the topology information stored in topology.dat
 *
 * @param fromNodeID
 * @param toNodeID
 * Return the cost of the direct link between the two given nodes. 
 * Return INFINITE_COST if no direct link between the two nodes. 
 */
unsigned int topology_getCost(int fromNodeID, int toNodeID);

int indexOf(int *arraym, int array_size, int number);

#endif //TOPOLOGY_H
