/**
 * @file
 *
 * Retrieves raw information from procfs, parses it, and provides easy-to-use
 * functions for displaying the data.
 */

#ifndef _PROCFS_H_
#define _PROCFS_H_

#include <sys/types.h>

/**
* Load average every one, five, and fifteen minutes
*/
struct load_avg {
    double one;
    double five;
    double fifteen;
};

/**
* CPU information to check cpu usage
*/
struct cpu_stats {
    long idle;
    long total;
};

/**
* Memory information to check memory usage
*/
struct mem_stats {
    double used;
    double total;
};

/**
* Task statistics
*/
struct task_stats {
    unsigned int total;
    unsigned int running;
    unsigned int waiting;
    unsigned int sleeping;
    unsigned int stopped;
    unsigned int zombie;

    struct task_info *active_tasks;
};

/**
* Task information
*/
struct task_info {
    pid_t pid;
    uid_t uid;
    char name[26];
    char state[13];
};

/**
* Retrieves hostname of the machine
*
* @param proc_dir the location of procfs
* @param hostname_buf stores the hostname
* @param buf_sz buf_sz size
*
* @return 0 if success, -1 otherwise
*/
int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz);

/**
* Retrieves kernel version of the machine
*
* @param proc_dir the location of procfs
* @param version_buf stores the kernel version
* @param buf_sz buf_sz size
*
* @return 0 if success, -1 otherwise
*/
int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz);

/**
* Retrieves cpu model name of the machine
*
* @param proc_dir the location of procfs
* @param model_buf stores the cpu model name
* @param buf_sz buf_sz size
*
* @return 0 if success, -1 otherwise
*/
int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz);

/**
* Sums the number of processing units of the machine
*
* @param proc_dir the location of procfs
*
* @return counter the sum of processing units
*/
int pfs_cpu_units(char *proc_dir);

/**
* Retrieves uptime(in seconds) of the machine
*
* @param proc_dir the location of procfs
*
* @return time the amount of uptime
*/
double pfs_uptime(char *proc_dir);

/**
* Formats uptime(in seconds) to days, hours, minutes, and seconds
*
* @param time the uptime(in seconds)
* @param uptime_buf stores the formatted uptime
*
* @return 0 if success
*/
int pfs_format_uptime(double time, char *uptime_buf);

/**
* Retrieves load average of the machine
*
* @param proc_dir the location of procfs
*
* @return lavg the load average
*/
struct load_avg pfs_load_avg(char *proc_dir);

/**
* Retrieves cpu usage of the machine
*
* @param procfs_dir the location of procfs
* @param cpu_stats *prev input that represents the last CPU usage state
* @param cpu_stats *curr output that represents the current CPU usage state,
* and will be passed back in to pfs_load_avg on its next call
*
* @return cpu_usage the cpu usage
*/
double pfs_cpu_usage(
        char *procfs_dir, struct cpu_stats *prev, struct cpu_stats *curr);

/**
* Retrieves memory usage of the machine
*
* @param procfs_dir the location of procfs
*
* @return mstats the memory usage
*/
struct mem_stats pfs_mem_usage(char *procfs_dir);

/**
* Creates memory for task_stats structs
*
* @return tstats the task_stats structs
*/
struct task_stats *pfs_create_tstats();

/**
* Frees memory for task_stats structs
*
* @param tstats the task_stats structs
*
*/
void pfs_destroy_tstats(struct task_stats *tstats);

/**
* Retrieves tasks info of the machine
*
* @param proc_dir the location of procfs
* @param tstats the task_stats structs
*
* @return 0 if success, or EXIT_FAILURE if otherwise
*/
int pfs_tasks(char *proc_dir, struct task_stats *tstats);

#endif
