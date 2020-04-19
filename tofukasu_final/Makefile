C = ./common

PROGS = localsync
LIBS = -pthread

CFLAGS = -Wall -pedantic -std=gnu11 -ggdb -I$C
CC = gcc
MAKE = make

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

############## default: make all libs and programs ##########
all: 
	$(MAKE) -C common
	$(MAKE) -C peer
	$(MAKE) -C tracker
	$(MAKE) $(PROGS)

localsync: localsync.c tracker/tracker.o peer/peer.o peer/monitor.o peer/bsdiff.o peer/bspatch.o tracker/trackerpeertable.o common/filetable.o common/seg.o
	gcc -Wall -pedantic -std=gnu11 -ggdb -I$C localsync.c tracker/tracker.o peer/peer.o peer/monitor.o peer/bsdiff.o peer/bspatch.o tracker/trackerpeertable.o common/filetable.o common/seg.o -pthread -lbz2 -o localsync
	#$(CC) $(CFLAGS) $^ $(LIBS) -o $@

common: 
	$(MAKE) -C common

peer: 
	$(MAKE) -C peer

tracker: 
	$(MAKE) -C tracker

.PHONY: valgrind clean

############## valgrind all programs ##########
valgrind: all
	$(MAKE) -C common valgrind
	$(MAKE) -C peer valgrind
	$(MAKE) -C tracker valgrind

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	$(MAKE) -C common clean
	$(MAKE) -C peer clean
	$(MAKE) -C tracker clean
	rm -rf localsync

