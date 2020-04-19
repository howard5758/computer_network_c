# Makefile for common files 
#
# CS 60, March 2018

CFLAGS = -Wall -pedantic -std=c11 -ggdb -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all


########### default ##################
seg.o: seg.h constants.h

pkt.o: pkt.h constants.h


.PHONY: valgrind clean

########### valgrind ##################
valgrind: 	

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store

