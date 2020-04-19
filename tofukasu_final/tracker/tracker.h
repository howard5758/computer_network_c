/*tracker.h*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/select.h>
#include <signal.h>
#include "../common/seg.h"
#include "./trackerpeertable.h"

#define TRACKER_HANDSHAKE_PORT 9613
#define TRACKER_MONITOR_PORT 9619

#define MAX_PENDING 5
#define PIECE_LEN 200

#define MONITOR_TIME_OUT 5 //seconds
#define PEER_ALIVE_INTERVAL 60 //any peer unheard of in 10 min is considered as dead

int tracker();
void *handshake(void *client_sock);
void *monitor (void *arg);
void tracker_stop();

