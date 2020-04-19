all: peer.o

peer.o: peer.c ../common/seg.c ../common/filetable.c monitor.c bsdiff.c bspatch.c
	gcc -Wall -pedantic -std=gnu11 -c peer.c monitor.c bsdiff.c bspatch.c ../common/seg.c ../common/filetable.c  -pthread -lbz2

monitor.o: monitor.c ../common/seg.c ../common/filetable.c bsdiff.c bspatch.c
	gcc -Wall -pedantic -std=gnu11 -lbz2 -c monitor.c ../common/seg.c ../common/filetable.c bsdiff.c bspatch.c

bsdiff.o: bsdiff.c
	gcc -Wall -pedantic -std=gnu11 -lbz2 -c bsdiff.c

bdpatch.o: bspatch.h
	gcc -Wall -pedantic -std=gnu11 -lbz2 -c bspatch.c


clean:
	rm -rf *~ *.o *.dSYM .DS_Store