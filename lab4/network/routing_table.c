
/*
 * network/routing_table.c: implementation of the routing table used 
 * by the routing protocol of MNP. A routing table is a hash table 
 * containing MAX_RT_ENTRY slot entries.
 *
 * CS60, March 2018. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../common/constants.h"
#include "../topology/topology.h"
#include "routing_table.h"

/** 
 * This is the hash function used the by the routing table
 * It takes the hash key - destination node ID as input,
 * and returns the hash value - slot number for this destination 
 * node ID.
 */
int makehash(int node) 
{
	return node % MAX_ROUTINGTABLE_SLOTS;
}

/** TODO: 
 * This function creates a routing table dynamically.
 * All the entries in the table are initialized to NULL pointers.
 * Then for all the neighbors with direct links, create a routing 
 * entry with the neighbor itself as the next hop node, and insert 
 * this routing entry into the routing table.
 *
 * Return The dynamically created routing table structure.
 */
routingtable_t *routingtable_create() 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	int *nbr_array = topology_getNbrArray();
	int idx;
	routingtable_t *routing_table = malloc(sizeof(routingtable_t));
	// create routingtable based on nbr_array
	for(int i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++) routing_table->hash[i] = NULL;
	
	for(int i = 0; i < nbr_num; i++){
		idx = makehash(nbr_array[i]);
		if(routing_table->hash[idx] == NULL){
			struct routingtable_entry *new = malloc(sizeof(struct routingtable_entry));
			new->destNodeID = nbr_array[i];
			new->nextNodeID = nbr_array[i];
			new->next = NULL;
			routing_table->hash[idx] = new;
		}
		else{
			routingtable_entry_t *new = malloc(sizeof(struct routingtable_entry));
			new->destNodeID = nbr_array[i];
			new->nextNodeID = nbr_array[i];
			new->next = NULL;

			routingtable_entry_t *head;
			head = routing_table->hash[idx];
			while(head->next != NULL) head = head->next;
			head->next = new;

		}
	}
	return routing_table;
}

/** TODO: 
 * This function destroys a routing table.
 * All dynamically allocated data structures for this routing table are freed. 
 */
void routingtable_destroy(routingtable_t *routing_table) 
{
	for(int i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++){
		if(routing_table->hash[i] != NULL){
			routingtable_entry_t *head;
			routingtable_entry_t *prev;
			head = routing_table->hash[i];
			prev = head;
			while(head->next != NULL){
				head = head->next;
				free(prev);
				prev = head;
			}
			free(head);
		}
	}
	free(routing_table);
	routing_table = NULL;
}

/** TODO: 
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
void routingtable_setnextnode(routingtable_t *routing_table, int destNodeID, int nextNodeID) 
{
	int idx = makehash(destNodeID);
	
	if(routing_table->hash[idx] == NULL){
		routingtable_entry_t *new = malloc(sizeof(struct routingtable_entry));
		new->destNodeID = destNodeID;
		new->nextNodeID = nextNodeID;
		new->next = NULL;
		routing_table->hash[idx] = new;
	}
	else{
		routingtable_entry_t *head;
		head = routing_table->hash[idx];
		while(head->next != NULL){
			if(head->destNodeID == destNodeID){
				head->nextNodeID = nextNodeID;
				break;
			}
			head = head->next;
		}
		if(head->destNodeID == destNodeID){
			head->nextNodeID = nextNodeID;
		}
		else{
			routingtable_entry_t *new = malloc(sizeof(struct routingtable_entry));;
			new->destNodeID = destNodeID;
			new->nextNodeID = nextNodeID;
			new->next = NULL;

			head->next = new;
		}
	}
}

/** TODO: 
 * This function looks up the destNodeID in the routing table.
 * Since routing table is a hash table, this opeartion has O(1) time complexity.
 * To find a routing entry for a destination node, first use the 
 * hash function makehash() to get the slot number and then go 
 * through the linked list in that slot to search for the routing 
 * entry.
 * Return nextNodeID if the destNodeID is found, else -1. 
 */
int routingtable_getnextnode(routingtable_t *routing_table, int destNodeID) 
{
	int idx = makehash(destNodeID);
	if(routing_table->hash[idx] != NULL){
		routingtable_entry_t *head;
		head = routing_table->hash[idx];
		while(head->next != NULL){
			if(head->destNodeID == destNodeID) return head->nextNodeID;
			head = head->next;
		}
	}
	return -1;
}

/** TODO: 
 * This function prints out the contents of the routing table. 
 */
void routingtable_print(routingtable_t *routing_table) {
	printf("-------------routing table------------\n");
	for(int i = 0; i < MAX_ROUTINGTABLE_SLOTS; i++){
		if(routing_table->hash[i] == NULL) printf("NULL\n");
		else{
			routingtable_entry_t *head;
			head = routing_table->hash[i];
			while(head->next != NULL){
				printf("dest %i next %i  ||  ", head->destNodeID, head->nextNodeID);
			 	head = head->next;
			}
			printf("dest %i next %i \n", head->destNodeID, head->nextNodeID);
		}
	}
	printf("--------------------------------------\n");
}
