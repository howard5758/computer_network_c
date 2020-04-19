# Makefile for Mini Network Protocol
#
# CS 60, March 2018

MAKE = make

############## default: make all libs and programs ##########
all: 
	$(MAKE) -C overlay
	$(MAKE) -C network	
	$(MAKE) -C client
	$(MAKE) -C server

simple: 
	$(MAKE) -C overlay
	$(MAKE) -C network	
	$(MAKE) -C client simple-client
	$(MAKE) -C server simple-server

stress: 
	$(MAKE) -C overlay
	$(MAKE) -C network	
	$(MAKE) -C client stress-client
	$(MAKE) -C server stress-server

.PHONY: valgrind clean

############## valgrind all programs ##########
valgrind: all
	$(MAKE) -C overlay valgrind
	$(MAKE) -C network valgrind
	$(MAKE) -C client valgrind
	$(MAKE) -C server valgrind

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	$(MAKE) -C overlay clean
	$(MAKE) -C network clean
	$(MAKE) -C client clean
	$(MAKE) -C server clean	
	$(MAKE) -C common clean
	$(MAKE) -C topology clean			
