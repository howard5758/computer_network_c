CFLAGS = -Wall -pedantic -c -std=gnu11 -ggdb -I$C
CC = gcc
MAKE = make
OBJ = filetable.o seg.o

# for memory-leak tests
VALGRIND = valgrind --leak-check=full --show-leak-kinds=all

########### default ##################
all: $(OBJ)
filetable.o: filetable.h
seg.o: seg.h
.PHONY: valgrind clean
########### valgrind ##################
valgrind: 	

############## clean  ##########
clean:
	rm -rf *~ *.o *.dSYM .DS_Store