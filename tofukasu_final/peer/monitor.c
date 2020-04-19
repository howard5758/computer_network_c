//
//  monitor.c
//
//  Created by Yanxin Yin on 5/21/18.
//  Copyright © 2018 Yanxin Yin. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
//#include "../common/seg.h"
#include "./monitor.h"
#include "monitor.h"

#define LEN_DIR     1024 /* Assuming that the length of directory won't exceed 256 bytes */
#define MAX_EVENTS  1024 /*Max. number of events to process at one go*/
#define LEN_NAME    512 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/

// ///=======================Global variables===================///
int inotify_fd;
DIR_MNT_table *mnt_table;
MNT_config *config;
file_table_t *file_table;

int monitor_socket;


///=======================Functions=========================///

void readConfigFile(MNT_config *config, char* filename ){
    /// open config file
    FILE *file = fopen(filename, "r");

    /// get dir
    char buf[LEN_DIR];
    fscanf(file,"%s", buf);
    config->root = malloc(strlen(buf) + 1);
    strcpy(config->root, buf);

    /// get recursive flag
    fscanf(file, "%d", &config->type);

    ///  Go through each line, get ignored dirs
    while(fscanf(file, "%s", buf) != EOF){
        if(strcmp(buf, "") != 0){
            config->ignore[config->ignoreNum] = malloc(strlen(buf) + 1);
            strcpy(config->ignore[config->ignoreNum], buf);
            config->ignoreNum++;
        }
    }

    /// close config
    fclose(file);

    /// add backup (absolute path)
    char backup[] = ".backup/"; /// path should be empty or end in '/'
    char *backabspath = malloc(strlen(backup) + 1 + strlen(config->root));
    strcpy(backabspath, config->root);
    strcat(backabspath, backup);
    config->backup = backabspath;

    /// add delta (absolute path)
    char delta[] = ".delta/";
    char *deltaabspath = malloc(strlen(delta) + 1 + strlen(config->root));
    strcpy(deltaabspath, config->root);
    strcat(deltaabspath, delta);
    config->delta = deltaabspath;

    /// add backup to ignore (relative path), different from above
    config->ignore[config->ignoreNum] = malloc(strlen(backup) + 1);
    strcpy(config->ignore[config->ignoreNum], backup);
    config->ignoreNum++;

    /// add delta to ignore (relative path), different from above
    config->ignore[config->ignoreNum] = malloc(strlen(delta) + 1);
    strcpy(config->ignore[config->ignoreNum], delta);
    config->ignoreNum++;
}


int initWatch(DIR_MNT_table *mnt_table, MNT_config *config, file_table_t *file_table){
    /// creating the inotify instance
    int inotify_fd;
    if((inotify_fd = inotify_init()) < 0) {
        perror("inotify_init");
        return -1;
    }

    printf("Create the inotify instance, fd %d\n", inotify_fd);
    /// add directory and subdir to watch recursively
    // root dir is set to be empty string
    char *path = malloc(1);
    path[0] = '\0';
    traverseDir(inotify_fd, mnt_table, config, file_table, path);
    return inotify_fd;
}


/// directory must be relative path
int watchDirectory(int inotify_fd, char* path, DIR_MNT_table *mnt_table, MNT_config *config){
    if(mnt_table->entNum + 1 < MAX_DIR_NUM){

        /// Note directory here is relative path
        char *abspath = getAbsPath(config, path);

        int wd = inotify_add_watch(inotify_fd, abspath, IN_ALL_EVENTS);
        if(wd < 0){
            perror("watchDirectory: inotify_add_watch fails\n");
            free(abspath);
            return -1;
        }
        printf("Add %s to watch, wd: %d\n", abspath, wd);
        free(abspath);
        mnt_table->table[mnt_table->entNum].dirName = path;
        mnt_table->table[mnt_table->entNum].watchD = wd;
        mnt_table->entNum++;
        return 1;
    }
    perror("watchDirectory: exceed directory limits\n");
    return -1;
}


void removeDirectory(int inotify_fd, char* directory, DIR_MNT_table *mnt_table){
    for(int i = 0; i < mnt_table->entNum; i++){
        if(strcmp(mnt_table->table[i].dirName, directory) == 0){

            /// swap with the last one
            int wd = mnt_table->table[i].watchD;
            mnt_table->table[i].watchD = mnt_table->table[mnt_table->entNum - 1].watchD;
            /// remove watch
            inotify_rm_watch(inotify_fd, wd);
            mnt_table->table[mnt_table->entNum - 1].watchD = -1;
            /// free malloc memory
            char *temp = mnt_table->table[i].dirName;
            printf("remove %s from watch\n", temp);
            mnt_table->table[i].dirName = mnt_table->table[mnt_table->entNum - 1].dirName;
            mnt_table->table[mnt_table->entNum - 1].dirName = NULL;
            free(temp);

        }
    }
}


/// path is dir path relative to the root, and root dir is defined as empty string.
/// path must be either empty or ended with '/'
int inIgnore(MNT_config *config, char* path){
    for(int i = 0; i < config->ignoreNum; i++){
        if(strcmp(config->ignore[i], path) == 0){
            return 1;
        }
    }
    return -1;
}


/// path is dir path relative to the root, and root dir is defined as empty string.
/// path must be either empty or ended with '/'
int watchFilter(MNT_config *config, char* path){

    /// check if any dir hierarchy is in ignore table
    int l = (int)strlen(path);
    for(int i = l-1; i >= 0; i--){
        if(path[i] == '/'){
            char *check = malloc(i+2);
            memcpy((void*)check, (void*)path, i+1);
            check[i+1] = '\0';
            if(inIgnore(config, check) > 0){
                /// in an ignore subdir
                free(check);
                return -1;
            }
            free(check);
        }
    }
    return 1;
}


// TODO - merge with isFile()?
void getFileInfo(FileInfo* info){
    struct stat path_stat;
    stat(info->filepath, &path_stat);
    info->lastModifyTime = path_stat.st_mtim;
    info->size = path_stat.st_size;
}


/// path is dir path relative to the root, and root dir is defined as empty string.
/// path must be either empty or ended with '/'
void traverseDir(int inotify_fd, DIR_MNT_table *mnt_table, MNT_config *config, file_table_t *file_table, char *path){

    /// watch filtering using relative path
    if(watchFilter(config, path) < 0){
        /// not survive filtering, ignore
        printf("filter out %s\n", path);
        free(path);
        return;
    }

    /// concat to create abspath from root and relative path
    char *abspath = getAbsPath(config, path);
    printf("Traveling into %s\n", abspath);

    /// create corresponding backup dir
    char *subdir = concatPath(config->backup, path);
    mkdir(subdir, 0700);
    printf("Mkdir\t%s\n", subdir);
    free(subdir);

    /// create corresponding delta dir
    char *deltadir = concatPath(config->delta, path);
    mkdir(deltadir, 0700);
    printf("Mkdir\t%s\n", deltadir);
    free(deltadir);

    /// deal with files under this dir and recursively find subdirectory
    DIR* dir;
    struct dirent *ent;
    if((dir=opendir(abspath)) != NULL){
        while (( ent = readdir(dir)) != NULL){
            if(ent->d_type == DT_DIR && strcmp(ent->d_name, ".") != 0  && strcmp(ent->d_name, "..")){
                /// directory
                /// create relative path to root for sub-directory
                char *newpath = malloc(strlen(path) + 2 + strlen(ent->d_name));
                strcpy(newpath, path);
                strcat(newpath, ent->d_name);
                newpath[strlen(path)+strlen(ent->d_name)] = '/';
                newpath[strlen(path)+strlen(ent->d_name)+1] = '\0';
                printf("DIR \t%s\n", newpath);

                /// recursive call if recursive mode is on
                if(config->type){
                    traverseDir(inotify_fd, mnt_table, config, file_table, newpath);
                }
            }else if(ent->d_type == DT_REG){
                /// regular file
                /// create relative path of the file
                char *newpath = concatPath(path, ent->d_name);
                printf("FILE\t%s\n", newpath);

                /// make copy
                char *scr = getAbsPath(config, newpath);
                char *dest = concatPath(config->backup, newpath);
                if (copyFile(scr,dest) < 0){
                    printf("cannot copy file %s\n", scr);
                }
                printf("make copy of %s\n", newpath);

                /// get file info
                FileInfo* info = malloc(sizeof(FileInfo));
                info->filepath = scr;
                getFileInfo(info);

                /// get self ip
                // TODO: implement getMyIP and double check peerIP type
                char **peerIP = malloc(1 * sizeof(char*));
                char *myIP = getMyIP();
                peerIP[0] = myIP;

                /// add to file table
                if(filetable_add_file(file_table, 0, READ_N_WRITE, newpath, info->size, info->lastModifyTime.tv_sec, 1, peerIP) < 0){
                    printf("error! fail to add into file table");
                }

                free(myIP);
                free(peerIP);
                free(scr);
                free(dest);
                free(newpath);
                free(info);
            }
        }
        closedir(dir);
    }

    /// add dir to watch
    /* if add dir to watch before while loop, when making copies, that event will be tracked as IS_OPEN (without IS_CLOSEWRITE since open('r')) */
    if(watchDirectory(inotify_fd, path, mnt_table, config) < 0){
        printf("traverseDir: cannot add %s to watch\n", abspath);
        // TODO - error handle
    }
    free(abspath);
}


void dirCreate(char* path, int inotify_fd, file_table_t *table, DIR_MNT_table *mnt_table, MNT_config *config){
    /// add to watch
    printf("DIR CREATE:\t%s\n", path);
    mnt_table->table[mnt_table->entNum].dirName = path;
    mnt_table->table[mnt_table->entNum].watchD = watchDirectory(inotify_fd, path, mnt_table, config);

    // TODO: update filetable and send to tracker
}


void dirDelete(char* path, int inotify_fd, file_table_t *table, DIR_MNT_table *mnt_table){
    /// remove watch
    removeDirectory(inotify_fd, path, mnt_table);
    printf("DIR DELETE:\t%s\n", path);
    // NOTE: no need to recursively delete files inside, inotify will first notify file deleted in that dir and then dir deleted
    // TODO: update filetable and send to tracker
}


void fileOpened(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE OPEN:\t%s\n", filename);
    // TODO: change self state to 4, send state 1 to tracker

}


int fileAdded(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE ADD:\t%s\n", filename);
    // TODO: 1. add into filetable 2. send to tracker 3. makeCopy

    /// make copy
    char *scr = getAbsPath(config, filename);
    char *dest = concatPath(config->backup, filename);
    if (copyFile(scr,dest) < 0){
        printf("cannot copy file %s\n", scr);
    }
    printf("make copy of %s\n", filename);

    /// get file info
    FileInfo* info = malloc(sizeof(FileInfo));
    info->filepath = scr;
    getFileInfo(info);

    /// get self ip
    char **peerIP = malloc(1 * sizeof(char*));
    char *myIP = getMyIP();
    peerIP[0] = myIP;

    /// add to file table
    if(filetable_add_file(file_table, 0, READ_N_WRITE, filename, info->size, info->lastModifyTime.tv_sec, 1, peerIP) < 0){
        printf("error! fail to add into file table");
    }

    free(myIP);
    free(peerIP);
    free(scr);
    free(dest);
    free(info);

    return 1;
}


int fileModified(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE MODIFIED:\t%s\n", filename);
    // TODO: change self state to 0, send state 0 to tracker, copy and delta, update timestamp using real last modified
    /// make delta at first (copy will be overwritten)
    char *origin = getAbsPath(config, filename);
    char *backup = concatPath(config->backup, filename);
    char *delta = concatPath(config->delta, filename);
    /// old: backup-copy; new: modified file; delta
    if(makeDelta(backup, origin, delta) < 0){
        printf("cannot make delta of %s\n", filename);
    }
    printf("make delta of %s\n", filename);

    /// make copy
    if (copyFile(origin,backup) < 0){
        printf("cannot copy file %s\n", filename);
    }
    printf("make copy of %s\n", filename);

    /// update filetable
    FileInfo *info = malloc(sizeof(FileInfo));
    info->filepath = origin;
    getFileInfo(info);
    file_t *fileEntry = filetable_find_file(table, filename);
    fileEntry->prev_timestamp = fileEntry->latest_timestamp;
    fileEntry->latest_timestamp = info->lastModifyTime.tv_sec;
    fileEntry->status = READ_N_WRITE;
    fileEntry->file_size = info->size;

    printf("change %s to RNW state\n", filename);

    /// free mem
    free(origin);
    free(backup);
    free(delta);
    free(info);

    /// send to tracker
    return 1;
}


int fileDeleted(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE DELETED:\t%s\n", filename);

    struct timeval now;
    gettimeofday(&now, NULL);
    file_t *fileEntry = filetable_find_file(table, filename);
    if(fileEntry == NULL){
        // e.g. a.txt~
        printf("not in filetable, continue\n");
        return 0;
    } else{
        // TODO: timestamp update, set self to 2 and sent 2 to tracker
        fileEntry->prev_timestamp = fileEntry->latest_timestamp;
        fileEntry->latest_timestamp = (unsigned int)now.tv_sec;
        fileEntry->status = DELETED;
        printf("change %s to delete state\n", filename);

        return 1;
    }
}


int fileCloseWrite(char* filename, file_table_t *table, MNT_config *config){
    file_t *fileEntry = filetable_find_file(table, filename);
    if(fileEntry == NULL){
        // newly created file
        return fileAdded(filename, table, config);
    }else{
        // modified or synced
        if(fileEntry->status == SYNCED){
            // no extra operation this time, change state back to read and write
            printf("FILE SYNCED:\t%s\n", filename);
            fileEntry->status = READ_N_WRITE;
            // TODO - copy and make delta


            return 0;
        }else{
            return fileModified(filename, table, config);
        }
    }
//    printf("FILE CLOSEWRITE:\t%s\n", filename);
}

void fileCloseNoWrite(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE CLOSE_NO_WRITE:\t%s\n", filename);
}

void fileAccess(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE ACCESS:\t%s\n", filename);
}

void fileInModified(char* filename, file_table_t *table, MNT_config *config){
    printf("FILE IN_MODIFY:\t%s\n", filename);
}

void sendTracker(file_table_t *table, int sockfd){
    p2t_seg_t *packet = malloc(sizeof(p2t_seg_t));
    packet->type = FILE_TABLE_UPDATE;
    packet->filetable = *table;//updatedTable;
    char *myIP = getMyIP();
    printf("send to tracker: %s\n", myIP);
    memcpy(packet->peer_ip, myIP, IP_LEN);
    free(myIP);
    //packet->port = trackerPort;

    if(send_p2t_segment(sockfd, packet) < 0){
        printf("p2t send fail\n");
        return;
    }
    printf("p2t send!\n");
    free(packet);
}



// void clearAll(){
//     printf("start to clear all...\n");
//     /* free monitor config table */
//     free(config->root);
//     free(config->backup);
//     free(config->delta);

//     for(int i = 0; i < config->ignoreNum; i++){
//         free(config->ignore[i]);
//     }
//     free(config);
//     config = NULL;
//     printf("Free config\n");

//     /* free monitor watch table and rm watches */
//     for(int i = 0; i < mnt_table->entNum; i++){
//         if(mnt_table->table[i].dirName != NULL)
//             free(mnt_table->table[i].dirName);
//         if(mnt_table->table[i].watchD >= 0)
//             inotify_rm_watch(inotify_fd, mnt_table->table[i].watchD);
//     }
//     free(mnt_table);
//     mnt_table = NULL;
//     printf("Free mnt_table\n");

//     /* close inotify file descriptor*/
//     close(inotify_fd);
//     inotify_fd = -1;
//     printf("close inotify\n");

//     /* free file table */
//     filetable_destroy(file_table);

//     exit(0);
// }

char *getPathByWd(DIR_MNT_table *mnt_table, int wd){
    for(int i = 0; i < mnt_table->entNum; i++){
        if(mnt_table->table[i].watchD == wd){
            return mnt_table->table[i].dirName;
        }
    }
    return NULL;
}

int isFile(char *path){
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

int copyFile (char scr[], char dest[]){
    int c;
    FILE *stream_R;
    FILE *stream_W;

    // open the src file to read
    stream_R = fopen(scr, "r");
    if (stream_R == NULL)
        return -1;
    // create and open the dest file to write
    stream_W = fopen(dest, "w");
    if (stream_W == NULL){
        fclose (stream_R);
        return -1;
    }
    // read from src and write to dest
    while ((c = fgetc(stream_R)) != EOF)
        fputc (c, stream_W);

    fclose (stream_R);
    fclose (stream_W);
    return 1;
}

char* concatPath(char* preflix, char* path){
    char *newPath = malloc(strlen(preflix) + 1 + strlen(path));
    strcpy(newPath, preflix);
    strcat(newPath, path);
    return newPath;
}

char* getAbsPath(MNT_config *config, char* path){
    return concatPath(config->root, path);
}

char* getMyIP(){
    struct utsname unameData;
    uname(&unameData);
    struct hostent *hostp = gethostbyname(unameData.nodename);
    if(hostp == NULL){
        fprintf(stderr, "unknown host '%s'\n", unameData.nodename);
        return NULL;
    }
    char* myIP = malloc(IP_LEN);
    memcpy((void*)myIP, (void*)inet_ntoa(*(struct in_addr*)hostp->h_addr_list[0]), IP_LEN);
    return myIP;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}


/// argv[1]: filename
/// main for now, change to thread later
// int main( int argc, char *argv[] ){

//     /* Set inotify_fd as -1 */
//     int inotify_fd = -1;

//     /* Initialize Table */
//     mnt_table = malloc(sizeof(DIR_MNT_table));
//     mnt_table->entNum = 0;

//     /* Read Config */
//     config = malloc(sizeof(MNT_config));
//     readConfigFile(config, argv[1]);

//     /* Remove .backup dir from last session if exists */
//     struct stat st = {0};
//     if (stat(config->backup, &st) == 0) {
//         rmdir(config->backup);
//     }

//     /* initialize file table  */
//     file_table = filetable_create();

//     /* initialize inotify watch */
//     if ((inotify_fd = initWatch(mnt_table, config, file_table)) < 0){
//         printf("error initWatch\n");
//     }

//     /* signal */
//     signal(SIGINT, clearAll);

//     /* handle event */
//     int length = 0, i = 0;
//     char buffer[BUF_LEN];

//     filetable_print(file_table);


//     printf("start to read...\n");

//     while((length = read(inotify_fd, buffer, BUF_LEN )) > 0){
//         /* return the list of change events happens. Read the change event one by one and process it accordingly. set i as pointer */
//         i = 0;
//         while(i < length){
//             /* get event info */
//             struct inotify_event *event = (struct inotify_event *)&buffer[i];
//             if(event->len){
//                 char *preflix = getPathByWd(mnt_table, event->wd);//cannot be freed here!
//                 char *fullname;
//                 if(event->mask & IN_ISDIR){
//                     /// add slash to the end, consistent with monitor table
//                     fullname = malloc(strlen(preflix) + strlen(event->name) + 2);
//                     strcpy(fullname, preflix);
//                     strcat(fullname, event->name);
//                     fullname[strlen(preflix) + strlen(event->name)] = '/';
//                     fullname[strlen(preflix) + strlen(event->name) + 1] = '\0';
//                 }else{
//                     /// file
//                     /// filtering out vim backup
//                     if(strcmp(get_filename_ext(event->name), "swp") == 0 || strcmp(get_filename_ext(event->name), "swx") == 0 ||
//                        strcmp(event->name, "4913") == 0) {
//                         i += EVENT_SIZE + event->len;
//                         continue;
//                     }
//                     fullname = concatPath(preflix, event->name);
//                 }

//                 /// filtering out undesired event, i.e. vim backup or ignored dir
//                 if(watchFilter(config, fullname) < 0){
//                     printf("filtering out %s\n", fullname);
//                     free(fullname);
//                     i += EVENT_SIZE + event->len;
//                     continue;
//                 }

//                 /// handle event
//                 if((event->mask & IN_OPEN) && !(event->mask & IN_ISDIR)){
//                     // TODO
//                     fileOpened(fullname, file_table, config);

//                 }else if ((event->mask & IN_CREATE) && (event->mask & IN_ISDIR)) {
//                     // TODO
//                     dirCreate(fullname, inotify_fd, file_table, mnt_table, config);

//                 }else if( event->mask & IN_DELETE ) {
//                     if ( event->mask & IN_ISDIR ) {
//                         // TODO
//                         dirDelete(fullname, inotify_fd, file_table,mnt_table);

//                     }else {
//                         // TODO
//                         fileDeleted(fullname, file_table, config);

//                     }
//                 }else if((event->mask & IN_CLOSE_WRITE) && !(event->mask & IN_ISDIR)){
//                     fileCloseWrite(fullname, file_table, config);
//                 }
// //                else if((event->mask & IN_CLOSE_NOWRITE) && !(event->mask & IN_ISDIR)){
// //                    fileCloseNoWrite(fullname, file_table, config);
// //                }
// //                else if((event->mask & IN_ACCESS) && !(event->mask & IN_ISDIR)){
// //                    fileAccess(fullname, file_table, config);
// //                }else if((event->mask & IN_MODIFY) && !(event->mask & IN_ISDIR)){
// //                    fileInModified(fullname, file_table, config);
// //                }
//                 free(fullname);
//             }
//             /* update pointer */
//             i += EVENT_SIZE + event->len;
//         }
//     }
    
//     return 0;
// }
