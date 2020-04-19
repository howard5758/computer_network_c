/*
 * server/mrt_server.c: implementation of MRT server socket 
 * interfaces. 
 *
 * CS60, March 2018. 
 * Ping-Jung Liu
 */

#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>
#include <memory.h>
#include <string.h>
#include "mrt_server.h"

#include "../common/constants.h"
#include "../common/pkt.h"
#include "../topology/topology.h"


/**************** global variables ****************/
server_tcb_t *p_server_tcb_table[MAX_TRANSPORT_CONNECTIONS]; // TCB table
int overlay_connection; // overlay_connection


/**************** functions ****************/

// TODO - mrt server initialization
void mrt_server_init(int conn) {
	/* initialize the global variables */
	for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {
		p_server_tcb_table[i] = NULL;
	}
	overlay_connection = conn;

	/* Create a thread for handling connections */
	pthread_t seghandler_thread;
	pthread_create(&seghandler_thread, NULL, seghandler, (void *) 0);
  pthread_detach(seghandler_thread);
}

// TODO - Create a server sock and setting the recv buffer
int mrt_server_sock(unsigned int port) {
  // find first NULL
  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++){
    if (p_server_tcb_table[i] == NULL){
      // initialize server
      struct svr_tcb *svr = (struct svr_tcb*)malloc(sizeof(struct svr_tcb));
      svr->svr_portNum = port;
      svr->svr_nodeID = topology_getMyNodeID();
      svr->state = SVR_CLOSED;
      svr->usedBufLen = 0;
      char *buf = malloc(RECEIVE_BUF_SIZE);
      svr->recvBuf = buf;

      pthread_mutex_t *mutex = malloc(sizeof(*mutex));
      pthread_mutex_init(mutex, NULL);
      svr->bufMutex = mutex;

      p_server_tcb_table[i] = svr;

      return i;
      break;
    }
  }
	return -1;
}

// TODO - Accept overlay_connection from mrt client
int mrt_server_accept(int sockfd) {
  // change state to listening
	p_server_tcb_table[sockfd]->state = SVR_LISTENING;

  printf("enter accept wait loop %i\n", sockfd);
  while(1){
    if(p_server_tcb_table[sockfd]->state == SVR_CONNECTED)
      break;
  }

  return -1;
}

// TODO - Receive data from a mrt client
int mrt_server_recv(int sockfd, void *buf, unsigned int length) {
  printf("sock %i try to receive \n", sockfd);
  char *temp = (char *) buf;
  //if(p_server_tcb_table[sockfd]->state == SVR_CONNECTED){
    while(1){
      printf("while %i\n", p_server_tcb_table[sockfd]->usedBufLen);
      if(p_server_tcb_table[sockfd]->usedBufLen>=length){

        pthread_mutex_lock(p_server_tcb_table[sockfd]->bufMutex);
        memcpy(temp, p_server_tcb_table[sockfd]->recvBuf, length);
        p_server_tcb_table[sockfd]->usedBufLen -= length;

        p_server_tcb_table[sockfd]->recvBuf += length;
        pthread_mutex_unlock(p_server_tcb_table[sockfd]->bufMutex);
        return 1;
      }
      sleep(RECVBUF_POLLING_INTERVAL);
    }
  //}
  //else{
   // printf("not connected\n");
 //}
	return 1;
}

// TODO - Close the mrt server and Free all the space
int mrt_server_close(int sockfd) {
  printf("mrt close!!!!\n");
	if(p_server_tcb_table[sockfd]->state != SVR_CLOSEWAIT)
    return -1;
  else{ 
    free(p_server_tcb_table[sockfd]->bufMutex);
    free(p_server_tcb_table[sockfd]);
    p_server_tcb_table[sockfd] = NULL;
    
    return 0;
  }
}


// TODO - Thread handles incoming segments
/**
 * @example
 *
 * while(1)
  Get segment using mnp_recvseg()
  Get tcb by looking up tcb table by dest port in segment header
  switch(tcb state)
    case CLOSED: break;

    case LISTENING:
      if(seg.header.type==SYN)
        tcb.client_port = seg.header.srcport
        send SYNACK to client (syn_received call)
        tcb.state = CONNECTED
      break;

    case CONNECTED:
      if(seg.header.type==SYN && tcb.client_port==seg.header.srcport)
        duplicate SYN received
        send SYNACK to client (syn_received call)

      else if(seg.header.type==DATA && tcb.client_port == seg.header.srcport)
        update tcb with data received (data_recieved call)

      else if(seg.header.type==FIN && tcb.client_port==seg.header.srcport)
        send FINACK with fin_received call
        tcb state = CLOSEWAIT
        spawn thread to handle CLOSEWAIT state transition (see closewait below)
      break;

    case CLOSEWAIT: 
      if (seg.header.type==FIN && tcb.client_port == seg.header.srcport)
        send FINACK with fin_received call
      break;
 *
 */
void *seghandler(void *arg) {
  struct segment seg;
  int src_nodeID;

  int flag;

  while(1){
    int recv = mnp_recvseg(overlay_connection, &src_nodeID, &seg);
    if(recv < 0)
      continue;

    flag = 0;
    for(int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++){
      if(p_server_tcb_table[i]!=NULL && 
         p_server_tcb_table[i]->svr_portNum == seg.header.dest_port){
        flag = 1;
        // svr state machine
        switch(p_server_tcb_table[i]->state){
          // closed, do nothing
          case SVR_CLOSED:
            break;
          case SVR_LISTENING:

            if(seg.header.type == SYN){

              printf("received SYN from %i\n", seg.header.src_port);
              struct mrt_hdr hdr;
              struct segment new_seg;
              hdr.src_port = p_server_tcb_table[i]->svr_portNum;
              hdr.dest_port = seg.header.src_port;
              hdr.type = SYNACK;
              new_seg.header = hdr;

              p_server_tcb_table[i]->state = SVR_CONNECTED;
              p_server_tcb_table[i]->client_nodeID = src_nodeID;
              p_server_tcb_table[i]->client_portNum = seg.header.src_port;
              p_server_tcb_table[i]->expect_seqNum = 0;

              int send = mnp_sendseg(overlay_connection, p_server_tcb_table[i]->client_nodeID, &new_seg);
              if(send!=1) printf("mnp_sendseg fail!\n");


            }
            break;
          case SVR_CLOSEWAIT:
            if(seg.header.type == FIN){

              printf("received FIN from %i\n", seg.header.src_port);
              struct mrt_hdr hdr;
              struct segment new_seg;
              hdr.dest_port = seg.header.src_port;
              hdr.type = FINACK;
              new_seg.header = hdr;


              int send = mnp_sendseg(overlay_connection, p_server_tcb_table[i]->client_nodeID, &new_seg);
              if(send!=1) printf("mnp_sendseg fail!\n");

              p_server_tcb_table[i]->state = SVR_CLOSEWAIT;
              p_server_tcb_table[i]->client_portNum = seg.header.src_port;
            }
            break;
          case SVR_CONNECTED:
            printf("correct connected\n");
            if(seg.header.type == SYN){

              printf("received SYN from %i\n", seg.header.src_port);
              struct mrt_hdr hdr;
              struct segment new_seg;
              hdr.src_port = p_server_tcb_table[i]->svr_portNum;
              hdr.dest_port = seg.header.src_port;
              hdr.type = SYNACK;
              new_seg.header = hdr;
   

              int send = mnp_sendseg(overlay_connection, p_server_tcb_table[i]->client_nodeID, &new_seg);
              if(send!=1) printf("mnp_sendseg fail!\n");

              p_server_tcb_table[i]->state = SVR_CONNECTED;
              p_server_tcb_table[i]->client_portNum = seg.header.src_port;
              p_server_tcb_table[i]->expect_seqNum = 0;

            }
            if(seg.header.type == FIN){

              printf("received FIN from %i\n", seg.header.src_port);
              struct mrt_hdr hdr;
              struct segment new_seg;
              hdr.dest_port = seg.header.src_port;
              hdr.type = FINACK;
              new_seg.header = hdr;
     

              int send = mnp_sendseg(overlay_connection, p_server_tcb_table[i]->client_nodeID, &new_seg);
              if(send!=1) printf("mnp_sendseg fail!\n");

              p_server_tcb_table[i]->state = SVR_CLOSEWAIT;
              p_server_tcb_table[i]->client_portNum = seg.header.src_port;

            }

            if(seg.header.type == DATA){
              pthread_mutex_lock(p_server_tcb_table[i]->bufMutex);
              if(p_server_tcb_table[i]->expect_seqNum == seg.header.seq_num){
                p_server_tcb_table[i]->expect_seqNum += seg.header.length;

                memcpy(&p_server_tcb_table[i]->recvBuf[p_server_tcb_table[i]->usedBufLen], seg.data, seg.header.length);
            
                p_server_tcb_table[i]->usedBufLen += seg.header.length;
                printf("received DATA %i\n", seg.header.seq_num);
                struct mrt_hdr hdr;
                struct segment new_seg;
                hdr.dest_port = seg.header.src_port;
                hdr.type = DATAACK;
                hdr.seq_num = p_server_tcb_table[i]->expect_seqNum;
                new_seg.header = hdr;


                int send = mnp_sendseg(overlay_connection, p_server_tcb_table[i]->client_nodeID, &new_seg);
                if(send!=1) printf("mnp_sendseg fail!\n");

              }
              else{
                printf("received DATA bad seqnum %i expect: %i\n", seg.header.seq_num, p_server_tcb_table[i]->expect_seqNum);
                struct mrt_hdr hdr;
                struct segment new_seg;
                hdr.dest_port = seg.header.src_port;
                hdr.type = DATAACK;
                hdr.seq_num = p_server_tcb_table[i]->expect_seqNum;
                new_seg.header = hdr;
            

                int send = mnp_sendseg(overlay_connection, p_server_tcb_table[i]->client_nodeID, &new_seg);
                if(send!=1) printf("mnp_sendseg fail!\n");

              }
              pthread_mutex_unlock(p_server_tcb_table[i]->bufMutex);
            }
            break;

        }
        
      }
      if(flag == 1) break;
    }
    
  }
  return NULL;
}
