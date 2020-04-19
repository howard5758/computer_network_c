//tracker.c
#include "./tracker.h"

//local function to get IP address of tracker
static int get_my_ip();
static int handle_register(int client_sock, p2t_seg_t pkt);
static int handle_filetable_update(int client_sock, p2t_seg_t pkt);
static int broadcast_filetable();
static int remove_dead_peers();

trc_pr_table_t *peer_table;
pthread_mutex_t *peer_table_mutex;
file_table_t *file_table;
pthread_mutex_t *file_table_mutex;
int handshake_listen_sock;
int monitor_listen_sock;

/* 
 * initiate handshake server address
 * create socket to listen on
 * bind sock
 * listen to connections
 * 
 * create monitor thread
 * 
 * while (1)
 *  accept connection from Peer
 *  deep-copy client sock fd
 *  create handshake thread
 *  
 */
int tracker() {
    int client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);
    char addr_buf[INET_ADDRSTRLEN];
    pthread_t handshake_th;

    //create peer table
    peer_table = trc_pr_tb_create();
    peer_table_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(peer_table_mutex, NULL);

    //create file table
    file_table = filetable_create();
    file_table_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(file_table_mutex, NULL);

    //register signal
    signal(SIGINT, tracker_stop);

    //set up listen sock
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TRACKER_HANDSHAKE_PORT);
    handshake_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (handshake_listen_sock == 0) {
        exit(1);
    }
    if (bind(handshake_listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) !=0) {
        exit(1);
    }
    if (listen(handshake_listen_sock, MAX_PENDING) != 0) {
        exit(1);
    }

    //create monitor thread
    pthread_t monitor_th;
    pthread_create(&monitor_th, NULL, monitor, (void*)0);

    //print tracker ip
    if (get_my_ip() < 0)
        exit(1);

    while (1) {
        //accept a connection
        client_sock = accept(handshake_listen_sock, (struct sockaddr* )&client_addr, &len);
        if (client_sock <0 )
            continue;
        if ( inet_ntop( AF_INET, &client_addr.sin_addr, addr_buf, len) == NULL ) { 
            continue;
        }
        printf("Tracker: accepted connection from %s\n", addr_buf);

        //create handshake thread
        int *client_sock_copy = (int *) malloc(sizeof client_sock_copy);
        *client_sock_copy = client_sock;
        pthread_create(&handshake_th, NULL, handshake, (void *) client_sock_copy);
        pthread_detach(handshake_th); 
    }

    return 0;
}

/* 
 * cast client sock into int
 * free (client_sock)
 * keep receiving seg from peer
 *  if (seg.type == REGISTER)
 *      handle register
 * else if (seg.type == UPDATE_FILE_TABLE)
 *      handle update file table
 * for peer in peer table
 *  if peer.sockfd == client_sock
 *      update peer alive timestamp
 *  recv data, memcpy into file table
 *  for theirFile in their file table
 *      if theirFile in my file table
 *          if their timestamp > my timestamp
 *              myFile = theirFile
 *          else if theirFile.timestamp == myFile.timeStamp
 *               update peerNum, peerIP
 *      else theirFile not in my file table
 *          add theirFile to my file table
 *          update file table size
 */
void *handshake(void *client_sock_void) {
    int client_sock = * ((int *) client_sock_void);
    free(client_sock_void);

    p2t_seg_t pkt;
    while (receive_p2t_segment(client_sock, &pkt) > 0) {
        if (pkt.type == REGISTER) {
            if (handle_register(client_sock, pkt) < 0) {
                perror("handling register");
                continue;
            }
        } else if (pkt.type == FILE_TABLE_UPDATE) {
            if (handle_filetable_update(client_sock, pkt)<0) {
                perror("handling filetable update");
                continue;
            }
       } else {
            perror("Client sent wrong type of pkt");
        }
    }
    

    //update peer table sock fd
    pthread_mutex_lock(peer_table_mutex);
    trc_pr_entry_t *peer = trc_pr_tb_find_peer_by_sockfd (peer_table, client_sock);
    peer->sockfd = -1;
    pthread_mutex_unlock(peer_table_mutex);
    close(client_sock);
    pthread_exit(NULL);
}
/*
 * initiate monitor server address
 * create socket to listen on
 * bind sock
 * listen to connectons
 * 
 * while (1)
 *  select() timeout = 5 secs
 *  if (FD_ISSET(listen_sock, NULL))
 *      accept
 *      update peer alive time
 *  for peer in peer table
 *      if peer timeout
 *      remove peer from peer table
 *      for file in file table
 *          for peer in file.peers
 *              if peer is the dead peer
 *                  remove it
 *                  update peerNum
 *  broadcast file table
 */

void *monitor (void *arg) {
    printf("Tracker: started monitor thread\n");
    int client_sock, max_sd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);
    char addr_buf[INET_ADDRSTRLEN];
    struct timeval timeout;
    int res;
    fd_set temp_set, main_set;
    
    //set up monitor listen sock
    memset(&server_addr, 0, sizeof server_addr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(TRACKER_MONITOR_PORT);
    monitor_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (monitor_listen_sock == 0) {
        pthread_exit(NULL);
    }
    if (bind(monitor_listen_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        pthread_exit(NULL);
    }
    if (listen(monitor_listen_sock, MAX_PENDING) != 0) {
        pthread_exit(NULL);
    }

    FD_ZERO(&main_set);
    max_sd = monitor_listen_sock;
    FD_SET(monitor_listen_sock, &main_set);

    while (1) {
        FD_ZERO(&temp_set);
        memcpy(&temp_set, &main_set, sizeof(main_set));
        timeout.tv_sec = MONITOR_TIME_OUT;
        timeout.tv_usec = 0;

        res = select(max_sd+1, &temp_set, NULL, NULL, &timeout);
        // if (res == 0) {
        //     // trc_pr_tb_print(peer_table);
        //     // filetable_print(file_table);

        //     // continue;
        //     printf("select timeout\n");
        //     continue;
        // }
        if (res > 0) {
        //see if connection has been made
            if (FD_ISSET(monitor_listen_sock, &temp_set)) {
                client_sock = accept(monitor_listen_sock, ( struct sockaddr* )&client_addr, &len );
                if (client_sock < 0) {
                    break;
                }
                if ( inet_ntop( AF_INET, &client_addr.sin_addr, addr_buf,INET_ADDRSTRLEN ) == NULL ) { 
                    break;
                }
                //add client sock to main set
                FD_SET(client_sock, &main_set);
                if (client_sock > max_sd) {
                    max_sd = client_sock;
                }
                printf("Tracker: monitor connected with from %s\n", addr_buf);
            }
            //read from client sockets
            for (int i=0; i <=max_sd; i++) {
                if (i != monitor_listen_sock && i != handshake_listen_sock && FD_ISSET(i, &temp_set)) {
                    printf("sock: %d\n", i);
                    client_sock = i;
                    p2t_seg_t p2t_pkt;
                    if (receive_p2t_segment(client_sock, &p2t_pkt)<0) {
                        perror("receiving p2t seg");
                        pthread_mutex_lock(peer_table_mutex);
                        trc_pr_entry_t *peer = trc_pr_tb_find_peer_by_sockfd(peer_table, client_sock);
                        if (peer) {
                            peer->sockfd = -1;
                        }
                        pthread_mutex_unlock(peer_table_mutex);
                        close(client_sock);
                        FD_CLR(client_sock, &main_set);
                        if (i == max_sd) {
                            printf("deleting\n");
                            while (!FD_ISSET(max_sd, &main_set))
                                max_sd -= 1;
                        }
                    }
                    else if (p2t_pkt.type == ALIVE) {
                        printf("Tracker: received ALIVE from %s\n", p2t_pkt.peer_ip);
                        pthread_mutex_lock(peer_table_mutex);
                        trc_pr_tb_update_alive_timestamp(peer_table, p2t_pkt.peer_ip);
                        pthread_mutex_unlock(peer_table_mutex);
                    }
                }
            }
        }

        if (remove_dead_peers() <0) {
            perror("removeing dead peers");
        }

        if (broadcast_filetable() <0) {
            perror("broadcasting filetable");
        }

 
    }
    close(monitor_listen_sock);
    monitor_listen_sock = -1;
    pthread_exit(NULL);
}

//close and free everything
void tracker_stop() {
    trc_pr_tb_destroy(peer_table);
    filetable_destroy(file_table);
    free(peer_table_mutex);
    free(file_table_mutex);
    close(monitor_listen_sock);
    close(handshake_listen_sock);
    printf("Thank you!\n");
    exit(0);
}

int handle_register(int client_sock, p2t_seg_t pkt) {
    printf("Tracker: received REGISTER pkt\n");

    //add peer to peer table
    pthread_mutex_lock(peer_table_mutex);
    trc_pr_tb_add_peer(peer_table, pkt.peer_ip, client_sock);
    pthread_mutex_unlock(peer_table_mutex);

    //send ACCEPT
    t2p_seg_t send_pkt;
    send_pkt.interval = PEER_ALIVE_INTERVAL;
    send_pkt.piece_len = PIECE_LEN;
    //send an empty filetable?
    memset(&send_pkt.filetable, 0, sizeof(file_table_t));
    if (send_t2p_segment(client_sock, &send_pkt)<0) {
        perror("Sending t2p seg");
        return -1;
    }
    
    return 0;
}

int handle_filetable_update(int client_sock, p2t_seg_t pkt) {
    printf("Tracker: received UPDATE_FILE_TABLE pkt from peer %s\n", pkt.peer_ip);
    pthread_mutex_lock(peer_table_mutex);

    //update peer timestamp
    if (trc_pr_tb_update_alive_timestamp(peer_table, pkt.peer_ip) < 0)
        return -1;
     pthread_mutex_unlock(peer_table_mutex);
    
    pthread_mutex_lock(file_table_mutex);
    file_t *client_file = pkt.filetable.head;
    while (client_file) {
        file_t *tracker_file = filetable_find_file(file_table, client_file->filename);
        if (tracker_file) {
            if (client_file->latest_timestamp > tracker_file->latest_timestamp) {
                printf("Tracker: new timestamp, file: %s\n", client_file->filename);
                tracker_file->delta_size = client_file->delta_size;
                tracker_file->file_size = client_file->file_size;
                tracker_file->isDir = client_file->isDir;
                tracker_file->latest_timestamp = client_file->latest_timestamp;
                tracker_file->status = client_file->status;
                tracker_file->prev_timestamp = client_file->prev_timestamp;
                for (int i = 0; i < tracker_file->peerNum; i++) {
                    memset(tracker_file->peerIP[i], 0, IP_LEN);
                }
                for (int i = 0; i < client_file->peerNum; i++) {
                    memcpy(tracker_file->peerIP[i], client_file->peerIP[i], strlen(client_file->peerIP[i]));
                }
                tracker_file->peerNum = client_file->peerNum;
            } else if (client_file->latest_timestamp == tracker_file->latest_timestamp) {
                memcpy(tracker_file->peerIP[tracker_file->peerNum], pkt.peer_ip, strlen(pkt.peer_ip));
            }
        } else {
            printf("%s\n", client_file->peerIP[0]);
            char **temp = malloc(500);
            temp[0] = client_file->peerIP[0];
            filetable_add_file(file_table, client_file->isDir, client_file->status, client_file->filename, client_file->file_size, client_file->latest_timestamp, client_file->peerNum,temp);
            file_table->size++;
            free(temp);
        }
        printf("///////////////////////UPDATED FILETABLE////////////////////////\n");
        filetable_print(file_table);
        client_file = client_file->next;
    }
    pthread_mutex_unlock(file_table_mutex);
   return 0;
}

//source: https://www.geeksforgeeks.org/c-program-display-hostname-ip-address/
static int get_my_ip() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
 
    // To retrieve hostname
    if(gethostname(hostbuffer, sizeof(hostbuffer))<0){
        return -1;
    }
    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    if (!host_entry)
        return -1;
 
    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    printf("Tracker IP: %s\n", IPbuffer);
 
    return 0;
}

static int broadcast_filetable() {
    //broadcast filetable to all peers
    pthread_mutex_lock(peer_table_mutex);
    pthread_mutex_lock(file_table_mutex);
    trc_pr_entry_t *curr = peer_table->head;
    t2p_seg_t seg;
    seg.interval = PEER_ALIVE_INTERVAL;
    seg.piece_len = PIECE_LEN;
    memset(&seg.filetable, 0, sizeof(file_table_t));
    memcpy(&seg.filetable, file_table, sizeof(file_table_t));
    while (curr) {
        if (curr->sockfd > 0 && send_t2p_segment(curr->sockfd, &seg)<0) {
            // printf("sending filetable to %s, sockfd: %d\n", curr->ip, curr->sockfd);
            // perror("Sending filetable");
            continue;
        }
        curr = curr->next;
    }
    //clean up
    pthread_mutex_unlock(file_table_mutex);
    pthread_mutex_unlock(peer_table_mutex);
    return 0;
}

static int remove_dead_peers() {
    int now;
    //remove dead peers from peer table and file table
    pthread_mutex_lock(peer_table_mutex);
    // trc_pr_tb_print(peer_table);
    trc_pr_entry_t *dummy = malloc(sizeof (trc_pr_entry_t));//dummy head
    trc_pr_entry_t *prev = dummy;
    prev->next = peer_table->head;
    trc_pr_entry_t *curr = peer_table->head;
    while (curr) {
        now = trc_pr_tb_get_time();
        if (now - curr->last_time_stamp > PEER_ALIVE_INTERVAL+1) {
            printf("Tracker: peer %s timeout\n", curr->ip);
            pthread_mutex_lock(file_table_mutex);
            filetable_remove_peerIP(file_table, curr->ip);
            pthread_mutex_unlock(file_table_mutex);
            prev->next = curr->next;
            free(curr);
            curr = prev->next;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    printf("//////////UPDATEED TABLES//////////\n");
    trc_pr_tb_print(peer_table);
    filetable_print(file_table);
    peer_table->head = dummy->next;
    free(dummy);
    dummy = NULL;
    pthread_mutex_unlock(peer_table_mutex);
    return 0;
}
