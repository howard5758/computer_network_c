# Makefile for Mini Overlay Network
#
# CS 60, March 2018

MAKE = make

############## default: make all libs and programs ##########
all: 
	$(MAKE) -C overlay
	$(MAKE) -C network	

.PHONY: clean

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
	$(MAKE) -C overlay clean
	$(MAKE) -C network clean
	rm -rf ./common/*~ ./common/*.o ./common/*.dSYM ./common/.DS_Store
	rm -rf ./topology/*~ ./topology/*.o ./topology/*.dSYM ./topology/.DS_Store	
