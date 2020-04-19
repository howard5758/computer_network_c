/*
 * network/nbrcosttable.h: definitions of data structures and 
 * operations for the neighbor cost table used by the routing 
 * protocol of MNP. 
 *
 * CS60, March 2018. 
 */

#ifndef NBRCOSTTABLE_H
#define NBRCOSTTABLE_H

#include <arpa/inet.h>

/**************** global types ****************/

/* neighbor cost table entry definition */
typedef struct neighborcostentry {
	unsigned int nodeID;    //neighbor's node ID
	unsigned int cost;    //direct link cost to the neighbor
} nbr_cost_entry_t;


/**************** functions ****************/

/**
 * This function creates a neighbor cost table dynamically
 * and initializes the table with all its neighbors' node IDs and 
 * direct link costs. The neighbors' node IDs and direct link costs 
 * are retrieved from topology.dat file.
 *
 * Return the pointer to the table
 */
nbr_cost_entry_t *nbrcosttable_create();


/**
 * This function destroys a neighbor cost table.
 * It frees all the dynamically allocated memory for the neighbor 
 * cost table.
 */
void nbrcosttable_destroy(nbr_cost_entry_t *nct);


/**
 * This function is used to get the direct link cost from neighbor 
 * specified by its node ID.
 * Return the direct link cost if the neighbor is found in the 
 * table, otherwise INFINITE_COST if the node is not found. 
 */
unsigned int nbrcosttable_getcost(nbr_cost_entry_t *nct, int nodeID);


/**
 * This function prints out the contents of a neighbor cost table.
 */
void nbrcosttable_print(nbr_cost_entry_t *nct);

#endif //NBRCOSTTABLE_H
