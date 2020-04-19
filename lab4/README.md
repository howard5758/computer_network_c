All required functionalities completed. The connection waittime for both overlay and network are 60.
The cs machines are kind of unstable these few days so I set SYN, FIN, DATA TIMEOUT to 5 times the original values. 

In extremely rare cases, when the machines are lagging like crazy, the assertion in seg.c and pkt.c might fail and will cause the program to terminate. Please try to grade our hw with decent connection quality haha.


# Mini Network Protocol 

Four machines are used: flume, moose, wildcat, and whaleback. 

### Building and running:

Use the provided Makefile to build the code. 

To run the application:

1. start the overlay processes: 
	at each node, go to the overlay/ directory, run `./overlay&`. The overlay processes on 4 nodes should be started within 1 minute. Wait until you see "waiting for connection from MNP process..." before moving to step 2. 

2. start the network processes: 
	at each node, go to the network/ directory, run `./network&`. Wait until you see "waiting for connection from MRT process..." before moving to step 3. 

3. start the transport processes and run the application:
	at one node, go to the server directory, run `./simple_server` or `./stress_server`; 
	at another node, go to the client directory: run `./app_simple_app` or `./app_stress_app`. 

To stop the program:

use `kill -s 2 processID` to kill the network processes and overlay processes. 

If the port numbers used by the program are used already, the program exits.

### File breakdown:

##### client directory:
* app_simple_client.c - simple client application
* app_stress_client.c - stress client application
* mrt_client.c - MRT client side source file
* mrt_client.h - MRT client side header file
* send_this_text.txt - text file to send for the stress test

##### server directory:
* app_simple_server.c - simple server application
* app_stress_server.c - stress server application
* mrt_server.c - MRT server source file
* mrt_server.h - MRT server header file

##### common directory:
* constants.h: overlay and network layer parameters
* pkt.h: definition of MNP packets and related data structures
* pkt.c: implementation of interfaces to send/receive MNP packets using the overlay layer
* seg.c - MNP function implementation, act on segments
* seg.h - MNP function header

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
