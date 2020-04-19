CFLAGS = -Wall -pedantic -c -std=gnu11 -ggdb -I$C
CC = gcc
MAKE = make
OBJS = tracker.o trackerpeertable.o

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all


########### default ##################
all: $(OBJS)
trackerpeertable.o: trackerpeertable.h
tracker.o: tracker.h trackerpeertable.h ../common/filetable.h ../common/seg.h
.PHONY: valgrind clean

########### valgrind ##################
valgrind: 	

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store
