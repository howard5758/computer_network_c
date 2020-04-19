/*
 * topology/topology.c: implementation of functions to parse the 
 * topology file 
 *
 * CS60, March 2018. 
 */

#include "topology.h"
#include <sys/utsname.h>

/**************** constants ****************/
#define TOPOLOGYFILE "../topology/topology.dat"







/**************** functions ****************/
// TODO: return node ID of the given hostname. 
// 1) get host structure from gethostbyname
// 2) get ip address using inet_ntoa(host->h_addr_list[0].sin_addr)
// 3) find index of last . of ip address, call it i
// 4) return atoi(ipaddress + i + 1) // + 1 to get character after .
int topology_getNodeIDfromname(char* hostname) {

	struct hostent *hostp = gethostbyname(hostname);
	if(hostp == NULL){
		fprintf(stderr, "unknown host '%s'\n", hostname);
    	return -1;
	}
	
	return topology_getNodeIDfromip((struct in_addr*)hostp->h_addr_list[0]);
}


// TODO: return the node ID based on an IP address.  
// 1) ipaddress = inet_ntoa(*addr)
// 2) find index of last . of ip adress, call it i
// 3) return atoi(ipaddress + i + 1) // + 1 to get character after .
int topology_getNodeIDfromip(struct in_addr* addr) {

	char* ip = inet_ntoa(*(addr));
	int i = (int)strlen(ip) - 1;
	while(i >= 0){
		if (ip[i] == '.') break;
		i = i - 1;
	}

	return atoi(ip + i + 1);
}


// TODO: return my node ID. 
// Pseudocode
// 1) get system information in ugnm using uname syscall
// 2) reutnr getNodeIDfromname(ugnm.nodename)
int topology_getMyNodeID() {

	struct utsname unameData;
	uname(&unameData);
	return topology_getNodeIDfromname(unameData.nodename); 
}


// TODO: parse the topology file and return the number of neighbors.
// 1) myNodeID = getMyNodeID(); nbrNum = 0
// 2) open TOPOLOGYFILE
// 3) Go through each line, get host1 and host2 names
//      if host1 == myNodeID || host2 == myNodeID
//        nbrNum++
// 4) close TOPOLGYFILE
// 5) return nbrNum
int topology_getNbrNum() {

	FILE *fp;
	fp = fopen(TOPOLOGYFILE, "r");
	char one[200];
	char two[200];
	int cost = 0;
	int myID = topology_getMyNodeID();
	int oneID = 0;
	int twoID = 0;
	int nbr = 0;

	while(fscanf(fp, "%s %s %i", one, two, &cost)!=EOF){
		oneID = topology_getNodeIDfromname(one);
		twoID = topology_getNodeIDfromname(two);
		if(oneID == myID || twoID == myID) nbr++;
	}

	fclose(fp);
	return nbr; 
}


// TODO: parse the topology file and return the number of nodes in 
// the overlay network. 
// 1) open TOPOLOGY FILE
// 2) Go through each line and get host1 and host2 names
//      node1 = getNodeIDfromname(host1)
//      for each node
//        see if node1 needs to be added
//      if node1 needs to be added
//        append node1 to node array
//      node2 = getNodeIDfromname(host2)
//      for each node
//        see if node2 needs to be added
//      if node1 needs to be added
//        append node2 to node array
// 3) close TOPOLOGYFILE
int topology_getNodeNum() {

	int *temp_array = malloc(MAX_NODE_NUM);
	int i = 0;

	FILE *fp;
	fp = fopen(TOPOLOGYFILE, "r");
	char one[200];
	char two[200];
	int cost = 0;
	int oneID = 0;
	int twoID = 0;

	while(fscanf(fp, "%s %s %i", one, two, &cost)!=EOF){
		oneID = topology_getNodeIDfromname(one);
		twoID = topology_getNodeIDfromname(two);
		if(indexOf(temp_array, MAX_NODE_NUM, oneID) == -1){
			temp_array[i] = oneID;
			i++;
		}
		if(indexOf(temp_array, MAX_NODE_NUM, twoID) == -1){
			temp_array[i] = twoID;
			i++;
		}
	}

	free(temp_array);
	fclose(fp);

	return i;
}


// TODO: parse the topology file and return a dynamically allocated 
// array with all nodes' IDs in the overlay network. 
// 1) Same steps to parse file above
// 2) Allocate nodeArray
// 3) Copy nodes[] into nodeArray[]
// 4) Return nodeArray[]
int* topology_getNodeArray() {

	int *node_array = malloc(MAX_NODE_NUM);
	int i = 0;

	FILE *fp;
	fp = fopen(TOPOLOGYFILE, "r");
	char one[200];
	char two[200];
	int cost = 0;
	int oneID = 0;
	int twoID = 0;

	while(fscanf(fp, "%s %s %i", one, two, &cost)!=EOF){
		oneID = topology_getNodeIDfromname(one);
		twoID = topology_getNodeIDfromname(two);
		if(indexOf(node_array, MAX_NODE_NUM, oneID) == -1){
			node_array[i] = oneID;
			i++;
		}
		if(indexOf(node_array, MAX_NODE_NUM, twoID) == -1){
			node_array[i] = twoID;
			i++;
		}
	}

	free(node_array);
	fclose(fp);

	return node_array;
}


// TODO: parse the topology file and return a dynamically allocated 
// array with all neighbors' IDs. 
// 1) open TOPOLOGYFILE
// 2) for each line of the file, get host1 and host2 names
//      node1 = getNodeIDfromname(host1)
//      node2 = getNodeIDfromname(host2)
//      if(node1 == myNodeID)
//        nodes[nodeNum] = node2; nodeNum++
//      else if (node2 == myNodeID)
//        nodes[nodeNum] = node1; nodeNum++
// 3) allocate nodeArray
// 4) copy nodes[] into nodeArray[]
int* topology_getNbrArray() {

	int *nbr_array = malloc(MAX_NODE_NUM);
	int i = 0;

	FILE *fp;
	fp = fopen(TOPOLOGYFILE, "r");
	char one[200];
	char two[200];
	int cost = 0;
	int myID = topology_getMyNodeID();
	int oneID = 0;
	int twoID = 0;

	while(fscanf(fp, "%s %s %i", one, two, &cost)!=EOF){
		oneID = topology_getNodeIDfromname(one);
		twoID = topology_getNodeIDfromname(two);
		if(oneID == myID){
			nbr_array[i] = twoID;
			printf("%i\n", twoID);
			i++;
		}
		else if(twoID == myID){
			nbr_array[i] = oneID;
			printf("%i\n", oneID);
			i++;
		}
		
	}

	free(nbr_array);
	fclose(fp);

	return nbr_array;
}


// TODO: parse the topology information stored in the topology file 
// and return the link cost between two given nodes. 
unsigned int topology_getCost(int fromNodeID, int toNodeID) {

	FILE *fp;
	fp = fopen(TOPOLOGYFILE, "r");
	char one[200];
	char two[200];
	int cost = 0;
	int oneID = 0;
	int twoID = 0;

	while(fscanf(fp, "%s %s %i", one, two, &cost)!=EOF){
		oneID = topology_getNodeIDfromname(one);
		twoID = topology_getNodeIDfromname(two);
		if((fromNodeID == oneID && toNodeID == twoID) || 
			(fromNodeID == twoID && toNodeID == oneID)){
			fclose(fp);
			return cost;
		}
	}
	
	fclose(fp);
	return INFINITE_COST;
}

// helper function to find the index of a given number
int indexOf(int *array, int array_size, int number){
	for(int i = 0; i < array_size; ++i){
		if(array[i] == number){
			return i;
		}
	}
	return -1;
}
