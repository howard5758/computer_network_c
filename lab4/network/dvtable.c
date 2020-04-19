/*
 * network/dvtable.c: implementation of functions for the distance 
 * vector table used by the routing protocol of MNP. 
 * Each node has an ON process maintaining a distance vector table 
 * for the node that it is running on.
 *
 * CS60, March 2018. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../common/constants.h"
#include "../topology/topology.h"
#include "dvtable.h"

/** TODO: 
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
dv_t *dvtable_create() 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	int node_num = topology_getNodeNum();

	dv_t *dv_table = malloc((nbr_num+1)*sizeof(dv_t));
	int *nbrArray = topology_getNbrArray();
	int *nodeArray = topology_getNodeArray();
	int myID = topology_getMyNodeID();

	// initialize dvtable based on topology.dat
	for(int i = 0; i <= nbr_num; i++){
		if(i == nbr_num){
			dv_table[i].nodeID = myID;
			dv_table[i].dvEntry = malloc(node_num*sizeof(dv_entry_t));
			for(int j = 0; j < node_num; j++){
				dv_table[i].dvEntry[j].nodeID = nodeArray[j];
				dv_table[i].dvEntry[j].cost = topology_getCost(nodeArray[j], myID);
 			}
		}
		else{
			dv_table[i].nodeID = nbrArray[i];
			dv_table[i].dvEntry = malloc(node_num*sizeof(dv_entry_t));
			for(int j = 0; j < node_num; j++){
				dv_table[i].dvEntry[j].nodeID = nodeArray[j];
				dv_table[i].dvEntry[j].cost = INFINITE_COST;
 			}
		}
	}

	free(nbrArray);
	free(nodeArray);

	return dv_table; 
}

/** TODO: 
 * This function destroys a dvtable.
 * It frees all the dynamically allocated memory for the dvtable.
 */
void dvtable_destroy(dv_t *dvtable) 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	for(int i = 0; i <= nbr_num; i++){
		free(dvtable[i].dvEntry);
		dvtable[i].dvEntry = NULL;
	}
	free(dvtable);
	dvtable = NULL;
}

/** TODO: 
 * This function sets the link cost between two nodes in dvtable.
 * Return 1 if these two nodes are found in the table and the link 
 * cost is set, otherwise -1.
 */
int dvtable_setcost(dv_t *dvtable, int fromNodeID, int toNodeID, unsigned int cost) 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	int node_num = topology_getNodeNum();
	int found = -1;

	// if found, set cost and return
	for(int i = 0; i <= nbr_num; i++){
		if(dvtable[i].nodeID == fromNodeID){
			for(int j = 0; j <= node_num; j++){
				if(dvtable[i].dvEntry[j].nodeID == toNodeID){
					found = 1;
					dvtable[i].dvEntry[j].cost = cost;
					return found;
				}
			}
		}
	}
	found = -1;
	return found;
}

/** TODO: 
 * This function returns the link cost between two nodes in dvtable. 
 * Return the link cost if these two nodes are found in dvtable, 
 * otherwise INFINITE_COST.
 */
unsigned int dvtable_getcost(dv_t *dvtable, int fromNodeID, int toNodeID) 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	int node_num = topology_getNodeNum();

	for(int i = 0; i <= nbr_num; i++){
		if(dvtable[i].nodeID == fromNodeID){
			for(int j = 0; j < node_num; j++){
				if(dvtable[i].dvEntry[j].nodeID == toNodeID) return dvtable[i].dvEntry[j].cost;
			}
		}
	}
	return INFINITE_COST;
}

/** TODO: 
 * This function prints out the contents of a dvtable. 
 */
void dvtable_print(dv_t *dvtable) 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	printf("-------------dv table-------------\n");
	for(int i = 0; i <= nbr_num; i++){
		printf("src %i:\n", dvtable[i].nodeID);
		for(int j = 0; j < 4; j++)
			printf("	dest %i %i\n", dvtable[i].dvEntry[j].nodeID, dvtable[i].dvEntry[j].cost);
	}
	printf("----------------------------------\n");
}
