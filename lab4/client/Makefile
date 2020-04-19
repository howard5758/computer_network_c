# Makefile for the client side of Mini Reliable Transport
#
# CS 60, March 2018

C = ../common

PROGS = simple_client stress_client
LIBS = -pthread

FLAGS = #-DTEST
CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS) -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

########### simple ##################
simple_client: app_simple_client.o mrt_client.o $(C)/seg.o ../topology/topology.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

app_simple_client.o: mrt_client.h $(C)/constants.h $(C)/seg.h ../topology/topology.h

########### stress ##################
stress_client: app_stress_client.o mrt_client.o $(C)/seg.o ../topology/topology.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

app_stress_client.o: mrt_client.h $(C)/constants.h $(C)/seg.h ../topology/topology.h

##### source dependencies
$(C)/seg.o: $(C)/seg.h $(C)/constants.h

mrt_client.o: mrt_client.h $(C)/seg.h $(C)/constants.h ../topology/topology.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: simple_client	stress_client
	$(VALGRIND) ./simple_client
	$(VALGRIND) ./stress_client

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

