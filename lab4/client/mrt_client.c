/*
 * client/mrt_client.h: implementation of MRT client socket 
 * interfaces. 
 *
 * CS60, March 2018. 
 * Ping-Jung Liu
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include "mrt_client.h"

#include "../common/constants.h"
#include "../common/pkt.h"
#include "../topology/topology.h"


/**************** global variables ****************/
client_tcb_t *p_client_tcb_table[MAX_TRANSPORT_CONNECTIONS];
int overlay_connection;

/**************** functions ****************/

// TODO - mrt client initialization
void mrt_client_init(int conn) {
  /* initialize the global variables */
  for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++) {
    p_client_tcb_table[i] = NULL;
  }
  overlay_connection = conn;

  /* Create a thread for handling connections */
  pthread_t seghandler_thread;
  pthread_create(&seghandler_thread, NULL, seghandler, (void *) 0);
  pthread_detach(seghandler_thread);
}

// TODO - Create a client sock
int mrt_client_sock(unsigned int client_port) {

  // loop through tcb table to find first NULL
	for (int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++){
    if (p_client_tcb_table[i] == NULL){

      // initialize client
      struct client_tcb *client = (struct client_tcb*)malloc(sizeof(struct client_tcb));
      client->client_portNum = client_port;
      client->state = CLIENT_CLOSED;
      client->next_seqNum = 0;
      client->unAck_segNum = 0;
      client->client_nodeID = topology_getMyNodeID();

      struct segBuf *head;
      struct segBuf *tail;
      struct segBuf *unsent;
      unsent = NULL;
      head = NULL;
      tail = NULL;
      client->sendBufunSent = unsent;
      client->sendBufHead = head;
      client->sendBufTail = tail;
      
      // initialize utex
      pthread_mutex_t *mutex = malloc(sizeof(*mutex));
      pthread_mutex_init(mutex, NULL);
      client->bufMutex = mutex;
      p_client_tcb_table[i] = client;

      return i;
      break;
    }
  }
  return -1;
}

// TODO - Connect to the server
int mrt_client_connect(int sockfd, int nodeID, unsigned int server_port) {
  // check if client state is CLOSED
  switch(p_client_tcb_table[sockfd]->state){
    case CLIENT_CLOSED:
      // set state to SYNSENT
      p_client_tcb_table[sockfd]->state = CLIENT_SYNSENT;
      p_client_tcb_table[sockfd]->svr_nodeID = nodeID;
      struct mrt_hdr hdr;
      struct segment seg;
      hdr.src_port = p_client_tcb_table[sockfd]->client_portNum;
      hdr.dest_port = server_port;
      printf("port %i -> port %i\n", p_client_tcb_table[sockfd]->client_portNum, server_port);
      hdr.type = SYN;
      seg.header = hdr;
      
      int send = mnp_sendseg(overlay_connection, p_client_tcb_table[sockfd]->svr_nodeID, &seg);
      if(send!=1) printf("mnp_sendseg fail!\n");
      int try = 0;
      // try to connect to server port
      while(try < SYN_MAX_RETRY){
        struct timeval timer = {0, SYN_TIMEOUT/100};
        select(0, NULL, NULL, NULL, &timer);
        printf("SYN try\n");
        // check if connected already
        if(p_client_tcb_table[sockfd]->state == CLIENT_CONNECTED){
          printf("connected to %i\n", server_port);
          return 0;
        }
        else{
          send = mnp_sendseg(overlay_connection, p_client_tcb_table[sockfd]->svr_nodeID, &seg);
          printf("send to %i\n", p_client_tcb_table[sockfd]->svr_nodeID);
          if(send!=1) printf("mnp_sendseg fail!\n");
        }
        try = try + 1;
      }
      
      // last check
      if(p_client_tcb_table[sockfd]->state == CLIENT_CONNECTED){
        return 0;
      }
      else{
        p_client_tcb_table[sockfd]->state = CLIENT_CLOSED;
      }
      break;

    case CLIENT_SYNSENT:
      printf("in SYNSENT\n");
      break;
    case CLIENT_CONNECTED:
      printf("in CONNECTED\n");
      break;
    case CLIENT_FINWAIT:
      printf("in FINWAIT\n");
      break;
  }

	return -1;
}

// TODO - Send data to a mrt server
int mrt_client_send(int sockfd, void *data, unsigned int length) {
  // initialize some variables
  struct timeval timee;
  int seg_num = length / MAX_SEG_LEN;
  // check if connected
  if(p_client_tcb_table[sockfd]->state == CLIENT_CONNECTED){
    // cast to cahr*
    char *test = (char *) data;
    // the following for loop separate the data into multiple segments if needed
    struct segBuf *newhead, *prev;
    prev = NULL;
    for(int i = 0; i < seg_num + 1; i++){
      struct mrt_hdr hdr;
      struct segment seg;
      hdr.src_port = p_client_tcb_table[sockfd]->client_portNum;
      printf("client_send %i\n", p_client_tcb_table[sockfd]->client_portNum);
      hdr.dest_port = p_client_tcb_table[sockfd]->svr_portNum;
      hdr.type = DATA;
      if(i == seg_num){

        hdr.length = length % MAX_SEG_LEN;
        hdr.seq_num = p_client_tcb_table[sockfd]->next_seqNum;
        printf("%i\n", hdr.seq_num);
        p_client_tcb_table[sockfd]->next_seqNum = p_client_tcb_table[sockfd]->next_seqNum +(length%MAX_SEG_LEN);
        memcpy(seg.data, &test[i*MAX_SEG_LEN], length%MAX_SEG_LEN);
      }
      else{
        hdr.length = MAX_SEG_LEN;
        hdr.seq_num = p_client_tcb_table[sockfd]->next_seqNum;
        p_client_tcb_table[sockfd]->next_seqNum = p_client_tcb_table[sockfd]->next_seqNum + MAX_SEG_LEN;
        memcpy(seg.data, &test[i*MAX_SEG_LEN], MAX_SEG_LEN);
      }
      seg.header = hdr;
      struct segBuf *new = (struct segBuf*)malloc(sizeof(struct segBuf));
      new->next = NULL;
      new->seg = seg;
      if(prev == NULL){
        if(p_client_tcb_table[sockfd]->sendBufunSent == NULL){
          p_client_tcb_table[sockfd]->sendBufunSent = new;
        }
        newhead = new;
        prev = new;
      }
      else{
        prev->next = new;
        prev = new;
      }
    }

    // put the newly created list of segments into segBuffer
    struct segBuf *head = p_client_tcb_table[sockfd]->sendBufHead;
    // if current buffer is empty
    if(head == NULL){
      p_client_tcb_table[sockfd]->sendBufHead = newhead;
      p_client_tcb_table[sockfd]->sendBufTail = prev;
      int *socknum = malloc(1);
      *socknum = sockfd;
      pthread_t new_thread;
      // start timer thread 
      pthread_create(&new_thread, NULL, sendBuf_timer, (void*) socknum);
      pthread_detach(new_thread);
    }
    else{
      p_client_tcb_table[sockfd]->sendBufTail -> next = newhead;
      p_client_tcb_table[sockfd]->sendBufTail = prev;
    }

    // send all the unsent segments in buffer
    while(p_client_tcb_table[sockfd]->unAck_segNum <= GBN_WINDOW && 
          p_client_tcb_table[sockfd]->sendBufunSent != NULL){
      gettimeofday(&timee, NULL);
      p_client_tcb_table[sockfd]->sendBufunSent->sentTime = (long int)&timee;
      mnp_sendseg(overlay_connection, p_client_tcb_table[sockfd]->svr_nodeID, &p_client_tcb_table[sockfd]->sendBufunSent->seg);
      p_client_tcb_table[sockfd]->unAck_segNum += 1;
      p_client_tcb_table[sockfd]->sendBufunSent = p_client_tcb_table[sockfd]->sendBufunSent->next;
    }
    //send unsent

  }
  // not connected
  else{
    printf("not connected\n");
  }
	return -1;
}

// TODO - Disconnect from the server
int mrt_client_disconnect(int sockfd) {
  //check if connected
  if(p_client_tcb_table[sockfd]->state == CLIENT_CONNECTED){
    // change state to FINWAIT
  	p_client_tcb_table[sockfd]->state = CLIENT_FINWAIT;

    struct mrt_hdr hdr;
    struct segment seg;
    hdr.src_port = p_client_tcb_table[sockfd]->client_portNum;
    hdr.dest_port = p_client_tcb_table[sockfd]->svr_portNum;
    printf("client %i tries to disconnect from %i\n", hdr.src_port, hdr.dest_port);
    hdr.type = FIN;
    seg.header = hdr;
    
    int send = mnp_sendseg(overlay_connection, p_client_tcb_table[sockfd]->svr_nodeID, &seg);
    if(send!=1) printf("mnp_sendseg fail!\n");
    int try = 0;
    // try to disconnect and wait for FINACK
    while(try < SYN_MAX_RETRY){
      struct timeval timer = {0, FIN_TIMEOUT/1000};
      select(0, NULL, NULL, NULL, &timer);
      printf("FIN try \n");
      // closed
      if(p_client_tcb_table[sockfd]->state == CLIENT_CLOSED){
        printf("closed\n");
        return 0;
      }
      else{
        send = mnp_sendseg(overlay_connection, p_client_tcb_table[sockfd]->svr_nodeID, &seg);
        if(send!=1) printf("mnp_sendseg fail!\n");
      }
      try = try + 1;
    }
    //last check
    if(p_client_tcb_table[sockfd]->state == CLIENT_CLOSED){
      return 0;
    }
    p_client_tcb_table[sockfd]->state = CLIENT_CLOSED;
    return -1;
  
  }
  else{
    printf("not connected\n");
    return -1;
  }
}

// TODO - Close client
int mrt_client_close(int sockfd) {
	printf("mrt close!!!!\n");
  if(p_client_tcb_table[sockfd]->state != CLIENT_CLOSED)
    return -1;
  else{
    free(p_client_tcb_table[sockfd]->bufMutex);
    free(p_client_tcb_table[sockfd]);
    p_client_tcb_table[sockfd] = NULL;
    
    return 0;
  }
}

// sendBuf_timer thread
void *sendBuf_timer(void *sock){
  int sockfd = *(int *)sock;

  while(p_client_tcb_table[sockfd]->state == CLIENT_CONNECTED){
    struct timeval timer = {0, SENDBUF_POLLING_INTERVAL/1000};
    struct timeval timee;
    select(0, NULL, NULL, NULL, &timer);
    // if DATA_TIMEOUT, resend all the sent-not-ACK segments
    if(p_client_tcb_table[sockfd]->sendBufHead!=NULL){
      gettimeofday(&timee, NULL);
      if((long int)&timee - p_client_tcb_table[sockfd]->sendBufHead->sentTime > DATA_TIMEOUT){
        struct segBuf *head = p_client_tcb_table[sockfd]->sendBufHead;
        while(head!=p_client_tcb_table[sockfd]->sendBufunSent || head!=NULL){
            gettimeofday(&timee, NULL);
            head->sentTime = (long int)&timee;
            printf("resend %i\n", head->seg.header.seq_num);
            mnp_sendseg(overlay_connection, p_client_tcb_table[sockfd]->svr_nodeID, &head->seg);
            head = head->next;
        }
      }
    }
    else
      break;
    
  }
  free(sock);
  return NULL;
} 

// TODO - Thread handles incoming segment
/**
* @example
*
* while(1)
1) Get segment using function mnp_recvseg(), kill thread if something
 went wrong
2) Find tcb to handle this segment by looking up tcbs using the dest
 port in the segment header
3) switch(tcb state)
  case CLOSED: break;  
  case SYNSENT:
    if(seg.header.type==SYNACK && tcb->svrport==seg.header.srcport)
      set tcb state to CONNECTED
    break;
  case CONNECTED: 
    if segBuf.type == DATAACK && tcb.svrport==segBuf.srcport
      if segBuf.ack_num >= tcb.sendBufHead.seq_num
        update send buf with ack_num
        send new segments in send buffer
    break;
  case FINWAIT:
    if(seg.header.type==FINACK && tcb->svrport==seg.header.srcport)
      set tcb state to CLOSED
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
    //  find target sock
    for(int i = 0; i < MAX_TRANSPORT_CONNECTIONS; i++){
      if(p_client_tcb_table[i]!=NULL && p_client_tcb_table[i]->client_portNum == seg.header.dest_port){
        flag = 1;
        // state machine
        switch(p_client_tcb_table[i]->state){
          // closed, do nothing
          case CLIENT_CLOSED:
            break;
          // synsent, wait for synack
          case CLIENT_SYNSENT:
            if(seg.header.type == SYNACK){
              p_client_tcb_table[i]->state = CLIENT_CONNECTED;  
              p_client_tcb_table[i]->svr_portNum = seg.header.src_port;
            }
            
            break;
          // connected, wait for DATAACK
          case CLIENT_CONNECTED:

            pthread_mutex_lock(p_client_tcb_table[i]->bufMutex);
            if(seg.header.type == DATAACK){
              printf("received DATAACK %i\n", seg.header.seq_num);
              struct segBuf *head = p_client_tcb_table[i]->sendBufHead;
              struct segBuf *temp;
              // free all unacked with smaller seq_num
              while(head!=NULL){
                if(seg.header.seq_num > head->seg.header.seq_num){
                  p_client_tcb_table[i]->unAck_segNum -= 1;
                  printf("FREEEE %i\n", head->seg.header.seq_num);
                  temp = head;
                  head = head->next;
                  temp = NULL;
                  free(temp);
                }
                else
                  break;
              }
              // new head after freeing small segments
              p_client_tcb_table[i]->sendBufHead = head;
              
            }
            pthread_mutex_unlock(p_client_tcb_table[i]->bufMutex);
            break;
          // finwait, wait for FINACK
          case CLIENT_FINWAIT:

            if(seg.header.type == FINACK){
              p_client_tcb_table[i]->state = CLIENT_CLOSED;
              p_client_tcb_table[i]->svr_portNum = seg.header.src_port;
            }
            break;

        }
        if(flag == 1) break;
      }
    }

  }
  printf("no connection\n");
  return NULL;
}
