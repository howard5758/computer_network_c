/*
 * network/routing_table.h: definitions of data structures and 
 * operations for the routing table used by the routing protocol of 
 * MNP. A routing table is a hash table containing MAX_RT_ENTRY slot 
 * entries.
 *
 * CS60, March 2018. 
 */

#ifndef ROUTINGTABLE_H
#define ROUTINGTABLE_H

/**************** global types ****************/

/* the routing entry contained in the routing table */
typedef struct routingtable_entry {
	int destNodeID;	// destination node ID
	int nextNodeID;	// next-hop node ID to forward the packet
	struct routingtable_entry *next; // pointer to the next routingtable_entry_t in the same routing table slot
} routingtable_entry_t;

/* routing table as a hash table with MAX_ROUTINGTABLE_SLOTS slots.
 * Each slot is a linked list of routing entries. */
typedef struct routing_table {
	routingtable_entry_t *hash[MAX_ROUTINGTABLE_SLOTS];
} routingtable_t;


/**************** functions ****************/

/**
 * This is the hash function used the by the routing table
 * It takes the hash key - destination node ID as input,
 * and returns the hash value - slot number for this destination 
 * node ID.
 *
 * @example
 * int makehash(int node) {
 * 		return node%MAX_ROUTINGTABLE_ENTRIES;
 * }
 */
int makehash(int node);


/**
 * This function creates a routing table dynamically.
 * All the entries in the table are initialized to NULL pointers.
 * Then for all the neighbors with direct links, create a routing 
 * entry with the neighbor itself as the next hop node, and insert 
 * this routing entry into the routing table.
 *
 * Return The dynamically created routing table structure.
 */
routingtable_t *routingtable_create();


/**
 * This function destroys a routing table.
 * All dynamically allocated data structures for this routing table are freed. 
 */
void routingtable_destroy(routingtable_t *routing_table);


/**
 * This function updates the routing table using the given 
 * destination node ID and next hop's node ID.
 * If the routing entry for the given destination already exists, 
 * update the existing routing entry.
 * If the routing entry of the given destination is not there, add 
 * one with the given next node ID.
 * Each slot in the routing table contains a linked list of routing 
 * entries due to conflicting hash keys (different hash keys - 
 * destination node ID - may have the same hash value (slot entry 
 * number).
 * To add an routing entry to the hash table:
 * First use the hash function makehash() to get the slot number in 
 * which this routing entry should be stored.
 * Then append the routing entry to the linked list in that slot. 
 */
void routingtable_setnextnode(routingtable_t *routing_table, int destNodeID, int nextNodeID);


/**
 * This function looks up the destNodeID in the routing table.
 * Since routing table is a hash table, this opeartion has O(1) time complexity.
 * To find a routing entry for a destination node, first use the 
 * hash function makehash() to get the slot number and then go 
 * through the linked list in that slot to search for the routing 
 * entry.
 * Return nextNodeID if the destNodeID is found, else -1. 
 */
int routingtable_getnextnode(routingtable_t *routing_table, int destNodeID);


/**
 * This function prints out the contents of the routing table. 
 */
void routingtable_print(routingtable_t *routing_table);

#endif //ROUTINGTABLE_H
