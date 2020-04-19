# REDME for Mini Reliable Transport
# CS 60, March 2018
# Ping-Jung Liu

The file breakdowns are below in the original description. 

Program objective is to simulate a reliable mrt transport on top of
a steady overlay connection.

Use "make" to compile all four executable files:
	- simple_client
	- simple_server
	- stress_client
	- stress_server

"make clean" to clean up

Testing:

Open up two terminals. First run ./simple_server in one, then run ./simple_client in the other. 
Enter 127.0.0.1 when the client asks for host. Client and Server should transmit a few simple 
strings and verify that the program works.

After simple, run ./stress_server in one terminal, then run ./stress_client in the other. The host is default to 127.0.0.1 in case any of the user does not have access to Dartmouth CS Machines. To change target host, go to line 124 of ./client/app_stress_client.c and modify the string. In the stress version, client will send a large text document to server, and server will save the file in receivedtext.txt. Follow the steps and make sure it is correctly received.

Seglost probability is 10%. The program should be able to take care of occational seglosts.

All requirements of this HW are complete and functional!
(tested on moose.cs.dartmouth.edu, no warning for compilation, programs functional)


# Mini Reliable Transport

### Top directory:
send_this_text.txt - simple test data

### File breakdown:
##### client directory:
* app_simple_client.c - simple client application
* app_stress_client.c - stress client application
* mrt_client.c - MRT client side source file
* mrt_client.h - MRT client side header file

##### server directory:
* app_simple_server.c - simple server application
* app_stress_server.c - stress server application
* mrt_server.c - MRT server source file
* mrt_server.h - MRT server header file

##### common directory:
* seg.c - MNP function implementation, act on segments
* seg.h - MNP function header
* constants.h - define some useful constants

**Your job is to implement the functions in the various .c files  that have been left empty (i.e. they just return immediately).**
