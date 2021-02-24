#ifndef _UTIL_H_
#define _UTIL_H_

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

struct task_stats;
/**
* Opens specified path
*
* @param proc_dir the location of procfs
* @param path the path of the file
*
* @return file directory
*/
int open_path(char *proc_dir, char *path);

/**
* Reads one line of file
*
* @param fd the file directory
* @param buf stores the line
* @param sz the size
* @param delim the deliminator
*
* @return amount of bytes
*/
ssize_t one_lineread(int fd, char *buf, size_t sz, char *delim);

/**
* Reads a file line by line
*
* @param fd the file directory
* @param buf stores the line
* @param sz the size
*
* @return amount of bytes
*/
ssize_t lineread(int fd, char *buf, size_t sz);

/**
* Tokenize string
*
* @param str_ptr point of tokenization
* @param delim deliminator
*
* @return token
*/
char *next_token(char **str_ptr, const char *delim);

/**
* Draws percentage bar
*
* @param buf stores information
* @param frac amount of usage
*
*/
void draw_percbar(char *buf, double frac);

/**
* Maps ID numbers (UID) associated with running processes to corresponding username
*
* @param name_buf stores names
* @param uid the user identification
*
*/
void uid_to_uname(char *name_buf, uid_t uid);

/**
* Fill in tasks with task information
*
* @param tstats the task statistics
* @param pid the process ID
* @param uid the user ID
* @param task_name the task name
* @param task_state the task state
* @param counter the counter that keeps track of tasks
*
*/
void fill_tasks(struct task_stats *tstats, int pid, int uid, char *task_name, char *task_state, int *counter);

#endif
