/*
 * overlay/neighbortable.h: definitions of data structures and APIs 
 * for neighbor table.  
 *
 * CS60, March 2018. 
 */

#ifndef NEIGHBORTABLE_H 
#define NEIGHBORTABLE_H

#include <arpa/inet.h>

/**************** global types ****************/

/**
 * Definition of a neighbor table entry. 
 * A neighbor table contains n entries where n is the number of 
 * neighbors. Each Node has a Overlay Network (ON) process running, 
 * and each ON process maintains the neighbor table for the node 
 * that the process is running on.
 */
typedef struct neighborentry {
  int nodeID;	      	// neighbor's node ID
  in_addr_t nodeIP; 	// neighbor's IP address
  int conn;	      		// TCP connection's socket descriptor to the neighbor
} nbr_entry_t;


/**************** functions ****************/

/**
 * This function first creates a neighbor table dynamically.
 * It then parses the topology/topology.dat file and fills the 
 * nodeID and nodeIP fields in all the entries, initialize conn 
 * field as -1.
 *
 * @return the created neighbor table
 */
nbr_entry_t* nt_create();


/**
 * This function destroys a neighbortable. It closes all the 
 * connections and frees all the dynamically allocated memory.
 *
 * @param nt
 */
void nt_destroy(nbr_entry_t* nt);


/**
 * This function assigns a TCP connection to a neighbor table entry 
 * for a neighboring node.
 *
 * @param nt
 * @param nodeID
 * @param conn
 * Return 1 if successful and -1 otherwise. 
 */
int nt_addconn(nbr_entry_t* nt, int nodeID, int conn);

#endif //NEIGHBORTABLE_H
