#include "./trackerpeertable.h"

int trc_pr_tb_get_time () {
    struct timeval now;
    gettimeofday(&now, NULL);
    return now.tv_sec;
}

trc_pr_table_t *trc_pr_tb_create() {
    trc_pr_table_t *table = malloc(sizeof(trc_pr_table_t));
    if (!table)
        return NULL;
    table->head = NULL;
    table->size = 0;
    return table;
}

void trc_pr_tb_destroy(trc_pr_table_t *table) {
    if (!table)
        return;
    trc_pr_entry_t *curr = table->head;
    trc_pr_entry_t *temp;
    while (curr) {
        temp = curr->next;
        free(curr);
        curr = temp;
    }
    free(table);
    return;
}

int trc_pr_tb_add_peer(trc_pr_table_t *table, char *peerIP, int sockfd) {
    if (!table)
        return -1;
    trc_pr_entry_t *peer;
    peer = trc_pr_tb_find_peer_by_ip(table, peerIP);
    if (peer) {
        peer->sockfd = sockfd;
        peer->last_time_stamp = trc_pr_tb_get_time();
        return 0;
    }
    peer = malloc(sizeof(trc_pr_entry_t));
    if (!peer)
        return -1;
    memset(peer->ip, 0, IP_LEN);
    memcpy(peer->ip, peerIP, strlen(peerIP));
    peer->sockfd = sockfd;
    peer->last_time_stamp = trc_pr_tb_get_time();
    if (!table->head) {
        table->head = peer;
        peer->next = NULL;
    } else {
        peer->next = table->head->next;
        table->head->next = peer;
    }
    return 0;
}

// int trc_pr_tb_remove_dead_peer(trc_pr_table_t *table) {

//     if (!table)
//         return -1;
//     int now = trc_pr_tb_get_time();
//     trc_pr_entry_t *sentinel = malloc(sizeof(trc_pr_entry_t));
//     sentinel->next = table->head;
//     trc_pr_entry_t *prev = sentinel;
//     trc_pr_entry_t *curr = sentinel->next;
//     while (curr) {
//         if (now - curr->last_time_stamp > PEER_ALIVE_INTERVAL) {
//             prev->next = curr->next;
//             free(curr);
//         }
//     }


//     free(sentinel);
//     return 0;
// }

trc_pr_entry_t *trc_pr_tb_find_peer_by_ip(trc_pr_table_t *table, char *peerIP) {
    if (!table)
        return NULL;
    trc_pr_entry_t *curr = table->head;
    while (curr) {
        if (strcmp(curr->ip, peerIP)==0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

trc_pr_entry_t *trc_pr_tb_find_peer_by_sockfd(trc_pr_table_t *table, int sockfd) {
    if (!table)
        return NULL;
    trc_pr_entry_t *curr = table->head;
    while (curr) {
        if (curr->sockfd==sockfd)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

int trc_pr_tb_update_alive_timestamp(trc_pr_table_t *table, char *peerIP) {
    trc_pr_entry_t *peer = trc_pr_tb_find_peer_by_ip(table, peerIP);
    peer->last_time_stamp = trc_pr_tb_get_time();
    return 0;
}


void trc_pr_tb_print(trc_pr_table_t *table) {
    if (!table)
        return;
    printf("/******************tracker peer table**********************/\n");
    trc_pr_entry_t *curr = table->head;
    while (curr) {
        printf("peerIP: %s, sockfd: %d, timestamp: %lu\n", curr->ip, curr->sockfd, curr->last_time_stamp);
        curr = curr->next;
    }
}
