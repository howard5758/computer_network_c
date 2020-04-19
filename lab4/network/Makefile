# Makefile for the network layer 
#
# CS 60, March 2018

C = ../common

PROGS = network
LIBS = -pthread

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

network: network.o nbrcosttable.o dvtable.o routing_table.o $(C)/seg.o $(C)/pkt.o ../topology/topology.o 
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

##### source dependencies
network.o: network.h nbrcosttable.h dvtable.h routing_table.h $(C)/constants.h $(C)/pkt.h $(C)/seg.h ../topology/topology.h 

nbrcosttable.o: nbrcosttable.h ../topology/topology.h $(C)/constants.h

dvtable.o: dvtable.h ../topology/topology.h $(C)/constants.h

routing_table.o: routing_table.h ../topology/topology.h $(C)/constants.h

../topology/topology.o: ../topology/topology.h $(C)/constants.h

$(C)/pkt.o: $(C)/pkt.h $(C)/constants.h

$(C)/seg.o: $(C)/seg.h $(C)/constants.h	



.PHONY: valgrind clean

########### valgrind ##################
valgrind: network
	$(VALGRIND) ./network

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

