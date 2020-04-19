# Implemention Specs

## Peer

* get directory & tracker IP- config file
* send REGISTER packet
* recv ACCEPT?
* Start file monitor thread
    * init file table
    * loop
        * receive file change
            * make copy
            * compute Delta
            * update file table
        * send FILE_UPDATE packet to tracker - thread? P2T protocol?
* Start alive thread
    * send ALIVE packet
* Start P2P listeing thread
    * receive P2P request - P2P protocol?
    * P2P upload thread
* main loop
    * receive msg from tracker - T2P protocol?
    * parse msg
    * divide download into chunks, for each chunck:
        * P2P Download thread
        * write downloaded file into memory
        * handle pause???
        * update file table
        * handle conflict????

```
int main()

void *file_monitor(void *arg)

void *alive(void *arg)

void *p2p_download(void *arg)

void *p2p_upload(void *arg)

void *p2p_recv_req(void *arg)
```

Sample config file
```
//config.dat
directory=../Documents/mydirectory
trackerIP=192.23.7.106
```

### Tracker

* main loop
    * receive REGISTER packet
    * update peer table
    * handshake thread
        * receive FILE_UPDATE packet
        * update file table
    * monitor thread
        * receive ALIVE packet
        * update peer table?????
    * parse packet
    * update file table

```
int main()

void *handle_handshake(void *arg)

void *monitor_peer(void *arg)
```

## File Table

```
file {
    int size;
    char *name; //filepath included???
    unsigned long int timestamp;
    struct file *next;
    char *newpeerip; //tracker needs to record ip of all peers which has the latest version
}
```

```
filetable_create();
filetable_destroy();
filetable_add_file(filetable, file)
filetable_delete_file(filetable, file)
filetable_modify_file(filetable, file, update_pkt) //????????
```

## Peer Table

```
peer_t {
    char ip[IP_LEN];
    char filename[FILE_NAME_LEN];
    unsigned long file_time_stamp;
    int sockfd;
    struct peer_t *next;
}
```

```
peer_t_create();
peer_t_destroy();
peer_t_add_file(peer_t, filename);
peer_t_delete_file();
peer_t_pause_download(); //????
```

## Packet

Peer packet
```
/* The packet data structure sending from peer to tracker */
typedef struct segment_peer {
// protocol length
int protocol_len;
// protocol name
char protocol_name[PROTOCOL_LEN + 1];
// packet type : register, keep alive, update file table
int type;
// reserved space, you could use this space for your convenient, 8 bytes by default
char reserved[RESERVED_LEN];
// the peer ip address sending this packet
char peer_ip[IP_LEN];
// listening port number in p2p
int port;
// the number of files in the local file table -- optional
int file_table_size;
// file table of the client -- your own design
file_t file_table;
}ptp_peer_t;
```

Tracker packet
```
/* The packet data structure sending from tracker to peer */
typedef struct segment_tracker{
// time interval that the peer should sending alive message periodically
int interval;
// piece length
int piece_len;
// file number in the file table -- optional
int file_table_size;
// file table of the tracker -- your own design
file_t file_table;
} ptp_tracker_t;
```