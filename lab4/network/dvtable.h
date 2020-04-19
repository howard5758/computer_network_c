/*
 * network/dvtable.h: definitions of data structures and 
 * operations for the distance vector table used by the routing 
 * protocol of MNP. 
 *
 * CS60, March 2018. 
 */

#ifndef DVTABLE_H
#define DVTABLE_H

#include "../common/pkt.h"

/**************** global types ****************/

/* dv_entry_t structure definition */
typedef struct distancevectorentry {
	int nodeID;        	// destination nodeID
	unsigned int cost;	// cost to the destination
} dv_entry_t;


/* A distance vector table contains the n+1 dv_t entries,
 * where n is the number of the neighbors of this node,
 * and the rest one is for this node itself. */
typedef struct distancevector {
	int nodeID;        		// source nodeID
	dv_entry_t *dvEntry; 	// an array of N dv_entry_ts, each contains the destination node ID and the cost to the destination from the source node. N is the total number of nodes in the network.
} dv_t;


/**************** functions ****************/

/**
 * This function creates a distance vector table dynamically.
 * A distance vector table contains the n+1 entries, where n is the 
 * number of the neighbors of this node, and one is for this node 
 * itself.
 * Each entry in distance vector table is a dv_t structure which 
 * contains a source node ID and an array of N dv_entry_t structures 
 * where N is the number of nodes in the network.
 * Each dv_entry_t contains a destination node ID and the cost from 
 * the source node to this destination node.
 * The dvtable is initialized in this function.
 * The link costs from this node to its neighbors are initialized 
 * using direct link cost retrieved from topology.dat. Other link 
 * costs are initialized to INFINITE_COST.
 *
 * Return the dynamically created dvtable.
 */
dv_t *dvtable_create();


/**
 * This function destroys a dvtable.
 * It frees all the dynamically allocated memory for the dvtable.
 */
void dvtable_destroy(dv_t *dvtable);


/**
 * This function sets the link cost between two nodes in dvtable.
 * Return 1 if these two nodes are found in the table and the link 
 * cost is set, otherwise -1.
 */
int dvtable_setcost(dv_t *dvtable, int fromNodeID, int toNodeID, unsigned int cost);


/**
 * This function returns the link cost between two nodes in dvtable. 
 * Return the link cost if these two nodes are found in dvtable, 
 * otherwise INFINITE_COST.
 */
unsigned int dvtable_getcost(dv_t *dvtable, int fromNodeID, int toNodeID);


/**
 * This function prints out the contents of a dvtable. 
 */
void dvtable_print(dv_t *dvtable);

#endif //DVTABLE_H
