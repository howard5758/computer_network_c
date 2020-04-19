#include "tracker/tracker.h"
#include "peer/peer.h"

//gcc -Wall -pedantic -std=gnu11 -ggdb -o localsync localsync.c tracker/tracker.a
int main(const int argc, char *argv[]){
	//---check input
	if(argc != 2){
        printf("Usage: ./LocalSync -tracker OR ./LocalSync -peer\n");
        exit(1);
	}
	//---parse input: judge it's peer or tracker && which dir to monitor
	if (strcmp(argv[1], "-peer")==0){	
		printf("Do peer stuff\n");
		peer();
	} else if(strcmp(argv[1], "-tracker")==0){
		tracker();
	} else {
        printf("Usage: ./LocalSync -tracker OR ./LocalSync -peer\n");
        exit(1);
    }

	return 0;
}
