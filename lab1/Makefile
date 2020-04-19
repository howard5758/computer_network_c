all: TCPClient TCPServer_v1 TCPServer_v2 TCPServer_v3 TCPServer_v4

TCPClient: TCPClient.c
	gcc -o TCPClient TCPClient.c

TCPServer_v1: TCPServer_v1.c
	gcc -o TCPServer_v1 TCPServer_v1.c

TCPServer_v2: TCPServer_v2.c
	gcc -o TCPServer_v2 TCPServer_v2.c

TCPServer_v3: TCPServer_v3.c
	gcc -pthread -o TCPServer_v3 TCPServer_v3.c

TCPServer_v4: TCPServer_v4.c
	gcc -o TCPServer_v4 TCPServer_v4.c

clean:
	rm -f TCPClient TCPClient.o TCPServer_v1 TCPServer_v1.o TCPServer_v2 TCPServer_v2.o TCPServer_v3 TCPServer_v3.o TCPServer_v4 TCPServer_v4.o