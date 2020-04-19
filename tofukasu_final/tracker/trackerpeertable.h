#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define IP_LEN 16

typedef struct tracker_peer_entry {
    char ip[IP_LEN];
    unsigned long last_time_stamp;
    int sockfd;
    struct tracker_peer_entry *next;
} trc_pr_entry_t;

typedef struct tracker_peer_table {
    trc_pr_entry_t *head;
    int size; //number of peers in table
} trc_pr_table_t;

int trc_pr_tb_get_time ();
//return pointer to table if success, NULL if failure
trc_pr_table_t *trc_pr_tb_create();

void trc_pr_tb_destroy(trc_pr_table_t *table);

//return 0 on success, -1 on failure
int trc_pr_tb_add_peer(trc_pr_table_t *table, char *peerIP, int sockfd);
//return 0 on success, -1 on failure
// int trc_pr_tb_remove_dead_peer(trc_pr_table_t *table);

trc_pr_entry_t *trc_pr_tb_find_peer_by_ip(trc_pr_table_t *table, char *peerIP);

trc_pr_entry_t *trc_pr_tb_find_peer_by_sockfd(trc_pr_table_t *table, int sockfd);

int trc_pr_tb_update_alive_timestamp(trc_pr_table_t *table, char *peerIP);

void trc_pr_tb_print(trc_pr_table_t *table);
