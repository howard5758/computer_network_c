# README for Mini Overlay Network
# CS 60, April 2018
# Ping-Jung Liu

The provided README explains the testing methods and functionality pretty clearly. The files I worked on in this project are network.c, neighbortable.c, overlay.c, and topology.c
All functions are commented and should be easy to understand. If you follow the testing methods below, the program should work perfectly.

# Mini Overlay Network

Use the provided Makefile to build the code. 

We use the following four hosts to build the overlay network: 
* host1: flume.cs.dartmouth.edu
* host2: moose.cs.dartmouth.edu
* host3: wildcat.cs.dartmouth.edu
* host4: whaleback.cs.dartmouth.edu

The overlay process should be running on these four hosts. 

To start overlay on a host: go to overlay directory and run `./overlay&` -- make sure you switch directory, else you will get error when running overlay in the top directory. All overlay process should be started within 1 minute.

Then start the network processes on all 4 nodes. The network process should be started after the local overlay network is initialized and 
shows "Overlay: waiting for connection from MNP process...". 

To start network layer on a host: go to network directory and run `./network`.

To kill the overlay processes and network processes: use `kill -s 2 processID`. Here `-s 2` is the same as pressing CTRL+C to stop the process execution. 

If the ports used by the program are used already, the program will exit.

### File breakdown:
##### common directory:
* constants.h: overlay and network layer parameters
* pkt.h: definition of MNP packets and related data structures
* pkt.c: implementation of interfaces to send/receive MNP packets using the overlay layer

##### network directory:
* network.h: definition of MNP functions
* network.c: implementation of the MNP process

##### overlay directory:
* neighbortable.h: definitions of data structures and APIs for neighbor table 
* neighbortable.c: implementation of APIs for the neighbor table 
* overlay.h: definitions of functions used by ON process
* overlay.c: implementation of an ON process

##### topology directory:
* topology.dat: topology file 
* topology.h: definitions of functions to parse the topology file
* topology.c: implementation of functions to parse the topology file 


