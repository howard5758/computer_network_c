# Makefile for the overlay 
#
# CS 60, March 2018

C = ../common

PROGS = overlay
LIBS = -pthread

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

all: $(PROGS)

overlay: overlay.o neighbortable.o ../topology/topology.o $(C)/pkt.o 
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@

##### source dependencies
overlay.o: overlay.h neighbortable.h $(C)/constants.h $(C)/pkt.h ../topology/topology.h 

neighbortable.o: neighbortable.h ../topology/topology.h 

../topology/topology.o: ../topology/topology.h $(C)/constants.h

$(C)/pkt.o: $(C)/pkt.h $(C)/constants.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: overlay
	$(VALGRIND) ./overlay

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	rm -rf $(PROGS)

