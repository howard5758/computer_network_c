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

network: network.o $(C)/pkt.o ../topology/topology.o 
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

##### source dependencies
network.o: network.h $(C)/constants.h $(C)/pkt.h ../topology/topology.h 

../topology/topology.o: ../topology/topology.h $(C)/constants.h

$(C)/pkt.o: $(C)/pkt.h $(C)/constants.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: network
	$(VALGRIND) ./network

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

