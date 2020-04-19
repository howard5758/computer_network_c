/*
 * network/nbrcosttable.c: implement all operations for the neighbor 
 * cost table used by the routing protocol of MNP. 
 *
 * CS60, March 2018. 
 */


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "nbrcosttable.h"
#include "../common/constants.h"
#include "../topology/topology.h"

#define TOPOLOGYFILE "../topology/topology.dat"


/** TODO: 
 * This function creates a neighbor cost table dynamically
 * and initializes the table with all its neighbors' node IDs and 
 * direct link costs. The neighbors' node IDs and direct link costs 
 * are retrieved from topology.dat file.
 *
 * Return the pointer to the table
 */
nbr_cost_entry_t *nbrcosttable_create() 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	printf("nbrnum: %i\n", nbr_num);
	nbr_cost_entry_t *nbrc_table = malloc(nbr_num*sizeof(nbr_cost_entry_t));

	FILE *fp;
	fp = fopen(TOPOLOGYFILE, "r");
	char one[200];
	char two[200];
	int cost = 0;
	int myID = topology_getMyNodeID();
	int oneID = 0;
	int twoID = 0;
	int i = 0;
	// read topology.dat to find the neighbors and initialize neighbor table
	while(fscanf(fp, "%s %s %i", one, two, &cost)!=EOF){
		oneID = topology_getNodeIDfromname(one);
		twoID = topology_getNodeIDfromname(two);
		if(oneID == myID){
			nbrc_table[i].nodeID = twoID;
			nbrc_table[i].cost = cost;
			i++;
		}
		else if(twoID == myID){
			nbrc_table[i].nodeID = oneID;
			nbrc_table[i].cost = cost;
			i++;
		}

	}
	fclose(fp);

	return nbrc_table; 
}

/** TODO: 
 * This function destroys a neighbor cost table.
 * It frees all the dynamically allocated memory for the neighbor 
 * cost table.
 */
void nbrcosttable_destroy(nbr_cost_entry_t *nct) 
{
	free(nct);
	nct = NULL;
}

/** TODO: 
 * This function is used to get the direct link cost from neighbor 
 * specified by its node ID.
 * Return the direct link cost if the neighbor is found in the 
 * table, otherwise INFINITE_COST if the node is not found. 
 */
unsigned int nbrcosttable_getcost(nbr_cost_entry_t *nct, int nodeID) 
{
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	for(int i = 0; i < nbr_num; i++){
		if(nodeID == nct[i].nodeID) return nct[i].cost;
	}
	return INFINITE_COST;
}

/** TODO: 
 * This function prints out the contents of a neighbor cost table.
 */
void nbrcosttable_print(nbr_cost_entry_t *nct) 
{
	int nbr_num = topology_getNbrNum();
	printf("-------------nbrcost table------------\n");
	for(int i = 0; i < nbr_num; i++){
		printf("nbr %i cost %i\n", nct[i].nodeID, nct[i].cost);
	}
	printf("--------------------------------------\n");
}
