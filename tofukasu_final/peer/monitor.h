//
//  monitor.h
//
//  Created by Yanxin Yin on 5/21/18.
//  Copyright © 2018 Yanxin Yin. All rights reserved.
//



#include <sys/socket.h>
#include "../common/seg.h"
#include "bsdiff.h"
#include "bspatch.h"
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


#ifndef MONITOR_H
#define MONITOR_H

#define MAX_DIR_NUM 100 /* Max number of directories allowed */

extern int monitor_socket;
//======================Data Structure=====================//
/**
 * Entry in dir_monitor_table.
 *
 * */
typedef struct dir_monitor_entry{
    char *dirName; /* relative path, to the "root" directory monitored */
    int watchD; /* watch descriptor generated by inotify */
} DIR_MNT_e;

/**
 * Directory monitor table, which stores the dir_monitor_entry for each directory to be monitored.
 * New directories will be appended at the end. Directories to be deleted will first be swapped to the end, and deleted,
 * which ensures only slots indexed between 0 - entNum are occupied.
 *
 * Note: only MAX_DIR_NUM of directories are allowed.
 *
 * */
typedef struct dir_monitor_table{
    int entNum; /* number of entry stored */
    DIR_MNT_e table[MAX_DIR_NUM];
} DIR_MNT_table;

/**
 * monitor configuration table, which stores configuration for file monitoring.
 *
 * root:        an ABSOLUTE path ending with '/'. It is read from .config file provided by users.
 * backup:      a hidden directory generated by file monitor to store file copies, which have the value of root+".backup".
 *              type is a flag, which indicate if recursive mode is on or off. it is read from .config file provided by users.
 * ignore:      a table of directories, which store directories to be ignored. It is optional, specified by users in the
 *              configuration file. Note that backup will be automatically added to ignore.
 * ignoreNum:   the number of directories to be ignored.
 *
 * */
typedef struct monitor_config{
    char *root;                 /* absolute path for root dir, specified by user in .config */
    char *backup;               /* absolute path for backup dir, equals root+".backup" */
    char *delta;                /* absoulte path for delta file dir, equals root+".delta" */
    int type;                   /* 1 for recursively, 0 for off-recursive, specified by user in .config */
    int ignoreNum;              /* the number of ignore dirs*/
    char *ignore[MAX_DIR_NUM];  /* store dirs to be ignored, specified by user in .config */
} MNT_config;

/**
 * file info.
 *
 * Note: absolute path is required
 *
 * */
typedef struct {
    char* filepath;                     /* absolute path required */
    int size;                           /* total size, in bytes  */
    struct timespec lastModifyTime;     /* time of last modification */
} FileInfo;


//=============================Helper Functions========================//
/**
 * This is a function to tell if a path is dir or regular file.
 * It returns 1 when the path is a regular file, 0 otherwise
 *
 * @param path: absolute path for the file
 *
 * */
// TODO - Merge with getFileInfo
int isFile(char *path);

/**
 * This is a function to copy a file.
 * It return 1 if success, otherwise -1.
 *
 * @param scr: absolute path for the file to be copied
 * @param dest: absolute path for the destination
 *
 * Honor code: https://stackoverflow.com/questions/29079011/copy-file-function-in-c
 *
 * */
int copyFile (char scr[], char dest[]);

/**
 * This is a function to get file extension from filename.
 * It returns the pointer to when extension starts.
 *
 * @param filename: filename, where path is not necessary
 *
 * */
const char *get_filename_ext(const char *filename);

/**
 * This is a function to get the machine's own IP.
 * It returns a pointer to malloc memory.
 *
 * Note: Need to be freed later.
 * */
// TODO
char* getMyIP();

/**
 * This is a function to concat a path with its preflix.
 * It returns a pointer to malloc mem.
 *
 * @param preflix:          path preflix. MUST END WITH '/'
 * @param path:             relative path to the configured "root" dir
 *
 * Note: Need to be freed later.
 *
 * */
char* concatPath(char* preflix, char* path);

/**
 * This is a function to generate absolute path from relative path to the "root" dir configured in .config .
 * It returns a pointer to malloc mem.
 *
 * @param config:          pointer to config table
 * @param path:            relative path to the configured "root" dir
 *
 * Note: Need to be freed later.
 *
 * */
char* getAbsPath(MNT_config *config, char* path);

//=============================Interfaces============================//
/**
 * This is a function to read configuration file from disk, e.g. root dir to be monitored, flag for recursive sync, ignore dir
 * The configuration file is defined as:
 * First line: root dir, in absolute path, end with '/'
 * Second line: Turn on recursive or not, 0 for turn off recusive mode
 * Third line and on (optional): each line for a dir to be ignored, relative path
 * Note that all dir should end with '/'
 *
 * Example:
 * /Users/yanxin/Desktop/test/
 * 1
 * sub1/
 *
 * @param config:       pointer to MNT_config struct, which will store configuration
 * @param filename:     configuration file name
 * */
void readConfigFile(MNT_config *config, char* filename);

/**
 * This is a function to initialized file monitor.
 * It returns the inotify file descriptor. If fails, return -1
 *
 * @param mnt_table:       pointer to monitor table
 * @param config:          pointer config table
 * @param table:           pointer file table
 * */
int initWatch(DIR_MNT_table *mnt_table, MNT_config *config, file_table_t *file_table);

/**
 * This is a function to add a directory into watch.
 * It will return 1 if success, -1 otherwise.
 *
 * @param inotify_fd:   inotify file descriptor
 * @param path:         relative path to dir be watched, not ABSOLUTE!
 * @param mnt_table:    pointer to monitor table
 * @param config:       pointer to config table
 *
 * */
int watchDirectory(int inotify_fd, char* path, DIR_MNT_table *mnt_table, MNT_config *config);

/**
 * This is a function to remove a directory from monitoring.
 *
 * @param inotify_fd:   inotify file descriptor
 * @param path:         relative path to dir be watched, not ABSOLUTE!
 * @param mnt_table:    pointer to monitor table
 * */
void removeDirectory(int inotify_fd, char* path, DIR_MNT_table *mnt_table);


/**
 * This is a function to find if a relative path is stored in configured ignore table or not.
 * It returns 1 if the path is in config.ignore, -1 otherwise.
 *
 * @param config:          pointer to config table
 * @param path:            relative path to be checked
 *
 * */
int inIgnore(MNT_config *config, char* path);


/**
 * This is a function to filter out undesired file or directory.
 * It returns 1 when the file/dir survives filtering, -1 otherwise.
 *
 * @param inotify_fd:       inotify file descriptor
 * @param mnt_table:        point monitor table
 * @param config:           config table
 * @param path:             the path of file/dir to be checked
 * @param file_table:       file table
 * */
int watchFilter(MNT_config *config, char* path);


/**
 * This is a function to traverse subdir recursively. It add all dirs not in ignore table to watch and save entries
 * including relative path to "root" dir and inotify wd into monitor table. Besides, it create an exact copies for
 * every directories and files to be watched.
 *
 * @param inotify_fd:       inotify file descriptor
 * @param mnt_table:        pointer to monitor table
 * @param path:             directory path, relative to the root dir; root dir is defined as empty string. It must be
 *                          either empty or ended with '/'
 *
 * */
void traverseDir(int inotify_fd, DIR_MNT_table *mnt_table, MNT_config *config, file_table_t *file_table, char *path);

/**
 * This is a function to get the event's dir by looking up wd in the Dir monitor table.
 * It will return the pointer of the matching dir in dir_mnt_table
 *
 * @param mnt_table:        point monitor table
 * @wd:                     the watch descriptor of monitored event
 *
 * */
char *getPathByWd(DIR_MNT_table *mnt_table, int wd);


/**
 * This is a function to get file info through absolute path.
 *
 * @param info:             FileInfo struct defined as above. When doing query, the filepath field must be specified in
 *                          advance. The filepath should be absolute path.
 *
 * */
void getFileInfo(FileInfo* info);


void sendTracker(file_table_t *table, int sockfd);



void getAllFilesInfo(); //Get all filenames/sizes/timestamps in the directory


void fileOpened(char* filename, file_table_t *table, MNT_config *config);
int fileAdded(char* filename, file_table_t *table, MNT_config *config);
int fileModified(char* filename, file_table_t *table, MNT_config *config);
int fileDeleted(char* filename, file_table_t *table, MNT_config *config);
int fileCloseWrite(char* filename, file_table_t *table, MNT_config *config);
void fileCloseNoWrite(char* filename, file_table_t *table, MNT_config *config);
void fileAccess(char* filename, file_table_t *table, MNT_config *config);
void fileInModified(char* filename, file_table_t *table, MNT_config *config);
void dirCreate(char* path, int inotify_fd, file_table_t *table, DIR_MNT_table *mnt_table, MNT_config *config);
void dirDelete(char* path, int inotify_fd, file_table_t *table, DIR_MNT_table *mnt_table);


/**
 * This is a function to free all memory
 * */
void clearAll();



//blockFileAddListenning();
//unblockFuleAddListenning()
//blockFileWriteListenning()
//unblockFileWriteListenning()
//blockFileDeleteListenning()
//unblockFileDeleteListenning()
//Called when a file is added to the directory.
// Called when a file is modified.
//Called when a file is deleted.
//Prevent unnecessary alert when downloading new files //Unblock it.
//Prevent unnecessary alert when modifying a file //Unblock it.
//Prevent unnecessary alert when deleting file //Unblock it.

 //MONITOR_H

#endif
