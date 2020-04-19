# Makefile for the server side of Mini Reliable Transport
#
# CS 60, March 2018

C = ../common

PROGS = simple_server stress_server
LIBS = -pthread

FLAGS = #-DTEST
CFLAGS = -Wall -pedantic -std=c11 -ggdb $(FLAGS) -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

########### simple ##################
simple_server: app_simple_server.o mrt_server.o $(C)/seg.o ../topology/topology.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

app_simple_server.o: mrt_server.h $(C)/constants.h $(C)/seg.h ../topology/topology.o

########### stress ##################
stress_server: app_stress_server.o mrt_server.o $(C)/seg.o ../topology/topology.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

app_stress_server.o: mrt_server.h $(C)/constants.h $(C)/seg.h ../topology/topology.o

##### source dependencies
$(C)/seg.o: $(C)/seg.h $(C)/constants.h

mrt_server.o: mrt_server.h $(C)/seg.h $(C)/constants.h ../topology/topology.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: simple_server	stress_server
	$(VALGRIND) ./simple_server
	$(VALGRIND) ./stress_server

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)
	rm -rf receivedtext.txt

