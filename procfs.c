#include <linux/limits.h>
#include "procfs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <dirent.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "logger.h"
#include "procfs.h"
#include "util.h"

int pfs_hostname(char *proc_dir, char *hostname_buf, size_t buf_sz)
{
    LOGP("Getting the hostname\n");
    //opens path
    int fd = open_path(proc_dir, "sys/kernel/hostname");
    //error checking
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    //
    ssize_t read_sz = lineread(fd, hostname_buf, buf_sz);
    if (read_sz == -1) {
        return -1;
    }
    close(fd);

    hostname_buf[read_sz - 1] = '\0';
    return 0;
}

int pfs_kernel_version(char *proc_dir, char *version_buf, size_t buf_sz)
{
    LOGP("Getting the kernel version\n");

    int fd = open_path(proc_dir, "sys/kernel/osrelease");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }    
    
    ssize_t read_sz = lineread(fd, version_buf, buf_sz);
    if (read_sz == -1) {
        return -1;
    }
    close(fd);
    //iterate until "-"
    size_t dash_loc = strcspn(version_buf, "-");
    version_buf[dash_loc] = '\0';
    return 0;
}

int pfs_cpu_model(char *proc_dir, char *model_buf, size_t buf_sz)
{
    LOGP("Getting the cpu model\n");

    int fd = open_path(proc_dir, "cpuinfo");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    //model buf is 128 chars in display.c, so we allocate space for that plus allowance
    char line[256] = { 0 };

    ssize_t read_sz = 0;    
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "model name", 10) == 0) {
            size_t model_loc = strcspn(line, ":") + 2;
            size_t newline_loc = strcspn(&line[model_loc], "\n");
            strncpy(model_buf, &line[model_loc], newline_loc);
        }
    }   
    close(fd);

    return 0;
}

int pfs_cpu_units(char *proc_dir)
{
    LOGP("Getting the cpu units\n");

    int fd = open_path(proc_dir, "cpuinfo");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }    
    char line[256] = { 0 };
    int counter = 0;
    ssize_t read_sz = 0;    
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        if (strncmp(line, "processor", 9) == 0) {
            counter++;
        }
    }   
    close(fd);

    return counter;
}

double pfs_uptime(char *proc_dir)
{
    LOGP("Getting the uptime\n");

    int fd = open_path(proc_dir, "uptime");
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    char line[100] = { 0 };
    char *end_ptr;
    double time;
    one_lineread(fd, line, 100, "\n");
    // convert string to double
    time = strtod(line, &end_ptr);

    close(fd);
    return time;
}

int pfs_format_uptime(double time, char *uptime_buf)
{
    LOGP("Getting the uptime format\n");
    
    int years = (time / 31556952);
    int years_remainder = (int)time % 31556952;

    int days = (years_remainder/(24 * 3600));
    int days_remainder = (int)years_remainder % (24 * 3600);

    int hours = (days_remainder / 3600);
    int hours_remainder = (int)days_remainder % 3600;

    int minutes = (hours_remainder / 60);
    int minutes_remainder = (int)hours_remainder % 60;

    int seconds = minutes_remainder; 
    
    if (years == 0) {
        sprintf(uptime_buf, "%d days, %d hours, %d minutes, %d seconds", days, hours, minutes, seconds);
    }
    else if (days == 0) {
        sprintf(uptime_buf, "%d hours, %d minutes, %d seconds", hours, minutes, seconds);
    }
    else if (hours == 0) {
        sprintf(uptime_buf, "%d minutes, %d seconds", minutes, seconds);
    }
    else if (minutes == 0) {
        sprintf(uptime_buf, "%d minutes, %d seconds", 0, seconds);
    }
    else {
        sprintf(uptime_buf, "%d years, %d days, %d hours, %d minutes, %d seconds", years, days, hours, minutes, seconds);
    }

    return 0;
}

struct load_avg pfs_load_avg(char *proc_dir)
{  
    LOGP("Getting the load average\n");

    struct load_avg lavg = { 0 };

    int fd = open_path(proc_dir, "loadavg");
    if (fd == -1) {
        perror("open_path");
        return lavg;
    }
    char line[100] = { 0 };
    one_lineread(fd, line, 100, "\n");

    char *next_tok = line;
    char *curr_tok;
    char *end_ptr;
    curr_tok = next_token(&next_tok, " ");
    lavg.one = strtod(curr_tok, &end_ptr);
    curr_tok = next_token(&next_tok, " ");
    lavg.five = strtod(curr_tok, &end_ptr);
    curr_tok = next_token(&next_tok, " ");
    lavg.fifteen = strtod(curr_tok, &end_ptr);

    close(fd);
    return lavg;
}

double pfs_cpu_usage(char *proc_dir, struct cpu_stats *prev, struct cpu_stats *curr)
{
    LOGP("Getting the cpu usage\n");

    double idle1, idle2, total1, total2 = 0.0;
    idle1 = prev->idle;
    total1 = prev->total;
    //opens file
    int fd = open_path(proc_dir, "stat");
    //error checking
    if (fd == -1) {
        perror("open_path");
        return -1;
    }
    char line[256] = { 0 };
    size_t read_sz = 0;
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        //check for "cpu " in line
        if (strstr(line, "cpu ")) {
            char *next_tok = line;
            char *curr_tok;
            char *end_ptr;
            int counter = 0;
            //Iterate and tokenize line
            while ((curr_tok = next_token(&next_tok, " ")) != NULL) {
                    //checks if not in "cpu"
                    if (counter != 0) {
                        //add value to total2
                        total2 += strtod(curr_tok, &end_ptr);
                        //checks when in idle
                        if (counter == 4) {
                           //stores idle value to idle2 
                           idle2 = strtod(curr_tok, &end_ptr);
                        }
                    }
                    counter++;
            }
            curr->idle = idle2;
            curr->total = total2; 
        }
    }
    if ((idle2 - idle1 < 0) || (total2 - total1 < 0)) {
        return 0.0;
    }
    double cpu_usage = 1 - ((idle2-idle1) / (total2-total1));
    if (isnan(cpu_usage)) {
        return 0.0;
    }

    close(fd);
    return cpu_usage;
}

struct mem_stats pfs_mem_usage(char *proc_dir)
{
    LOGP("Getting the mem usage");

    struct mem_stats mstats = { 0 };
    double mem_total, mem_available = 0;

    int fd = open_path(proc_dir, "meminfo");
    if (fd == -1) {
        perror("open_path");
        return mstats;
    }
    char line[256] = { 0 };
    ssize_t read_sz = 0;    
    while ((read_sz = lineread(fd, line, 256)) > 0) {
        char *next_tok = line;
        char *curr_tok;
        char *end_ptr;
        if (strstr(line, "MemTotal:")) {
            LOG("LINE = %s\n", line);
            curr_tok = next_token(&next_tok, " ");
            curr_tok = next_token(&next_tok, " ");
            // size_t mem_total_loc = strcspn(line, )
            mem_total = strtod(curr_tok, &end_ptr);
            //convert to GiB
            mem_total = mem_total / 1024 / 1024;
            mstats.total = mem_total;
            LOG("MEM TOTAL = %f\n", mem_total);
        }
        if (strstr(line, "MemAvailable:"))  {
            LOG("LINE = %s\n", line);
            curr_tok = next_token(&next_tok, " ");
            curr_tok = next_token(&next_tok, " ");
            mem_available = strtod(curr_tok, &end_ptr);
            //convert to GiB
            mem_available = mem_available / 1024 / 1024;
            LOG("MEM AVAIL = %f\n", mem_available);
            // if (mstats.total - mem_available < 0) {
            //     return mstats;
            // }
        }
    }   
    mstats.used = mstats.total - mem_available;

    close(fd);
    return mstats;
}

struct task_stats *pfs_create_tstats()
{
    struct task_stats *ts = calloc(1, sizeof(struct task_stats));
    if (ts != NULL) {
        ts->total = 0;
        ts->running = 0;
        ts->waiting = 0;
        ts->sleeping = 0;
        ts->stopped = 0;
        ts->zombie = 0;

        ts->active_tasks = calloc(1, sizeof(struct task_info));
    }

    return ts;
}

void pfs_destroy_tstats(struct task_stats *tstats)
{
    //free active tasks
    free(tstats->active_tasks);
    //free tstats
    free(tstats);
}

int pfs_tasks(char *proc_dir, struct task_stats *tstats)
{
    return 0;
}