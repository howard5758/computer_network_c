# Makefile for Mini Reliable Transport
# simple_client, simple_server, stress_client, stress_server
# CS 60, March 2018
# Ping-Jung Liu


all: simple_client simple_server stress_client stress_server

simple_client: ./client/app_simple_client.c ./client/mrt_client.c ./common/seg.c 
	gcc -pthread -o simple_client ./client/app_simple_client.c ./client/mrt_client.c ./common/seg.c

simple_server: ./server/app_simple_server.c ./server/mrt_server.c ./common/seg.c 
	gcc -pthread -o simple_server ./server/app_simple_server.c ./server/mrt_server.c ./common/seg.c

stress_client: ./client/app_stress_client.c ./client/mrt_client.c ./common/seg.c 
	gcc -pthread -o stress_client ./client/app_stress_client.c ./client/mrt_client.c ./common/seg.c

stress_server: ./server/app_stress_server.c ./server/mrt_server.c ./common/seg.c 
	gcc -pthread -o stress_server ./server/app_stress_server.c ./server/mrt_server.c ./common/seg.c

############## clean  ##########
clean:
	rm -f simple_client simple_server stress_client stress_server

