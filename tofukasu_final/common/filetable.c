#include "./filetable.h"

//int filetable_get_time () {
//    struct timespec now;
//    clock_gettime(CLOCK_MONOTONIC, &now);
//    return now.tv_sec;
//}

file_table_t *filetable_create() {
    file_table_t *table = malloc(sizeof(file_table_t));
    if (!table)
        return NULL;
    
    table->head = NULL;
    table->size = 0;
    return table;
}

void filetable_destroy(file_table_t *table) {
    if (!table)
        return;
    
    file_t *curr = table->head;
    file_t *temp;
    while(curr) {
        temp = curr->next;
        free(curr);
        curr = temp;
    }

    free(table);
}

int filetable_add_file(file_table_t *table, int isDir, int status, char *filename, int file_size, int last_mod_time, int peerNum, char *peerIP[]) {
    if (!table)
        return -1;

    //create new file table entry
    file_t *file = malloc(sizeof(file_t));
    if(!file)
        return -1;
    memset(file->filename, 0, MAX_FILENAME_LEN);
    memcpy(file->filename, filename, strlen(filename));
    file->file_size = file_size;
    file->delta_size = 0;
    file->status = status;
    file->peerNum = peerNum;
    // printf("in filetable: %s\n", peerIP[0]);
    for (int i = 0; i < file->peerNum; i++) {
        // for (int j = 0; j < IP_LEN; j++) {
        //     printf ("Peer ip: %s\n", peerIP[i][j]);
        //      file->peerIP[i][j] =  peerIP[i][j];
        // }
        memset(file->peerIP[i], 0, IP_LEN);
        // printf ("Peer ip: %s\n", peerIP[i]);
        memcpy(file->peerIP[i], peerIP[i], IP_LEN);
    }
    file->latest_timestamp = last_mod_time;
    file->prev_timestamp = 0;
    file->isDir = isDir;

    //append file after table->head
    if (!table->head) {
        table->head = file;
        file->next = NULL;
    } else {
        file->next = table->head->next;
        table->head->next = file;
    }
    return 0;
}

//find file entry by filename
file_t *filetable_find_file(file_table_t *table, char *filename){
    if (!table)
        return NULL;
    file_t *curr = table->head;
    while (curr) {
        if (strcmp(curr->filename, filename)==0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

//when a peer is dead, remove the IP of the peer from all file table entries
int filetable_remove_peerIP(file_table_t *table, char *peerIP) {
    if (!table)
        return -1;

    file_t *curr = table->head;
    while(curr) {
        for (int i = curr->peerNum-1; i >= 0; i--) {
            if (strcmp(curr->peerIP[i], peerIP)==0) {
                memcpy(curr->peerIP[i], curr->peerIP[curr->peerNum-1], strlen(curr->peerIP[curr->peerNum-1]));
                memset(curr->peerIP[curr->peerNum-1], 0, IP_LEN);
                curr->peerNum--;
                break; //only 1 instance of the peerIP occurs
            }
        }
        curr = curr->next;
    }
    return 0;
}

void filetable_print(file_table_t *table) {
    if (!table)
        return;

    printf("/************************ FILETABLE ***********************/\n");
    file_t *curr = table->head;
    while(curr) {
        printf("filename: %s\n", curr->filename);
        printf("file_size: %d, delta_size: %d, status: %d, peerNum: %d, la_ts: %lu, prev_ts: %lu, isDir: %d\n", \
        curr->file_size, curr->delta_size, curr->status,curr->peerNum, curr->latest_timestamp, curr->prev_timestamp, curr->isDir);
        printf("peerIP: \n");
        for (int i = 0; i < curr->peerNum; i++) {
            printf("%s\n", curr->peerIP[i]);
        }
        printf("- - - - - - - - - - - - - - - - - - - - - - - - -\n");
        curr = curr->next;
    }
    printf("/********************* END OF FILETABLE ********************/\n");
}
