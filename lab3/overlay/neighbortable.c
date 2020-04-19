/*
 * overlay/neighbortable.c: implementation of APIs for the neighbor 
 * table.  
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

#include "neighbortable.h"
#include "../topology/topology.h"

/**************** local constants ****************/
#define TOPOLOGYFILE "../topology/topology.dat"

/**************** functions ****************/
// TODO: 
// 1) get number of neighbors
// 2) allocate neighbor table
// 3) open topology file
//      for each line of file
//        get nodeId from name1
//        get hostname
//        set nt[idx].nodeIP
//        nt[idx].conn = -1;
//        idx++
//
//        get nodeId from name2
//        get hostname
//        set nt[idx].nodeIP
//        nt[idx].conn = -1;
//        idx++
// 4) return nt
nbr_entry_t *nt_create() {
	// initialize some variables
	int nbr_num = topology_getNbrNum();
	printf("nbrnum: %i\n", nbr_num);
	nbr_entry_t *nbr_table = malloc(nbr_num*sizeof(nbr_entry_t));

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

			nbr_table[i].nodeID = twoID;
			nbr_table[i].conn = -1;
			struct hostent *hostp = gethostbyname(two);
			nbr_table[i].nodeIP = ((struct in_addr *)hostp->h_addr_list[0])->s_addr;
			i++;
		}
		else if(twoID == myID){
			nbr_table[i].nodeID = oneID;
			nbr_table[i].conn = -1;
			struct hostent *hostp = gethostbyname(one);

			nbr_table[i].nodeIP = ((struct in_addr *)hostp->h_addr_list[0])->s_addr;

			i++;
		}

	}
	fclose(fp);
	return nbr_table;
}

// TODO: 
// 1) get number of neighbors
// 2) for each neighbord
//      if(nt[i].conn != -1)
//        close the connection
// 3) free(nt)
void nt_destroy(nbr_entry_t *nt) {
	int nbr_num = topology_getNbrNum();
	for(int i = 0; i < nbr_num; i++){
		if(nt[i].conn != -1){
			close(nt[i].conn);
			nt[i].conn = -1;
		}
	}
	free(nt);
}


// TODO: 
// 1) get number of neighbors
// 2) for each neighbor
//      if nodeID == nt[i].nodeID
//        nt[i].conn = conn
//        return 1
int nt_addconn(nbr_entry_t *nt, int nodeID, int conn) {
	int nbr_num = topology_getNbrNum();
	for(int i = 0; i < nbr_num; i++){
		if(nt[i].nodeID == nodeID){
			printf("add %i to nt!!! \n", nodeID);
			nt[i].conn = conn;
			return 1;
		}
	}
	return 0;
}
